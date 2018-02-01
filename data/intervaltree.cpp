#include "intervaltree.h"
#include <stdio.h>
#include <assert.h>


//#define DEBUG

#ifdef DEBUG
#define DEBUG_OUT(...) fprintf(stdout, __VA_ARGS__)
#else
void dummyout(...) {}
#define DEBUG_OUT(...) dummyout(__VA_ARGS__)
#endif





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
bool IntervalTree<T>::IndexedInterval::findSmallerPoints(
    const std::vector<IndexedInterval> &bs, uint64_t b,
    std::vector<uint32_t> *res) {
  bool ret = false;
  typename std::vector<IndexedInterval>::const_iterator it;
  for (it = bs.begin(); it != bs.end(); ++it) {
    /* use all smaller starts */
    if (it->iv.s > b) {
      break;
    }
    ret = true;
    if (res) {
      /* mark result */
      (*res).push_back(it->srcIdx);
    }
  }
  return ret;
}


/*----------------------------------------------------------------------------*/

template<typename T>
bool IntervalTree<T>::IndexedInterval::findGreaterPoints(
    const std::vector<IndexedInterval> &es, uint64_t b,
    std::vector<uint32_t> *res) {
  bool ret = false;
  typename std::vector<IndexedInterval>::const_reverse_iterator it;
  for (it = es.rbegin(); it != es.rend(); ++it) {
    /* use all ends that are bigger
     *
     * query: 3
     * data: [1,2] [1,3] [1,4] [1,5]
     *        no    no    yes   yes
     *
     * */
    if (it->iv.e <= b) {
      break;
    }
    ret = true;
    if (res) {
      /* mark result */
      (*res).push_back(it->srcIdx);
    }
  }
  return ret;
}

/*----------------------------------------------------------------------------*/

template<typename T>
bool IntervalTree<T>::IndexedInterval::findRightOv(
    const std::vector<IndexedInterval> &bs, T i,
    std::vector<uint32_t> *res) {
  /*
   * query i:  s---------e
   *      bs:   s------------e
   *              s--------e
   *
   *  any bs-s in [i-s,i-e]?
   *
   */
  /* find lower bound endpoints */
  typename std::vector<IndexedInterval>::const_iterator beg =
      std::lower_bound(bs.begin(), bs.end(), i.s, IndexedInterval::CmpStart());
  /* find upper bound endpoints */
  typename std::vector<IndexedInterval>::const_iterator end =
      std::upper_bound(bs.begin(), bs.end(), i.e > 0 ? i.e - 1 : 0, IndexedInterval::CmpStart());
  for (; res != NULL && beg != end; ++beg) {
    res->push_back(beg->srcIdx);
  }
  return beg != end;
}

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
    m_s = new ItNode(ismaller);
  }
  if (!igreater.empty()) {
    m_g = new ItNode(igreater);
  }
  /* sort overlapping points */
  m_bs.assign(ioverlap.begin(), ioverlap.end());
  m_es.assign(ioverlap.begin(), ioverlap.end());
  std::sort(m_bs.begin(), m_bs.end(), typename IndexedInterval::CmpStart());
  std::sort(m_es.begin(), m_es.end(), typename IndexedInterval::CmpEnd());
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
  if (p >  m_center && !m_es.empty() && m_es.back().iv.e > p) { //NOTE:
    return true;
  }
  return false;
}

/*----------------------------------------------------------------------------*/

template <typename T>
bool IntervalTree<T>::ItNode::getOverlaps(uint64_t p, std::vector<uint32_t> *res, uint32_t depth) const {
  bool ret = false;
  DEBUG_OUT("%*s (%lu)\n", depth, "", m_center);
  /* point is outside of node */
  if (p < m_center && m_s != NULL) {
    DEBUG_OUT("%*s s\n", depth, "");
    ret = m_s->getOverlaps(p, res, depth+1) || ret;
  } else
  /* point is outside of node */
  if (p > m_center && m_g != NULL) {
    DEBUG_OUT("%*s g\n", depth, "");
    ret = m_g->getOverlaps(p, res, depth+1) || ret;
  }
  /* check for overlap with node - their bs must be smaller */
  if (p <= m_center && !m_bs.empty()) {
    DEBUG_OUT("%*s bs\n", depth, "");
    ret = IndexedInterval::findSmallerPoints(m_bs, p, res) || ret;
    for (uint32_t i = 0; i < m_bs.size(); i++) {
      DEBUG_OUT("%*s %lu,%lu\n", depth, "", m_bs[i].iv.s, m_bs[i].iv.e);
    }
    return ret;
  }
  /* check for overlap with node - es must be equal or greater  */
  if (p >  m_center && !m_es.empty()) {
    DEBUG_OUT("%*s es p(%lu) > center(%lu) : \n", depth, "", p, m_center);
    ret = IndexedInterval::findGreaterPoints(m_es, p, res) || ret;
    return ret;
  }
  return ret;
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
    m_numNodes = m_root->countNodes();
  }
  m_numIntervals = is.size();
  m_bs = IndexedInterval::build(is);
  m_es = IndexedInterval::build(is);
  std::sort(m_bs.begin(), m_bs.end(), typename IndexedInterval::CmpStart());
  std::sort(m_es.begin(), m_es.end(), typename IndexedInterval::CmpEnd());
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
bool IntervalTree<T>::overlapsInterval(const T &i, std::vector<uint32_t> *res) const {
  bool ret = false;
  if (res) {
    res->clear();
  }
  if (m_root != NULL) {
    ret = m_root->getOverlaps(i.s, res, 0) || ret;
  }
  /* skip this step, if result list isn't required overlap already found */
  if (!res) {
    ret = ret || IndexedInterval::findRightOv(m_bs, i, res);
  } else {
    ret = IndexedInterval::findRightOv(m_bs, i, res) || ret;
  }
  //IndexedInterval::findLeftOv(m_bs, i, &res);
  return ret;
}

/*----------------------------------------------------------------------------*/


template <typename T>
bool IntervalTree<T>::overlapsInterval_(const T &i, std::vector<char> *res) const {
  assert(false && "broken function");
  return false;
}

/*----------------------------------------------------------------------------*/
template <typename T>
bool IntervalTree<T>::overlapsPoint(uint64_t p, std::vector<uint32_t> *res) const {
  T t;
  t.s = p;
  t.e = p+1;
  const bool ret = overlapsInterval(t, res);
#ifdef DEBUG
  //assert(ret == overlaps(p));
#endif
  return ret;
}
