#include "dendrogram.h"
#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

typedef DendrogramNode DeNode;

/*----------------------------------------------------------------------------*/

DeNode::DendrogramNode(int32_t id) {
  m_isLeaf = true;
  m_id = id;
  m_p = NULL;
  m_r = NULL;
  m_l = NULL;
  m_dist = 0;
  m_depth = 0;
  m_minDist = 0;
}

/*----------------------------------------------------------------------------*/

bool DeNode::isRoot() const {
  return m_p == NULL;
}

/*----------------------------------------------------------------------------*/

bool DeNode::isLeaf() const {
  return m_isLeaf;
}

/*----------------------------------------------------------------------------*/

int32_t DeNode::id() const {
  return m_id;
}

/*----------------------------------------------------------------------------*/

DeNode * DeNode::merge(int32_t id, DeNode *l, DeNode *r, float dist) {
  DeNode *p = NULL;
  {
    /* which one is bigger? */
    DeNode *huge = NULL;
    DeNode *tiny = NULL;
    if (l->m_depth > r->m_depth) {
      huge = l;
      tiny = r;
    } else {
      huge = r;
      tiny = l;
    }
    /* does he want left or right side? */
    if (huge->m_dist > huge->m_dist) {
      p = new DeNode(id, huge, tiny, dist);
    } else {
      p = new DeNode(id, tiny, huge, dist);
    }
  }
  p->m_l->m_p = p;
  p->m_r->m_p = p;
  return p;
}

/*----------------------------------------------------------------------------*/

std::vector<uint32_t> DeNode::geGlobalLeafIds(const DeNode *root) {
  std::vector<uint32_t> leafIds;
  root->getGlobalLeafIds(leafIds);
  return leafIds;
}

/*----------------------------------------------------------------------------*/

DeNode::DendrogramNode(int32_t id, DeNode *l, DeNode *r, float dist) {
  m_r = r;
  m_l = l;
  m_isLeaf = false;
  m_p = NULL;
  m_dist = dist;
  m_minDist = std::min(m_r->m_minDist, m_l->m_minDist);
  m_id = id;
  m_depth = std::max(m_r->m_depth, m_l->m_depth) + 1;
}

/*----------------------------------------------------------------------------*/

void DeNode::getGlobalLeafIds(std::vector<uint32_t> &leafIds) const {
  if (!m_isLeaf) {
    /* show small distances first */
    if (m_l->m_dist < m_r->m_dist) {
      m_l->getGlobalLeafIds(leafIds);
      m_r->getGlobalLeafIds(leafIds);
    } else {
      m_r->getGlobalLeafIds(leafIds);
      m_l->getGlobalLeafIds(leafIds);
    }
    return;
  }
  const int32_t globalId = abs(m_id) - 1;
  assert(globalId >= 0);
  leafIds.push_back(globalId);
}

/*----------------------------------------------------------------------------*/

Dendrogram::Dendrogram(uint32_t n) {
  m_n = n;
  m_dict.resize(m_n * 2);
  for (uint32_t i = 0; i < m_dict.size(); i++) {
    m_dict[i] = i;
  }
  m_nodes.assign(m_n * 2, NULL);
  for (uint32_t i = 0; i < m_n; i++) {
    m_nodes[i] = new DeNode(int32_t(i + 1) * -1);
  }
  m_numPairs = 0;
}

/*----------------------------------------------------------------------------*/

Dendrogram::~Dendrogram() {
  for (uint32_t i = 0; i < m_nodes.size(); i++) {
    delete m_nodes[i];
  }
}

/*----------------------------------------------------------------------------*/

bool Dendrogram::pair(uint32_t i, uint32_t j, float dist) {
  m_dict[m_numPairs + m_n] = m_dict[i];
  m_nodes[m_numPairs + m_n] = DeNode::merge(m_numPairs + 1, m_nodes[i],
      m_nodes[j], dist);
  m_numPairs++;
  return true;
}

/*----------------------------------------------------------------------------*/

std::vector<uint32_t> Dendrogram::getOrder() const {
  std::vector<uint32_t> order;
  for (uint32_t i = 0; i < m_nodes.size(); i++) {
    if (m_nodes[i] != NULL && m_nodes[i]->isRoot() == true) {
      std::vector<uint32_t> ls = DeNode::geGlobalLeafIds(m_nodes[i]);
      order.insert(order.end(), ls.begin(), ls.end());
    }
  }
  return order;
}
