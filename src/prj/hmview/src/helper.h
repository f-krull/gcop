#ifndef SRC_HELPER_CPP_
#define SRC_HELPER_CPP_

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

#endif /* SRC_HELPER_CPP_ */
