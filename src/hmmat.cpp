#include "hmmat.h"
#include "buffer.h"
#include "distmatrix.h"
#include "clusterersl.h"
#include "clusterercl.h"
#include "correlation.h"
#include <stdio.h>
#include <float.h>
#include <numeric>
#include <algorithm>
#include <assert.h>
#include <map>
#include <string.h>

/*----------------------------------------------------------------------------*/

#define HCLUS_TYPE_SL 's'
#define HCLUS_TYPE_CL 'c'

/*----------------------------------------------------------------------------*/

const char* HmMat::DistFuncTypeStr[] = {
#define  ENUM_GET_STR(name, str) str,
        ENUM_DISTFUNCTYPE(ENUM_GET_STR)
        "undefined"
#undef ENUM_GET_NAME
};

/*----------------------------------------------------------------------------*/

typedef std::vector<double> HmmRow;

/*----------------------------------------------------------------------------*/

class HmmRowDistFunc {
public:
  virtual ~HmmRowDistFunc() {}
  virtual double operator()(const HmmRow &r1, const HmmRow &r2) = 0;
};

/*----------------------------------------------------------------------------*/

class HmmRowDistEuclidean : public HmmRowDistFunc {
public:
  double operator()(const HmmRow &r1, const HmmRow &r2) {
    double sumsq = 0.f;
#define POW2(x) ((x)*(x))
    for (uint32_t j = 0; j < r1.size(); j++) {
      sumsq += POW2(r1[j] - r2[j]);
    }
#undef POW2
    return sqrt(sumsq);
  }
private:
};

/*----------------------------------------------------------------------------*/

class HmmRowDistCorrPearsonCache : public HmmRowDistFunc {
public:
  double operator()(const HmmRow &r1, const HmmRow &r2) {
    const AvgSdInf i1 = getAvgSdInf(&r1);
    const AvgSdInf i2 = getAvgSdInf(&r2);
    return (1.-corrPearson(r1, r2, i1.avg, i2.avg, i1.stddev, i2.stddev))/2;
  }
private:
  struct AvgSdInf {
    double avg;
    double stddev;
  };
  const AvgSdInf & getAvgSdInf(const HmmRow *r1) {
    auto it = m_row2inf.find(r1);
    if (it != m_row2inf.end()) {
      return it->second;
    }
    const double mean = avg(*r1);
    const double sd  = stddev(*r1, mean);
    auto p = std::make_pair(r1, AvgSdInf{mean, sd});
    return m_row2inf.insert(p).first->second;
  }
  std::map<const HmmRow*, AvgSdInf > m_row2inf;
};

/*----------------------------------------------------------------------------*/

class HmmRowDistCorrSpearsCache : public HmmRowDistFunc {
public:
  double operator()(const HmmRow &r1, const HmmRow &r2) {
    const std::vector<uint32_t> &cr1 = getCachedRank(&r1);
    const std::vector<uint32_t> &cr2 = getCachedRank(&r2);
    return (1.-corrSpearsRanked(cr1, cr2))/2;
  }
private:
  const std::vector<uint32_t> & getCachedRank(const HmmRow *r1) {
    auto it = m_row2rank.find(r1);
    if (it != m_row2rank.end()) {
      return it->second;
    }
    auto p = std::make_pair(r1, rank(*r1));
    return m_row2rank.insert(p).first->second;
  }
  std::map<const HmmRow*, std::vector<uint32_t> > m_row2rank;
};

/*----------------------------------------------------------------------------*/

class HmmRowDistCmp {
public:
  HmmRowDistCmp(HmMat::DistFuncType t) {
    m_f = NULL;
    setType(t);
  }
  ~HmmRowDistCmp() {
    delete m_f;
  }
  void setType(HmMat::DistFuncType t) {
    delete  m_f;
    switch (t) {
      case HmMat::DISTFUNC_EUCLIDEAN:
        m_f = new HmmRowDistEuclidean;
        break;
      case HmMat::DISTFUNC_CORRPEARSON:
        m_f = new HmmRowDistCorrPearsonCache;
        break;
      case HmMat::DISTFUNC_CORRSPEARS:
        m_f = new HmmRowDistCorrSpearsCache;
        break;
      default:
        assert(false);
        break;
    }
  }
  double operator()(const HmmRow &r1, const HmmRow &r2) const {
    return (*m_f)(r1, r2);
  }
private:
  HmmRowDistFunc *m_f;
};

/*----------------------------------------------------------------------------*/

HmMat::HmMat() : m_dendX(NULL), m_dendY(NULL) {
  m_distFunc = DISTFUNC_EUCLIDEAN;
}

/*----------------------------------------------------------------------------*/

