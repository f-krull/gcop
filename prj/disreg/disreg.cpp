
#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

std::vector<std::string> readList(const char * filename, char comment = '#') {
  std::vector<std::string> ret;
  std::ifstream file;
  std::string line;

  file.open(filename);
  if (!file) {
    fprintf(stderr, "error - cannot open file %s\n", filename);
    exit(1);
  }
  while (getline((file), line)) {
     if ((line.length() > 0) && (line.at(0) != comment)) {
        ret.push_back(line);
     }
  }
  file.close();
  return ret;
}

/*----------------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

#include "../../data/gcords.h"
#include "../../data/chrinfo.h"
#include "../../data/intervaltree.h"

/*----------------------------------------------------------------------------*/

class LengthsSum {
public:
  LengthsSum(const std::vector<GCord> &g, ChrInfo::CType c) {
    m_sum = 0;
    for (uint32_t i = 0; i < g.size(); i++) {
      m_sum += g[i].len();
    }
  }
  uint64_t get() const {
    return m_sum;
  }
private:
  uint64_t m_sum;
};

/*----------------------------------------------------------------------------*/

class GCordsChrInfo {
public:
  GCordsChrInfo(const GCords * gc, ChrInfo::CType c) : m_gc(gc), m_chr(c) {
    m_it  = NULL;
    m_len = NULL;
    m_g = m_gc->getChr(m_chr);
  }

  ~GCordsChrInfo() {
    delete m_len;
    delete m_it;
  }

  const std::vector<GCord> & vgcords() const {
    return m_g;
  }

  const LengthsSum * lengthssum() {
    if (m_len == NULL) {
      m_len = new LengthsSum(m_g, m_chr);
    }
    return m_len;
  }

  const IntervalTree<GCord> * it() {
    if (m_it == NULL) {
      m_it = new IntervalTree<GCord>(m_g);
    }
    return m_it;
  }

private:
  IntervalTree<GCord> *m_it;
  LengthsSum *m_len;
  std::vector<GCord> m_g;
  const GCords *m_gc;
  const ChrInfo::CType m_chr;
};

/*----------------------------------------------------------------------------*/

class GCordsInfoCache {
public:
  GCordsInfoCache(const GCords *gc) : m_gc(gc) {
  }

  ~GCordsInfoCache() {
    std::map<ChrInfo::CType, GCordsChrInfo*>::iterator it;
    for (it = m_ginf.begin(); it != m_ginf.end(); ++it) {
      delete it->second;
    }
  }

  GCordsInfoCache( const GCordsInfoCache & o);
  const GCordsInfoCache & operator=(const GCordsInfoCache & o);

  GCordsChrInfo * chr(ChrInfo::CType c) {
    std::map<ChrInfo::CType, GCordsChrInfo*>::iterator it = m_ginf.find(c);
    /* exists? */
    if (it != m_ginf.end()) {
      return it->second;
    }
    /* create */
    GCordsChrInfo *gi = new GCordsChrInfo(m_gc, c);
    m_ginf.insert(std::make_pair(c, gi));
    return gi;
  }

  const GCords * gcords() const  {
    return m_gc;
  }

private:
  const GCords *m_gc;
  std::map<ChrInfo::CType, GCordsChrInfo*> m_ginf;
};

/*----------------------------------------------------------------------------*/


/* forbes: A and B */
static uint64_t calcAvsB(const std::vector<GCord> &ac,
                         const std::vector<GCord> &bc,
                         const IntervalTree<GCord> &bit) {
  uint64_t res = 0;
  std::vector<GCord> bovac;
  /* for all regions of a */
  for (uint64_t j = 0; j < ac.size(); j++) {
    /* get all overlapping intervals */
    std::vector<uint32_t> bova_idx;
    bit.overlapsInterval(ac[j], &bova_idx);
    /* to collection of gcords */
    bovac.clear();
    for (uint32_t k = 0; k < bova_idx.size(); k++) {
      bovac.push_back(bc[bova_idx[k]]);
    }
    /* compute length of overlap */
    for (uint32_t i = 0; i < bovac.size(); i++) {
      res += Interval::numOverlap(ac[j], bovac[i]);
    }
  }
  return res;
}

/*----------------------------------------------------------------------------*/

static float forbes(GCordsInfoCache & g1inf, GCordsInfoCache & g2inf) {
 /*
  * forbes = N * |A and B| / ( |A| * |B| )
  */
  assert(g1inf.gcords()->chrinfo() == g2inf.gcords()->chrinfo());
  const ChrInfo & chrinfo = g1inf.gcords()->chrinfo();
  uint64_t n_ab = 0;
  uint64_t n_a = 0;
  uint64_t n_b = 0;
  uint64_t n_c = 0;
  /* by chromosome */
  for (uint32_t i = 0; i < g1inf.gcords()->chrinfo().chrs().size(); i++) {
    ChrInfo::CType chr_curr = g1inf.gcords()->chrinfo().chrs()[i];
    printf("%s\n", g1inf.gcords()->chrinfo().ctype2str(chr_curr));
    const std::vector<GCord> &ac = g1inf.chr(chr_curr)->vgcords();
    const std::vector<GCord> &bc = g2inf.chr(chr_curr)->vgcords();
    n_c += chrinfo.chrlen(chr_curr);
    if (ac.empty() && bc.empty()) {
      continue;
    }
    const IntervalTree<GCord> &bit = *g2inf.chr(chr_curr)->it();
    n_ab += calcAvsB(ac, bc, bit);
    n_a += g1inf.chr(chr_curr)->lengthssum()->get();
    n_b += g2inf.chr(chr_curr)->lengthssum()->get();
  }
  printf("ab: %lu\n", n_ab);
  printf("a: %lu\n", n_a);
  printf("b: %lu\n", n_b);

  float num = n_c * n_ab;
  float den = n_a * n_b;
  float ret = den > 0 ? num/den : 0;
  printf("forbes: %f\n", ret);
  return ret;
}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
  assert(argc == 3);
  const char* fnl1 = argv[1];
  const char* fnl2 = argv[2];

  auto l1 = readList(fnl1);
  auto l2 = readList(fnl2);

  ChrInfoHg19 hg19;
  for (uint32_t i = 0; i < l1.size(); i++) {
    GCords g1;
    g1.read(l1[i].c_str(), "cse", 0, &hg19);
    GCordsInfoCache g1inf(&g1);
    for (uint32_t j = 0; j < l2.size(); j++) {
      GCords g2;

      GCordsInfoCache g2inf(&g2);
      g2.read(l2[j].c_str(), "...........cs", 0, &hg19);
      forbes(g2inf, g1inf);
    }
  }
  printf("ok\n");
}
