#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

#include "chrdef.h"
#include "segdata.h"
#include "snpdata.h"
#include "intervaltree.cpp"


/*----------------------------------------------------------------------------*/

class ISegAnnot {
public:
  virtual ~ISegAnnot() {};
  virtual std::vector<char> intersect(const ISegData &seg, const SnpData &snp) = 0;
private:
};

/*----------------------------------------------------------------------------*/

class SegAnnotSimple : public ISegAnnot {
public:

  std::vector<char> intersect(const ISegData &seg, const SnpData &snp);
private:
};

std::vector<char> SegAnnotSimple::intersect(const ISegData &seg, const SnpData &snp) {
  std::vector<char> annot(snp.data().size(), 0);
  ChrMap::ChrType chr_curr = ChrMap::CHRTYPE_NUMENTRIES;
  std::vector<Segment> seg_curr;
  for (uint32_t i = 0; i < snp.data().size(); i++) {
    if (chr_curr != snp.data()[i].chr) {
      chr_curr = snp.data()[i].chr;
      seg_curr = seg.getChr(chr_curr);
      printf("%s (%lu)", ChrMap::chrTypeStr(chr_curr), seg_curr.size());
    }
    const uint64_t bp = snp.data()[i].bp;
    for (uint32_t j = 0; j < seg_curr.size(); j++) {
      bool inside = true;
      inside = inside && bp >= seg_curr[j].s;
      inside = inside && bp < seg_curr[j].e;
      if (inside == true) {
        annot[i] = true;
        break;
      }
    }
    if (i % 10000 == 0) {
      printf(".");
      fflush(stdout);
    }
  }
  printf("\n");
  return annot;
}

/*----------------------------------------------------------------------------*/


class SegAnnot : public ISegAnnot {
public:

  std::vector<char> intersect(const ISegData &seg, const SnpData &snp);
private:
};

bool istrue(const char &a) {
  return a != 0;
}

std::vector<char> SegAnnot::intersect(const ISegData &seg, const SnpData &snp) {
  std::vector<char> annot(snp.data().size(), 0);
  ChrMap::ChrType chr_curr = ChrMap::CHRTYPE_NUMENTRIES;
  IntervalTree<Segment> *seg_curr = NULL;
  for (uint32_t i = 0; i < snp.data().size(); i++) {
    if (chr_curr != snp.data()[i].chr) {
      chr_curr = snp.data()[i].chr;
      delete seg_curr;
      seg_curr = new IntervalTree<Segment>(seg.getChr(chr_curr));
      //printf("%s (%u)", ChrMap::chrTypeStr(chr_curr), seg_curr->numNodes());
    }
    const uint64_t bp = snp.data()[i].bp;
    annot[i] = seg_curr->overlaps(bp);
    if (i % 10000 == 0) {
      //printf(".");
      fflush(stdout);
    }
  }
  delete seg_curr;
  //printf("\n");
  return annot;
}

/*----------------------------------------------------------------------------*/

#if 0
void intervalltree_test(const IntervalTree<Interval> &it,
    const std::vector<Interval> &is, uint64_t p) {
  bool in_it = it.overlaps(p);
  bool in_is = false;
  for (uint32_t i = 0; i < is.size(); i++) {
    if (p >= is[i].s && p < is[i].e) {
      in_is = true;
      break;
    }
  }
  //printf("%lu %s\n", p, in_it ? "found" : "");
  assert(in_it == in_is);
}

void intervalltree_test() {
  std::vector<Interval> is;
  SimpleSegData sd;
  sd.read("data/wgEncodeUwDnaseCd20ro01778PkRep1.narrowPeak");
  std::vector<Segment> seg = sd.getChr(ChrMap::CHRTYPE_1);
  for (uint32_t i = 0; i < seg.size(); i++) {
    is.push_back(Interval(seg[i].s, seg[i].e));
  }
  IntervalTree<Interval> it(is);
  std::vector<uint64_t> ps;
  SnpData snps;
  snps.read("data/scz.txt", 1);
  //it.print();
  for (uint32_t i = 0; i < is.back().e; i+=10) {
    intervalltree_test(it, is, i);
  }
  printf("ok\n");
}
#endif

void usage(const char *s) {
  printf("Usage: %s [OPTIONS] SNPS REGIONS1 [REGIONS2..]\n\n", s);

}

int main(int argc, char **argv) {
  int o = 0;
  std::string cfg_out;
  while ((o = getopt(argc, argv, "ho:")) != -1) {
    switch (o) {
      case 'h':
        usage(argv[0]);
        exit(0);
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
  snps.read(fnsnp, 1);
  for (uint32_t i = optind + 1; i < (uint32_t)argc; i++) {
    SimpleSegData sd;
    sd.read(argv[i]);
    SegAnnot sa;
    std::vector<char> annot = sa.intersect(sd, snps);
    printf("annot - %s: %lu\n", argv[i], std::count_if(annot.begin(), annot.end(), istrue));
  }
  printf("ok\n");
}
