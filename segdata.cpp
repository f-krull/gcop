#include "segdata.h"
#include "data/tokenreader.h"
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

char* read_token(char *pos, char delim, char fieldId, Segment *s, const TokenReader &tr) {
  switch (fieldId) {
    case 'c':
      return tr.read_chr(pos, delim, &s->chr);
      break;
    case 's':
      return tr.read_uint64(pos, delim, &s->s);
      break;
    case 'e':
      return tr.read_uint64(pos, delim, &s->e);
      break;
    case '.':
      return tr.read_forget(pos, delim);
      break;
    default:
      assert(false);
      break;
  }
  return NULL;
}

/*----------------------------------------------------------------------------*/

bool SimpleSegData::read(const char *filename, const char *fmt, uint32_t skip) {
  ChrMap chrmap;
  const char delim = '\t';
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "error: cannot open file %s\n", filename);
    exit(1);
  }
  TokenReader tr;
  char buffer[1024];
  uint32_t lineno = 0;
  while (fgets(buffer, sizeof(buffer)-1, f) != NULL) {
    lineno++;
    if (lineno <= skip) {
      continue;
    }
    /* parse line */
    Segment seg;
    char *pos = buffer;
    for (const char *f = fmt; f[0] != '\0'; f++) {
      pos = read_token(pos, delim, f[0], &seg, tr);
    }
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
