
#ifndef DATA_INTERVAL_H_
#define DATA_INTERVAL_H_

#include <stdint.h>
#include <algorithm>

/*----------------------------------------------------------------------------*/

class Interval {
public:
  uint64_t s;
  uint64_t e; /* open interval */
  Interval(uint64_t ps, uint64_t pe) : s(ps), e(pe) {};
  template <typename T>
  static uint64_t center(T b, T e) {
    const uint64_t count = std::distance(b, e);
    if (count == 0) {
      return 0;
    }
    uint64_t center = 0;
    for (; b != e; ++b) {
      center += (b->e + b->s) / 2;
    }
    return center / count;
  }

  bool inside(uint64_t p) const {
    return p >= s && p < e;
  }
  static bool smaller(Interval i, uint64_t p) {
    return i.e <= p;
  }
  static bool greater(Interval i, uint64_t p) {
    return i.s > p;
  }

  static bool cmpStart(const Interval &a, const Interval &b) {
    return a.s < b.s;
  }
  static bool cmpEnd(const Interval &a, const Interval &b) {
    return a.e < b.e;
  }
};



#endif /* DATA_INTERVAL_H_ */