#ifndef HMMAT_H_
#define HMMAT_H_

#include <stdint.h>
#include <vector>
#include <string>

/*----------------------------------------------------------------------------*/

class HmMat {
public:
  HmMat(){};
  bool read(const char* fn);
  float get(uint32_t i, uint32_t j) const { return m_d[i][j]; }
  const char* xlab(uint32_t j) const;
  const char* ylab(uint32_t i) const;
  uint32_t nrow() const { return m_d.size(); }
  uint32_t ncol() const { return m_d.empty() ? 0 : m_d.front().size(); }
  void print() const;
  float maxVal() const;
  float minVal() const;
  void transpose();
private:
  std::vector<std::vector<float>> m_d;
  std::vector<std::string> m_xlab;
  std::vector<std::string> m_ylab;
  bool check() const;
  void reset();
};

#endif /* HMMAT_H_ */
