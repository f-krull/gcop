
#ifndef SRC_GETTOKEN_H_
#define SRC_GETTOKEN_H_


#include <string.h>

/*----------------------------------------------------------------------------*/

/* null-terminate token + point to \0 if nothing is found */
static char *gettoken(char *s, char d) {
  char *end = strchrnul(s, d);
  if (end[0] == d) {
    end[0] = '\0';
    end++;
  }
  return end;
}

/*----------------------------------------------------------------------------*/


#endif /* SRC_GETTOKEN_H_ */
