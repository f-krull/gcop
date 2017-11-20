
class GcObj {
public:


private:
};

class GcPoints : public GcObj {
public:


private:
};

#include <string>
#include <stdint.h>
class GcScriptInfo {
public:
  GcScriptInfo(const std::string &fn, uint32_t line) :
      m_filename(fn), m_line(line) {}

private:
  std::string m_filename;
  uint32_t    m_line;
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
  int32_t valInt() const;
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

/*----------------------------------------------------------------------------*/

int32_t GcCmdParam::valInt() const {
  return m_val_int;
}

class GcObjSpace;

class GcCommand {
public:
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
void GcCommand::execute(const char* str, GcObjSpace *os) {
  parseParams(str);
  executeChild(str, os);
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
    if (str[0] == '\0') {
      break;
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
    std::map<std::string, GcCmdParam>::iterator it = m_params.find(start); //NOTE: seems to seg fault if cmd not found
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

/*----------------------------------------------------------------------------*/

class GcObjSpace {
public:
  GcCommand* getCmd(const char *cmd_str);
  void addCmd(GcCommand *cmd);
  void addObj(const std::string &name, void *o);
  template <typename T>
  T *getObj(const char* name);
private:
  std::map<std::string, void*> m_obs;
  std::map<std::string, GcCommand*> m_cmds;
};

GcCommand* GcObjSpace::getCmd(const char *cmd_str) {
  std::map<std::string, GcCommand*>::iterator it = m_cmds.find(cmd_str);
  if (it == m_cmds.end()) {
    return NULL;
  }
  return it->second;
}

#include <typeinfo>

template <typename T>
T *GcObjSpace::getObj(const char* name) {
  std::map<std::string, void*>::iterator it = m_obs.find(name);
  if (it == m_obs.end()) {
    return NULL;
  }
  T *o = static_cast<T*>(it->second); //TODO: change to dyn cast
  if (o == NULL) {
    fprintf(stderr, "error: object '%s' is not of type %s\n", name, typeid(T).name());
    exit(1);
  }
  return o;
}

#include <assert.h>

void GcObjSpace::addCmd(GcCommand *cmd) {
  assert(getCmd(cmd->name()) == NULL && "command already loaded");
  m_cmds[cmd->name()] = cmd;
}

void GcObjSpace::addObj(const std::string &name, void *obj) {
  //TODO: assert(getCmd(cmd->name()) == NULL && "command already loaded");
  m_obs[name] = obj;
}

/*----------------------------------------------------------------------------*/

class CmdLoadSnp : public GcCommand {
public:
  CmdLoadSnp() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT_STR, GcCmdParam::PARAM_STRING, "cbe"));
    addParam(GcCmdParam(PARAM_SKIP_INT,   GcCmdParam::PARAM_INT,    "0"));
  }
  const char* name() const {
    return "load_snp";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
  static std::string PARAM_FORMAT_STR;
  static std::string PARAM_SKIP_INT;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadSnp::PARAM_DST_STR    = "dst";
std::string CmdLoadSnp::PARAM_FILE_STR   = "file";
std::string CmdLoadSnp::PARAM_FORMAT_STR = "format";
std::string CmdLoadSnp::PARAM_SKIP_INT   = "skip";



#include <stdio.h>
#include "snpdata.h"
void CmdLoadSnp::executeChild(const char *, GcObjSpace *os) {
  SnpData *snps = new SnpData();
  snps->readdyn(getParam(PARAM_FILE_STR)->valStr().c_str(),
                getParam(PARAM_FORMAT_STR)->valStr().c_str(),
                getParam(PARAM_SKIP_INT)->valInt());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), snps);
}

/*----------------------------------------------------------------------------*/

class CmdLoadSeg : public GcCommand {
public:
  CmdLoadSeg() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT_STR, GcCmdParam::PARAM_STRING, "cbe"));
    addParam(GcCmdParam(PARAM_SKIP_INT,   GcCmdParam::PARAM_INT,    "0"));
  }
  const char* name() const {
    return "load_seg";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
  static std::string PARAM_FORMAT_STR;
  static std::string PARAM_SKIP_INT;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadSeg::PARAM_DST_STR    = "dst";
std::string CmdLoadSeg::PARAM_FILE_STR   = "file";
std::string CmdLoadSeg::PARAM_FORMAT_STR = "format";
std::string CmdLoadSeg::PARAM_SKIP_INT   = "skip";



#include <stdio.h>
#include "segdata.h"
void CmdLoadSeg::executeChild(const char *, GcObjSpace *os) {
  SimpleSegData *seg = new SimpleSegData();
  seg->read(getParam(PARAM_FILE_STR)->valStr().c_str(),
            getParam(PARAM_FORMAT_STR)->valStr().c_str(),
            getParam(PARAM_SKIP_INT)->valInt());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), seg);
}

/*----------------------------------------------------------------------------*/

class CmdSnpInfo : public GcCommand {
public:
  CmdSnpInfo() {
    addParam(GcCmdParam(PARAM_SRC_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "snp_info";
  }
  static std::string PARAM_SRC_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdSnpInfo::PARAM_SRC_STR = "src";



#include <stdio.h>
#include "snpdata.h"
void CmdSnpInfo::executeChild(const char *, GcObjSpace *os) {
  const char *src = getParam(PARAM_SRC_STR)->valStr().c_str();
  SnpData *snps = os->getObj<SnpData>(src);
  printf("%s: number of snps %lu\n", src, snps->data().size());
}



/*----------------------------------------------------------------------------*/

class GcScriptEnv {
public:
  GcScriptEnv();
  void run(const char *s, uint32_t line);
private:
  GcObjSpace m_os;
};

#include <string.h>
#include <stdio.h>
#include <assert.h>

GcScriptEnv::GcScriptEnv() {
  m_os.addCmd(new CmdLoadSnp);
  m_os.addCmd(new CmdSnpInfo);
  m_os.addCmd(new CmdLoadSeg);
}

void GcScriptEnv::run(const char *s, uint32_t line) {
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
  cmd->execute(ce, &m_os);
}


int main(int argc, char **argv) {
  GcScriptEnv e;
  e.run("load_seg dst=b file=data/wgEncodeUwDnaseCd20ro01778PkRep1.narrowPeak format=cse skip=0", 1);
  e.run("load_snp dst=a file=data/scz.txt format=1...2...3 skip=1", 1);

  e.run("snp_info src=a", 2);
  return 0;
}

/*
 * a = load_snp file=fn format=1..2.3 skip=1
 * b = load_seg file=fn format=1..2.3 skip=1
 * b = intersect a b
 *
 *
 * */
