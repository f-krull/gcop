
#ifndef TOKENREADER_H_
#define TOKENREADER_H_

#include "chrdef.h"
#include <stdint.h>

class TokenReader {
public:
  char* read_chr(char *pos, char delim, ChrMap::ChrType *v) const;
  char* read_uint64(char *pos, char delim, uint64_t *v) const;
  char* read_float(char *pos, char delim, float *v) const;
  char* read_forget(char *pos, char delim) const;
private:
  ChrMap m_cm;
};


#endif /* TOKENREADER_H_ */
