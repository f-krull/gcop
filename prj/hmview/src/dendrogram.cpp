#include "dendrogram.h"
#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>


/*----------------------------------------------------------------------------*/

class DendrogramNode {
public:
  DendrogramNode(int32_t id);
  bool isRoot() const;
  bool isLeaf() const;
  int32_t id() const;
  static DendrogramNode * merge(int32_t id, DendrogramNode *l, DendrogramNode *r, float dist);
  const DendrogramNode* getL() const {return m_l;}
  const DendrogramNode* getR() const {return m_r;}
  float dist() const {return m_dist;}

private:
  DendrogramNode *m_p;
  DendrogramNode *m_l;
  DendrogramNode *m_r;
  float m_dist;
  int32_t m_id;
  uint32_t m_depth;

  DendrogramNode(int32_t id, DendrogramNode *l, DendrogramNode *r, float dist);
};

/*----------------------------------------------------------------------------*/

DendrogramNode::DendrogramNode(int32_t id) {
  m_id = id;
  m_p = NULL;
  m_r = NULL;
  m_l = NULL;
  m_dist = 0;
  m_depth = 0;
}

/*----------------------------------------------------------------------------*/

bool DendrogramNode::isRoot() const {
  return m_p == NULL;
}

/*----------------------------------------------------------------------------*/

bool DendrogramNode::isLeaf() const {
  return m_l == NULL; /* <-> m_r == NULL */
}

/*----------------------------------------------------------------------------*/

int32_t DendrogramNode::id() const {
  return m_id;
}

/*----------------------------------------------------------------------------*/

DendrogramNode * DendrogramNode::merge(int32_t id, DendrogramNode *l, DendrogramNode *r, float dist) {
  DendrogramNode *p = NULL;
  {
    /* which one is bigger? */
    DendrogramNode *huge = NULL;
    DendrogramNode *tiny = NULL;
    if (l->m_depth > r->m_depth) {
      huge = l;
      tiny = r;
    } else {
      huge = r;
      tiny = l;
    }
    /* does he want left or right side? */
    if (huge->m_dist > huge->m_dist) {
      p = new DendrogramNode(id, huge, tiny, dist);
    } else {
      p = new DendrogramNode(id, tiny, huge, dist);
    }
  }
  p->m_l->m_p = p;
  p->m_r->m_p = p;
  return p;
}

/*----------------------------------------------------------------------------*/

DendrogramNode::DendrogramNode(int32_t id, DendrogramNode *l, DendrogramNode *r, float dist) {
  m_r = r;
  m_l = l;
  m_p = NULL;
  m_dist = dist;
  m_id = id;
  m_depth = std::max(m_r->m_depth, m_l->m_depth) + 1;
}

/*----------------------------------------------------------------------------*/

DendrogramBuilder::DendrogramBuilder(uint32_t n) {
  m_n = n;
  m_nodes.assign(m_n * 2, NULL);
  for (uint32_t i = 0; i < m_n; i++) {
    m_nodes[i] = new DendrogramNode(i);
  }
  m_numPairs = 0;
}

/*----------------------------------------------------------------------------*/

DendrogramBuilder::~DendrogramBuilder() {
  for (uint32_t i = 0; i < m_nodes.size(); i++) {
    delete m_nodes[i];
  }
}

/*----------------------------------------------------------------------------*/

bool DendrogramBuilder::pair(uint32_t i, uint32_t j, float dist) {
  m_nodes[m_numPairs + m_n] = DendrogramNode::merge(m_numPairs + 1, m_nodes[i],
      m_nodes[j], dist);
  m_numPairs++;
  return true;
}

/*----------------------------------------------------------------------------*/

Dendrogram* DendrogramBuilder::getDend() {
  std::vector<DendrogramNode*> roots;
  for (uint32_t i = 0; i < m_nodes.size(); i++) {
    if (m_nodes[i] != NULL && m_nodes[i]->isRoot() == true) {
      roots.push_back(m_nodes[i]);
    }
  }
  return new Dendrogram(roots);
}

/*----------------------------------------------------------------------------*/

Dendrogram::Dendrogram(const std::vector<DendrogramNode*> &roots) {
  m_roots = DgNode::build(roots, &m_orderedNodes);
}

/*----------------------------------------------------------------------------*/

Dendrogram::~Dendrogram() {
  for (uint32_t i = 0; i < m_roots.size(); i++) {
    delete m_roots[i];
  }
}

/*----------------------------------------------------------------------------*/

std::vector<const DgNode*> Dendrogram::getRoots(uint32_t i0, uint32_t i1) const {
  std::vector<const DgNode*> res;
  for (uint32_t i = 0; i < m_roots.size(); i++) {
    m_roots[i]->getRoots(i0, i1, &res);
  }
  return res;
}

/*----------------------------------------------------------------------------*/

