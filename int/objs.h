
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



#endif /* INT_OBJS_H_ */
