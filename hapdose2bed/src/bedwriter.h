
#ifndef SRC_BEDWRITER_H_
#define SRC_BEDWRITER_H_

#include "hapindex.h"
#include <vector>
#include <stdio.h>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class BedWriter {
public:
  BedWriter();
  ~BedWriter();

  void setDelta(float f) { m_delta = f; }
  float getDelta() { return m_delta; }
  bool open(const char*, uint64_t numVar);
  bool write(uint64_t varIdx, uint32_t sampIdx, double d1, double d2);
  bool close();

private:

  FILE *m_f;
  std::vector<uint64_t> m_numStat;
  std::vector<std::vector<uint8_t>>  m_outBuf;
  float m_delta;
};

#endif /* SRC_BEDWRITER_H_ */
