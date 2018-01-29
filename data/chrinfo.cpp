#include "chrinfo.h"
#include "../util/file.h"
#include "../util/tokenreader.h"
#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/

class ChrInfoPriv {
public:
  typedef std::map<ChrInfo::CType, uint64_t>    MapLengths;
  typedef std::map<std::string, ChrInfo::CType> MapStr2type;
  MapLengths lenghts;
  MapStr2type str2type;
  std::vector<std::string> strs;
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

    const uint32_t currChrId = m->strs.size();
    m->lenghts.insert(std::make_pair(currChrId, chrLen));
    m->str2type.insert(std::make_pair(chrIdStr, currChrId));
#if 0
    /* skip "chr" in "chrX.." */
    assert(chrIdStr != NULL);
    assert(strlen(chrIdStr) > 3);
    /* check "chr" prefix */
    assert(chrIdStr[0] == 'c' && chrIdStr[1] == 'h' && chrIdStr[2] == 'r');
    m->id2type.insert(std::make_pair(&chrIdStr[3], currChrId));
#endif
    m->strs.push_back(chrIdStr);
  }
  delete buffer;
}

/*----------------------------------------------------------------------------*/

void ChrInfo::print() const {
  /* print */
  printf("chromosome lengths:\n");
  for (uint32_t i = 0; i < m->strs.size(); i++) {
    /* test chaining */
    printf("%u\t",  str2type(m->strs[i].c_str()));
    printf("%s\t",  ctype2str(str2type(m->strs[i].c_str())));
    printf("%lu\n", chrlen(str2type(m->strs[i].c_str())));
  }
}

/*----------------------------------------------------------------------------*/

uint64_t ChrInfo::chrlen(CType t) const {
  const ChrInfoPriv::MapLengths::const_iterator it = m->lenghts.find(t);
  assert(it != m->lenghts.end());
  return it->second;
}

/*----------------------------------------------------------------------------*/

const char * ChrInfo::ctype2str(CType t) const {
  return m->strs[t].c_str();
}

/*----------------------------------------------------------------------------*/

ChrInfo::CType ChrInfo::str2type(const char *str) const {
  const ChrInfoPriv::MapStr2type::const_iterator it = m->str2type.find(str);
  assert(it != m->str2type.end());
  return it->second;
}

