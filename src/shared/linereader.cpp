#include "linereader.h"
#include <zlib.h>
#include <string.h>
#include <stdio.h>

/*----------------------------------------------------------------------------*/

#define LINE_LENGTH_INIT (1 * 1024 * 1024)

/*----------------------------------------------------------------------------*/

class LineReaderPriv {
public:
  char *buf;
  gzFile f;
  uint32_t buflen;
};

/*----------------------------------------------------------------------------*/

LineReader::LineReader(uint32_t sinit) {
  m = new LineReaderPriv;
  m->buflen = sinit ? sinit : LINE_LENGTH_INIT;
  m->buf = new char[m->buflen];
  m->f = NULL;
}

/*----------------------------------------------------------------------------*/

LineReader::~LineReader() {
  delete [] m->buf;
  delete m;
}

/*----------------------------------------------------------------------------*/

bool LineReader::open(const char *fn) {
  m->f = gzopen(fn, "r");
  if (!m->f) {
    //fprintf(stderr, "error - cannot open '%s'\n", fn);
    return false;
  }
  return true;
}

/*----------------------------------------------------------------------------*/

char* LineReader::readLine() {
  char *line = NULL;
  z_off_t p_cur = gztell(m->f);
  while (true) {
    line = gzgets(m->f, m->buf, m->buflen);
    if (line == NULL) {
      break;
    }
    /* buffer not too small? */
    if (strlen(m->buf) + 1 != m->buflen) {
      break;
    }
    /* realloc and retry the same line */
    m->buflen = m->buflen * 2;
    delete[] m->buf;
    m->buf = new char[m->buflen];
    gzseek(m->f, p_cur, 0);
  }
  return line;
}

/*----------------------------------------------------------------------------*/

void LineReader::close() {
  if (m->f) {
    gzclose(m->f);
    m->f = NULL;
  }
}

/*----------------------------------------------------------------------------*/

bool LineReader::isOpen() const {
  return m->f;
}
