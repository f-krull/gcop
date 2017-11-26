#include <stdint.h>
class MatEntryList;
class MatEntryReader;

/*----------------------------------------------------------------------------*/

class SparseMat {
public:
  SparseMat(const MatEntryList &mel);
  SparseMat(MatEntryReader &mer);
  SparseMat(const char *fn);
  ~SparseMat();
  void reset();

  uint32_t n() const;
  uint8_t get(uint32_t i, uint32_t j) const;
  
  void write(const char* fn) const;
  void read(const char* fn);

  void printSize() const;
  void print() const;
  void printMat() const;
private:
  uint32_t m_n;
  uint32_t m_numVal;
  uint8_t*  m_values;
  uint32_t* m_colIdx;
  uint32_t* m_rowPtr;
};
