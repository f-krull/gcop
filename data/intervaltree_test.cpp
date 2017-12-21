#include "intervaltree.cpp"
#include "interval.h"

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
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
      std::vector<char> ov;
      it.overlaps(queries[i], &ov);
      printf("ov size %lu\n", ov.size());
      printf("is size %lu\n", is.size());
      fflush(stdout);
      assert(ov.size() == is.size());
      uint32_t residx = 0;
      for (uint32_t j = 0; j < ov.size(); j++) {
        if (ov[j] == false) {
          continue;
        }
        printf("%u,%u   result: (%lu/%lu) ", i,j, is[j].s, is[j].e);
        printf("expected: (%lu/%lu)\n", res.at(i).at(residx).s, res.at(i).at(residx).e);
        fflush(stdout);
        assert(is[j] == res.at(i).at(residx));
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
      std::vector<char> ov;
      it.overlaps(queries[i], &ov);
      assert(ov.size() == is.size());
      for (uint32_t j = 0; j < ov.size(); j++) {
        if (ov[j] == false) {
          continue;
        }
        printf("   result: (%lu/%lu)", is[j].s, is[j].e);
      }
      printf("\n");
    }
  }
}
