#include <vector>
#include <stdint.h>
#include <algorithm>
#include <stdio.h>

static bool cmpSmaller(const std::pair<double, uint32_t>& a, const std::pair<
    double, uint32_t>& b) {
   return a.first < b.first;
}

#if 0
/* small values get small ranks. (0.5, 0.2, 0.4) -> (3, 1, 2) */
static std::vector<uint32_t> rank1(const std::vector<double> &c) {
  std::vector<uint32_t> order(c.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(),
        [&c](uint32_t i1, uint32_t i2) {return c[i1] < c[i2];});
   return order;
}
#endif

static std::vector<uint32_t> rank2(const std::vector<double> &c) {
   std::vector<uint32_t> rank;
   rank.reserve(c.size());

   std::vector<std::pair<double, uint32_t> > tmp;
   tmp.resize(c.size());
   for (uint32_t i = 0; i < c.size(); ++i) {
      tmp[i] = (std::pair<double, uint32_t>(c[i], i));
   }
   std::sort(tmp.begin(), tmp.end(), cmpSmaller);

   std::vector<std::pair<double, uint32_t>> tmp2;
   tmp2.resize(c.size());
   for (uint32_t i = 0; i < c.size(); ++i) {
      tmp2[i] = (std::pair<double, uint32_t>(tmp[i].second, i));
   }
   std::sort(tmp2.begin(), tmp2.end(), cmpSmaller);

   for (uint32_t i = 0; i < c.size(); ++i) {
      rank[i] = tmp2[i].second + 1;
   }
   return rank;
}

int main(int argc, char **argv) {
  std::vector<double> d;
  d.push_back(0.5);
  d.push_back(0.2);
  d.push_back(0.4);
  std::vector<uint32_t> r = rank2(d);
  for (uint32_t i = 0; i < d.size(); i++) {
    printf("%u %lf\n", r[i], d[i]);
  }

}
