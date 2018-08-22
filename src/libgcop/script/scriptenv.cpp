#include "scriptenv.h"
#include "cmd_base.h"
#include "command.h"
#include "objs_base.h"
#include "../timer.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

GcScriptEnv::GcScriptEnv() {
  cmd_base_add(this);
  objs_base_add(this);
}

/*----------------------------------------------------------------------------*/

void GcScriptEnv::runFile(const char *fnscript) {
  FILE *f = stdin;
  if (fnscript != NULL) {
    f = fopen(fnscript, "r");
  }
  if (f == NULL) {
    fprintf(stderr, "error: cannot open file '%s'\n", fnscript ? fnscript : "stdin");
    exit(1);
  };
  runFile(f);
  if (fnscript) {
    fclose(f);
  }
}

/*----------------------------------------------------------------------------*/

void GcScriptEnv::runLine(const char *s, uint32_t linenum) {
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
    fprintf(stderr, "error: command not found - line %u:\n", linenum);
    fprintf(stderr, "%s\n", s);
    exit(1);
  }
  assert(ce != s);
  std::string cmd_str;
  cmd_str.assign(s, ce);
  printf("(%u) exec: %s %s%s", linenum, cmd_str.c_str(),
      ce, strlen(ce)>0 && ce[strlen(ce)-1]=='\n' ? "" : "\n");
  GcCommand *cmd = m_os.getCmd(cmd_str.c_str());
  if (cmd == NULL) {
    fprintf(stderr, "error: command '%s' not found - line %u:\n", cmd_str.c_str(), linenum);
    exit(1);
  }
  Timer t;
  cmd->execute(ce, &m_os);
  t.stop();
  printf("(%u) done - took %.3f seconds\n", linenum, t.getSec());
}

/*----------------------------------------------------------------------------*/

void GcScriptEnv::addCmd(GcCommand *cmd) {
  m_os.addCmd(cmd);
}

/*----------------------------------------------------------------------------*/

void GcScriptEnv::addObj(const char *name, GcObj *obj) {
  m_os.addObj(name, obj);
}

/*----------------------------------------------------------------------------*/

void GcScriptEnv::runFile(FILE *f) {
  const char line_cont_char = '\\';
  char buffer[1024];
  uint32_t len = 0;
  uint32_t line = 0;
  while (fgets(&buffer[len], sizeof(buffer) - len - 1, f) != NULL) {
    line++;
    len = strlen(buffer);
    /* skip new-line chars at end of buffer */
    while (len > 0 && (buffer[len] == '\n' || buffer[len] == '\0')) {
      len--;
    }
    /* do we see a line-continuation character? */
    if (len == 0 || (buffer[len] != line_cont_char)) {
      /* no - execute line */
      runLine(buffer, line);
      /* reset buffer start */
      len = 0;
      continue;
    }
    /* yes - skip '\' and append to buffer on next iteration */
    assert(len && buffer[len] == line_cont_char);
    len -= 1;
  }
}
