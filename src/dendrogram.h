#ifndef DENDROGRAM_H_
#define DENDROGRAM_H_

#include <vector>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class DendrogramNode {
public:
  DendrogramNode(int32_t id);
  bool isRoot() const;
  bool isLeaf() const;
  int32_t id() const;
  static DendrogramNode * merge(int32_t id, DendrogramNode *l, DendrogramNode *r, float dist);
  static std::vector<uint32_t> geGlobalLeafIds(const DendrogramNode *root);

private:
  DendrogramNode *m_p;
  DendrogramNode *m_l;
  DendrogramNode *m_r;
  float m_dist;
  float m_minDist;
  bool m_isLeaf;
  int32_t m_id;
  uint32_t m_depth;

  DendrogramNode(int32_t id, DendrogramNode *l, DendrogramNode *r, float dist);
  void getGlobalLeafIds(std::vector<uint32_t> &leafIds) const;
};

/*----------------------------------------------------------------------------*/

class Dendrogram {
public:
  Dendrogram(uint32_t n);
  ~Dendrogram();
  bool pair(uint32_t i, uint32_t j, float dist);
  std::vector<uint32_t> getOrder() const;

private:
  std::vector<DendrogramNode*> m_nodes;
  uint32_t m_n;
  uint32_t m_numPairs;
};

#endif /* DENDROGRAM_H_ */
