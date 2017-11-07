#include "segdata.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

std::vector<Segment> ISegData::getChr(ChrMap::ChrType ct) const {
  std::vector<Segment> cseg;
  /* TODO: optimize */
  for (uint32_t i = 0; i < m_d.size(); i++) {
    if (m_d[i].chr == ct) {
      cseg.push_back(m_d[i]);
    }
  }
  return cseg;
}

/*----------------------------------------------------------------------------*/

#include "mac_readfield.cpp"
bool SimpleSegData::read(const char *filename) {
  ChrMap chrmap;
  const char delim = '\t';
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "error: cannot open file %s\n", filename);
    exit(1);
  }
  char buffer[1024];
  uint32_t lineno = 0;
  while (fgets(buffer, sizeof(buffer)-1, f) != NULL) {
    lineno++;
    /* parse line */
    Segment seg;
    char *pos = buffer;
    READ_CHR(pos, delim, seg.chr, chrmap);
    READ_BP(pos,  delim, seg.s);
    READ_BP(pos,  delim, seg.e);
    /* add segment */
    m_d.push_back(seg);

  }
  fclose(f);
#if 0
  printf("read %lu segments\n", m_d.size());
  for (uint32_t i = 0; i < std::min(m_d.size(), (size_t)10); i++) {
    printf("%s %lu %lu\n", chrmap.chrTypeStr(m_d[i].chr), m_d[i].s, m_d[i].e);
  }
#endif
  return true;
}
