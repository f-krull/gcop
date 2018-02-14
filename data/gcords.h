
#ifndef DATA_GCORDS_H_
#define DATA_GCORDS_H_

#include "interval.h"
#include "chrinfo.h"
#include "tabfield.h"

/*----------------------------------------------------------------------------*/

class GCord : public Interval {
public:
  ChrInfo::CType chr;
  GCord(std::vector<FieldType> fts = std::vector<FieldType>());
  FieldValue & d(uint32_t i) {return m_d[i];}
  const FieldValue & d(uint32_t i) const {return m_d[i];}
  uint32_t nCols() const { return m_d.size(); }
  float & flt(uint32_t i) { return m_d[i].d().flt; }
  const float & flt(uint32_t i) const { return m_d[i].d().flt; }
private:
  std::vector<FieldValue> m_d;
};

/*----------------------------------------------------------------------------*/

#include <vector>
#include <map>
class GCordsPriv;

class GCords {
public:
  virtual ~GCords() {};
  bool read(const char *filename, const char *format, uint32_t skip, const ChrInfo *ci);
  void clear() {m_d.resize(0);}
  uint32_t ncols() const { return m_d.empty() ? 0 : m_d.back().nCols(); }
  const std::vector<GCord> & cdata() const {return m_d;}
  std::vector<GCord> & data() {return m_d;}
  std::vector<GCord> getChr(ChrInfo::CType) const;
  void expand(uint64_t len);

  static void intersect(const GCords* gca, const GCords* gcb, GCords* gci);
  static void forbes(const GCords* gca, const GCords* gcb);
  const ChrInfo & chrinfo() const {return m_ci;}

protected:
  std::vector<GCord> m_d;
  ChrInfo m_ci;
};


#endif /* DATA_GCORDS_H_ */
