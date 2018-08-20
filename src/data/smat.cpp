#include "smat.h"
#include "melist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>


/*----------------------------------------------------------------------------*/

SparseMat::SparseMat(const MatEntryList &mel) : m_n(mel.n()) {
  assert(mel.n() && "got matrix without any rows");
  m_values  = new uint8_t[mel.l().size()];
  m_colIdx  = new uint32_t[mel.l().size()];
  m_rowPtr  = new uint32_t[mel.n()+1];
  m_rowPtr[0] = 0;
  m_numVal = mel.l().size();
  /* build matrix */
  uint32_t lastRow = 0;    /* to check for row changes */
  uint32_t numE    = 0;    /* count number of elements per row */
  for (uint32_t i = 0; i < mel.l().size(); i++) {
    const MatEntryList::MatEntry &e = mel.l()[i];
    if (lastRow != e.j) {
      assert(lastRow < e.j);
      /* assign numE for current and all previous rows */
      for (uint32_t j = lastRow+1; j <= e.j; j++) {
        m_rowPtr[j] = numE;
      }
      lastRow = e.j;
    }
    m_colIdx[i] = e.i;
    m_values[i] = e.v;
    numE++;
  }
  /* assign numE for current and all previous rows, again */
  for (uint32_t j = lastRow+1; j <= mel.n(); j++) {
    m_rowPtr[j] = numE;
  }
}

/*----------------------------------------------------------------------------*/

SparseMat::SparseMat(MatEntryReader &mer) {
  assert(mer.numEntries() && "got empty matrix");
  assert(mer.n() && "got matrix without any rows");
  m_values  = new uint8_t[mer.numEntries()];
  m_colIdx  = new uint32_t[mer.numEntries()];
  m_rowPtr  = new uint32_t[mer.n()+1];
  m_rowPtr[0] = 0;
  m_numVal = mer.numEntries();
  m_n = mer.n();
  /* build matrix */
  uint32_t lastRow = 0;    /* to check for row changes */
  uint32_t numE    = 0;    /* count number of elements per pow */
  MatEntryList::MatEntry e;
  uint32_t i = 0;
  while (mer.read(&e)) {
    //printf("%u %u %u\n", e.i, e.j, e.v);
    if (lastRow != e.j) {
      assert(lastRow < e.j);
      /* assign numE for current and all previous rows */
      for (uint32_t j = lastRow+1; j <= e.j; j++) {
        m_rowPtr[j] = numE;
      }
      lastRow = e.j;
    }
    m_colIdx[i] = e.i;
    m_values[i] = e.v;
    numE++;
    i++;
  }
  /* assign numE for current and all previous rows, again */
  for (uint32_t j = lastRow+1; j <= mer.n(); j++) {
    m_rowPtr[j] = numE;
  }
}

/*----------------------------------------------------------------------------*/

SparseMat::SparseMat(const char *fn) {
  m_values  = NULL;
  m_colIdx  = NULL;
  m_rowPtr  = NULL;
  read(fn);
}

/*----------------------------------------------------------------------------*/

uint8_t SparseMat::get(uint32_t i, uint32_t j) const {
  int64_t l = m_rowPtr[j];;
  int64_t r = int64_t(m_rowPtr[j+1])-1;
  int64_t m = 0;
  uint8_t res = 0;
  /* binary search */
  while (l < r) {
    m = (l+r)/2;
    /* look right */
    if (m_colIdx[m] < i) {
      l = m+1;
      continue;
    }
    /* look left */
    if (m_colIdx[m] > i) {
      r = m-1;
      continue;
    }
    /* colIdx[m] == i */
    res = m_values[m];
    break;
  }
  return res;
}

/*----------------------------------------------------------------------------*/

SparseMat::~SparseMat() {
  reset();
}

/*----------------------------------------------------------------------------*/

void SparseMat::print() const {
  printf("v: ");
  for (uint32_t i = 0; i < m_numVal; i++) {
    printf("%u ", m_values[i]);
  }
  printf("\n");
  printf("c: ");
  for (uint32_t i = 0; i < m_numVal; i++) {
    printf("%u ", m_colIdx[i]);
  }
  printf("\n");
  printf("r: ");
  for (uint32_t i = 0; i < m_n+1; i++) {
    printf("%u ", m_rowPtr[i]);
  }
  printf("\n");
}

