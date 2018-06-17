
#ifndef SRC_HAPDOSEREADER_H_
#define SRC_HAPDOSEREADER_H_

#include "hapindex.h"
#include <stdint.h>
#include <vector>
#include <array>
#include <string>

/*----------------------------------------------------------------------------*/

class HapDoseReaderPriv;

/*----------------------------------------------------------------------------*/

class HapDoseReader {
public:
  HapDoseReader();
  ~HapDoseReader();

  struct SampleInfo {
    std::string id;
    std::string familyid;
  };

  bool open(const char *fn, uint32_t numVar);
  const std::vector<SampleInfo> sampleInfo() const;
  const std::vector<std::array<float, HAPLINDEX_NUMENRIES>> & nextVar();
  int32_t currVarIdx() const;
  void close();

private:
  HapDoseReaderPriv *m;
  bool allocVarBuf(uint32_t numVar, uint32_t numSamples);
};


#endif /* SRC_HAPDOSEREADER_H_ */
