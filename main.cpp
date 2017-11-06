#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "chrdef.h"
#include "segdata.h"
#include "snpdata.h"



/*----------------------------------------------------------------------------*/

class ISegAnnot {
public:
  virtual ~ISegAnnot() {};
  virtual void instersect(const ISegData &seg, const SnpData &snp) = 0;
private:
};

/*----------------------------------------------------------------------------*/

class SegAnnotSimple : public ISegAnnot {
public:

  void instersect(const ISegData &seg, const SnpData &snp);
private:
};

void SegAnnotSimple::instersect(const ISegData &seg, const SnpData &snp) {
  std::vector<char> annot(0, snp.data().size());
  ChrMap::ChrType chr_curr = ChrMap::CHRTYPE_NUMENTRIES;
  std::vector<Segment> seg_curr;
  for (uint32_t i = 0; i < snp.data().size(); i++) {
    if (chr_curr != snp.data()[i].chr) {
      chr_curr = snp.data()[i].chr;
      seg_curr = seg.getChr(chr_curr);
      printf("chr %d ", chr_curr);
    }
    const uint64_t bp = snp.data().size();
    for (uint32_t j = 0; j < seg_curr.size(); j++) {
      bool inside = true;
      inside = inside && bp > seg_curr[j].bps;
      inside = inside && bp < seg_curr[j].bpe;
      if (inside == true) {
        annot[i] = false;
        break;
      }
    }
    if (i % 10000 == 0) {
      printf(".");
      fflush(stdout);
    }
  }
  printf("\n");
}

/*----------------------------------------------------------------------------*/


class SegAnnot : public ISegAnnot {
public:

  void instersect(const ISegData &seg, const SnpData &snp);
private:
};

void SegAnnot::instersect(const ISegData &seg, const SnpData &snp) {
  std::vector<char> annot(0, snp.data().size());
  ChrMap::ChrType chr_curr = ChrMap::CHRTYPE_NUMENTRIES;
  std::vector<Segment> seg_curr;
}


/*----------------------------------------------------------------------------*/


int main(int argc, char **argv) {
  SimpleSegData sd;
  sd.read("data/wgEncodeUwDnaseCd20ro01778PkRep1.narrowPeak");
  SnpData snps;
  snps.read("data/scz.txt", 1);
  SegAnnotSimple sa;
  sa.instersect(sd, snps);
  printf("ok\n");
}
