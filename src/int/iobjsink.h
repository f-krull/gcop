
#ifndef INT_IOBJSINK_H_
#define INT_IOBJSINK_H_

class GcObj;

/*----------------------------------------------------------------------------*/

class IObjSink {
public:
  virtual ~IObjSink() {};
  virtual void addObj(const char *name, GcObj *obj) = 0;
private:
};

#endif /* INT_IOBJSINK_H_ */
