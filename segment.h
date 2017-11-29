
#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "data/chrdef.h"
#include "data/interval.h"
#include <stdint.h>
#include <algorithm>
#include <vector>

/*----------------------------------------------------------------------------*/

class Segment : public Interval {
public:
  ChrMap::ChrType chr;
  Segment() : Interval(0,0), chr(ChrMap::CHRTYPE_NUMENTRIES) {}
};




#endif /* SEGMENT_H_ */
