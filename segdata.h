
#ifndef SEGDATA_H_
#define SEGDATA_H_

#include "segment.h"
#include <vector>

/*----------------------------------------------------------------------------*/

class ISegData {
public:
  virtual ~ISegData() {};
  virtual bool read(const char* filename) = 0;
  const std::vector<Segment> & data() const {return m_d;}
  std::vector<Segment> data() {return m_d;}
  std::vector<Segment> getChr(ChrMap::ChrType) const;
protected:
  std::vector<Segment> m_d;
};

/*----------------------------------------------------------------------------*/

class SimpleSegData : public ISegData {
public:
  virtual ~SimpleSegData() {};
  bool read(const char *filename);
private:
};

#endif /* SEGDATA_H_ */

