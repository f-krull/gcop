
#ifndef DATA_LDINFO_H_
#define DATA_LDINFO_H_

#include "chrdef.h"
#include <map>


class LdDataList;
class SnpData;

/*----------------------------------------------------------------------------*/

class LdInfo {
public:
  ~LdInfo();
  void read(const char *fn);
  float getLd(const char* c, uint64_t bpa, uint64_t bpb) const;
  float getLd(ChrMap::ChrType ct, uint64_t bpa, uint64_t bpb) const;

  void test(const SnpData *s) const;

private:
  typedef std::map<ChrMap::ChrType, LdDataList*> LdlMap;
  LdlMap m_lddat;
  LdDataList* find(ChrMap::ChrType c);
  const LdDataList* find(ChrMap::ChrType c) const;
  void addEntry(uint64_t chr, uint64_t bpa, uint64_t bpb, float r2, const ChrMap &cm);
};


#endif /* DATA_LDINFO_H_ */
