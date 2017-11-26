
#ifndef SEGANNOT_H_
#define SEGANNOT_H_

#include "segdata.h"
#include "snpdata.h"

/*----------------------------------------------------------------------------*/

class ISegAnnot {
public:
  virtual ~ISegAnnot() {};
  virtual std::vector<char> intersect(const ISegData &seg, const SnpData &snp) = 0;
private:
};

/*----------------------------------------------------------------------------*/

class SegAnnotSimple : public ISegAnnot {
public:

  std::vector<char> intersect(const ISegData &seg, const SnpData &snp);
private:
};


/*----------------------------------------------------------------------------*/


class SegAnnot : public ISegAnnot {
public:

  std::vector<char> intersect(const ISegData &seg, const SnpData &snp);
private:
};



class Forbes {
public:
  float get(const ISegData &seg, const SnpData &snp);
};


#endif /* SEGANNOT_H_ */
