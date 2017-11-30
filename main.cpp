#include "int/scriptenv.h"
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

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
   e.run(f);
   if (fnscript) {
     fclose(f);
   }
}
