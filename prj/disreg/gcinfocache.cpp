#include "../../data/gcords.h"
#include "../../data/chrinfo.h"
#include "../../data/intervaltree.h"

/*----------------------------------------------------------------------------*/

class LengthsSum {
public:
  LengthsSum(const std::vector<GCord> &g, ChrInfo::CType c) {
    m_sum = 0;
    for (uint32_t i = 0; i < g.size(); i++) {
      m_sum += g[i].len();
    }
  }
  uint64_t get() const {
    return m_sum;
  }
private:
  uint64_t m_sum;
};

/*----------------------------------------------------------------------------*/

class GCordsChrInfo {
public:
  GCordsChrInfo(const GCords * gc, ChrInfo::CType c) : m_gc(gc), m_chr(c) {
    m_it  = NULL;
    m_len = NULL;
    m_g = m_gc->getChr(m_chr);
  }

  ~GCordsChrInfo() {
    delete m_len;
    delete m_it;
  }

  const std::vector<GCord> & vgcords() const {
    return m_g;
  }

  const LengthsSum * lengthssum() {
    if (m_len == NULL) {
      m_len = new LengthsSum(m_g, m_chr);
    }
    return m_len;
  }

  const IntervalTree<GCord> * it() {
    if (m_it == NULL) {
      m_it = new IntervalTree<GCord>(m_g);
    }
    return m_it;
  }

private:
  IntervalTree<GCord> *m_it;
  LengthsSum *m_len;
  std::vector<GCord> m_g;
  const GCords *m_gc;
  const ChrInfo::CType m_chr;
};

/*----------------------------------------------------------------------------*/

class GCordsInfoCache {
public:
  GCordsInfoCache(const GCords *gc) : m_gc(gc) {
  }

  ~GCordsInfoCache() {
    std::map<ChrInfo::CType, GCordsChrInfo*>::iterator it;
    for (it = m_ginf.begin(); it != m_ginf.end(); ++it) {
      delete it->second;
    }
  }

  GCordsInfoCache( const GCordsInfoCache & o);
  const GCordsInfoCache & operator=(const GCordsInfoCache & o);

  GCordsChrInfo * chr(ChrInfo::CType c) {
    std::map<ChrInfo::CType, GCordsChrInfo*>::iterator it = m_ginf.find(c);
    /* exists? */
    if (it != m_ginf.end()) {
      return it->second;
    }
    /* create */
    GCordsChrInfo *gi = new GCordsChrInfo(m_gc, c);
    m_ginf.insert(std::make_pair(c, gi));
    return gi;
  }

  const GCords * gcords() const  {
    return m_gc;
  }

private:
  const GCords *m_gc;
  std::map<ChrInfo::CType, GCordsChrInfo*> m_ginf;
};
