
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

class SnpData;

class GcObjSnpData : public GcObj {
public:
  GcObjSnpData();
  ~GcObjSnpData();
  SnpData* data();
  const SnpData* data() const;
private:
  SnpData* m_snp;
};

/*----------------------------------------------------------------------------*/

class ISegData;

class GcObjSegData : public GcObj {
public:
  GcObjSegData();
  ~GcObjSegData();
  ISegData* data();
  const ISegData* data() const;
private:
  ISegData* m_seg;
};


/*----------------------------------------------------------------------------*/

class LdInfo;

class GcObjLdInfo : public GcObj {
public:
  GcObjLdInfo();
  ~GcObjLdInfo();
  LdInfo* data();
  const LdInfo* data() const;
private:
  LdInfo *m_ldi;
};

/*----------------------------------------------------------------------------*/

#define OBJS_DECL_GCCLASS(name) \
  class Gc##name : public GcObj { \
  public: \
      Gc##name() : GcObj("##name") {} \
  };


#endif /* INT_OBJS_H_ */
