#include "debuglog.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

Log::Log(const char* pfx, ...) {
  m_prefix = NULL;
  va_list args;
  va_start(args, pfx);
  setPrefix(pfx, args);
  va_end(args);
}

/*----------------------------------------------------------------------------*/

Log::~Log() {
  rmPfx();
}

/*----------------------------------------------------------------------------*/

void Log::setPrefix(const char *pfx, ...) {
  va_list args;
  va_start(args, pfx);
  setPrefix(pfx, args);
  va_end(args);
}

/*----------------------------------------------------------------------------*/

void Log::rmPfx() {
  if (m_prefix != NULL) {
    free(m_prefix);
  }
  m_prefix = NULL;
}

/*----------------------------------------------------------------------------*/

void Log::dbg(const char* msg, ...) {
   va_list args;
   va_start(args, msg);
   vsnprintf(m_buffer, sizeof(m_buffer), msg, args);
   va_end(args);
   printf("%s%s%s\n", m_prefix ? m_prefix : "", m_prefix ? ": " : "", m_buffer);
}

/*----------------------------------------------------------------------------*/

void Log::err(const char* msg, ...) {
   va_list args;
   va_start(args, msg);
   vsnprintf(m_buffer, sizeof(m_buffer), msg, args);
   va_end(args);
   printf("%s%serror: %s\n", m_prefix ? m_prefix : "", m_prefix ? " " : "", m_buffer);
}

/*----------------------------------------------------------------------------*/

void Log::setPrefix(const char *pfx, va_list args) {
  rmPfx();
  if (pfx == NULL) {
    return;
  }
  vsnprintf(m_buffer, sizeof(m_buffer), pfx, args);
  m_prefix = strdup(m_buffer);
}
