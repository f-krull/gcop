#include "infofile.h"
#include "gettoken.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

/*----------------------------------------------------------------------------*/

#define INFO_LINE_LENGTH_INIT (16*1024)
#define DELIMITER '\t'

/*----------------------------------------------------------------------------*/

const char* InfoFileEntry::InfoFieldTypeStr[] = {
#define  ENUM_GET_STR(name, str) str,
  ENUM_INFOFIELDTYPE(ENUM_GET_STR)
  "undefined"
#undef ENUM_GET_NAME
};

/*----------------------------------------------------------------------------*/


bool InfoFileFilter::checkMaf(const InfoFileEntry &e) const {
  const float maf = atof(e.get(InfoFileEntry::FIELD_MAF));
  return m_mafMin <= maf;
}

/*----------------------------------------------------------------------------*/

bool InfoFileFilter::checkAvgCall(const InfoFileEntry &e) const {
  const float avgc = atof(e.get(InfoFileEntry::FIELD_AVGCALL));
  return m_avgCallMin <= avgc;
}

/*----------------------------------------------------------------------------*/

bool InfoFileFilter::checkRsq(const InfoFileEntry &e) const {
  const float rsq = atof(e.get(InfoFileEntry::FIELD_RSQ));
  return m_rsqMin <= rsq;
}
/*----------------------------------------------------------------------------*/

#define READ_RETURN(b) delete [] buf; \
                       gzclose(gzf); \
                       return b;

bool InfoFile::read(const char *fn, const InfoFileFilter &filter) {
  gzFile gzf = gzopen(fn, "r");
  if (gzf == NULL) {
    fprintf(stderr, "error - cannot open file '%s'\n", fn);
    return false;
  }
  const uint32_t buflen = INFO_LINE_LENGTH_INIT;
  char *buf = new char[buflen];


  uint32_t linec = 1;
  while (true) {
    char* line = gzgets(gzf, buf, buflen);
    if (line == NULL) {
      break;
    }
    /* line too long? */
    if (strlen(buf) + 1 == buflen) {
      /* if this is ever happenes for a reason - realloc line buffer */
      fprintf(stderr, "error - line %u is longer than %u bytes (file '%s')\n",
          linec, buflen, fn);
      READ_RETURN(false);
    }
    if (buf[strlen(buf) - 1] != '\n') {
      fprintf(stderr, "error - exprected newline at %zu (file '%s', line %u)\n",
          strlen(buf) - 1, fn, linec);
      READ_RETURN(false);
    }
    /* patch '\n' for parsing */
    buf[strlen(buf) - 1] = DELIMITER;
    while (true) { /* dummy; increase linec */
      /* on first line */
      if (linec == 1) {
        char *field_cur = line;
        char *field_nxt = gettoken(field_cur, DELIMITER);
        for (uint32_t i = 0; i < InfoFileEntry::INFOFIELDTPYE_NUMENTRIES; i++) {
          if (strcmp(field_cur, InfoFileEntry::InfoFieldTypeStr[i]) != 0) {
            fprintf(stderr, "error - header does not contains column '%s'\n",
                InfoFileEntry::InfoFieldTypeStr[i]);
            READ_RETURN(false);
          }
          field_cur = field_nxt;
          field_nxt = gettoken(field_cur, DELIMITER);
        }
        break;
      }
      /* on any line > 1 */
      char *field_cur = line;
      char *field_nxt = gettoken(field_cur, DELIMITER);
      InfoFileEntry e;
      for (uint32_t i = 0; i < InfoFileEntry::INFOFIELDTPYE_NUMENTRIES; i++) {
        e.set((InfoFileEntry::InfoFieldType)i, field_cur);
        field_cur = field_nxt;
        field_nxt = gettoken(field_cur, DELIMITER);
      }
      m_variants.push_back(e);
      m_variantStatus.push_back(filter.isOk(e));
      m_numVarOk += m_variantStatus.back() ? 1 : 0;
      break;
    }
    linec++;
  }

  if (!gzeof(gzf)) {
    fprintf(stderr, "error - while reading file '%s'\n", fn);
    READ_RETURN(false);
  }
#if 0
  for (uint32_t i = 0; i < m_variants.size(); i++) {
    m_variants[i].print();
  }
#endif
  READ_RETURN(true);
#undef READ_RETURN
}
