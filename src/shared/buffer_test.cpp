#include "buffer.h"
#include <string.h>

int main(int argc, char **argv) {
  char str[] = "sure.";
  BufferDyn in((uint8_t*)str, sizeof(str));
  printf("org         : %s\n", str);
  printf("buffer      : %s\n", in.cdata());
  BufferDyn a(in.cdata(), in.len());
  printf("buffer cpy  : %s\n", a.cdata());
  a.toAsci();
  printf("buffer ascii: %s\n", a.cdata());
  BufferDyn x(in.data(), in.len()-1);
  x.printinfo();
  x.toHexstr();
  printf("buffer hex  : %s\n", x.cdata());
  BufferDyn b64(in.data(), in.len()-1);
  b64.toBase64();
  printf("buffer b64  : %s\n", b64.cdata());
}

