#include "../data/gcords.h"
#include "../data/intervaltree.h"
#include "assert.h"

static bool is_flat(const GCords &g) {
  for (auto chr_it = g.begin(); chr_it != g.end(); ++chr_it) {
    if (chr_it->empty()) {
      continue;
    }
    IntervalTree<GCord> itree(*chr_it);
    for (auto gc_it = chr_it->begin(); gc_it != chr_it->end(); ++gc_it) {
      std::vector<uint32_t> res;
      itree.overlapsInterval(*gc_it, &res);
      if (res.size() > 1) {
        return false;
      }
    }
  }
  return true;
}

int main(int argc, char **argv) {
  GCords g;
  ChrInfoHg19 hg19;
  g.read("test/flatten_data.txt", "cse", 1, &hg19);

  printf("\n");
  g.write(stdout);
  g.flatten();
  assert(is_flat(g));
  printf("flattened\n");
  g.write(stdout);
  printf("\n");

  const uint32_t nexp = 2;
  printf("expand: %u\n", nexp);
  g.expand(nexp);
  g.write(stdout);
  printf("flattened\n");
  g.flatten();
  g.write(stdout);
  assert(is_flat(g));


  printf("expand: %u\n", nexp);
  g.expand(nexp);
  g.write(stdout);
  g.flatten();
  printf("flattened\n");
  g.write(stdout);
  assert(is_flat(g));

  g.read("./testdata/gwascat/Body_mass_index.txt.gz", "...........cs", 0, &hg19);

  g.flatten();
  printf("flat:\n");
  g.write(stdout, 10);

  g.expand(25000);
  printf("exp:\n");
  g.write(stdout, 10);

  g.flatten();
  printf("flat:\n");
  g.write(stdout, 10);
  //is_flat(g);
  //assert(is_flat(g));

  return 0;
}
