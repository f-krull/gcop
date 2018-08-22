#include "../libgcop/tokenreader.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/*----------------------------------------------------------------------------*/

char* TokenReader::read_uint64(char *pos, char delim, uint64_t *v) {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*v) = atol(s);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_float(char *pos, char delim, float *v) {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*v) = atof(s);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_string(char *pos, char delim, char **str) {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*str) = strdup(s);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_forget(char *pos, char delim) {
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos++;
  return pos;
}

/*----------------------------------------------------------------------------*/

char* TokenReader::read_conv(char *pos, char delim, IConv *c) {
  const char *s = pos;
  assert(pos != NULL);
  pos = strchr(pos, delim);
  pos[0] = '\0';
  (*c)(s);
  pos++;
  return pos;
}
