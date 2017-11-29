#include "file.h"
#include <stdlib.h>
#include <assert.h>

class FilePriv {
public:
  virtual ~FilePriv() {};
  virtual bool open(const char *fn, const char *mode) = 0;
  virtual char *gets(char *buf, int n) = 0;
  virtual void close() = 0;
private:
};

#include <stdio.h>
class FilePrivDef : public FilePriv {
public:
  FilePrivDef() : m_f(NULL) {}
  bool open(const char *fn, const char *mode) {
    m_f = fopen(fn, mode);
    return m_f != NULL;
  }
  char *gets(char *buf, int n) {
    assert(m_f);
    return fgets(buf, n, m_f);
  }
  void close() {
    assert(m_f);
    fclose(m_f);
    m_f = NULL;
  }
private:
  FILE *m_f;
};

#include <zlib.h>
class FilePrivGz : public FilePriv {
public:
  FilePrivGz() : m_f(NULL) {}
  bool open(const char *fn, const char *mode) {
    m_f = gzopen(fn, mode);
    return m_f != NULL;
  }
  char *gets(char *buf, int n) {
    assert(m_f);
    return gzgets(m_f, buf, n );
  }
  void close() {
    assert(m_f);
    gzclose(m_f);
    m_f = NULL;
  }
private:
  gzFile m_f;
};

File::File() : m(NULL) {
}

File::~File() {
  delete m;
}


#include <string.h>
const char* getFileExtention(const char *fn) {
  const char *ext = strrchr(fn, '.');
  if (ext != NULL && ext != fn) {
    return ext+1;
  }
  return "";
}

File::FileType getFileType(const char *fn) {
  const char *ext = getFileExtention(fn);
  if (strcmp(ext, "GZ")) {
    return File::FILETYPE_GZ;
  }
  return File::FILETYPE_TXT;
}


bool File::open(const char *fn, const char *mode, FileType ft) {
  ft = ft == FILETYPE_AUTO ? getFileType(fn) : ft;
  assert(m == NULL);
  switch (ft) {
    case FILETYPE_GZ:
      m = new FilePrivGz;
      break;
    default:
      m = new FilePrivDef;
      break;
  }
  return m->open(fn, mode);
}

char * File::gets(char *buf, int n) {
  assert(m);
  return m->gets(buf, n);
}

void File::close() {
  assert(m);
  m->close();
  delete m;
  m = NULL;
}
