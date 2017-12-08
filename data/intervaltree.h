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

  std::vector<char> getOverlaps(uint64_t) const;
  std::vector<char> getOverlaps(const T &i) const;

  void print() const;
  uint32_t numNodes() const { return m_numNodes; }
  uint32_t numIntervals() const { return m_numIntervals; }
private:
  struct IndexedInterval {
    Interval iv;
    uint32_t srcIdx;
    static bool cmpStart(const IndexedInterval &a, const IndexedInterval &b) { return T::cmpStart(a.iv, b.iv);}
    static bool cmpEnd(const IndexedInterval &a, const IndexedInterval &b) { return !T::cmpEnd(a.iv, b.iv);}
    IndexedInterval(const T& i, uint32_t idx) : iv(i.s, i.e), srcIdx(idx) {
    };
    static std::vector<IndexedInterval> build(const std::vector<T> &);
    static void findSmallerPoints(const std::vector<IndexedInterval> &es,
        uint64_t e, std::vector<char> *res);
    static void findGreaterPoints(const std::vector<IndexedInterval> &bs,
        uint64_t s, std::vector<char> *res);
    static void findLeftOv(const std::vector<IndexedInterval> &es,
        T i, std::vector<char> *res);
    static void findRightOv(const std::vector<IndexedInterval> &bs,
        T i, std::vector<char> *res);
  };
  class ItNode {
  public:
    static ItNode* build_tree(const std::vector<T> &is);
    ~ItNode();
    bool overlaps(uint64_t p) const;
    void getOverlaps(uint64_t p, std::vector<char> *res, uint32_t depth) const;
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
