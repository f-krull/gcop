
#ifndef DATA_LDINFO_H_
#define DATA_LDINFO_H_

#include "chrinfo.h"
#include <stdlib.h>
#include <map>


class LdDataList;
class GCords;

/*----------------------------------------------------------------------------*/

class LdInfo {
public:
  ~LdInfo();
  void read(const char *fn, const ChrInfo *ci = NULL);
  float getLd(const char* c, uint64_t bpa, uint64_t bpb) const;
  float getLd(ChrInfo::CType ct, uint64_t bpa, uint64_t bpb) const;
  void test(const GCords *s) const;

private:
  typedef std::map<ChrInfo::CType, LdDataList*> LdlMap;
  LdlMap m_lddat;
  LdDataList* find(ChrInfo::CType c);
  const LdDataList* find(ChrInfo::CType c) const;
  void addEntry(uint64_t chr, uint64_t bpa, uint64_t bpb, float r2, const ChrInfo *ci);
  ChrInfo m_ci;
};


#endif /* DATA_LDINFO_H_ */
