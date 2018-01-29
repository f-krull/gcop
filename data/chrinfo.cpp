#include "chrinfo.h"
#include "../util/file.h"
#include "../util/tokenreader.h"
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/

class ChrInfoPriv {
public:
  std::map<ChrInfo::CType, uint64_t>    lenghts;
  std::map<std::string, ChrInfo::CType> str2type;
  std::map<std::string, ChrInfo::CType> id2type;
};

/*----------------------------------------------------------------------------*/

ChrInfo::ChrInfo() {
  m = new ChrInfoPriv();
}

/*----------------------------------------------------------------------------*/

ChrInfo::~ChrInfo() {
  delete m;
}

/*----------------------------------------------------------------------------*/

/* format: chrid\tlength\n
 *         str    uint64
 * */
void ChrInfo::read(const char *fn) {
  uint8_t currChrId = 0;
  File f;
  if (!f.open(fn, "r", File::FILETYPE_AUTO)) {
    fprintf(stderr, "error: cannot open file %s\n", fn);
    exit(1);
  }
  const uint32_t bufsize = 1024*64*8;
  char *buffer = new char[bufsize];
  const char delim = '\t';
  while (f.gets(buffer, bufsize-1) != NULL) {
    char *pos = buffer;
    {
      /* change line end to delimiter to simplify parsing */
      assert(buffer[strlen(buffer)-1] == '\n');
      buffer[strlen(buffer)-1] = delim;
    }
    char *chrIdStr;
    pos = TokenReader::read_string(pos, delim, &chrIdStr);
    uint64_t chrLen;
    pos = TokenReader::read_uint64(pos, delim, &chrLen);

    m->lenghts.insert(std::make_pair(currChrId, chrLen));
    m->str2type.insert(std::make_pair(chrIdStr, currChrId));
    /* skip "chr" in "chrX.." */
    assert(chrIdStr != NULL);
    assert(strlen(chrIdStr) > 3);
    assert(chrIdStr[0] == 'c' && chrIdStr[1] == 'h' && chrIdStr[2] == 'r');
    m->id2type.insert(std::make_pair(&chrIdStr[3], currChrId));
    currChrId++;
  }
  delete buffer;

  /* print */
  std::map<std::string, ChrInfo::CType>::const_iterator it;
  printf("chromosome lengths:\n");
  for (it = m->str2type.begin(); it != m->str2type.end(); it++) {
    printf("%s %lu\n", it->first.c_str(), getLen(it->second));
  }
}

/*----------------------------------------------------------------------------*/

uint64_t ChrInfo::getLen(CType t) const {
  const std::map<CType, uint64_t>::const_iterator it = m->lenghts.find(t);
  assert(it != m->lenghts.end());
  return it->second;
}
#if 0
ChrInfo::CType ChrInfo::str2type(const char *str) const {

}
ChrInfo::CType ChrInfo::id2type(const char *str) const {
}
#endif
