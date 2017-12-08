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
std::vector<typename IntervalTree<T>::IndexedInterval> IntervalTree<T>::IndexedInterval::build(const std::vector<T> &in) {
  std::vector<IntervalTree<T>::IndexedInterval> indexed;
  for (uint32_t i = 0; i < in.size(); i++) {
    indexed.push_back(IndexedInterval(in[i], i));
  }
  return indexed;
}

/*----------------------------------------------------------------------------*/

template <typename T>
typename IntervalTree<T>::ItNode* IntervalTree<T>::ItNode::build_tree(const std::vector<T> &is) {
  std::vector<IndexedInterval> is_indexed = IndexedInterval::build(is);
  return new ItNode(is_indexed);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void IntervalTree<T>::IndexedInterval::findSmallerPoints(
    const std::vector<IndexedInterval> &bs, uint64_t b,
    std::vector<char> *res) {
  typename std::vector<IndexedInterval>::const_iterator it;
  for (it = bs.begin(); it != bs.end(); ++it) {
    /* use all smaller starts */
    if (it->iv.s > b) {
      break;
    }
    /* mark result */
    (*res)[it->srcIdx] = true;
  }
}


/*----------------------------------------------------------------------------*/

template<typename T>
void IntervalTree<T>::IndexedInterval::findGreaterPoints(
    const std::vector<IndexedInterval> &es, uint64_t b,
    std::vector<char> *res) {
  typename std::vector<IndexedInterval>::const_iterator it;
  for (it = es.begin(); it != es.end(); ++it) {
    /* use all greater or equal ends starts */
    if (it->iv.e <= b) {
      break;
    }
    /* mark result */
    (*res)[it->srcIdx] = true;
  }
}

/*----------------------------------------------------------------------------*/

//template<typename T>
//void IntervalTree<T>::IndexedInterval::findLeftOv(
//    const std::vector<IndexedInterval> &es, T i,
//    std::vector<char> *res) {
//  typename std::vector<IndexedInterval>::const_iterator it;
//  /* find lower bound endpoints */
//  /* find upper bound endpoints */
//  std::lower_bound(es.begin(), es.end());
//
//  /* check for start point */
//}

/*----------------------------------------------------------------------------*/
template <typename T>
IntervalTree<T>::ItNode::ItNode(const std::vector<IndexedInterval> &is) {
  m_s = NULL;
  m_g = NULL;
  assert(!is.empty());
  /* get center of is */
  {
    m_center = 0;
    const uint64_t count = std::distance(is.begin(), is.end());
    typename std::vector<IndexedInterval>::const_iterator b = is.begin();
    typename std::vector<IndexedInterval>::const_iterator e = is.end();
    for (; b != e; ++b) {
      m_center += (b->iv.e + b->iv.s) / 2;
    }
    m_center = m_center / count;
  }

  /* determine categories of is */
  std::vector<IndexedInterval> ismaller;
  std::vector<IndexedInterval> igreater;
  std::vector<IndexedInterval> ioverlap;
  for (uint32_t i = 0; i < is.size(); i++) {
    if (T::smaller(is[i].iv, m_center)) {
      ismaller.push_back(is[i]);
      continue;
    }
    if (T::greater(is[i].iv, m_center)) {
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
  std::sort(m_bs.begin(), m_bs.end(), IndexedInterval::cmpStart);
  std::sort(m_es.begin(), m_es.end(), IndexedInterval::cmpEnd);
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
  if (p <= m_center && !m_bs.empty() && m_bs.front().iv.s <= p) {
    return true;
  }
  if (p >  m_center && !m_es.empty() && m_es.back().iv.e > p) {
    return true;
  }
  return false;
}

/*----------------------------------------------------------------------------*/

template <typename T>
void IntervalTree<T>::ItNode::getOverlaps(uint64_t p, std::vector<char> *res, uint32_t depth) const {
  printf("%*s (%lu)\n", depth, "", m_center);
  /* point is outside of node */
  if (p < m_center && m_s != NULL) {
    printf("%*s s\n", depth, "");
    m_s->getOverlaps(p, res, depth+1);
    //return;
  } else
  /* point is outside of node */
  if (p > m_center && m_g != NULL) {
    printf("%*s g\n", depth, "");
    m_g->getOverlaps(p, res, depth+1);
    //return;
  }
  /* check for overlap with node - bs must be smaller */
  if (p <= m_center && !m_bs.empty()) {
    printf("%*s bs\n", depth, "");
    IndexedInterval::findSmallerPoints(m_bs, p, res);
    return;
  }
  /* check for overlap with node - es must be equal or greater  */
  if (p >  m_center && !m_es.empty()) {
    printf("%*s es\n", depth, "");
    IndexedInterval::findGreaterPoints(m_es, p, res);
    return;
  }
}

/*----------------------------------------------------------------------------*/
template <typename T>
void IntervalTree<T>::ItNode::print(uint32_t lvl, const char *s) const {
  if (m_s) {
    m_s->print(lvl+1, "/");
  }
  printf("%*s%s %lu:", lvl, "", s, m_center);
  for (uint32_t i = 0; i < m_bs.size(); i++) {
    printf("(%lu,%lu)", m_bs[i].iv.s, m_bs[i].iv.e);
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
    m_root = ItNode::build_tree(is);
  }
  m_numNodes = m_root->countNodes();
  m_numIntervals = is.size();

  m_bs = IndexedInterval::build(is);
  m_es = IndexedInterval::build(is);
  std::sort(m_bs.begin(), m_bs.end(), IndexedInterval::cmpStart);
  std::sort(m_es.begin(), m_es.end(), IndexedInterval::cmpEnd);
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
template <typename T>
std::vector<char> IntervalTree<T>::getOverlaps(const T &i) const {
  std::vector<char> res(m_numIntervals, false);
  if (m_root != NULL) {
    m_root->getOverlaps(i.s, &res, 0);
  }
  return res;
  //IndexedInterval::findLeftOv(m_bs, i, res);
  //IndexedInterval::findRightOv(m_bs, i, res);
}

/*----------------------------------------------------------------------------*/
template <typename T>
std::vector<char> IntervalTree<T>::getOverlaps(uint64_t p) const {
  std::vector<char> res(m_numIntervals, false);
  if (m_root != NULL) {
    m_root->getOverlaps(p, &res, 0);
  }
  return res;
}
