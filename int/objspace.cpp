#include "objspace.h"
#include "objs.h"
#include "command.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

GcCommand* GcObjSpace::getCmd(const char *cmd_str) {
  std::map<std::string, GcCommand*>::iterator it = m_cmds.find(cmd_str);
  if (it == m_cmds.end()) {
    return NULL;
  }
  return it->second;
}


/*----------------------------------------------------------------------------*/

void GcObjSpace::addCmd(GcCommand *cmd) {
  assert(getCmd(cmd->name()) == NULL && "command already loaded");
  m_cmds[cmd->name()] = cmd;
}

/*----------------------------------------------------------------------------*/

void GcObjSpace::addObj(const std::string &name, GcObj *obj) {
  //TODO: assert(getCmd(cmd->name()) == NULL && "command already loaded");
  m_obs[name] = obj;
}

/*----------------------------------------------------------------------------*/


void GcObjSpace::err(const char* name, const char* type) {
  fprintf(stderr, "error: object '%s' is not of type %s\n", name, type);
  exit(1);
}

