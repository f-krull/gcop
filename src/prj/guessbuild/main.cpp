#include <zlib.h>
#include <assert.h>
#include <stdio.h>

/*----------------------------------------------------------------------------*/

static void usage(const char *pname) {
  printf("Usage: %s [-t <thres>] IN_BIM IN_REF\n\n", pname);
  printf("Computes the coverage of SNPs in input by reference using chomosome\n");
  printf("ID and basepair position. Result is given as percentage [0,100]\n");
  printf("Expected IN_BIM format: CHR RS CM A1 A2 (no header) \n");
  printf("Expected IN_REF format: CHR POS A1 A2 COUNT (no header) \n\n");
  printf("Options:\n");
  printf("  -t <thres>   Exit with error, if overlap is lower than <thres>.\n");
}

/*----------------------------------------------------------------------------*/
#include <gcords.h>
int main(int argc, char **argv) {
  uint32_t thres = 0;
  while (true) {
    int32_t c = getopt (argc, argv, "q:h");
    if (c < 0) {
      break;
    }
    switch (c) {
      case 'q':
        thres = (atof(optarg));
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
  const char* finp = argv[optind+0];
  const char* fref = argv[optind+1];

  ChrInfoUnknown hgXX;
  GCords g_ref;
  GCords g_inp;
  /* read ref and input SNPs into memory */
  const char ref_fmt[] = "cs..."; /* chr pos a1 a2 count */
  g_ref.read(fref, ref_fmt, 1, &hgXX, true);
  const char in_fmt[] = "c..s.."; /* chr rs cM pos a1 a2 */
  g_inp.read(finp, in_fmt, 1, &hgXX, true);
  /* for all chromosomes */
  const ChrInfo & cinf = g_ref.chrinfo();
  uint64_t num_found = 0;
  uint64_t ref_size = 0;
  for (auto cit = cinf.chrs().begin(); cit != cinf.chrs().end(); ++cit) {
    std::vector<GCord> res;
    const std::vector<GCord> &c_inp = g_inp.getChr(*cit);
    const std::vector<GCord> &c_ref = g_ref.getChr(*cit);
    /* intersect inp with ref */
    std::set_intersection(c_inp.begin(), c_inp.end(),
                          c_ref.begin(), c_ref.end(),
                          std::back_inserter(res));
    num_found += res.size();
    ref_size += c_inp.size();
  }
  printf("num_found: %lu\n", num_found);
  const float coverage = float(num_found)*100/ref_size;
  printf("coverage: %.f\n", coverage);
  return coverage >= thres ? EXIT_SUCCESS : EXIT_FAILURE;
}
