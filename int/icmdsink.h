
#ifndef INT_ICMDSINK_H_
#define INT_ICMDSINK_H_

class GcCommand;

/*----------------------------------------------------------------------------*/

class ICmdSink {
public:
  virtual ~ICmdSink() {};
  virtual void addCmd(GcCommand *cmd) = 0;
private:
};


#endif /* INT_ICMDSINK_H_ */
