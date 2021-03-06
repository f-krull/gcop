
#ifndef DATA_CHRINFO_H_
#define DATA_CHRINFO_H_

#include <stdint.h>
#include <stdlib.h>
#include <vector>

class ChrInfoPriv;

/*----------------------------------------------------------------------------*/

class ChrInfo {
public:
  typedef uint8_t CType; /* chromosome id */
  static CType CTYPE_UNDEFINED;

  ChrInfo();
  ChrInfo(const ChrInfo &c);
  virtual ~ChrInfo();
  ChrInfo & operator=(const ChrInfo &o);
  bool operator==(const ChrInfo &o) const;

  uint32_t numchrs() const;
  void read(const char *fn);
  uint64_t chrlen(CType t) const;
  uint64_t len() const;
  CType str2type(const char *str) const;       /* str with or without "chr" pfx */
  CType str2typeStrict(const char *str) const; /* str with "chr" prefix */
  const std::vector<CType> & chrs() const;
  const char *ctype2str(CType t) const;
  void print() const;
protected:
  void addEntry(const char* str, uint64_t len);
private:
  ChrInfoPriv * m;
};

/*----------------------------------------------------------------------------*/

class ChrInfoHg19 : public ChrInfo {
public:
  ChrInfoHg19();
};

/*----------------------------------------------------------------------------*/

class ChrInfoUnknown : public ChrInfo {
public:
  ChrInfoUnknown();
};


#endif /* DATA_CHRINFO_H_ */
