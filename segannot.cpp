#include "segannot.h"
#include "intervaltree.cpp"

/*----------------------------------------------------------------------------*/

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
