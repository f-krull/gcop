
#ifndef INT_CMDPARAM_H_
#define INT_CMDPARAM_H_



#include <map>
#include <string>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class GcCmdParam {
public:
#define ENUM_PARAMTYPE(select_fun) \
  select_fun(PARAM_STRING, "String") \
  select_fun(PARAM_INT,    "Integer")

  enum ParamType {
#define ENUM_GET_ENAME(name, str) name,
          ENUM_PARAMTYPE(ENUM_GET_ENAME)
                PARAM_NUMENTRIES
#undef ENUM_GET_ENAME
  };

  static const char* ParamStr[];

  GcCmdParam(const std::string &name, ParamType type, std::string def);
  bool parse(const char *str);
  const std::string & name() const;
  const ParamType & type() const;
  const std::string & valStr() const;
  int32_t valInt() const;
  void toStr() const;

private:
  std::string   m_name;
  ParamType     m_type;

  std::string m_val_str;
  int32_t     m_val_int;
};



#endif /* INT_CMDPARAM_H_ */
