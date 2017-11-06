
#ifndef SNPDATA_H_
#define SNPDATA_H_

#include "chrdef.h"
#include <stdint.h>
#include <vector>

/*----------------------------------------------------------------------------*/

struct Snp {
  ChrMap::ChrType chr;
  uint64_t bp;
  float pval;
};

class SnpData {
public:
  bool read(const char *filename, uint32_t skip);

  const std::vector<Snp> & data() const { return m_d;}
private:
  std::vector<Snp> m_d;
};




#endif /* SNPDATA_H_ */
