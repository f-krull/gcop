#include "objs_base.h"
#include "objs.h"

void objs_base_add(IObjSink *s) {
  GcObjChrInfo *ci = new GcObjChrInfo();
  ci->replace(new ChrInfoHg19);
  s->addObj(OBJ_CHRINFO_HG19, ci);
}
