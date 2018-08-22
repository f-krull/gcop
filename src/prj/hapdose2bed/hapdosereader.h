
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

  bool nextSample();
  const std::vector<float> & getHapDose1() const;
  const std::vector<float> & getHapDose2() const;
  const SampleInfo & getSampleInfo() const;
  void close();

private:
  HapDoseReaderPriv *m;
  bool allocVarBuf(uint32_t numVar, uint32_t numSamples);
};


#endif /* SRC_HAPDOSEREADER_H_ */
