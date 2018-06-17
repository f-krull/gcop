#include "buffer.h"
#include "infofile.h"
#include "hapdosereader.h"
#include "famwriter.h"
#include "bedwriter.h"
#include "gettoken.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/

#define LINE_LENGTH_INIT (16)
#define DELIMITER '\t'

/*----------------------------------------------------------------------------*/

static void usage(const char *pname) {
  printf("Usage: %s [OPTIONS] IN_PREFIX OUT_PREFIX\n\n", pname);
  printf("Options:\n");
  printf("  -f <freq>   Minor allele frequency threshold for inclusion [default: %.2f]\n", InfoFileFilter().getMafMin());
  printf("  -c <avcall> Average call threshold for inclusion [default: %.2f]\n", InfoFileFilter().getAvgCallMin());
  printf("  -d <delta>  Max difference between hard-call (0,1,2) genotype and pseudo count [default: %.2f]\n", BedWriter().getDelta());
  printf("  -r <rsq>    Imputation r-squared threshold for inclusion [default: %.2f]\n", InfoFileFilter().getRsqMin());
}

/*----------------------------------------------------------------------------*/

static bool writeBim(const char *fn, const InfoFile &info) {
  const char dummy_pos_morgan[] = "0";
  FILE *f = fopen(fn, "w");
  if (f == NULL) {
    fprintf(stderr, "error - cannot open file '%s'\n", fn);
    return false;
  }
  bool ret = true;
  for (uint32_t i = 0; i < info.variants().size(); i++) {
    /* skip variant with bad quality */
    if (!info.variantStatus()[i]) {
      continue;
    }
    const InfoFileEntry & e = info.variants()[i];
    BufferDyn str_snp(e.get(InfoFileEntry::FIELD_SNP));
    char *str_chr = str_snp.str();
    char *str_bp = gettoken(str_chr, ':');
    ret = ret && fprintf(f, "%s",  str_chr)                                      >= 1;
    ret = ret && fprintf(f, "%c%s", DELIMITER, e.get(InfoFileEntry::FIELD_SNP))  >= 1;
    ret = ret && fprintf(f, "%c%s", DELIMITER, dummy_pos_morgan)                 >= 1;
    ret = ret && fprintf(f, "%c%s", DELIMITER, str_bp)                           >= 1;
    ret = ret && fprintf(f, "%c%s", DELIMITER, e.get(InfoFileEntry::FIELD_REF0)) >= 1;
    ret = ret && fprintf(f, "%c%s", DELIMITER, e.get(InfoFileEntry::FIELD_ALT1)) >= 1;
    ret = ret && fprintf(f, "\n")                                                >= 1;
  }
  fclose(f);
  if (!ret) {
    fprintf(stderr, "error - cannot write file '%s'\n", fn);
  }
  return ret;
}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
  BedWriter bedw;
  InfoFileFilter variantFilter;
  while (true) {
    int32_t c = getopt (argc, argv, "c:d:f:r:h");
    if (c < 0) {
      break;
    }
    switch (c) {
      case 'c':
        variantFilter.setAvgCallMin(atof(optarg));
        break;
      case 'd':
        bedw.setDelta(atof(optarg));
        break;
      case 'f':
        variantFilter.setMafMin(atof(optarg));
        break;
      case 'r':
        variantFilter.setRsqMin(atof(optarg));
        break;
      case 'h':
        usage(argv[0]);
        return EXIT_SUCCESS;
        break;
      default:
        usage(argv[0]);
        return EXIT_FAILURE;
        break;
    }
  }

  if (argc < optind+2) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  const char* finpfx  = argv[optind+0];
  const char* foutpfx = argv[optind+1];

  const BufferDyn fin_hapd("%s.hapDose.gz", finpfx);
  const BufferDyn fin_info("%s.info.gz",    finpfx);
  const BufferDyn fout_bed("%s.bed", foutpfx);
  const BufferDyn fout_bim("%s.bim", foutpfx);
  const BufferDyn fout_fam("%s.fam", foutpfx);

  printf("reading info file '%s'\n", fin_info.cstr());
  InfoFile info;
  if (!info.read(fin_info.cstr(), variantFilter)) {
    return EXIT_FAILURE;
  }
  printf("  num variants:          %zu\n", info.numVariants());
  printf("  num variants after QC: %zu (%.2f%%)\n",
    info.numVariantsOk(), float(info.numVariantsOk())/info.numVariants()*100);
  printf("reading hapdose file '%s'\n", fin_hapd.cstr());
  HapDoseReader hapd;
  if (!hapd.open(fin_hapd.cstr(), info.numVariants())) {
    return EXIT_FAILURE;
  }
  printf("  num samples: %zu\n", hapd.sampleInfo().size());
  /* write bim file */
  writeBim(fout_bim.cstr(), info);
  /* write fam file */
  FamWriter famw;
  if (!famw.open(fout_fam.cstr())) {
    return EXIT_FAILURE;
  }
  for (uint32_t i = 0; i < hapd.sampleInfo().size(); i++) {
    FamWriter::Entry e(hapd.sampleInfo()[i].id.c_str());
    e.fid = hapd.sampleInfo()[i].familyid;
    famw.write(e);
  }
  famw.close();
  /* open bed file */
  if (!bedw.open(fout_bed.cstr())) {
    return EXIT_FAILURE;
  }
  for (uint32_t i = 0; i < info.numVariants(); i++) {
    printf("%s variant %u (%.2f%%)\n",
      info.variantStatus()[i] ? "writing" : "skipping",
      i+1 , float(i+1)/info.numVariants()*100);
    const std::vector<std::array<float, HAPLINDEX_NUMENRIES>> & sh = hapd.nextVar();
    assert(sh.size() == hapd.sampleInfo().size());
    /* skip bad variant? */
    if (!info.variantStatus()[i]) {
      continue;
    }
    for (uint32_t j = 0; j < sh.size(); j++) {
      bedw.write(HAPLINDEX_1, sh[j][HAPLINDEX_1]);
      bedw.write(HAPLINDEX_2, sh[j][HAPLINDEX_2]);
      //printf("var %u sample %u - %f %f\n", i, j, sh[j][HAPLINDEX_1], sh[j][HAPLINDEX_2]);
    }
    bedw.closeVariant();
  }
  /* cleanup */
  hapd.close();
  bedw.close();
  return EXIT_SUCCESS;
}
