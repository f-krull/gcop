
#ifndef DISTMATRIX_H_
#define DISTMATRIX_H_

#include <vector>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class DistanceMatrix {
public:
  DistanceMatrix(uint32_t numElements, double initValue = 0);
  ~DistanceMatrix();
  double get(uint32_t i, uint32_t j) const;
  void set(uint32_t i, uint32_t j, double value);
  void setAll(double value);
  uint32_t getNumElements();
  void print();
  double& operator()(uint32_t i, uint32_t j);

  bool save(const char* filename);
  bool load(const char* filename);
private:
  double ** m_matrix;
  double m_null;
protected:
  uint32_t m_numElements;
};

/*----------------------------------------------------------------------------*/

class DistanceMatrixFactory {
private:
protected:
public:
  template<typename U, typename C>
  static DistanceMatrix* getFilled(std::vector<U> *elements, const C &cmp);
};

/*----------------------------------------------------------------------------*/

template<typename U, typename C>
DistanceMatrix* DistanceMatrixFactory::getFilled(std::vector<U> *elements,
    const C &cmp) {
  DistanceMatrix* dm = new DistanceMatrix(elements->size());
  uint32_t numdist;
  uint32_t maxnumdist;

  numdist = 0;
  maxnumdist = (*dm).getNumElements() * (*dm).getNumElements();
  maxnumdist += (*dm).getNumElements();
  maxnumdist /= 2;
  for (uint32_t i = 0; i < (*dm).getNumElements(); i++) {
    //#pragma omp parallel for shared(numdist)
    for (uint32_t j = i + 1; j < (*dm).getNumElements(); j++) {
      (*dm)(i, j) = cmp((*elements)[i], (*elements)[j]);

      //# pragma omp atomic
      numdist++;
    }
  }
  return dm;
}

#endif /*DISTMATRIX_H_*/
