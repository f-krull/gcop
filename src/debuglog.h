#ifndef DEBUGLOG_H_
#define DEBUGLOG_H_

#include <stdlib.h>
#include <stdio.h>

/*----------------------------------------------------------------------------*/

class Log {
public:
  Log(const char *prefix = NULL, ...);
  ~Log();
  void setPrefix(const char *pfx = NULL, ...);
  void dbg(const char* msg, ...);
  void err(const char* msg, ...);
private:
  void setPrefix(const char *fmt, va_list args);
  char m_buffer[4048];
  char *m_prefix;
  void rmPfx();
};

#endif /* DEBUGLOG_H_ */
