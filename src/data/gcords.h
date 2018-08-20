
#ifndef DATA_GCORDS_H_
#define DATA_GCORDS_H_

#include "interval.h"
#include "chrinfo.h"
#include "tabfield.h"
#include <stdio.h>

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
  GCords() : m_ncols(0) {}
  virtual ~GCords() {};
  bool read(const char *filename, const char *format, uint32_t skip, const ChrInfo *ci, bool allowUndefChr);
  bool write(FILE *f, uint64_t maxlines = 0) const;
  void clear() {m_d.resize(0);}
  uint32_t ncols() const { return m_ncols; }
  const std::vector<GCord> & getChr(ChrInfo::CType) const;
  void expand(uint64_t len);
  void flatten();
  void toPoints();
  typedef std::vector<GCord> Chr;
  std::vector<Chr>::iterator begin() {return m_d.begin();}
  std::vector<Chr>::const_iterator begin() const {return m_d.begin();}
  std::vector<Chr>::iterator end() {return m_d.end();}
  std::vector<Chr>::const_iterator end() const {return m_d.end();}

  static void forbes(const GCords* gca, const GCords* gcb);
  const ChrInfo & chrinfo() const {return m_ci;}
  uint64_t numgc() const;


protected:
  std::vector<Chr> m_d;
  void rebuild();
  ChrInfo m_ci;
  uint32_t m_ncols;
};


#endif /* DATA_GCORDS_H_ */
