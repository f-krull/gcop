#include "tabfield.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

FieldValue::FieldValue(FieldType t) : m_type(t) {
}

/*----------------------------------------------------------------------------*/

FieldValue::FieldValue(const FieldValue &o) : m_type(o.m_type) {
  switch (o.m_type) {
    case FIELD_TYPE_UINT:
      m_u.uint = o.m_u.uint;
      break;
    case FIELD_TYPE_STRING:
      m_u.str = strdup(o.m_u.str);
      break;
    case FIELD_TYPE_FLOAT:
      m_u.flt = o.m_u.flt;
      break;
    default:
      fprintf(stderr, "error: fieldtype %s\n", fieldTypeStr[o.m_type]);
      assert(false);
      break;
  }
}

/*----------------------------------------------------------------------------*/

FieldValue::~FieldValue() {
  if (m_type == FIELD_TYPE_STRING) {
    free(m_u.str);
  }
}