HmMat::~HmMat() {
  reset();
}

/*----------------------------------------------------------------------------*/

bool HmMat::read(const char* fn) {
  /*
   * mat format:
   *         [sep] <xlab0> [sep] <xlab1> [sep] <xlab2> ...
   * <ylab0> [sep] <val00> [sep] <val01> [sep] ...
   * <ylab1> [sep] ...
   * ...
   */
  reset();
  FILE *f = fopen(fn, "r");
  if (f == NULL) {
    return false;
  }
  const char sep = '\t';
  bool ret = true;
  /* read header */
  while (true) {
    /* read one char */
    char c;
    if ((fread(&c, 1, 1, f)) != 1) {
      ret = false;
      break;
    }
    /* if sep -> create new label */
    if (c == sep) {
      m_xlab.push_back(std::string());
      continue;
    }
    /* if new line -> break */
    if (c == '\n') {
      break;
    }
    /* if char -> add to last label */
    if (m_xlab.empty()) {
      ret = false;
      break;
    }
    m_xlab.back().push_back(c);
  }
  /* read body */
  {
    std::string valbuf;
    char c;
    enum {
      READ_LINESTART,
      READ_YLAB,
      READ_VAL
    } state;
    state = READ_LINESTART;
    while (fread(&c, 1, 1, f) == 1) {
      switch (state) {
        case READ_LINESTART:
          m_ylab.push_back(std::string());
          m_ylab.back().push_back(c);
          state = READ_YLAB;
          break;
        case READ_YLAB:
          switch (c) {
            case sep:
              m_d.push_back(std::vector<double>());
              state = READ_VAL;
              break;
            default:
              m_ylab.back().push_back(c);
              break;
          }
          break;
          case READ_VAL:
            switch (c) {
              case sep:
                m_d.back().push_back(atof(valbuf.c_str()));
                valbuf.clear();
                break;
              case '\n':
                m_d.back().push_back(atof(valbuf.c_str()));
                valbuf.clear();
                state = READ_LINESTART;
                break;
              default:
                valbuf.push_back(c);
                break;
            }
            break;
        default:
          break;
      }
    }
  }
  fclose(f);
  ret &= check();
  if (!ret) {
    reset();
  }
  init();
  return ret;
}

/*----------------------------------------------------------------------------*/

void HmMat::print() const {
  printf("ncol=%u\n", ncol());
  printf("nrow=%u\n", nrow());
  for (uint32_t i = 0; i < ncol(); i++) {
    printf("\t%s", m_xlab[i].c_str());
  }
  printf("\n");
  for (uint32_t i = 0; i < nrow(); i++) {
    printf("%s", m_ylab[i].c_str());
    for (uint32_t j = 0; j < ncol(); j++) {
      printf("\t%f", m_d[i][j]);
    }
    printf("\n");
  }
}

/*----------------------------------------------------------------------------*/

bool HmMat::check() const {
  bool ret = true;
  ret &= m_ylab.size() == nrow();
  ret &= m_xlab.size() == ncol();
  uint32_t i = 0;
  while (ret && i < nrow()) {
    ret &= m_d[i].size() == ncol();
    i++;
  }
  return ret;
}

/*----------------------------------------------------------------------------*/

void HmMat::reset() {
  m_xlab.clear();
  m_ylab.clear();
  m_d.clear();
  m_sel.clear();
  m_selLabX.clear();
  m_selLabY.clear();
  delete m_dendX;
  delete m_dendY;
  m_dendX = NULL;
  m_dendY = NULL;
}

/*----------------------------------------------------------------------------*/

double HmMat::minVal() const {
  double r = DBL_MAX;
  for (uint32_t i = 0; i < nrow(); i++) {
    for (uint32_t j = 0; j < ncol(); j++) {
      const double v = m_d[i][j];
      r = r < v ? r : v;
    }
  }
  return r;
}

/*----------------------------------------------------------------------------*/

double HmMat::maxVal() const {
  double r = DBL_MIN;
  for (uint32_t i = 0; i < nrow(); i++) {
    for (uint32_t j = 0; j < ncol(); j++) {
      const double v = m_d[i][j];
      r = r > v ? r : v;
    }
  }
  return r;
}

/*----------------------------------------------------------------------------*/

