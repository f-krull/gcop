
#ifndef DATA_CHRINFO_H_
#define DATA_CHRINFO_H_

#include <stdint.h>

class ChrInfoPriv;

/*----------------------------------------------------------------------------*/

class ChrInfo {
public:
  ChrInfo();
  virtual ~ChrInfo();
  typedef uint8_t CType; /* chromosome id */

  void read(const char *fn);
  uint64_t chrlen(CType t) const;
  CType str2type(const char *str) const; /* str with "chr" prefix */
  const char *ctype2str(CType t) const;
  void print() const;

  static CType CTYPE_UNDEFINED;
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


#endif /* DATA_CHRINFO_H_ */
