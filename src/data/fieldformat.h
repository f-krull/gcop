
#ifndef DATA_FIELDFORMAT_H_
#define DATA_FIELDFORMAT_H_

#include "fieldtypes.h"
#include <stdint.h>
#include <vector>

/*----------------------------------------------------------------------------*/

class FieldFormat {
public:
  FieldFormat(const char *fmt);
  ~FieldFormat();
  FieldFormat(const FieldFormat &o);
  bool hasField(FieldType ft) const;
  void printFields() const;
  struct Entry {
    FieldType type;
    uint32_t idx;
    Entry(FieldType t, uint32_t i = 0) : type(t), idx(i) {}
  };
  const Entry & get(uint32_t i) const { return m_fs[i];}
  const std::vector<FieldType> & types() const { return m_types; }
  uint32_t numEntries() const {return m_fs.size();}
  uint32_t numData() const {return m_numData;}
private:
  std::vector<Entry> m_fs;
  char* m_fmt;
  uint32_t m_numData;
  std::vector<FieldType> m_types;
};

/*----------------------------------------------------------------------------*/


#endif /* DATA_FIELDFORMAT_H_ */
