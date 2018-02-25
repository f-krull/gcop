#include "strlist.cpp"
#include "gcinfocache.cpp"
#include <assert.h>
#include <stdint.h>
#include <regex>

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
  uint64_t n_a = 0;  /* |A|       */
  uint64_t n_b = 0;  /* |B|       */
  uint64_t n_c = 0;  /*  N        */
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

void writeMat_cg(const char *fn, const std::vector<std::vector<float>> & mat,
    const StrTable &l1, const StrTable &l2) {
  FILE *f = fopen(fn, "w");
  assert(f);
  /* header */
  for (uint32_t j = 0; j < l1.nrows(); j++) {
    fprintf(f, "\t(");
    for (uint32_t k = 1; k < l1.ncols(); k++) {
      fprintf(f, "%s'%s: %s'", k == 1 ? "" : ", ", l1.header()[k].c_str(), l1.body()[j][k].c_str());
    }
    fprintf(f, ")");
  }
  fprintf(f, "\n");
  /* data */
  for (uint32_t i = 0; i < l2.nrows(); i++) {
    fprintf(f, "(");
    for (uint32_t k = 1; k < l2.ncols(); k++) {
      fprintf(f, "%s'%s: %s'", k == 1 ? "" : ", ", l2.header()[k].c_str(), l2.body()[i][k].c_str());
    }
    fprintf(f, ")");
    for (uint32_t j = 0; j < l1.nrows(); j++) {
      fprintf(f, "%s%f", "\t", mat[j][i]);
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

/*----------------------------------------------------------------------------*/

static std::vector<std::vector<float>> disreg(const StrTable &l1,
    const StrTable &l2, const char *fmt1, const char *fmt2, uint64_t expand2,
    const char *outfn) {
  assert(l1.ncols() >= 1);
  assert(l2.ncols() >= 1);
  ChrInfoHg19 hg19;
  std::vector<std::vector<float>> mat;
  mat.resize(l1.nrows());
  for (uint32_t i = 0; i < l1.nrows(); i++) {
    mat[i].resize(l2.nrows(), 0.f);
    GCords g1;
    g1.read(l1.body()[i][0].c_str(), fmt1, 0, &hg19);
    GCordsInfoCache g1inf(&g1);
    for (uint32_t j = 0; j < l2.nrows(); j++) {
      GCords g2;
      g2.read(l2.body()[j][0].c_str(), fmt2, 0, &hg19);
      g2.expand(expand2);
      GCordsInfoCache g2inf(&g2);
      mat[i][j] = forbes(g2inf, g1inf);
    }
  }

  class _TmpFunc {
  public:
    static std::string cleanFn(const std::string &fn) {
      std::string res = fn;
      res = std::regex_replace(res, std::regex(".*/"),   "");
      res = std::regex_replace(res, std::regex(".gz$"),  "");
      res = std::regex_replace(res, std::regex(".txt$"), "");
      return res;
    }
  };

  /* write matrix */
  writeMat_cg(outfn, mat, l1, l2);
#if 0
  {
    FILE *f = fopen(outfn, "w");
    assert(f);
    /* header */
    for (uint32_t j = 0; j < l1.nrows(); j++) {
      fprintf(f, "\t%s",_TmpFunc::cleanFn(l1.body()[j][0]).c_str());
    }
    fprintf(f, "\n");
    /* data */
    for (uint32_t i = 0; i < l2.nrows(); i++) {
      fprintf(f, "%s", _TmpFunc::cleanFn(l2.body()[i][0]).c_str());
      for (uint32_t j = 0; j < l1.nrows(); j++) {
        fprintf(f, "%s%f", "\t", mat[j][i]);
      }
      fprintf(f, "\n");
    }
    fclose(f);
  }
  {
  FILE *f = fopen(outfn, "w");
    assert(f);
    /* header */
    for (uint32_t j = 0; j < l2.get().size(); j++) {
      fprintf(f, "\t%s", _TmpFunc::cleanFn(l2.get()[j]).c_str());
    }
    fprintf(f, "\n");
    /* data */
    for (uint32_t i = 0; i < mat.size(); i++) {
      fprintf(f, "%s", _TmpFunc::cleanFn(l1.get()[i]).c_str());
      for (uint32_t j = 0; j < mat[i].size(); j++) {
        fprintf(f, "%s%f", "\t", mat[i][j]);
      }
      fprintf(f, "\n");
    }
    fclose(f);
  }
#endif
  return mat;
}

/*----------------------------------------------------------------------------*/

#include "../../int/objs_decl.h"

OBJS_DECL_GCCLASS(StrList,  StrList)
OBJS_DECL_GCCLASS(StrTable, StrTable)

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

#include "../../int/command.h"
#include "../../int/objspace.h"

class CmdLoadStrTable : public GcCommand {
public:
  CmdLoadStrTable() {
    addParam(GcCmdParam(PARAM_DST_STR,      GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,     GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_HEADER_INT,   GcCmdParam::PARAM_INT,    "0"));
    addParam(GcCmdParam(PARAM_MAXLINES_INT, GcCmdParam::PARAM_INT,    "0"));
  }
  const char* name() const {
    return "load_strtable";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
  static std::string PARAM_HEADER_INT;
  static std::string PARAM_MAXLINES_INT;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadStrTable::PARAM_DST_STR      = "dst";
std::string CmdLoadStrTable::PARAM_FILE_STR     = "file";
std::string CmdLoadStrTable::PARAM_HEADER_INT   = "header";
std::string CmdLoadStrTable::PARAM_MAXLINES_INT = "maxlines";

void CmdLoadStrTable::executeChild(const char *, GcObjSpace *os) {
  GcObjStrTable *gcsl = new GcObjStrTable();
  const char     *filename = getParam(PARAM_FILE_STR)->valStr().c_str();
  const bool      header   = getParam(PARAM_HEADER_INT)->valInt();
  const uint32_t  maxlines = getParam(PARAM_MAXLINES_INT)->valInt();
  gcsl->d()->read(filename, header, maxlines);
  os->addObj(getParam(PARAM_DST_STR)->valStr(), gcsl);
}

/*----------------------------------------------------------------------------*/

class CmdDisReg : public GcCommand {
public:
  CmdDisReg() {
    addParam(GcCmdParam(PARAM_TABLE1_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_TABLE2_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT1_STR, GcCmdParam::PARAM_STRING, "cse"));
    addParam(GcCmdParam(PARAM_FORMAT2_STR, GcCmdParam::PARAM_STRING, "cse"));
    addParam(GcCmdParam(PARAM_EXPAND2_INT, GcCmdParam::PARAM_INT, "0"));
    addParam(GcCmdParam(PARAM_OUTPUT_STR,  GcCmdParam::PARAM_STRING, "/tmp/disreg_mat.txt"));
  }
  const char* name() const {
    return "disreg";
  }
  static std::string PARAM_TABLE1_STR;
  static std::string PARAM_TABLE2_STR;
  static std::string PARAM_FORMAT1_STR;
  static std::string PARAM_FORMAT2_STR;
  static std::string PARAM_EXPAND2_INT;
  static std::string PARAM_OUTPUT_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdDisReg::PARAM_TABLE1_STR  = "tab1";
std::string CmdDisReg::PARAM_TABLE2_STR  = "tab2";
std::string CmdDisReg::PARAM_FORMAT1_STR  = "fmt1";
std::string CmdDisReg::PARAM_FORMAT2_STR  = "fmt2";
std::string CmdDisReg::PARAM_EXPAND2_INT  = "expand2";
std::string CmdDisReg::PARAM_OUTPUT_STR   = "output";

void CmdDisReg::executeChild(const char *, GcObjSpace *os) {
  GcObjStrTable *gcl1 = os->getObj<GcObjStrTable>(getParam(PARAM_TABLE1_STR)->valStr().c_str());
  GcObjStrTable *gcl2 = os->getObj<GcObjStrTable>(getParam(PARAM_TABLE2_STR)->valStr().c_str());
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
    StrTable l1;
    StrTable l2;
    l1.read(fnl1, true);
    l2.read(fnl2, true);
    disreg(l1, l2, "cse", "...........cs", expand2, "/tmp/disreg_mat.txt");
  } else {
    GcScriptEnv e;
    e.addCmd(new CmdLoadStrList);
    e.addCmd(new CmdLoadStrTable);
    e.addCmd(new CmdDisReg);
    e.runFile(argc >= 2 ? argv[1] : NULL);
  }
  printf("ok\n");
}
