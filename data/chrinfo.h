
#ifndef DATA_CHRINFO_H_
#define DATA_CHRINFO_H_

#include <stdint.h>

class ChrInfoPriv;

/*----------------------------------------------------------------------------*/

class ChrInfo {
public:
  ChrInfo();
  ~ChrInfo();
  typedef uint8_t CType; /* chromosome id */

  void read(const char *fn);
  uint64_t getLen(CType t) const;
  CType str2type(const char *str) const; /* str with    "chr" prefix */
  CType id2type(const char *str) const;  /* str without "chr" prefix */
private:
  ChrInfoPriv * m;
};



#endif /* DATA_CHRINFO_H_ */
