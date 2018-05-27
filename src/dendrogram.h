#ifndef DENDROGRAM_H_
#define DENDROGRAM_H_

#include <vector>
#include <stdint.h>

class DendrogramNode;

typedef std::vector<uint32_t> DgNodeHPosMap;

class DgNode {
public:
  ~DgNode();
  bool isLeaf() const { return !m_lo; }
  const DgNode *getLo() const {return m_lo;}
  const DgNode *getHi() const {return m_hi;}
  void getRoots(uint32_t i0, uint32_t i1, std::vector<const DgNode*> *res) const;
  static std::vector<DgNode*> build(std::vector<DendrogramNode*> roots, std::vector<uint32_t> *order);
  double distRel() const {return m_distRel;}
  double distAbs() const {return m_distAbs;}
  double distAbsMin() const {return m_distAbsMin;}
  double hPos() const {return m_hpos;}
  void print(uint32_t l=0, char s = '=') const;
  uint32_t idxLo() const {return m_idxLo;}
  uint32_t idxHi() const {return m_idxHi;}
private:
  DgNode   *m_lo;
  DgNode   *m_hi;
  uint32_t  m_lvl;
  uint32_t  m_id;
  double m_distAbs;
  double m_distAbsMin;
  double m_distRel;
  /* these will be set by setHPos(): */
  uint32_t  m_idxLo; /* outer lo ID; used for range search */
  uint32_t  m_idxHi; /* outer hi ID */
  double m_hpos; /* [0,1]; 0 <= lo < hi <= 1 */
  DgNode(const DendrogramNode*, double maxdist);
  void getOrder(std::vector<uint32_t> *o);
  void setHPosMap(const std::vector<uint32_t> &m);
  bool isInside(uint32_t i0, uint32_t i1) const;
  bool hasOverlap(uint32_t i0, uint32_t i1) const;
};


/*
 *                o root
 *   root o      / \
 *       / \    /   \
 *      /   o  |     \
 *     o    |  o     o
 *    / \   o  |\   / \
 *    | |  / \ | |  | |
 * lo                   hi
 *
 *       >- order ->
 *
 */




/*----------------------------------------------------------------------------*/

class Dendrogram {
public:
  Dendrogram(const std::vector<DendrogramNode*> &roots);
  ~Dendrogram();
  std::vector<uint32_t> getOrder() const {return m_orderedNodes;}
  std::vector<const DgNode*> getRoots(uint32_t i0, uint32_t i1) const;

private:
  std::vector<uint32_t> m_orderedNodes;
  DgNodeHPosMap         m_id2idx; /* nodeid -> hpos/order */
  std::vector<DgNode*>  m_roots;
  Dendrogram(const Dendrogram &o) = delete;
  Dendrogram & operator=(const Dendrogram &o) = delete;
};

/*----------------------------------------------------------------------------*/

class DendrogramBuilder {
public:
  DendrogramBuilder(uint32_t n);
  ~DendrogramBuilder();
  bool pair(uint32_t i, uint32_t j, float dist);
  Dendrogram* getDend();
private:
  std::vector<DendrogramNode*> m_nodes;
  uint32_t m_n;
  uint32_t m_numPairs;
};

#endif /* DENDROGRAM_H_ */

