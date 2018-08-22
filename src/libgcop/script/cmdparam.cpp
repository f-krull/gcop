#include "cmdparam.h"
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

GcCmdParam::GcCmdParam(const std::string &name, ParamType type,
    std::string def) {
  m_name = name;
  m_type = type;
  parse(def.c_str());
}

/*----------------------------------------------------------------------------*/

bool GcCmdParam::parse(const char *str) {
  m_val_str = str;
  switch (m_type) {
    case PARAM_INT: {
        char *end;
        m_val_int = strtol(str, &end, 10);
        if (end == NULL || end[0] != '\0') {
          return false;
        }
      }
      break;
    default:
      break;
  }
  return true;
}

/*----------------------------------------------------------------------------*/

const std::string & GcCmdParam::name() const {
  return m_name;
}

/*----------------------------------------------------------------------------*/

const GcCmdParam::ParamType & GcCmdParam::type() const {
  return m_type;
}

/*----------------------------------------------------------------------------*/

const std::string & GcCmdParam::valStr() const {
  return m_val_str;
}

/*----------------------------------------------------------------------------*/

int32_t GcCmdParam::valInt() const {
  return m_val_int;
}