std::vector<DgNode*> DgNode::build(std::vector<DendrogramNode*> rtsin,
    std::vector<uint32_t> *order) {
  order->clear();
  /* compute max distance */
  float distmax = 0;
  for (uint32_t i = 0; i < rtsin.size(); i++) {
    distmax = std::max(distmax, rtsin[i]->dist());
  }
  /* build dendrograms */
  std::vector<DgNode*> rtsout;
  for (uint32_t i = 0; i < rtsin.size(); i++) {
    rtsout.push_back(new DgNode(rtsin[i], distmax));
    rtsout.back()->getOrder(order);
  }
  /* id -> idx */
  std::vector<uint32_t> id2idx(order->size());
  for (uint32_t i = 0; i < order->size(); i++) {
    id2idx[(*order)[i]] = i;
  }
  /* apply id2idx to compute h positions */
  for (uint32_t i = 0; i < rtsout.size(); i++) {
    rtsout[i]->setHPosMap(id2idx);
  }
  return rtsout;
}

/*----------------------------------------------------------------------------*/

DgNode::DgNode(const DendrogramNode* n, double maxdist) {
  m_id = n->id();
  if (!n->isLeaf()) {
    m_lo = new DgNode(n->getL(), maxdist);
    m_hi = new DgNode(n->getR(), maxdist);
    m_lvl = std::max(m_lo->m_lvl, m_hi->m_lvl) + 1;
    m_distAbs = n->dist();
    m_distRel = n->dist() / maxdist;
    m_distAbsMin = m_distAbs;
    m_distAbsMin = m_lo->isLeaf() ? m_distAbsMin : std::min(m_distAbsMin, m_lo->distAbsMin());
    m_distAbsMin = m_hi->isLeaf() ? m_distAbsMin : std::min(m_distAbsMin, m_hi->distAbsMin());
  } else {
    m_lvl = 0;
    m_distAbs = 0;
    m_distAbsMin = 0;
    m_distRel = 0;
    m_hi = NULL;
    m_lo = NULL;
  }
  m_idxHi = 0;
  m_idxLo = 0;
  m_hpos = 0;
}

/*----------------------------------------------------------------------------*/

DgNode::~DgNode() {
  delete m_lo;
  delete m_hi;
}

/*----------------------------------------------------------------------------*/

void DgNode::getOrder(std::vector<uint32_t> *o) {
  if (!m_lo) {
    assert(!m_hi);
    o->push_back(m_id);
    return;
  }
  m_lo->getOrder(o);
  m_hi->getOrder(o);
}

/*----------------------------------------------------------------------------*/

void DgNode::setHPosMap(const std::vector<uint32_t> &m) {
  if (!m_lo) {
    assert(!m_hi);
    m_hpos = double(m[m_id]) / m.size();
    m_idxLo = m[m_id];
    m_idxHi = m[m_id];
    return;
  }
  m_lo->setHPosMap(m);
  m_hi->setHPosMap(m);
  m_hpos = (m_lo->hPos() + m_hi->hPos()) / 2;
  m_idxLo = m_lo->m_idxLo;
  m_idxHi = m_hi->m_idxHi;
  assert(m_idxLo < m_idxHi);
}

/*----------------------------------------------------------------------------*/

void DgNode::getRoots(uint32_t i0, uint32_t i1, std::vector<const DgNode*> *res) const {
  /* full overlap */
  if (isInside(i0, i1)) {
    res->push_back(this);
    return;
  }
  /* cannot search deeper */
  if (!m_lo) {
    return;
  }
  /* both children have overlap */
  if (m_lo->hasOverlap(i0,i1) && m_hi->hasOverlap(i0,i1)) {
    res->push_back(this);
    return;
  }
  m_lo->getRoots(i0, i1, res);
  m_hi->getRoots(i0, i1, res);
}

/*----------------------------------------------------------------------------*/

void DgNode::print(uint32_t l, char s) const {
  if (!m_lo) {
    printf("%*c [%u]        %u h=%f,d=%f\n", l+3, s, m_idxLo, m_id, m_hpos, m_distRel);
    return;
  }
  m_lo->print(l+2, '/');
  printf("%*c (%u, %u)     h=%f,d=%f\n", l+3, s, m_idxLo, m_idxHi, m_hpos, m_distRel);
  m_hi->print(l+2, '\\');
}

/*----------------------------------------------------------------------------*/

bool DgNode::isInside(uint32_t i0, uint32_t i1) const {
  return i0 <= m_idxLo && m_idxHi <= i1;
}

/*----------------------------------------------------------------------------*/

bool DgNode::hasOverlap(uint32_t i0, uint32_t i1) const {
  bool ov = false;
  ov |= (i0 <= m_idxLo && m_idxLo <= i1);
  ov |= (i0 <= m_idxHi && m_idxHi <= i1);
  return ov;
}
