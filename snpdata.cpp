#include "snpdata.h"
#include "mac_readfield.cpp"
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
  printf("read %lu snps\n", m_d.size());
  for (uint32_t i = 0; i < m_d.size() / 944423; i++) {
    printf("%s %lu %f\n", chrmap.chrTypeStr(m_d[i].chr), m_d[i].bp, m_d[i].pval);
  }
  fclose(f);
  return true;
}
