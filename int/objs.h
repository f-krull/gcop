
#ifndef INT_OBJS_H_
#define INT_OBJS_H_

/*----------------------------------------------------------------------------*/

class GcObjPriv;

class GcObj {
public:
  GcObj(const char *name);
  virtual ~GcObj();
  const char * name() const;
private:
  GcObjPriv *m;
};

/*----------------------------------------------------------------------------*/

#define OBJS_DECL_GCCLASS(name, _class) \
  class GcObj##name : public GcObj { \
  public: \
      GcObj##name() : GcObj("##name") { m_c = new _class; } \
      ~GcObj##name() { delete m_c; } \
      _class* d() { return m_c; } \
      const _class* d() const { return m_c; } \
  private: \
      _class* m_c; \
  };

#include "../data/ldinfo.h"
OBJS_DECL_GCCLASS(LdInfo, LdInfo)

#include "../data/gcords.h"
OBJS_DECL_GCCLASS(GCords, GCords)

#include "../data/chrinfo.h"
OBJS_DECL_GCCLASS(ChrInfo, ChrInfo)


#undef OBJS_DECL_GCCLASS


#endif /* INT_OBJS_H_ */
