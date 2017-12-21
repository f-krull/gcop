#include "tokenreader.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

char* TokenReader::read_chr(char *pos, char delim, ChrMap::ChrType *v) const {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*v) = m_cm.unifyChr(s);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_uint64(char *pos, char delim, uint64_t *v) const {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*v) = atol(s);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_float(char *pos, char delim, float *v) const {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*v) = atof(s);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_string(char *pos, char delim, char **str) const {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*str) = strdup(s);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_forget(char *pos, char delim) const {
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos++;
  return pos;
}
