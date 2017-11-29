
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




/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "scriptenv.h"
int main(int argc, char **argv) {
  const char *fnscript = NULL;
  if (argc > 1) {
    fnscript = argv[1];
  }
  FILE *f = stdin;
  if (fnscript != NULL) {
    f = fopen(fnscript, "r");
  }
  if (f == NULL) {
    fprintf(stderr, "error: cannot open file '%s'\n", fnscript ? fnscript : "stdin");
    exit(1);
  }
  GcScriptEnv e;
  char buffer[1024];
  uint32_t line = 1;
  while (fgets(buffer, sizeof(buffer)-1, f) != NULL) {
    e.run(buffer, line);
    line++;
  }
  if (fnscript) {
    fclose(f);
  }

//  GCords g;
//  g.get(1).getFloat(1);


  return 0;
}

/*
 * a = load_snp file=fn format=1..2.3 skip=1
 * b = load_seg file=fn format=1..2.3 skip=1
 * b = intersect a b
 *
 *
 * */
