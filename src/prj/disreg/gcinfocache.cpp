#include <gcords.h>
#include <chrinfo.h>
#include <intervaltree.h>


class LengthsSumChr {
public:
  LengthsSumChr(const std::vector<GCord> &g, ChrInfo::CType) {
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

  const LengthsSumChr * lengthssum() {
    if (m_len == NULL) {
      m_len = new LengthsSumChr(m_g, m_chr);
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
  LengthsSumChr *m_len;
  std::vector<GCord> m_g;
  const GCords *m_gc;
  const ChrInfo::CType m_chr;
};

/*----------------------------------------------------------------------------*/

class LengthsSum;

/*----------------------------------------------------------------------------*/

class GCordsInfoCache {
public:
  GCordsInfoCache(const GCords *gc) : m_gc(gc) {
    m_len = NULL;
  }

  ~GCordsInfoCache();

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


  const LengthsSum *len();

private:
  const GCords *m_gc;
  LengthsSum *m_len;
  std::map<ChrInfo::CType, GCordsChrInfo*> m_ginf;
};

class LengthsSum {
public:
  LengthsSum(GCordsInfoCache &gic) {
    m_sum = 0;
    for (uint32_t i = 0; i < gic.gcords()->chrinfo().chrs().size(); i++) {
      m_sum += gic.chr(i)->lengthssum()->get();
    }
  }
  uint64_t get() const {
    return m_sum;
  }
private:
  uint64_t m_sum;
};

GCordsInfoCache::~GCordsInfoCache() {
  std::map<ChrInfo::CType, GCordsChrInfo*>::iterator it;
  for (it = m_ginf.begin(); it != m_ginf.end(); ++it) {
    delete it->second;
  }
  delete m_len;
}

const LengthsSum *GCordsInfoCache::len() {
  if (m_len == NULL) {
    m_len = new LengthsSum(*this);
  }
  return m_len;
}

