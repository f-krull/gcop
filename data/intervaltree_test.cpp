#include "intervaltree.cpp"
#include "interval.h"
#include <set>

/*----------------------------------------------------------------------------*/
static void test_interval_end() {
  std::vector<Interval> is;
  is.push_back(Interval(1,3));
  IntervalTree<Interval> it(is);
  assert(it.overlaps(3) == false);
  assert(it.overlapsPoint(3) == false);
  std::vector<uint32_t> res;
  assert(it.overlapsPoint(3, &res) == false);
}
/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
  test_interval_end();
  std::vector<Interval> is;
  is.push_back(Interval( 5, 10));
  is.push_back(Interval(15, 25));
  is.push_back(Interval( 1, 12));
  is.push_back(Interval( 8, 16));
  is.push_back(Interval(14, 20));
  is.push_back(Interval(18, 21));
  is.push_back(Interval( 2,  8));
  IntervalTree<Interval> it(is);
  it.print();
  {
    std::vector<uint64_t> queries;
    queries.push_back(0);
    queries.push_back(8);
    queries.push_back(10);
    queries.push_back(20);
    queries.push_back(22);
    queries.push_back(25);
    std::vector<std::vector<Interval> > res;
    res.push_back(std::vector<Interval>());
    res.push_back(std::vector<Interval>());
    res.back().push_back(Interval(5,10));
    res.back().push_back(Interval(1,12));
    res.back().push_back(Interval(8,16));
    res.push_back(std::vector<Interval>());
    res.back().push_back(Interval(1,12));
    res.back().push_back(Interval(8,16));
    res.push_back(std::vector<Interval>());
    res.back().push_back(Interval(15,25));
    res.back().push_back(Interval(18,21));
    res.push_back(std::vector<Interval>());
    res.back().push_back(Interval(15,25));
    res.push_back(std::vector<Interval>());
    for (uint32_t i = 0; i < queries.size(); i++) {
      printf("query: (%lu)\n", queries[i]);
      std::vector<uint32_t> ov;
      it.overlapsPoint(queries[i], &ov);
      printf("ov size %lu\n", ov.size());
      printf("is size %lu\n", is.size());
      fflush(stdout);
      uint32_t residx = 0;
      std::set<Interval> resset(res[i].begin(), res[i].end());
      for (uint32_t j = 0; j < ov.size(); j++) {
        printf("%u,%u   result: (%lu/%lu) ", i,ov[j], is[ov[j]].s, is[ov[j]].e);
        printf("expected: %s\n", resset.erase(Interval(is[ov[j]].s, is[ov[j]].e)) ? "yes" : "no");
        fflush(stdout);
        //assert(is[ov[j]] == res.at(i).at(residx));
        residx++;
      }
      printf("\n");
    }
  }
  printf("\n");
  printf("\n");
  printf("\n");
  {
    std::vector<Interval> queries;
    queries.push_back(Interval(8, 10));
    queries.push_back(Interval(20,22));
    queries.push_back(Interval(0,10));
    for (uint32_t i = 0; i < queries.size(); i++) {
      printf("query: (%lu/%lu)\n", queries[i].s, queries[i].e);
      std::vector<uint32_t> ov;
      it.overlapsInterval(queries[i], &ov);
      for (uint32_t j = 0; j < ov.size(); j++) {
        printf("   result: (%lu/%lu)", is[ov[j]].s, is[ov[j]].e);
      }
      printf("\n");
    }
  }
}
