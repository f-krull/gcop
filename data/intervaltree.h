#ifndef INTERVALTREE_H_
#define INTERVALTREE_H_


#include "segment.h"
#include <vector>

/*----------------------------------------------------------------------------*/

template <typename T>
class IntervalTree {
public:
  IntervalTree(const std::vector<T> &is);
  ~IntervalTree();
  bool overlaps(uint64_t p) const;

  bool overlaps(uint64_t p, std::vector<char> *res = NULL) const;
  bool overlaps(const T &i, std::vector<char> *res = NULL) const;

  void print() const;
  uint32_t numNodes() const     { return m_numNodes; }
  uint32_t numIntervals() const { return m_numIntervals; }
private:
  struct IndexedInterval {
    Interval iv;
    uint32_t srcIdx;
    class CmpEnd {
    public:
      bool operator()(const IndexedInterval &a, const IndexedInterval &b)   { return T::cmpEnd(a.iv, b.iv);  }
      bool operator()(const IndexedInterval &a, const uint64_t &b)   { return a.iv.e < b;}
      bool operator()(const uint64_t &a, const IndexedInterval &b)   { return a < b.iv.e;}
    };
    class CmpStart {
    public:
      bool operator()(const IndexedInterval &a, const IndexedInterval &b) { return T::cmpStart(a.iv, b.iv);}
      bool operator()(const IndexedInterval &a, const uint64_t &b)   { return a.iv.s < b;}
      bool operator()(const uint64_t &a, const IndexedInterval &b)   { return a < b.iv.s;}
    };

    IndexedInterval(const T& i, uint32_t idx) : iv(i.s, i.e), srcIdx(idx) {
    };
    static std::vector<IndexedInterval> build(const std::vector<T> &);
    static bool findSmallerPoints(const std::vector<IndexedInterval> &es,
        uint64_t e, std::vector<char> *res);
    static bool findGreaterPoints(const std::vector<IndexedInterval> &bs,
        uint64_t s, std::vector<char> *res);
    static bool findLeftOv(const std::vector<IndexedInterval> &es,
        T i, std::vector<char> *res);
    static bool findRightOv(const std::vector<IndexedInterval> &bs,
        T i, std::vector<char> *res);
  };
  class ItNode {
  public:
    static ItNode* build_tree(const std::vector<T> &is);
    ~ItNode();
    bool overlaps(uint64_t p) const;
    bool getOverlaps(uint64_t p, std::vector<char> *res, uint32_t depth) const;
    void print(uint32_t lvl, const char *s) const;
    uint32_t countNodes() const;
  private:
    ItNode(const std::vector<IndexedInterval> &is);
    uint64_t m_center;
    std::vector<IndexedInterval> m_bs;
    std::vector<IndexedInterval> m_es;
    ItNode* m_s;
    ItNode* m_g;
  };

  ItNode *m_root;
  std::vector<IndexedInterval> m_bs;
  std::vector<IndexedInterval> m_es;
  uint32_t m_numNodes;
  uint32_t m_numIntervals;
};

#endif /* INTERVALTREE_H_ */
