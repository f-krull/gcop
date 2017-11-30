
#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "chrdef.h"
#include "interval.h"
#include <vector>

/*----------------------------------------------------------------------------*/

class Segment : public Interval {
public:
  ChrMap::ChrType chr;
  Segment() : Interval(0,0), chr(ChrMap::CHRTYPE_NUMENTRIES) {}
};




#endif /* SEGMENT_H_ */
