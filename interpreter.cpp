
class GcObj {
public:


private:
};

class GcPoints : public GcObj {
public:


private:
};



#include <map>
#include <string>
#include <stdint.h>
#include <stdlib.h>


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
  void toStr() const;

private:
  std::string   m_name;
  ParamType     m_type;

  std::string m_val_str;
  int32_t     m_val_int;
};

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



class GcCommand {
public:
  virtual ~GcCommand() {};
  virtual const char* name() const = 0;
  void execute(const char *str);
protected:
  void addParam(const GcCmdParam &p);
  void parseParams(const char* s);
  const GcCmdParam * getParam(const std::string &n) const;
  virtual void executeChild(const char *str) = 0;
  static const std::string STR_TYPE;
  static const std::string STR_DELIM;
  static const std::string STR_ASSIGN;

  std::map<std::string, GcCmdParam> m_params;
};

#include <assert.h>

const GcCmdParam * GcCommand::getParam(const std::string &n) const {
  std::map<std::string, GcCmdParam>::const_iterator it = m_params.find(n);
  if (it == m_params.end()) {
    assert(false);
    return NULL;
  }
  return &it->second;
}
#include <stdio.h>
void GcCommand::execute(const char* str) {
  parseParams(str);
  executeChild(str);
}

#include <string.h>
#include <stdio.h>

void GcCommand::parseParams(const char *s) {
  /* make a copy */
  char buffer[1024];
  buffer[0] = '\0';
  strncat(buffer, s, sizeof(buffer)-1);
  char *str = buffer;
  printf("line %d %s\n", __LINE__, str);
  while (str[0] != '\0') {
  printf("line %d %s\n", __LINE__, str);
    /* skip whitespace */
    while (str[0] == ' ') {
      str++;
    }
    /* find name (str before '=') */
    char *start = str;
    str = strchr(str, '=');
    if (str == NULL || str[0] != '=') {
      fprintf(stderr, "error: '=' not found\n");
      exit(1);
    }
    str[0] = '\0';
    str++;
    std::map<std::string, GcCmdParam>::iterator it = m_params.find(start);
    if (it == m_params.end()) {
      fprintf(stderr, "error: param '%s' not found\n", start);
      exit(1);
    }
    start = str;
    while (str[0] != ' ' && str[0] != '\n' && str[0] != '\0') {
      str++;
    }
    if (str[0] != '\0') {
      str[0] = '\0';
      str++;
    }
    printf("line %d %s\n", __LINE__, start);
    it->second.parse(start);
  }
}

void GcCommand::addParam(const GcCmdParam &p) {
  m_params.insert(std::make_pair(std::string(p.name()), p));
}

class GcObjSpace {
public:
  GcCommand* getCmd(const char *cmd_str);
  void addCmd(GcCommand *cmd);
private:
  std::map<std::string, GcObj*> m_obs;
  std::map<std::string, GcCommand*> m_cmds;
};

GcCommand* GcObjSpace::getCmd(const char *cmd_str) {
  std::map<std::string, GcCommand*>::iterator it = m_cmds.find(cmd_str);
  if (it == m_cmds.end()) {
    return NULL;
  }
  return it->second;
}

#include <assert.h>

void GcObjSpace::addCmd(GcCommand *cmd) {
  assert(getCmd(cmd->name()) == NULL && "command already loaded");
  m_cmds[cmd->name()] = cmd;
}

/*----------------------------------------------------------------------------*/

class CmdLoadSnp : public GcCommand {
public:
  CmdLoadSnp() {
    addParam(GcCmdParam("dst",    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam("file",   GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam("format", GcCmdParam::PARAM_STRING, "cbe"));
    addParam(GcCmdParam("skip",   GcCmdParam::PARAM_INT,    "0"));
  }
  const char* name() const {
    return "load_snp";
  }
  static std::string PARAM_DST_STR;
protected:
  void executeChild(const char *);
};

std::string CmdLoadSnp::PARAM_DST_STR = "dst";

#include <stdio.h>

void CmdLoadSnp::executeChild(const char *) {
  printf("hi: %s\n", getParam(PARAM_DST_STR)->valStr().c_str());
}

/*----------------------------------------------------------------------------*/

class GcExpression {
public:
  void parse(const char *s, uint32_t line);
private:
  GcObjSpace m_os;
};

#include <string.h>
#include <stdio.h>
#include <assert.h>

void GcExpression::parse(const char *s, uint32_t line) {
  m_os.addCmd(new CmdLoadSnp);
  /* skip spaces */
  while (s[0] == ' ' || s[0] == '\t' || s[0] == '\n') {
    s++;
  }
  /* ignore an empty line */
  if (s[0] == '\0') {
    return;
  }
  /* ignore a comment line */
  if (s[0] == '#') {
    return;
  }
  /* get cmd */
  const char *ce = strchr(s, ' ');
  if (ce == NULL) {
    ce = strchr(s, '\n');
  }
  if (ce == NULL) {
    fprintf(stderr, "error: command not found - line %u:\n", line);
    fprintf(stderr, "%s\n", s);
    exit(1);
  }
  assert(ce != s);
  std::string cmd_str;
  cmd_str.assign(s, ce);
  GcCommand *cmd = m_os.getCmd(cmd_str.c_str());
  printf("executing: param=%s\n", ce);
  cmd->execute(ce);
}


int main(int argc, char **argv) {
  GcExpression e;
  e.parse("load_snp dst=a file=fn format=1..2.3 skip=1", 1);
  return 0;
}

/*
 * a = load_snp file=fn format=1..2.3 skip=1
 * b = load_regions file=fn format=1..2.3 skip=1
 * b = intersect a b
 *
 *
 * */
