#include <script/scriptenv.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
  const char *fnscript = NULL;
   if (argc > 1) {
     fnscript = argv[1];
   }
   GcScriptEnv e;
   e.runFile(fnscript);
   return(0);
}
