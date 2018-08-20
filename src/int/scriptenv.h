
#ifndef INT_SCRIPTENV_H_
#define INT_SCRIPTENV_H_

#include "objspace.h"
#include "icmdsink.h"
#include "iobjsink.h"
#include <stdint.h>
#include <stdio.h>

/*----------------------------------------------------------------------------*/

class GcScriptEnv : public ICmdSink, public IObjSink {
public:
  GcScriptEnv();
  void runFile(const char * filename = NULL); /* fn=NULL for stdin */
  void addCmd(GcCommand *cmd);
  void addObj(const char *name, GcObj *obj);
private:
  void runLine(const char *s, uint32_t line);
  GcObjSpace m_os;
  void runFile(FILE* f);
};

#endif /* INT_SCRIPTENV_H_ */
