#include "melist.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#define LD_CUTOFF 0.1

/*----------------------------------------------------------------------------*/

void MatEntryList::MatEntry::print() {
  printf("%u %u %u\n", i, j, v);
}

/*----------------------------------------------------------------------------*/

bool MatEntryList::MatEntry::operator < (const MatEntry& o) const{
  return (j < o.j || (j == o.j && (i < o.i)));
}

/*----------------------------------------------------------------------------*/

MatEntryList::MatEntryList(uint32_t n) : m_n(n) {
}

/*----------------------------------------------------------------------------*/

bool MatEntryList::readMatPlinkTri(const char* fn, MatEntryList* mel) {
  size_t  linesiz = 0;
  char*   linebuf = 0;
  ssize_t linelen = 0;
  uint32_t j = 0;
  const uint32_t report_intvl = 1000;

  FILE *f = stdin; 
  if (fn) {
    f = fopen(fn, "r");
  }
  if (!f) {
    fprintf(stderr, "error: opening file \"%s\"\n", fn);
    exit(1);
  }

  while ((linelen=getline(&linebuf, &linesiz, f)) > 0) {
    char* pos = linebuf;
    char* end = NULL;
    uint32_t i = 0;
    
    /* break if new line or null */
    while (pos[0] != '\n' && pos[0] != '\0') {
      float value = strtof(pos, &end);
      if (end == pos) {
        break;
      }
      pos=end;
      mel->addValue(i, j, value);
      i++;
    }
    free(linebuf);
    linebuf=NULL;
    j++;
    if ((j % report_intvl) == 0) {
      fprintf(stderr, "line: %u - entries: %lu\n", j, mel->l().size());
    }
  }
  free(linebuf);
  mel->m_n = j;
  printf("read matrix of size %u\n", mel->m_n);
  if (fn) {
    fclose(f);
  }
  return true;
}

/*----------------------------------------------------------------------------*/

void MatEntryList::print() {
  for (uint32_t i = 0; i < m_list.size(); i++) {
    const MatEntry e = m_list[i];
    printf("%3u %3u : %u\n", e.i, e.j, e.v); 
  }
}

/*----------------------------------------------------------------------------*/

void MatEntryList::printLen() {
  printf("mel len: %lu  (n=%u)\n", l().size(), m_n);
}

/*----------------------------------------------------------------------------*/

const std::vector<MatEntryList::MatEntry>& MatEntryList::l() const {
  return m_list;
}

/*----------------------------------------------------------------------------*/

uint32_t MatEntryList::n() const {
  return m_n;
}

/*----------------------------------------------------------------------------*/

void MatEntryList::addValue(uint32_t i, uint32_t j, float value) {
  if (value >= LD_CUTOFF && i != j) {
    value = value > 1 ? 1. : value;
    value = value < 0 ? 0 : value;
    MatEntry mle;
    /* specific for triangle matrix: */
    mle.i = i < j ? i : j;
    mle.j = i < j ? j : i;
    mle.v = (0xFF*value);
    m_list.push_back(mle);
  }
}

/*----------------------------------------------------------------------------*/

void MatEntryList::push(uint32_t i, uint32_t j, float value) {
  if (i != j) {
    value = value > 1 ? 1. : value;
    value = value < 0 ? 0 : value;
    MatEntry mle;
    /* specific for triangle matrix: */
    mle.i = i < j ? i : j;
    mle.j = i < j ? j : i;
    mle.v = (0xFF*value);
    m_list.push_back(mle);
    m_n = m_list.size();
  }
}

/*----------------------------------------------------------------------------*/
#include <set>
void MatEntryList::close() {
  sort();
  std::set<uint32_t> keyset;
  for (uint32_t i = 0; i < m_list.size(); i++) {
    keyset.insert(m_list[i].i);
    keyset.insert(m_list[i].j);
  }

  for (std::set<uint32_t>::const_iterator it = keyset.begin(); it != keyset.end(); ++it) {
    printf("keyset %u\n", *it);
  }

  m_n = keyset.size();
}