/*----------------------------------------------------------------------------*/

uint32_t SparseMat::n() const {
  return m_n;
}

/*----------------------------------------------------------------------------*/

void SparseMat::printMat() const {
  for (uint32_t j = 0; j < n(); j++) {
    for (uint32_t i = 0; i < n(); i++) {
      printf("%3u ", get(i,j));
    }
    printf("\n");
  }
}

/*----------------------------------------------------------------------------*/

void SparseMat::write(const char* fn) const {
  const uint32_t version = 1;
  FILE* f = fopen(fn, "wb");
  if (f == NULL) {
    fprintf(stderr, "error: opening file %s\n", fn);
    exit(1);
  }
  /* file format
  *
  * header:  0 4 str  magic number ('NORM')  
  *          4 4 u32  version 
  *          8 4 u32  n
  *         12 4 u32  num entries
  *  data:  16 - u32  m_rowPtr
  *         -  - u32  m_colIdx
  *         -  - u8   m_values      
  */
  bool ok = true;
  ok = ok && fwrite(&"NORM", 4, 1, f) == 1;
  ok = ok && fwrite(&version, sizeof(version), 1, f) == 1;
  ok = ok && fwrite(&m_n, sizeof(m_n), 1, f) == 1;
  ok = ok && fwrite(&m_numVal, sizeof(m_numVal), 1, f) == 1;
  ok = ok && fwrite(m_rowPtr, sizeof(uint32_t), m_n+1, f) == m_n+1;
  ok = ok && fwrite(m_colIdx, sizeof(uint32_t), m_numVal, f) == m_numVal;
  ok = ok && fwrite(m_values, sizeof(uint8_t), m_numVal, f) == m_numVal;
  if (!ok) {
    fprintf(stderr, "error: writing file %s\n", fn);
    exit(1);
  }
  fclose(f);
}

/*----------------------------------------------------------------------------*/

void SparseMat::reset() {
  m_n = 0;
  m_numVal = 0;
  delete [] m_colIdx;
  delete [] m_rowPtr;
  delete [] m_values;
  m_colIdx = NULL;
  m_rowPtr = NULL;
  m_values = NULL;
}

/*----------------------------------------------------------------------------*/

void SparseMat::read(const char* fn) {
  reset();
  uint32_t version = 0;
  uint8_t magicNum[4];
  FILE* f = fopen(fn, "rb");
  if (f == NULL) {
    fprintf(stderr, "error: opening file %s\n", fn);
    exit(1);
  }
  bool ok = true;
  ok = ok && fread(&magicNum, 4, 1, f) == 1;
  ok = ok && fread(&version, sizeof(version), 1, f) == 1;
  ok = ok && fread(&m_n, sizeof(m_n), 1, f) == 1;
  ok = ok && fread(&m_numVal, sizeof(m_numVal), 1, f) == 1;
  if (!ok) {
    fprintf(stderr, "error: reading file header %s\n", fn);
    reset();
    exit(1);
  }
  printf("numVal: %u\n", m_numVal);
  printf("n: %u\n", m_n);

  m_rowPtr = new uint32_t[m_n+1];
  m_colIdx = new uint32_t[m_numVal]; 
  m_values = new uint8_t[m_numVal];
  ok = ok && fread(m_rowPtr, sizeof(uint32_t), m_n+1, f) == m_n+1;
  ok = ok && fread(m_colIdx, sizeof(uint32_t), m_numVal, f) == m_numVal;
  ok = ok && fread(m_values, sizeof(uint8_t), m_numVal, f) == m_numVal;
  if (!ok) {
    fprintf(stderr, "error: reading file %s\n", fn);
    reset();
    exit(1);
  }
  fclose(f);
}

/*----------------------------------------------------------------------------*/

void SparseMat::printSize() const {
  uint32_t sizeV = 1*m_numVal;
  uint32_t sizeC = 4*m_numVal;
  uint32_t sizeR = 4*m_n;
  printf("size of val:    %uB\n", sizeV);
  printf("size of colIdx: %uB\n", sizeC);
  printf("size of rowPtr: %uB\n", sizeR);
  printf("total size:  %uM\n", (sizeV+sizeC+sizeR)/(1024*1024));
}


