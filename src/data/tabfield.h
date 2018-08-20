
#ifndef DATA_TABFIELD_H_
#define DATA_TABFIELD_H_

#include "fieldtypes.h"
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class FieldValue {
public:

  FieldValue(FieldType t);
  FieldValue(const FieldValue &o);
  ~FieldValue();

  union U {
    uint64_t uint;
    float    flt;
    char    *str;
  };
  U & d() { return m_u;}
  const U & d() const { return m_u;}
  FieldType type() const {return m_type;}

private:
  FieldType m_type;
  U m_u;
};




#endif /* DATA_TABFIELD_H_ */
