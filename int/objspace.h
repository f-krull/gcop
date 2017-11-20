
#ifndef INT_OBJSPACE_H_
#define INT_OBJSPACE_H_

#include <string>
#include <map>

class GcCommand;
class GcObj;

/*----------------------------------------------------------------------------*/

class GcObjSpace {
public:
  GcCommand* getCmd(const char *cmd_str);
  void addCmd(GcCommand *cmd);
  void addObj(const std::string &name, GcObj *o);
  template <typename T>
  T *getObj(const char* name);
private:
  std::map<std::string, GcObj*> m_obs;
  std::map<std::string, GcCommand*> m_cmds;
};


/*----------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>

template <typename T>
T *GcObjSpace::getObj(const char* name) {
  std::map<std::string, GcObj*>::iterator it = m_obs.find(name);
  if (it == m_obs.end()) {
    return NULL;
  }
  T *o = dynamic_cast<T*>(it->second);
  if (o == NULL) {
    fprintf(stderr, "error: object '%s' is not of type %s\n", name, typeid(T).name());
    exit(1);
  }
  return o;
}


#endif /* INT_OBJSPACE_H_ */
