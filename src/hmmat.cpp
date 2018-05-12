#include "hmmat.h"
#include "buffer.h"
#include <stdio.h>
#include <float.h>

/*----------------------------------------------------------------------------*/

bool HmMat::read(const char* fn) {
  /*
   * mat format:
   *         [sep] <xlab0> [sep] <xlab1> [sep] <xlab2> ...
   * <ylab0> [sep] <val00> [sep] <val01> [sep] ...
   * <ylab1> [sep] ...
   * ...
   */
  FILE *f = fopen(fn, "r");
  if (f == NULL) {
    return false;
  }
  const char sep = '\t';
  bool ret = true;
  /* read header */
  while (true) {
    /* read one char */
    char c;
    if ((fread(&c, 1, 1, f)) != 1) {
      ret = false;
      break;
    }
    /* if sep -> create new label */
    if (c == sep) {
      m_xlab.push_back(std::string());
      continue;
    }
    /* if new line -> break */
    if (c == '\n') {
      break;
    }
    /* if char -> add to last label */
    if (m_xlab.empty()) {
      ret = false;
      break;
    }
    m_xlab.back().push_back(c);
  }
  /* read body */
  {
    std::string valbuf;
    char c;
    enum {
      READ_LINESTART,
      READ_YLAB,
      READ_VAL
    } state;
    state = READ_LINESTART;
    while (fread(&c, 1, 1, f) == 1) {
      switch (state) {
        case READ_LINESTART:
          m_ylab.push_back(std::string());
          m_ylab.back().push_back(c);
          state = READ_YLAB;
          break;
        case READ_YLAB:
          switch (c) {
            case sep:
              m_d.push_back(std::vector<float>());
              state = READ_VAL;
              break;
            default:
              m_ylab.back().push_back(c);
              break;
          }
          break;
          case READ_VAL:
            switch (c) {
              case sep:
                m_d.back().push_back(atof(valbuf.c_str()));
                valbuf.clear();
                break;
              case '\n':
                m_d.back().push_back(atof(valbuf.c_str()));
                valbuf.clear();
                state = READ_LINESTART;
                break;
              default:
                valbuf.push_back(c);
                break;
            }
            break;
        default:
          break;
      }
    }
  }
  fclose(f);
  ret &= check();
  if (!ret) {
    reset();
  }
  return ret;
}

/*----------------------------------------------------------------------------*/

void HmMat::print() const {
  printf("ncol=%u\n", ncol());
  printf("nrow=%u\n", nrow());
  for (uint32_t i = 0; i < ncol(); i++) {
    printf("\t%s", m_xlab[i].c_str());
  }
  printf("\n");
  for (uint32_t i = 0; i < nrow(); i++) {
    printf("%s", m_ylab[i].c_str());
    for (uint32_t j = 0; j < ncol(); j++) {
      printf("\t%f", m_d[i][j]);
    }
    printf("\n");
  }
}

/*----------------------------------------------------------------------------*/

bool HmMat::check() const {
  bool ret = true;
  ret &= m_ylab.size() == nrow();
  ret &= m_xlab.size() == ncol();
  uint32_t i = 0;
  while (ret && i < nrow()) {
    ret &= m_d[i].size() == ncol();
    i++;
  }
  return ret;
}

/*----------------------------------------------------------------------------*/

void HmMat::reset() {
  m_xlab.clear();
  m_ylab.clear();
  m_d.clear();
}

/*----------------------------------------------------------------------------*/

float HmMat::minVal() const {
  float r = FLT_MAX;
  for (uint32_t i = 0; i < nrow(); i++) {
    for (uint32_t j = 0; j < ncol(); j++) {
      const float v = m_d[i][j];
      r = r < v ? r : v;
    }
  }
  return r;
}

/*----------------------------------------------------------------------------*/

float HmMat::maxVal() const {
  float r = FLT_MIN;
  for (uint32_t i = 0; i < nrow(); i++) {
    for (uint32_t j = 0; j < ncol(); j++) {
      const float v = m_d[i][j];
      r = r > v ? r : v;
    }
  }
  return r;
}

/*----------------------------------------------------------------------------*/

void HmMat::transpose() {
  HmMat t;
  t.m_xlab = m_ylab;
  t.m_ylab = m_xlab;
  for (uint32_t j = 0; j < ncol(); j++) {
    t.m_d.push_back(std::vector<float>());
    for (uint32_t i = 0; i < nrow(); i++) {
      t.m_d.back().push_back(get(i,j));
    }
  }
  *this= t;
}

/*----------------------------------------------------------------------------*/

const char* HmMat::xlab(uint32_t j) const {
  return m_xlab[j].c_str();
}

/*----------------------------------------------------------------------------*/

const char* HmMat::ylab(uint32_t i) const {
  return m_ylab[i].c_str();
}