void HmMat::transpose() {
  HmMat t;
  t.m_xlab = m_ylab;
  t.m_ylab = m_xlab;
  t.m_dendY = m_dendX;
  t.m_dendX = m_dendY;
  t.m_distFunc = m_distFunc;
  t.m_selLabX = m_selLabY;
  t.m_selLabY = m_selLabX;
  for (uint32_t j = 0; j < ncol(); j++) {
    t.m_d.push_back(std::vector<double>());
    t.m_sel.push_back(std::vector<char>());
    for (uint32_t i = 0; i < nrow(); i++) {
      t.m_d.back().push_back(get(i,j));
      t.m_sel.back().push_back(isSel(i,j));
    }
  }
  *this= t;
  t.m_dendY = NULL;
  t.m_dendX = NULL;
}

/*----------------------------------------------------------------------------*/

const char* HmMat::xlab(uint32_t j) const {
  return m_xlab[j].c_str();
}

/*----------------------------------------------------------------------------*/

const char* HmMat::ylab(uint32_t i) const {
  return m_ylab[i].c_str();
}

/*----------------------------------------------------------------------------*/

void HmMat::applyOrderY(const std::vector<uint32_t> &order) {
  std::vector<std::string> ylab_new;
  for (uint32_t i = 0; i < nrow(); i++) {
    ylab_new.push_back(m_ylab[order[i]]);
  }
  m_ylab = ylab_new;
  std::vector<std::vector<double>> d_new;
  for (uint32_t i = 0; i < nrow(); i++) {
    d_new.push_back(m_d[order[i]]);
  }
  m_d = d_new;
}

/*----------------------------------------------------------------------------*/

void HmMat::orderByNameY() {
  std::vector<uint32_t> order(nrow());
  std::iota(order.begin(), order.end(), 0);
  std::vector<std::string> &ylin = m_ylab;
  sort(order.begin(), order.end(),
      [&ylin](uint32_t i1, uint32_t i2) {return ylin[i1] < ylin[i2];});
  applyOrderY(order);
}

/*----------------------------------------------------------------------------*/

void HmMat::orderByNameX() {
  transpose(); /* hehe.. */
  orderByNameY();
  transpose();
}

/*----------------------------------------------------------------------------*/

Dendrogram * HmMat::orderByHClusterY(char t) {
  HmmRowDistCmp cmp(m_distFunc);
  DistanceMatrix *dm = DistanceMatrixFactory::getFilled(&m_d, cmp);
  std::vector<std::vector<uint32_t> > clusters;
  Dendrogram *d = (t == HCLUS_TYPE_SL) ? ClustererSl::clusterAll(dm) : ClustererCl::clusterAll(dm);
  delete dm;
  std::vector<uint32_t> order = d->getOrder();
  assert(order.size() == nrow());
  applyOrderY(order);
  return d;
}

/*----------------------------------------------------------------------------*/

Dendrogram * HmMat::orderByHClusterX(char t) {
  transpose();
  Dendrogram *d = orderByHClusterY(t);
  transpose();
  return d;
}
/*----------------------------------------------------------------------------*/

void HmMat::orderRandomY() {
  resetOrderY();
  std::vector<uint32_t> order(nrow());
  std::iota(order.begin(), order.end(), 0);
  auto rnde = std::default_random_engine {};
  std::shuffle(std::begin(order), std::end(order), rnde);
  applyOrderY(order);
}

/*----------------------------------------------------------------------------*/

void HmMat::orderRandomX() {
  transpose();
  orderRandomY();
  transpose();
}

/*----------------------------------------------------------------------------*/

void HmMat::resetOrderX() {
  delete m_dendX;
  m_dendX = NULL;
  m_selLabX.clear();
  m_selLabX.resize(ncol(), 0);
  unSel();
}

/*----------------------------------------------------------------------------*/

void HmMat::resetOrderY() {
  delete m_dendY;
  m_dendY = NULL;
  m_selLabY.clear();
  m_selLabY.resize(nrow(), 0);
  unSel();
}

/*----------------------------------------------------------------------------*/

void HmMat::order(OrderType ot) {
  switch (ot) {
    case ORDER_ALPHABELIC_X:
      resetOrderX();
      orderByNameX();
      break;
    case ORDER_ALPHABELIC_Y:
      resetOrderY();
      orderByNameY();
      break;
    case ORDER_RANDOM_X:
      resetOrderX();
      orderByNameX();
      break;
    case ORDER_RANDOM_Y:
      resetOrderY();
      orderByNameY();
      break;
    case ORDER_HCLUSTER_SL_X:
      resetOrderX();
      m_dendX = orderByHClusterX(HCLUS_TYPE_SL);
      break;
    case ORDER_HCLUSTER_SL_Y:
      resetOrderY();
      m_dendY = orderByHClusterY(HCLUS_TYPE_SL);
      break;
    case ORDER_HCLUSTER_CL_X:
      resetOrderX();
      m_dendX = orderByHClusterX(HCLUS_TYPE_CL);
      break;
    case ORDER_HCLUSTER_CL_Y:
      resetOrderY();
      m_dendY = orderByHClusterY(HCLUS_TYPE_CL);
      break;
    default:
      assert(false && "enum ordertype not handled");
      break;
  }
}

