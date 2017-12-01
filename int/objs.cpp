#include "objs.h"
#include <string>

class GcObjPriv {
public:
  std::string name;
private:
};

/*----------------------------------------------------------------------------*/

GcObj::GcObj(const char *name) {
  m = new GcObjPriv;
  m->name = name;
}

GcObj::~GcObj() {
  delete m;
}

const char * GcObj::name() const {
  return m->name.c_str();
}
