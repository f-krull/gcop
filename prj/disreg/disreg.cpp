#include <assert.h>
#include <stdint.h>
#include "strlist.cpp"
#include "gcinfocache.cpp"

/*----------------------------------------------------------------------------*/

/* cut before last "." */
static std::string removeExtension(const std::string & filename) {
   std::string res = filename;
   size_t pos_p = filename.find_last_of('.');
   if (pos_p != filename.npos) {
      res = res.substr(0, pos_p);
   }
   return res;
}

/*----------------------------------------------------------------------------*/

/* cut at last "/" */
static std::string removePath(const std::string & filename) {
   std::string res = filename;
   size_t pos_p = filename.find_last_of('/');
   if (pos_p != filename.npos && (pos_p + 1) < res.size()) {
      res = res.substr((pos_p + 1), res.size() - (pos_p + 1));
   }
   return res;
}

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
#if 0
  /* very expensive! should be checked higher up */
  assert(g1inf.gcords()->chrinfo() == g2inf.gcords()->chrinfo());
#endif
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

static std::vector<std::vector<float>> disreg(const StrList &l1,
    const StrList &l2, const char *fmt1, const char *fmt2, uint64_t expand2,
    const char *outfn) {
  ChrInfoHg19 hg19;
  std::vector<std::vector<float>> mat;
  mat.resize(l1.get().size());
  for (uint32_t i = 0; i < l1.get().size(); i++) {
    mat[i].resize(l2.get().size(), 0.f);
    GCords g1;
    g1.read(l1.get()[i].c_str(), fmt1, 0, &hg19);
    GCordsInfoCache g1inf(&g1);
    for (uint32_t j = 0; j < l2.get().size(); j++) {
      GCords g2;
      g2.read(l2.get()[j].c_str(), fmt2, 0, &hg19);
      g2.expand(expand2);
      GCordsInfoCache g2inf(&g2);
      mat[i][j] = forbes(g2inf, g1inf);
    }
  }

  /* write matrix */
  FILE *f = fopen(outfn, "w");
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
    addParam(GcCmdParam(PARAM_EXPAND2_INT, GcCmdParam::PARAM_INT, "0"));
    addParam(GcCmdParam(PARAM_OUTPUT_STR,  GcCmdParam::PARAM_STRING, "/tmp/disreg_mat.txt"));
  }
  const char* name() const {
    return "disreg";
  }
  static std::string PARAM_LIST1_STR;
  static std::string PARAM_LIST2_STR;
  static std::string PARAM_FORMAT1_STR;
  static std::string PARAM_FORMAT2_STR;
  static std::string PARAM_EXPAND2_INT;
  static std::string PARAM_OUTPUT_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdDisReg::PARAM_LIST1_STR  = "list1";
std::string CmdDisReg::PARAM_LIST2_STR  = "list2";
std::string CmdDisReg::PARAM_FORMAT1_STR  = "fmt1";
std::string CmdDisReg::PARAM_FORMAT2_STR  = "fmt2";
std::string CmdDisReg::PARAM_EXPAND2_INT  = "expand2";
std::string CmdDisReg::PARAM_OUTPUT_STR   = "output";

void CmdDisReg::executeChild(const char *, GcObjSpace *os) {
  GcObjStrList *gcl1 = os->getObj<GcObjStrList>(getParam(PARAM_LIST1_STR)->valStr().c_str());
  GcObjStrList *gcl2 = os->getObj<GcObjStrList>(getParam(PARAM_LIST2_STR)->valStr().c_str());
  const uint64_t expand2 = getParam(PARAM_EXPAND2_INT)->valInt();
  const char *fmt1 =  getParam(PARAM_FORMAT1_STR)->valStr().c_str();
  const char *fmt2 =  getParam(PARAM_FORMAT2_STR)->valStr().c_str();
  const char *outfn = getParam(PARAM_OUTPUT_STR)->valStr().c_str();
  disreg(*gcl1->d(), *gcl2->d(), fmt1, fmt2, expand2, outfn);
}

/*----------------------------------------------------------------------------*/

#include "../../int/scriptenv.h"
int main(int argc, char **argv) {
  if (argc == 4) {
    const char* fnl1 = argv[1];
    const char* fnl2 = argv[2];
    const uint64_t expand2 = atoll(argv[3]);
    StrList l1;
    StrList l2;
    l1.readList(fnl1);
    l2.readList(fnl2);
    disreg(l1, l2, "cse", "...........cs", expand2, "/tmp/disreg_mat.txt");
  } else {
    GcScriptEnv e;
    e.addCmd(new CmdLoadStrList);
    e.addCmd(new CmdDisReg);
    e.runFile(argc >= 2 ? argv[1] : NULL);
  }
  printf("ok\n");
}
