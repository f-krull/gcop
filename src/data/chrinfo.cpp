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
  ChrInfoPriv() : len(0) {}
  typedef std::map<ChrInfo::CType, uint64_t>    MapLengths;
  typedef std::map<std::string, ChrInfo::CType> MapStr2type;
  MapLengths lenghts;
  MapStr2type str2type;
  std::vector<std::string> strs;
  std::vector<ChrInfo::CType> cts;
  uint64_t len;

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

ChrInfo::ChrInfo(const ChrInfo &c) {
  m = new ChrInfoPriv(*c.m);
}

/*----------------------------------------------------------------------------*/

ChrInfo & ChrInfo::operator=(const ChrInfo &o) {
  delete m;
  m = new ChrInfoPriv(*o.m);
  return *this;
}

/*----------------------------------------------------------------------------*/

ChrInfo::CType ChrInfo::CTYPE_UNDEFINED = UINT8_MAX;

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
  const uint32_t bufsize = 1024;
  char buffer[bufsize];
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
    addEntry(chrIdStr, chrLen);
  }
}

/*----------------------------------------------------------------------------*/

void ChrInfo::addEntry(const char *chrIdStr, uint64_t chrLen) {
  const uint32_t currChrId = m->strs.size();
  m->lenghts.insert(std::make_pair(currChrId, chrLen));
  m->str2type.insert(std::make_pair(chrIdStr, currChrId));
  m->cts.push_back(currChrId);
  m->len += chrLen;
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

uint64_t ChrInfo::len() const {
  return m->len;
}

/*----------------------------------------------------------------------------*/

const char * ChrInfo::ctype2str(CType t) const {
  return m->strs[t].c_str();
}

/*----------------------------------------------------------------------------*/

ChrInfo::CType ChrInfo::str2type(const char *str) const {
  /* no chr prefix? */
  if (str[0] == 'c' || str[0] == '\0') {
    return str2typeStrict(str);
  }
  CType ret;
  const char chrpfx[] = "chr";
  char * sc = new char[strlen(chrpfx)+strlen(str)+1];
  strcpy(sc, chrpfx);
  strcat(sc, str);
  ret = str2typeStrict(sc);
  delete [] sc;
  return ret;
}

/*----------------------------------------------------------------------------*/

ChrInfo::CType ChrInfo::str2typeStrict(const char *str) const {
  const ChrInfoPriv::MapStr2type::const_iterator it = m->str2type.find(str);
  if (it == m->str2type.end()) {
    return CTYPE_UNDEFINED;
  }
  return it->second;
}

/*----------------------------------------------------------------------------*/

bool ChrInfo::operator==(const ChrInfo &b) const {
  bool ret = true;
  for (uint32_t i = 0; ret && i < m->strs.size(); i++) {
    std::string a_s = m->strs[i];
    CType       a_t = str2type(a_s.c_str());
    ret = ret && a_t == b.str2type(a_s.c_str());
    ret = ret && chrlen(a_t) == b.chrlen(a_t);
  }
  return ret;
}

/*----------------------------------------------------------------------------*/

const std::vector<ChrInfo::CType> & ChrInfo::chrs() const {
  return m->cts;
}

/*----------------------------------------------------------------------------*/

uint32_t ChrInfo::numchrs() const {
  return m->lenghts.size();
}

/*----------------------------------------------------------------------------*/

ChrInfoHg19::ChrInfoHg19() : ChrInfo() {
  // cat testdata/hg19/hg19.chrom.sizes_common.txt | awk '{printf "addEntry(\"%s\", %s);\n", $1, $2}'
  addEntry("chr1",  249250621);
  addEntry("chr2",  243199373);
  addEntry("chr3",  198022430);
  addEntry("chr4",  191154276);
  addEntry("chr5",  180915260);
  addEntry("chr6",  171115067);
  addEntry("chr7",  159138663);
  addEntry("chr8",  146364022);
  addEntry("chr9",  141213431);
  addEntry("chr10", 135534747);
  addEntry("chr11", 135006516);
  addEntry("chr12", 133851895);
  addEntry("chr13", 115169878);
  addEntry("chr14", 107349540);
  addEntry("chr15", 102531392);
  addEntry("chr16", 90354753);
  addEntry("chr17", 81195210);
  addEntry("chr18", 78077248);
  addEntry("chr19", 59128983);
  addEntry("chr20", 63025520);
  addEntry("chr21", 48129895);
  addEntry("chr22", 51304566);
  addEntry("chrX",  155270560);
  addEntry("chrY",  59373566);
}
