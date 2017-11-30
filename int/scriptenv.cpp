#include "scriptenv.h"
#include "cmd_base.h"
#include "command.h"
#include "../util/timer.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

GcScriptEnv::GcScriptEnv() {
  cmd_base_add(this);
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

/*----------------------------------------------------------------------------*/

void GcScriptEnv::addCmd(GcCommand *cmd) {
  m_os.addCmd(cmd);
}

/*----------------------------------------------------------------------------*/

void GcScriptEnv::run(FILE *f) {
  char buffer[1024];
  uint32_t line = 1;
  while (fgets(buffer, sizeof(buffer) - 1, f) != NULL) {
    run(buffer, line);
    line++;
  }
}

#if 0

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


class GcObjSpace;



/*
 * a = load_snp file=fn format=1..2.3 skip=1
 * b = load_seg file=fn format=1..2.3 skip=1
 * b = intersect a b
 *
 *
 * */
#endif

