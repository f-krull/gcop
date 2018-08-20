#include "famwriter.h"
#include <stdio.h>

/*----------------------------------------------------------------------------*/

#define DELIMITER '\t'

/*----------------------------------------------------------------------------*/


const char FamWriter::SEXCODE_MALE[]    = "1";
const char FamWriter::SEXCODE_FEMALE[]  = "2";
const char FamWriter::SEXCODE_NA[]      = "0";

const char FamWriter::PHENOTYPE_CONTROL[] = "1";
const char FamWriter::PHENOTYPE_CASE[]    = "2";
const char FamWriter::PHENOTYPE_NA[]      = "0";

/*----------------------------------------------------------------------------*/

FamWriter::FamWriter() {
  m_f = NULL;
  m_numWritten = 0;
}

/*----------------------------------------------------------------------------*/

bool FamWriter::open(const char *fn) {
  m_f = fopen(fn, "w");
  if (!m_f) {
    fprintf(stderr, "error - cannot open file '%s'\n", fn);
    return false;
  }
  m_fn = fn;
  m_numWritten = 0;
  return true;
}

/*----------------------------------------------------------------------------*/

bool FamWriter::write(const Entry &e) {
  if (!m_f) {
    return false;
  }
  bool ret = true;
  ret = ret && fprintf(m_f, "%s", e.fid.c_str()) >= 1;
  ret = ret && fprintf(m_f, "%c%s", DELIMITER, e.id.c_str()) >= 1;
  ret = ret && fprintf(m_f, "%c%s", DELIMITER, e.fatherId.c_str()) >= 1;
  ret = ret && fprintf(m_f, "%c%s", DELIMITER, e.motherId.c_str()) >= 1;
  ret = ret && fprintf(m_f, "%c%s", DELIMITER, e.sex.c_str()) >= 1;
  ret = ret && fprintf(m_f, "%c%s", DELIMITER, e.phenotype.c_str()) >= 1;
  ret = ret && fprintf(m_f, "\n") >= 1;
  if (!ret) {
    fprintf(stderr, "error - cannot write file '%s'\n", m_fn.c_str());
    return false;
  }
  m_numWritten++;
  return true;
}

/*----------------------------------------------------------------------------*/

void FamWriter::close() {
  if (m_f) {
    fclose(m_f);
  }
  m_fn = "";
}
