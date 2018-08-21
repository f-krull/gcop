#ifndef HMMAT_H_
#define HMMAT_H_

#include <stdint.h>
#include <vector>
#include <string>

/*----------------------------------------------------------------------------*/

class Dendrogram;
class HmMatPriv;

#define ENUM_DISTFUNCTYPE(select_fun) \
    select_fun(DISTFUNC_EUCLIDEAN  , "euclidean"   ) \
    select_fun(DISTFUNC_CORRPEARSON, "corrpearson" ) \
    select_fun(DISTFUNC_CORRSPEARS , "corrspears"  )


class HmMat {
public:
  enum OrderType {
    ORDER_RANDOM_X,
    ORDER_RANDOM_Y,
    ORDER_HCLUSTER_SL_X,
    ORDER_HCLUSTER_SL_Y,
    ORDER_HCLUSTER_CL_X,
    ORDER_HCLUSTER_CL_Y,
    ORDER_ALPHABELIC_X,
    ORDER_ALPHABELIC_Y
  };



  enum DistFuncType {
#define ENUM_GET_ENAME(name, str) name,
    ENUM_DISTFUNCTYPE(ENUM_GET_ENAME)
    DISTFUNC_NUMENTRIES
#undef ENUM_GET_ENAME
  };

  static const char* DistFuncTypeStr[];

  enum SeriesType {
    SERIESTYPE_ROW,
    SERIESTYPE_COL
  };

  HmMat();
  ~HmMat();
  bool read(const char* fn);
  double get(uint32_t row, uint32_t col) const { return m_d[row][col]; }
  char   isSel(uint32_t row, uint32_t col) const { return m_sel[row][col]; }
  void   sel(uint32_t row, uint32_t col) { m_sel[row][col] = 1; }
  void   sel(uint32_t r0, uint32_t c0, uint32_t r1, uint32_t c1);
  void unSel();
  void cropSel();
  const char* xlab(uint32_t j) const;
  const char* ylab(uint32_t i) const;

  void selLab(SeriesType, const char *regex);
  bool isSelLab(SeriesType, uint32_t k) const;

  uint32_t nrow() const { return m_d.size(); }
  uint32_t ncol() const { return m_d.empty() ? 0 : m_d.front().size(); }
  void print() const;
  double maxVal() const;
  double minVal() const;
  void transpose();
  void order(OrderType ot);
  bool setDistFunc(const char* str);
  const Dendrogram *getDendY() const { return m_dendY; }
  const Dendrogram *getDendX() const { return m_dendX; }
private:
  void init();
  void orderByNameX();
  void orderByNameY();
  Dendrogram* orderByHClusterY(char clusType);
  Dendrogram* orderByHClusterX(char clusType);
  void orderRandomX();
  void orderRandomY();
  void resetOrderX();
  void resetOrderY();
  std::vector<std::vector<double>> m_d;
  std::vector<std::vector<char>> m_sel;
  std::vector<char> m_selLabX;
  std::vector<char> m_selLabY;
  std::vector<std::string> m_xlab;
  std::vector<std::string> m_ylab;
  bool check() const;
  void reset();
  void applyOrderY(const std::vector<uint32_t> &o);
  Dendrogram *m_dendX;
  Dendrogram *m_dendY;
  DistFuncType m_distFunc;
};

#endif /* HMMAT_H_ */
