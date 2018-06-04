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
#if 0
static double forbes(GCordsInfoCache & g1inf, GCordsInfoCache & g2inf) {
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
    const std::vector<GCord> &ac = g1inf.gcords()->getChr(chr_curr);
    const std::vector<GCord> &bc = g2inf.gcords()->getChr(chr_curr);
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

  double num = n_c * n_ab;
  double den = n_a * n_b;
  double ret = den > 0 ? num/den : 0;
  printf("forbes: %f\n", ret);
  return ret;
}
#endif
/*----------------------------------------------------------------------------*/

void writeMat_cg(const char *fn, const std::vector<std::vector<double>> & mat,
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
      fprintf(f, "%s%lf", "\t", mat[j][i]);
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

/*----------------------------------------------------------------------------*/

void writeMat_txt(const char *fn, const std::vector<std::vector<double>> & mat,
    const StrTable &l1, const StrTable &l2) {
  FILE *f = fopen(fn, "w");
  assert(f);
  /* header */
  for (uint32_t j = 0; j < l1.nrows(); j++) {
    for (uint32_t k = 1; k < std::min(l1.ncols(), 2u); k++) {
      fprintf(f, "%s%s", (j == 0 ? "" : "\t"), l1.body()[j][k].c_str());
    }
  }
  fprintf(f, "\n");
  /* data */
  for (uint32_t i = 0; i < l2.nrows(); i++) {
    for (uint32_t k = 1; k < std::min(l2.ncols(), 2u); k++) {
      fprintf(f, "%s%s", "", l2.body()[i][k].c_str());
    }
    for (uint32_t j = 0; j < l1.nrows(); j++) {
      fprintf(f, "%s%lf", "\t", mat[j][i]);
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

/*----------------------------------------------------------------------------*/
#if 0
static std::vector<std::vector<double>> disreg(const StrTable &l1,
    const StrTable &l2, const char *fmt1, const char *fmt2, uint64_t expand2,
    const char *outfn) {
  assert(l1.ncols() >= 1);
  assert(l2.ncols() >= 1);
  ChrInfoHg19 hg19;
  std::vector<std::vector<double>> mat;
  mat.resize(l1.nrows());
  for (uint32_t i = 0; i < l1.nrows(); i++) {
    mat[i].resize(l2.nrows(), 0.f);
    GCords g1;
    g1.read(l1.body()[i][0].c_str(), fmt1, 0, &hg19);
    g1.flatten();
    GCordsInfoCache g1inf(&g1);
    for (uint32_t j = 0; j < l2.nrows(); j++) {
      GCords g2;
      g2.read(l2.body()[j][0].c_str(), fmt2, 0, &hg19);
      g2.expand(expand2);
      g2.flatten();
      GCordsInfoCache g2inf(&g2);
      mat[i][j] = forbes(g2inf, g1inf);
    }
  }
  /* write matrix */
  writeMat_cg(outfn, mat, l1, l2);
  return mat;
}
#endif

static uint64_t numOverlap(GCordsInfoCache & g1inf, GCordsInfoCache & g2inf) {
  uint64_t sum = 0;
  for (uint32_t i = 0; i < g1inf.gcords()->chrinfo().chrs().size(); i++) {
    ChrInfo::CType chr_curr = g1inf.gcords()->chrinfo().chrs()[i];
    const std::vector<GCord> &ac = g1inf.gcords()->getChr(chr_curr);
    const std::vector<GCord> &bc = g2inf.gcords()->getChr(chr_curr);
    if (ac.empty() && bc.empty()) {
      continue;
    }
    const IntervalTree<GCord> &bit = *g2inf.chr(chr_curr)->it();
    sum += calcAvsB(ac, bc, bit);
  }
  return sum;
}
#include <numeric>
#include <math.h>
static std::vector<std::vector<double>> disreg(const StrTable &l1,
    const StrTable &l2, const char *fmt1, const char *fmt2, uint64_t expand2,
    const char *outfn) {
  assert(l1.ncols() >= 1);
  assert(l2.ncols() >= 1);
  ChrInfoHg19 hg19;

  /*
   * s_N_vec: [1,s_m] segments/genes
   * p_M_vec: [1,p_n] points
   *
   *
   */

  const StrTable &inp_s_tab = l1;
  const StrTable &inp_p_tab = l2;
  const uint32_t s_m = inp_s_tab.nrows();
  const uint32_t p_n = inp_p_tab.nrows();

  std::vector<std::vector<double>>      o_mat(s_m);
  std::vector<std::vector<double>>      e_mat(s_m);
  std::vector<double>                 s_N_vec(s_m, 0.f);
  std::vector<double>                 p_M_vec(p_n, 0.f);
  //std::vector<double>              p_N_UP_vec(p_n, 0.f);

  /* compute O_ij */
  for (uint32_t i = 0; i < inp_p_tab.nrows(); i++) {
    GCords p_g;
    p_g.read(inp_p_tab.body()[i][0].c_str(), fmt2, 0, &hg19, true);
    p_g.toPoints();
    GCordsInfoCache p_ginf(&p_g);
    for (uint32_t j = 0; j < s_m; j++) {
      o_mat[j].resize(inp_p_tab.nrows(), 0.f);
      GCords s_g;
      s_g.read(inp_s_tab.body()[j][0].c_str(), fmt1, 0, &hg19, true);
      s_g.expand(expand2);
      s_g.flatten();
      GCordsInfoCache s_ginf(&s_g);
      //e_mat[j][i] = p_ginf.len()->get() * s_ginf.leqn()->get() / p_ginf.gcords()->chrinfo().len();
      o_mat[j][i] = numOverlap(p_ginf, s_ginf);
    }
  }
  /* compute N_1 .. N_m */
  s_N_vec.resize(s_m, 0);
  for (uint32_t j = 0; j < s_m; j++) {
    s_N_vec[j] = 0;
    for (uint32_t i = 0; i < p_n; i++) {
      s_N_vec[j] += o_mat[j][i];
    }
  }

  /* compute M_1 .. M_m */
  p_M_vec.resize(p_n, 0);
  for (uint32_t i = 0; i < p_n; i++) {
    p_M_vec[i] = 0;
    for (uint32_t j = 0; j < s_m; j++) {
      p_M_vec[i] += o_mat[j][i];
    }
  }

#if 0
  /* compute N_UP_1 .. N_UP_n */
  p_N_UP_vec.resize(0, p_n);
  for (uint32_t i = 0; i < p_n; i++) {
    p_N_UP_vec[i] = 0;
    for (uint32_t j = 0; j < s_m; j++) {
      p_N_UP_vec[i] += o_mat[j][i];
    }
  }
#endif

#if 0
  const double p_N_UP = std::accumulate(p_N_UP_vec.begin(), p_N_UP_vec.end(), 0.);
#endif
  const double sp_N  = std::accumulate(s_N_vec.begin(),     s_N_vec.end(),    0.);


  /* compute E_ij */
  assert(e_mat.size() == s_m);
  for (uint32_t j = 0; j < s_m; j++) {
    e_mat[j].resize(p_n, 0.);
    for (uint32_t i = 0; i < p_n; i++) {
      e_mat[j][i] =  s_N_vec[j] * p_M_vec[i] / sp_N;
    }
  }
  std::vector<std::vector<double>> z_mat(s_m); /**  z-scores
                                                * (O_ij - E_ij) /
                                                * (E_ij * (1-E_ij/N_j))^0.5
                                                */
  for (uint32_t j = 0; j < s_m; j++) {
    z_mat[j].resize(inp_p_tab.nrows(), 0.f);
    for (uint32_t i = 0; i < p_n; i++) {
      const double num = o_mat[j][i] - e_mat[j][i];
      const double den = sqrt(e_mat[j][i] * (1 - e_mat[j][i]/s_N_vec[j]));
      //const double num = e_mat[j][i];
      //const double den = 1.f;
      z_mat[j][i] = (den != 0) ? num / den : 0.f;
    }
  }
  /* write matrix */
  writeMat_txt(outfn, z_mat, inp_s_tab, inp_p_tab);
  return z_mat;
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
    addParam(GcCmdParam(PARAM_TABLE_SEG_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_TABLE_PNT_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT_SEG_STR, GcCmdParam::PARAM_STRING, "cse"));
    addParam(GcCmdParam(PARAM_FORMAT_PNT_STR, GcCmdParam::PARAM_STRING, "cse"));
    addParam(GcCmdParam(PARAM_EXPAND2_INT, GcCmdParam::PARAM_INT, "0"));
    addParam(GcCmdParam(PARAM_OUTPUT_STR,  GcCmdParam::PARAM_STRING, "/tmp/disreg_mat.txt"));
  }
  const char* name() const {
    return "disreg";
  }
  static std::string PARAM_TABLE_SEG_STR;
  static std::string PARAM_TABLE_PNT_STR;
  static std::string PARAM_FORMAT_SEG_STR;
  static std::string PARAM_FORMAT_PNT_STR;
  static std::string PARAM_EXPAND2_INT;
  static std::string PARAM_OUTPUT_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdDisReg::PARAM_TABLE_SEG_STR  = "tabs";
std::string CmdDisReg::PARAM_TABLE_PNT_STR  = "tabp";
std::string CmdDisReg::PARAM_FORMAT_SEG_STR  = "fmts";
std::string CmdDisReg::PARAM_FORMAT_PNT_STR  = "fmtp";
std::string CmdDisReg::PARAM_EXPAND2_INT  = "expand2";
std::string CmdDisReg::PARAM_OUTPUT_STR   = "output";

void CmdDisReg::executeChild(const char *, GcObjSpace *os) {
  GcObjStrTable *gcl1 = os->getObj<GcObjStrTable>(getParam(PARAM_TABLE_SEG_STR)->valStr().c_str());
  GcObjStrTable *gcl2 = os->getObj<GcObjStrTable>(getParam(PARAM_TABLE_PNT_STR)->valStr().c_str());
  const uint64_t expand2 = getParam(PARAM_EXPAND2_INT)->valInt();
  const char *fmt1 =  getParam(PARAM_FORMAT_SEG_STR)->valStr().c_str();
  const char *fmt2 =  getParam(PARAM_FORMAT_PNT_STR)->valStr().c_str();
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
