#ifndef INT_OBJS_DECL_H_
#define INT_OBJS_DECL_H_


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
      void replace(_class *n) {delete m_c; m_c = n;} \
  private: \
      _class* m_c; \
  };


#endif /* INT_OBJS_DECL_H_ */