/*----------------------------------------------------------------------------*/

void HmMat::sel(uint32_t _r0, uint32_t _c0, uint32_t _r1, uint32_t _c1) {
  /* make r0,c0 top-left and r1,c1 bottom-right */
  unSel();
  const uint32_t r0 = _r0 <= _r1 ? _r0 : _r1;
  const uint32_t r1 = _r1 >= _r0 ? _r1 : _r0;
  const uint32_t c0 = _c0 <= _c1 ? _c0 : _c1;
  const uint32_t c1 = _c1 >= _c0 ? _c1 : _c0;
  for (uint32_t i = r0; i <= r1; i++) {
    for (uint32_t j = c0; j <= c1; j++) {
      sel(i, j);
    }
  }
}

/*----------------------------------------------------------------------------*/

void HmMat::unSel() {
  for (uint32_t i = 0; i < m_sel.size(); i++) {
    for (uint32_t j = 0; j < m_sel[i].size(); j++) {
      m_sel[i][j] = 0;
    }
  }
}

/*----------------------------------------------------------------------------*/

void HmMat::init() {
  delete m_dendX;
  delete m_dendY;
  m_dendX = NULL;
  m_dendY = NULL;
  m_sel.resize(m_d.size());
  for (uint32_t i = 0; i < m_d.size(); i++) {
    m_sel[i].clear();
    m_sel[i].resize(m_d[i].size(), 0);
  }
  m_selLabX.clear();
  m_selLabY.clear();
  m_selLabX.resize(ncol(), 0);
  m_selLabY.resize(nrow(), 0);
}

/*----------------------------------------------------------------------------*/

void HmMat::cropSel() {
  /* get (i0,j0) (i1,j1) */
  uint32_t r0 = nrow() > 0 ? nrow()-1 : 0;
  uint32_t r1 = 0;
  uint32_t c0 = ncol() > 0 ? ncol()-1 : 0;
  uint32_t c1 = 0;
  for (uint32_t i = 0; i < m_sel.size(); i++) {
    for (uint32_t j = 0; j < m_sel[i].size(); j++) {
      if (m_sel[i][j]) {
        r0 = std::min(r0, i);
        r1 = std::max(r1, i);
        c0 = std::min(c0, j);
        c1 = std::max(c1, j);
      }
    }
  }
  if (r0 > r1 || c0 > c1) {
    return;
  }
  /* crop */
  std::vector<std::vector<double>> dnew;
  std::vector<std::string> ylabnew;
  std::vector<std::string> xlabnew;
  for (uint32_t i = r0; i <= r1; i++) {
    dnew.push_back(std::vector<double>());
    for (uint32_t j = c0; j <= c1; j++) {
      dnew.back().push_back(m_d[i][j]);
    }
    ylabnew.push_back(m_ylab[i]);
  }
  for (uint32_t j = c0; j <= c1; j++) {
    xlabnew.push_back(m_xlab[j]);
  }
  /* replace current data */
  m_d = dnew;
  m_xlab = xlabnew;
  m_ylab = ylabnew;
  init();
}

/*----------------------------------------------------------------------------*/

bool HmMat::setDistFunc(const char* str) {
  for (uint32_t i = 0; i < DISTFUNC_NUMENTRIES; i++) {
    if (strcmp(str, DistFuncTypeStr[i]) == 0) {
      m_distFunc = (DistFuncType)i;
      return true;
    }
  }
  return false;
}

/*----------------------------------------------------------------------------*/

#include <regex.h>
void HmMat::selLab(SeriesType st, const char *str) {
  regex_t regex;
  if (regcomp(&regex, str, REG_EXTENDED | REG_ICASE) != 0) {
    return;
  }
  std::vector<char> & labsel        = (st == SERIESTYPE_COL) ? m_selLabX : m_selLabY;
  std::vector<std::string> & labstr = (st == SERIESTYPE_COL) ? m_xlab    : m_ylab;
  assert(labsel.size() == labstr.size());
  for (uint32_t i = 0; i < labstr.size(); i++) {
    labsel[i] = regexec(&regex, labstr[i].c_str(), 0, NULL, 0) == 0;
  }
  regfree(&regex);
}

/*----------------------------------------------------------------------------*/

bool HmMat::isSelLab(SeriesType t, uint32_t k) const {
  const std::vector<char> & sel = (t == SERIESTYPE_COL) ? m_selLabX : m_selLabY;
  return sel[k];
}
