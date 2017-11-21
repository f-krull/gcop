#include "objspace.h"
#include "objs.h"
#include "command.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

GcObjSpace::~GcObjSpace() {
  for (GcObjMap::iterator it = m_obs.begin(); it != m_obs.end(); ++it) {
    delete it->second;
  }
  for (GcCmdMap::iterator it = m_cmds.begin(); it != m_cmds.end(); ++it) {
    delete it->second;
  }
}

/*----------------------------------------------------------------------------*/

GcCommand* GcObjSpace::getCmd(const char *cmd_str) {
  GcCmdMap::iterator it = m_cmds.find(cmd_str);
  if (it == m_cmds.end()) {
    return NULL;
  }
  return it->second;
}


/*----------------------------------------------------------------------------*/

void GcObjSpace::addCmd(GcCommand *cmd) {
  if (getCmd(cmd->name()) != NULL) {
    fprintf(stderr, "error: command '%s' already defined\n", cmd->name());
    exit(1);
  }
  m_cmds[cmd->name()] = cmd;
}

/*----------------------------------------------------------------------------*/

void GcObjSpace::addObj(const std::string &name, GcObj *obj) {
  if (m_obs.find(name) != m_obs.end()) {
    rmObj(name);
  }
  m_obs[name] = obj;
}

/*----------------------------------------------------------------------------*/

void GcObjSpace::rmObj(const std::string &name) {
  GcObjMap::iterator it = m_obs.find(name);
  if (it == m_obs.end()) {
    fprintf(stderr, "error: command '%s' not found\n", name.c_str());
    exit(1);
  }
  delete it->second;
  m_obs.erase(it);
}

/*----------------------------------------------------------------------------*/


void GcObjSpace::err(const char* name, const char* type) {
  fprintf(stderr, "error: object '%s' is not of type %s\n", name, type);
  exit(1);
}

