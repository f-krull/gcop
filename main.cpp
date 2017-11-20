#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

#include "chrdef.h"
#include "segdata.h"
#include "snpdata.h"
#include "segannot.h"

bool istrue(const char &a) {
  return a != 0;
}


/*----------------------------------------------------------------------------*/

void write(const char* fnoutdir, const char *fnin, const std::vector<char> &annot) {
  char fnout[1024];
  snprintf(fnout, sizeof(fnout)-1, "%s/%s.annot", fnoutdir, fnin);
  FILE *f = fopen(fnout, "w");
  if (f == NULL) {
    fprintf(stderr, "error: opening output file %s\n", fnout);
    exit(1);
  }
  for (uint32_t i = 0; i < annot.size(); i++) {
    fprintf(f, "%s\n", annot[i] ? "good_cat" : "bad_cat");
  }
  fclose(f);
}

/*----------------------------------------------------------------------------*/
#include <zlib.h>
void write_gz(const char* fnoutdir, const char *fnin, const std::vector<char> &annot) {
  char fnout[1024];
  snprintf(fnout, sizeof(fnout)-1, "%s/%s.annot.gz", fnoutdir, fnin);
  gzFile f = gzopen(fnout, "wb1");
  if (f == NULL) {
    fprintf(stderr, "error: opening output file %s\n", fnout);
    exit(1);
  }
  for (uint32_t i = 0; i < annot.size(); i++) {
    gzprintf(f, "%s\n", annot[i] ? "good_cat" : "bad_cat");
  }
  gzclose(f);
}

/*----------------------------------------------------------------------------*/

void usage(const char *s) {
  printf("Usage: %s [OPTIONS] SNPS REGIONS1 [REGIONS2..]\n\n", s);
}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int o = 0;
  std::string cfg_out;
  std::string cfg_snp_fields = "123";
  while ((o = getopt(argc, argv, "ho:p:")) != -1) {
    switch (o) {
      case 'h':
        usage(argv[0]);
        exit(0);
        break;
      case 'p':
        cfg_snp_fields = optarg;
        break;
      case 'o':
        cfg_out = optarg;
        break;
      default:
        fprintf(stderr, "error: unknown option \"-%c\"\n\n", (char)o);
        usage(argv[0]);
        exit(1);
        break;
    }
  }
  if (argc - optind < 2) {
    fprintf(stderr, "error: number of args\n");
    usage(argv[0]);
    exit(1);
  }
  const char *fnsnp = argv[optind];
  SnpData snps;
  snps.readdyn(fnsnp, cfg_snp_fields.c_str(), 1);
  for (uint32_t i = optind + 1; i < (uint32_t)argc; i++) {
    SimpleSegData sd;
    sd.read(argv[i]);
    SegAnnot sa;
    std::vector<char> annot = sa.intersect(sd, snps);
    printf("annot - %s: %lu\n", argv[i], std::count_if(annot.begin(), annot.end(), istrue));
    if (!cfg_out.empty()) {
      write_gz(cfg_out.c_str(), argv[i], annot);
    }
  }
  printf("ok\n");
}
