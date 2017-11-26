#include <time.h>

/*----------------------------------------------------------------------------*/

class Timer {
public:
  Timer() : m_begin(clock()), m_end(0) {
  }

  void stop() {
    m_end = clock();
  }

  float getSec() {
    return m_end ? (float(m_end-m_begin)/CLOCKS_PER_SEC) : 0;
  }

  clock_t m_begin;
  clock_t m_end;
};
