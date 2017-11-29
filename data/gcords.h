
#ifndef DATA_GCORDS_H_
#define DATA_GCORDS_H_

#include "interval.h"
#include "chrdef.h"

/*----------------------------------------------------------------------------*/

class GCord : public Interval {
public:
  ChrMap::ChrType chr;
  GCord() : Interval(0,0), chr(ChrMap::CHRTYPE_NUMENTRIES) {}
private:
};

/*----------------------------------------------------------------------------*/

#include <vector>
class GCordsPriv;

class GCords {
public:
  class RowPtr : public GCord {
  public:
    RowPtr(const GCord &g, GCords *p, uint64_t idx);
    float getFloat(uint32_t i);
  private:
    GCords *m_p;
    uint64_t m_ridx;
  };

  GCords();
  virtual ~GCords();
  bool read(const char *filename, const char *format, uint32_t skip);
  const std::vector<GCord> & data() const {return m_d;}
  std::vector<GCord> data() {return m_d;}
  std::vector<GCord> getChr(ChrMap::ChrType) const;

  RowPtr get(uint64_t i);
  float annotFloat(uint32_t annotIdx, float rowIdx) const;

  static void intersect(const GCords* gca, const GCords* gcb);

protected:
  std::vector<GCord> m_d;
  GCordsPriv *m;
};


#endif /* DATA_GCORDS_H_ */
