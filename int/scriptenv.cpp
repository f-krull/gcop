#include "scriptenv.h"
#include "../util/timer.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "cmd_base.cpp"

/*----------------------------------------------------------------------------*/

GcScriptEnv::GcScriptEnv() {
  m_os.addCmd(new CmdLoadSnp);
  m_os.addCmd(new CmdSnpInfo);
  m_os.addCmd(new CmdLoadSeg);
  m_os.addCmd(new CmdIntersectPS);
  m_os.addCmd(new CmdLoadLdInfo);
  m_os.addCmd(new CmdLdGet);
  m_os.addCmd(new CmdLdTest);
  m_os.addCmd(new CmdLoadGCords);
  m_os.addCmd(new CmdIntersectGc);
}

/*----------------------------------------------------------------------------*/

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
  printf("executing: %3u %s %s%s", line, cmd_str.c_str(),
      ce, strlen(ce)>0 && ce[strlen(ce)-1]=='\n' ? "" : "\n");
  GcCommand *cmd = m_os.getCmd(cmd_str.c_str());
  if (cmd == NULL) {
    fprintf(stderr, "error: command '%s' not found - line %u:\n", cmd_str.c_str(), line);
    exit(1);
  }
  Timer t;
  cmd->execute(ce, &m_os);
  t.stop();
  printf("took %.3f seconds\n", t.getSec());
}
