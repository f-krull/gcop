
#ifndef TOKENREADER_H_
#define TOKENREADER_H_

#include <stdint.h>

/*----------------------------------------------------------------------------*/


class TokenReader {
public:
  class IConv {
  public:
    ~IConv() {};
    virtual void operator()(const char*) {};
  };

  static char* read_uint64(char *pos, char delim, uint64_t *v);
  static char* read_float(char *pos, char delim, float *v);
  static char* read_string(char *pos, char delim, char **str);
  static char* read_forget(char *pos, char delim);
  static char* read_conv(char *pos, char delim, IConv*);
private:
};


#endif /* TOKENREADER_H_ */
