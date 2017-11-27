#include "command.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

const GcCmdParam * GcCommand::getParam(const std::string &n) const {
  std::map<std::string, GcCmdParam>::const_iterator it = m_params.find(n);
  if (it == m_params.end()) {
    assert(false);
    return NULL;
  }
  return &it->second;
}

/*----------------------------------------------------------------------------*/


void GcCommand::execute(const char* str, GcObjSpace *os) {
  parseParams(str);
  executeChild(str, os);
}

/*----------------------------------------------------------------------------*/

void GcCommand::parseParams(const char *s) {
  /* make a copy */
  char buffer[1024];
  buffer[0] = '\0';
  strncat(buffer, s, sizeof(buffer)-1);
  char *str = buffer;
  while (str[0] != '\0') {
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
    it->second.parse(start);
  }
}

/*----------------------------------------------------------------------------*/

void GcCommand::addParam(const GcCmdParam &p) {
  m_params.insert(std::make_pair(std::string(p.name()), p));
}
