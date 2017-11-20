#include "scriptenv.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "cmd_base.cpp"

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
  printf("executing: %s %s\n", cmd_str.c_str(), ce);
  cmd->execute(ce, &m_os);
}

