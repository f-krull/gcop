#include "snpdata.h"
#include "mac_readfield.cpp"
#include "tokenreader.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


bool SnpData::read(const char * filename, uint32_t skip) {
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
    if (lineno <= skip) {
      continue;
    }
    /* parse line */
    Snp snp;
    char *pos = buffer;
    /* chr */
    READ_CHR(pos, delim, snp.chr, chrmap);
    /* rs */
    SKIP(pos, delim);
    /* a1 */
    SKIP(pos, delim);
    /* a2 */
    SKIP(pos, delim);
    /* bp */
    READ_BP(pos, delim, snp.bp);
    /* info */
    SKIP(pos, delim);
    /* or */
    SKIP(pos, delim);
    /* se */
    SKIP(pos, delim);
    /* pval */
    READ_FLOAT(pos, delim, snp.pval);
    m_d.push_back(snp);
  }
#if 0
  printf("read %lu snps\n", m_d.size());
  for (uint32_t i = 0; i < std::min(m_d.size(), (size_t)10); i++) {
    printf("%s %lu %f\n", chrmap.chrTypeStr(m_d[i].chr), m_d[i].bp, m_d[i].pval);
  }
#endif
  fclose(f);
  return true;
}


char* read_token(char *pos, char delim, char fieldId, Snp *s, const TokenReader &tr) {
  switch (fieldId) {
    case '1':
      return tr.read_chr(pos, delim, &s->chr);
      break;
    case '2':
      return tr.read_uint64(pos, delim, &s->bp);
      break;
    case '3':
      return tr.read_float(pos, delim, &s->pval);
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

bool SnpData::readdyn(const char * filename, const char *fields, uint32_t skip) {
  ChrMap chrmap;
  const char delim = '\t';
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "error: cannot open file %s\n", filename);
    exit(1);
  }
  char buffer[1024];
  uint32_t lineno = 0;
  TokenReader tr;
  while (fgets(buffer, sizeof(buffer)-1, f) != NULL) {
    lineno++;
    if (lineno <= skip) {
      continue;
    }
    /* parse line */
    Snp snp;
    char *pos = buffer;
    for (const char *f = fields; f[0] != '\0'; f++) {
      pos = read_token(pos, delim, f[0], &snp, tr);
    }
    m_d.push_back(snp);
  }
#if 1
  printf("read %lu snps\n", m_d.size());
  for (uint32_t i = 0; i < std::min(m_d.size(), (size_t)10); i++) {
    printf("%s %lu %f\n", chrmap.chrTypeStr(m_d[i].chr), m_d[i].bp, m_d[i].pval);
  }
#endif
  fclose(f);
  return true;
}

