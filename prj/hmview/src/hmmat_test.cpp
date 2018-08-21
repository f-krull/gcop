#include "hmmat.h"

int main(int argc, char **argv) {
  HmMat hm;
  hm.read("/tmp/disreg_matrix.txt");
  hm.print();
}
