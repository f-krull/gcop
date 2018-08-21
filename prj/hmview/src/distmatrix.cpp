
#ifndef DISTMATRIX_CPP_
#define DISTMATRIX_CPP_

#include "distmatrix.h"
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>

/*----------------------------------------------------------------------------*/

DistanceMatrix::DistanceMatrix(uint32_t numElements, double initValue) {
  this->m_numElements = numElements;
  m_matrix = new double *[numElements - 1];
  for (uint32_t i = 0; i < numElements - 1; i++) {
    m_matrix[i] = new double[i + 1];
    for (uint32_t j = 0; j < i + 1; j++) {
      m_matrix[i][j] = initValue;
    }
  }
  this->m_null = (double) 0;
}

/*----------------------------------------------------------------------------*/

DistanceMatrix::~DistanceMatrix() {
  for (uint32_t i = 0; i < m_numElements - 1; i++) {
    delete[] m_matrix[i];
  }
  delete[] m_matrix;
}

/*----------------------------------------------------------------------------*/

double DistanceMatrix::get(uint32_t i, uint32_t j) const {
  assert(i < m_numElements);
  assert(j < m_numElements);
  if (i < j) {
    return m_matrix[j - 1][i];
  } else if (j < i) {
    return m_matrix[i - 1][j];
  } else {
    /* distance from i to i is 0 */
    return (double) 0;
  }
}

/*----------------------------------------------------------------------------*/

void DistanceMatrix::set(uint32_t i, uint32_t j, double value) {
  assert(i < m_numElements);
  assert(j < m_numElements);
  if (i < j) {
    m_matrix[j - 1][i] = value;
  } else if (j < i) {
    m_matrix[i - 1][j] = value;
  }
}

/*----------------------------------------------------------------------------*/

void DistanceMatrix::setAll(double value) {
  for (uint32_t i = 0; i < m_numElements; i++) {
    for (uint32_t j = i; j < m_numElements; j++) {
      set(i, j, value);
    }
  }
}

/*----------------------------------------------------------------------------*/

uint32_t DistanceMatrix::getNumElements() {
  return m_numElements;
}

/*----------------------------------------------------------------------------*/

void DistanceMatrix::print() {
  for (uint32_t i = 0; i < m_numElements; i++) {
    std::cout << "dm ";
    for (uint32_t j = 0; j < m_numElements; j++) {
      std::cout << std::fixed << std::setprecision(3) << std::setw(7);
      std::cout << get(i, j);
    }
    std::cout << std::endl;
  }
}

/*----------------------------------------------------------------------------*/


double& DistanceMatrix::operator()(uint32_t i, uint32_t j) {
  assert(i < m_numElements);
  assert(j < m_numElements);
  if (i < j) {
    return m_matrix[j - 1][i];
  } else if (j < i) {
    return m_matrix[i - 1][j];
  } else {
    /* distance from i to i is 0 */
    return m_null;
    /* TODO: i do not like this */
  }
}

/*----------------------------------------------------------------------------*/

bool DistanceMatrix::save(const char *filename) {
  FILE *f = NULL;

  f = fopen(filename, "wb");
  if (f == NULL) {
    return false;
  }
  bool ok = true;
  ok = ok && fwrite(&m_numElements, sizeof(m_numElements), 1, f) == 1;
  double tmp;
  for (uint32_t i = 0; i < m_numElements; i++) {
    for (uint32_t j = 0; j < m_numElements; j++) {
      tmp = get(i, j);
      ok = ok && fwrite(&tmp, sizeof(double), 1, f) == 1;
    }
  }
  fclose(f);
  return ok;
}

/*----------------------------------------------------------------------------*/

bool DistanceMatrix::load(const char *filename) {
  FILE *f = NULL;

  f = fopen(filename, "rb");
  if (f == NULL) {
    return false;
  }
  bool ok = true;
  ok = ok && fread(&m_numElements, sizeof(m_numElements), 1, f) == 1;
  double tmp;
  for (uint32_t i = 0; i < m_numElements; i++) {
    for (uint32_t j = 0; j < m_numElements; j++) {
      ok = ok && fread(&tmp, sizeof(double), 1, f) == 1;
      set(i, j, tmp);
    }
  }
  fclose(f);
  return ok;
}

#endif /*DISTMATRIX_CPP_*/

