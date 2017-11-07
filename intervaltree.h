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
  void print() const;
  uint32_t numNodes() const { return m_numNodes; }
private:
  class ItNode {
  public:
    ItNode(const std::vector<T> &is);
    ~ItNode();
    bool overlaps(uint64_t p) const;
    void print(uint32_t lvl, const char *s) const;
    uint32_t countNodes() const;
  private:
    uint64_t m_center;
    std::vector<T> m_bs;
    std::vector<T> m_es;
    ItNode* m_s;
    ItNode* m_g;
  };
  ItNode *m_root;
  uint32_t m_numNodes;
};

#endif /* INTERVALTREE_H_ */
