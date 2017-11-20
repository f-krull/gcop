
#ifndef INT_SCRIPTENV_H_
#define INT_SCRIPTENV_H_

#include "objspace.h"
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class GcScriptEnv {
public:
  GcScriptEnv();
  void run(const char *s, uint32_t line);
private:
  GcObjSpace m_os;
};

#endif /* INT_SCRIPTENV_H_ */