/*----------------------------------------------------------------------------*/

void MatEntryList::sort() {
  std::sort(m_list.begin(), m_list.end());
}

/*----------------------------------------------------------------------------*/

void MatrixEntrySink::addEntryF(uint32_t i, uint32_t j, float value) {
  if (value >= LD_CUTOFF && i != j) {
    MatEntryList::MatEntry mle;
    /* specific for triangle matrix: */
    mle.i = i < j ? i : j;
    mle.j = i < j ? j : i;
    mle.v = mle.v > 1. ? 1. : mle.v;
    mle.v = mle.v < 0  ? 0  : mle.v;
  }
}

/*----------------------------------------------------------------------------*/

void MatrixEntrySink::addEntryB(uint32_t i, uint32_t j, uint8_t byte) {
  if (i != j) {
    MatEntryList::MatEntry mle;
    /* specific for triangle matrix: */
    mle.i = i < j ? i : j;
    mle.j = i < j ? j : i;
    mle.v = byte;
    m_numE++;
    newEntry(mle);
  }
}

/*----------------------------------------------------------------------------*/

MatEntryWriter::MatEntryWriter() : m_f(NULL) {
}

/*----------------------------------------------------------------------------*/

bool MatEntryWriter::open(const char *fn) {
  m_f = fopen(fn, "wb");
  if (!m_f) {
    return false;
  }
  /* write placeholder */
  uint32_t n = 0xff;
  bool ok = true;
  ok = ok && fwrite(&n, sizeof(n), 1, m_f) == 1;
  uint64_t numEnries = 0x66;
  ok = ok && fwrite(&numEnries, sizeof(numEnries), 1, m_f) == 1;
  return ok;
}

/*----------------------------------------------------------------------------*/

bool MatEntryWriter::newEntry(const MatEntryList::MatEntry &e) {
  bool ok = true;
  ok = ok && fwrite(&e.i, sizeof(uint32_t), 1, m_f) == 1;
  ok = ok && fwrite(&e.j, sizeof(uint32_t), 1, m_f) == 1;
  ok = ok && fwrite(&e.v, sizeof(uint8_t), 1, m_f) == 1;
  return ok;
}

/*----------------------------------------------------------------------------*/

bool MatEntryWriter::close(uint32_t n) {
  if (m_f) {
    uint64_t numE = numEntries();
    rewind(m_f);
    bool ok = true;
    ok = ok && fwrite(&n, sizeof(n), 1, m_f) == 1;
    ok = ok && fwrite(&numE, sizeof(numE), 1, m_f) == 1;
    fclose(m_f);
    m_f = NULL;
    return ok;
  }
  return false;
}

/*----------------------------------------------------------------------------*/

MatEntryReader::MatEntryReader() : m_f(NULL), m_n(0), m_numEntries(0) {
}

/*----------------------------------------------------------------------------*/

bool MatEntryReader::open(const char *fn) {
  m_f = fopen(fn, "rb");
  if (!m_f) {
    return false;
  }
  bool ok = true;
  ok = ok && fread(&m_n, sizeof(m_n), 1, m_f) == 1;
  ok = ok && fread(&m_numEntries, sizeof(m_numEntries), 1, m_f) == 1;
  return ok;
}

/*----------------------------------------------------------------------------*/

bool MatEntryReader::read(MatEntryList::MatEntry *e) {
  bool ok = true;
  ok = ok && fread(&e->i, sizeof(uint32_t), 1, m_f) == 1;
  ok = ok && fread(&e->j, sizeof(uint32_t), 1, m_f) == 1;
  ok = ok && fread(&e->v, sizeof(uint8_t), 1, m_f) == 1;
  return ok;
}

/*----------------------------------------------------------------------------*/

void MatEntryReader::close() {
  if (m_f) {
    fclose(m_f);
  }
  m_f = NULL;
}
