
#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

std::string removeExtension(const std::string & filename) {
   std::string res = filename;
   // cut before last "."
   size_t pos_p = filename.find_last_of('.');
   if (pos_p != filename.npos) {
      res = res.substr(0, pos_p);
   }
   return res;
}

/*----------------------------------------------------------------------------*/

std::string removePath(const std::string & filename) {
   std::string res = filename;
   // cut at last "/"
   size_t pos_p = filename.find_last_of('/');
   if (pos_p != filename.npos && (pos_p + 1) < res.size()) {
      res = res.substr((pos_p + 1), res.size() - (pos_p + 1));
   }
   return res;
}

/*----------------------------------------------------------------------------*/

class StrList {
public:
  void readList(const char * filename, char comment = '#') {
    std::ifstream file;
    std::string line;

    file.open(filename);
    if (!file) {
      fprintf(stderr, "error - cannot open file %s\n", filename);
      exit(1);
    }
    while (getline((file), line)) {
       if ((line.length() > 0) && (line.at(0) != comment)) {
         m_l.push_back(line);
       }
    }
    file.close();
  }

  const std::vector<std::string> & get() const {
    return m_l;
  }
private:
  std::vector<std::string> m_l;
};

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
  //assert(g1inf.gcords()->chrinfo() == g2inf.gcords()->chrinfo());
  const ChrInfo & chrinfo = g1inf.gcords()->chrinfo();
  uint64_t n_ab = 0; /* |A and B| */
  uint64_t n_a = 0; /* |A| */
  uint64_t n_b = 0; /* |B| */
  uint64_t n_c = 0; /*  N */
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

static std::vector<std::vector<float>> disreg(const StrList &l1, const StrList &l2, const char *fmt1, const char *fmt2) {
  ChrInfoHg19 hg19;
  std::vector<std::vector<float>> mat;
  mat.resize(l1.get().size());
  for (uint32_t i = 0; i < l1.get().size(); i++) {
    mat[i].resize(l2.get().size(), 0.f);
#if 1
    GCords g1;
    g1.read(l1.get()[i].c_str(), fmt1, 0, &hg19);
    GCordsInfoCache g1inf(&g1);
    for (uint32_t j = 0; j < l2.get().size(); j++) {
      GCords g2;
      g2.read(l2.get()[j].c_str(), fmt2, 0, &hg19);
      g2.expand(25000);
      GCordsInfoCache g2inf(&g2);
      mat[i][j] = forbes(g2inf, g1inf);
    }
#endif
  }

  FILE *f = fopen("/tmp/mat.txt", "w");
  assert(f);
  /* header */
  for (uint32_t j = 0; j < l2.get().size(); j++) {
    fprintf(f, "\t%s", removePath(removeExtension(l2.get()[j])).c_str());
  }
  fprintf(f, "\n");
  /* data */
  for (uint32_t i = 0; i < mat.size(); i++) {
    fprintf(f, "%s", removePath(removeExtension(l1.get()[i])).c_str());
    for (uint32_t j = 0; j < mat[i].size(); j++) {
      fprintf(f, "%s%f", "\t", mat[i][j]);
    }
    fprintf(f, "\n");
  }
  fclose(f);
  return mat;
}

/*----------------------------------------------------------------------------*/

#include "../../int/objs_decl.h"

OBJS_DECL_GCCLASS(StrList, StrList)

/*----------------------------------------------------------------------------*/

#include "../../int/command.h"
#include "../../int/objspace.h"

class CmdLoadStrList : public GcCommand {
public:
  CmdLoadStrList() {
    addParam(GcCmdParam(PARAM_DST_STR,  GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "load_strlist";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadStrList::PARAM_DST_STR  = "dst";
std::string CmdLoadStrList::PARAM_FILE_STR = "file";

void CmdLoadStrList::executeChild(const char *, GcObjSpace *os) {
  GcObjStrList *gcsl = new GcObjStrList();
  gcsl->d()->readList(getParam(PARAM_FILE_STR)->valStr().c_str());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), gcsl);
}

/*----------------------------------------------------------------------------*/

class CmdDisReg : public GcCommand {
public:
  CmdDisReg() {
    addParam(GcCmdParam(PARAM_LIST1_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_LIST2_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT1_STR, GcCmdParam::PARAM_STRING, "cse"));
    addParam(GcCmdParam(PARAM_FORMAT2_STR, GcCmdParam::PARAM_STRING, "cse"));
  }
  const char* name() const {
    return "disreg";
  }
  static std::string PARAM_LIST1_STR;
  static std::string PARAM_LIST2_STR;
  static std::string PARAM_FORMAT1_STR;
  static std::string PARAM_FORMAT2_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdDisReg::PARAM_LIST1_STR  = "list1";
std::string CmdDisReg::PARAM_LIST2_STR  = "list2";
std::string CmdDisReg::PARAM_FORMAT1_STR  = "fmt1";
std::string CmdDisReg::PARAM_FORMAT2_STR  = "fmt2";

void CmdDisReg::executeChild(const char *, GcObjSpace *os) {
  GcObjStrList *gcl1 = os->getObj<GcObjStrList>(getParam(PARAM_LIST1_STR)->valStr().c_str());
  GcObjStrList *gcl2 = os->getObj<GcObjStrList>(getParam(PARAM_LIST2_STR)->valStr().c_str());
  const char *fmt1 = getParam(PARAM_FORMAT1_STR)->valStr().c_str();
  const char *fmt2 = getParam(PARAM_FORMAT2_STR)->valStr().c_str();
  disreg(*gcl1->d(), *gcl2->d(), fmt1, fmt2);
}

/*----------------------------------------------------------------------------*/
#include "../../int/scriptenv.h"
int main(int argc, char **argv) {
  if (argc == 3) {
    const char* fnl1 = argv[1];
    const char* fnl2 = argv[2];
    StrList l1;
    StrList l2;

    l1.readList(fnl1);
    l2.readList(fnl2);
    disreg(l1, l2, "cse", "...........cs");
  } else {
    GcScriptEnv e;
    e.addCmd(new CmdLoadStrList);
    e.addCmd(new CmdDisReg);
    e.runFile(argc >= 2 ? argv[1] : NULL);
  }
  printf("ok\n");
}
