
#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "chrdef.h"
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class Segment {
public:
  ChrMap::ChrType chr;
  uint64_t bps;
  uint64_t bpe;
};




#endif /* SEGMENT_H_ */
