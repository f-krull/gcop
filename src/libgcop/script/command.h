
#ifndef INT_COMMAND_H_
#define INT_COMMAND_H_

#include "cmdparam.h"
#include <string>
#include <map>
class GcObjSpace;

/*----------------------------------------------------------------------------*/

class GcCommand {
public:
  GcCommand() {};
  virtual ~GcCommand() {};
  virtual const char* name() const = 0;
  void execute(const char *str, GcObjSpace *os);
protected:
  void addParam(const GcCmdParam &p);
  void parseParams(const char* s);
  const GcCmdParam * getParam(const std::string &n) const;
  virtual void executeChild(const char *str, GcObjSpace *os) = 0;
  static const std::string STR_TYPE;
  static const std::string STR_DELIM;
  static const std::string STR_ASSIGN;

  std::map<std::string, GcCmdParam> m_params;
private:
  GcCommand(const GcCommand &o);
};



#endif /* INT_COMMAND_H_ */
