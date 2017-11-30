#include "intervaltree.h"
#include <stdio.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/

template <typename T>
IntervalTree<T>::~IntervalTree() {
  delete m_root;
}

/*----------------------------------------------------------------------------*/
template <typename T>
IntervalTree<T>::ItNode::ItNode(const std::vector<T> &is) {
  m_s = NULL;
  m_g = NULL;
  assert(!is.empty());
  /* get center of is */
  m_center = T::center(is.begin(), is.end());
  /* determine categories of is */
  std::vector<T> ismaller;
  std::vector<T> igreater;
  std::vector<T> ioverlap;
  for (uint32_t i = 0; i < is.size(); i++) {
    if (T::smaller(is[i], m_center)) {
      ismaller.push_back(is[i]);
      continue;
    }
    if (T::greater(is[i], m_center)) {
      igreater.push_back(is[i]);
      continue;
    }
    ioverlap.push_back(is[i]);
  }
  /* create children */
  if (!ismaller.empty()) {
    //printf("s: %lu\n", ismaller.size());
    m_s = new ItNode(ismaller);
  }
  if (!igreater.empty()) {
    //printf("g: %lu\n", igreater.size());
    m_g = new ItNode(igreater);
  }
  /* sort overlapping points */
  m_bs.assign(ioverlap.begin(), ioverlap.end());
  m_es.assign(ioverlap.begin(), ioverlap.end());
  std::sort(m_bs.begin(), m_bs.end(), Interval::cmpStart);
  std::sort(m_es.begin(), m_es.end(), Interval::cmpEnd);
}

/*----------------------------------------------------------------------------*/
template <typename T>
bool IntervalTree<T>::ItNode::overlaps(uint64_t p) const {
  if (p < m_center && m_s != NULL && m_s->overlaps(p)) {
    return true;
  }
  if (p > m_center && m_g != NULL && m_g->overlaps(p)) {
    return true;
  }
  if (p <= m_center && !m_bs.empty() && m_bs.front().s <= p) {
    return true;
  }
  if (p >  m_center && !m_es.empty() && m_es.back().e > p) {
    return true;
  }
  return false;
}

/*----------------------------------------------------------------------------*/
template <typename T>
void IntervalTree<T>::ItNode::print(uint32_t lvl, const char *s) const {
  if (m_s) {
    m_s->print(lvl+1, "/");
  }
  printf("%*s%s %lu:", lvl, "", s, m_center);
  for (uint32_t i = 0; i < m_bs.size(); i++) {
    printf("(%lu,%lu)", m_bs[i].s, m_bs[i].e);
  }
  printf("\n");
  if (m_g) {
    m_g->print(lvl+1, "\\");
  }
}

/*----------------------------------------------------------------------------*/

template <typename T>
uint32_t IntervalTree<T>::ItNode::countNodes() const {
  return 1 + (m_g ? m_g->countNodes() : 0) + (m_s ? m_s->countNodes() : 0);
}

/*----------------------------------------------------------------------------*/
template <typename T>
IntervalTree<T>::ItNode::~ItNode() {
  delete m_s;
  delete m_g;
}

/*----------------------------------------------------------------------------*/
template <typename T>
IntervalTree<T>::IntervalTree(const std::vector<T> &is) {
  m_numNodes = 0;
  m_root = NULL;
  if (!is.empty()) {
    m_root = new ItNode(is);
  }
  m_numNodes = m_root->countNodes();
}

/*----------------------------------------------------------------------------*/
template <typename T>
bool IntervalTree<T>::overlaps(uint64_t p) const {
  return m_root != NULL ? m_root->overlaps(p) : false;
}

/*----------------------------------------------------------------------------*/
template <typename T>
void IntervalTree<T>::print() const {
  if (m_root) {
    m_root->print(0, "-");
  }
}

/*----------------------------------------------------------------------------*/

