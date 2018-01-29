#include "fieldformat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

FieldFormat::FieldFormat(const char *fmt) : m_numData(0) {
  m_fmt = strdup(fmt);
  for (; fmt[0] != '\0'; fmt++) {
    switch (fmt[0]) {
    case fieldTypeChars[FIELD_TYPE_GCCHR]:
      m_fs.push_back(Entry(FIELD_TYPE_GCCHR));
      break;
    case fieldTypeChars[FIELD_TYPE_GCBPS]:
    m_fs.push_back(Entry(FIELD_TYPE_GCBPS));
      break;
    case fieldTypeChars[FIELD_TYPE_GCBPE]:
    m_fs.push_back(Entry(FIELD_TYPE_GCBPE));
      break;
    case fieldTypeChars[FIELD_TYPE_SKIP]:
    m_fs.push_back(Entry(FIELD_TYPE_SKIP));
      break;
    default:
      {
        /* check if token is a number */
        char *end;
        uint64_t v = strtoul(fmt, &end, 10);
        if (fmt != end) {
          fmt = end;
          switch (fmt[0]) {
            case fieldTypeChars[FIELD_TYPE_FLOAT]:
              m_fs.push_back(Entry(FIELD_TYPE_FLOAT, v));
              m_numData++;
              break;
            case fieldTypeChars[FIELD_TYPE_CHR]:
              m_fs.push_back(Entry(FIELD_TYPE_CHR, v));
              m_numData++;
              break;
            case fieldTypeChars[FIELD_TYPE_UINT]:
              m_fs.push_back(Entry(FIELD_TYPE_UINT, v));
              m_numData++;
              break;
            case fieldTypeChars[FIELD_TYPE_STRING]:
              m_fs.push_back(Entry(FIELD_TYPE_STRING, v));
              m_types.push_back(FIELD_TYPE_STRING);
              break;
            default:
              fprintf(stderr, "error: unknown type at %lu in format '%s'\n", v, fmt);
              exit(1);
              break;
          }
          m_numData++;
          m_types.push_back(m_fs.back().type);
          /* fmt++ is done by for */
          continue;
        }
        /* unknown field type */
        fprintf(stderr, "error: unknown format '%s'\n", fmt);
        exit(1);
      }
      break;
    }
  }
}

/*----------------------------------------------------------------------------*/

FieldFormat::~FieldFormat() {
  free(m_fmt);
}

/*----------------------------------------------------------------------------*/

bool FieldFormat::hasField(FieldType t) const {
  return strchr(m_fmt, fieldTypeChars[t]) != NULL;
}

/*----------------------------------------------------------------------------*/

void FieldFormat::printFields() const {
  printf("input format:\n");
  for (uint32_t i = 0; i < m_fs.size(); i++) {
    printf("column %u : %s %u\n", i, fieldTypeStr[m_fs[i].type], m_fs[i].idx);
  }
}
