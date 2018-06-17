
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
  bool open(const char*);
  bool write(HaplIndexType h, double d);
  bool closeVariant();
  void close();

private:

  FILE *m_f;
  std::vector<std::vector<float>> m_sampleBuf;
  uint8_t *m_outBuf;
  uint32_t m_outBufLen;
  std::vector<uint64_t> m_num;
  float m_delta;
};

#endif /* SRC_BEDWRITER_H_ */
