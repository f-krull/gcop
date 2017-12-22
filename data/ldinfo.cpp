#include "ldinfo.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <stdlib.h>
#include <algorithm>

/*----------------------------------------------------------------------------*/

#define READ_FIELD_ULL(dst,src,len,offset,wd) \
   if (offset + wd <= len) { \
      dst = strtoull(&src[offset], NULL, 10); \
   } else { \
      dst = 0; \
   }

#define READ_FIELD_FLOAT(dst,src,len,offset,wd) \
   if (offset + wd <= len) { \
      dst = strtof(&src[offset], NULL); \
   } else { \
      dst = 0; \
   }

/*----------------------------------------------------------------------------*/

struct LdDataEntry {
  uint64_t i;
  uint64_t j;
  float v;
  LdDataEntry(uint64_t bpa, uint64_t bpb, float r2) : i(bpa), j(bpb), v(r2) {}
  static bool cmp(const LdDataEntry& a, const LdDataEntry& b) {
    return a.i < b.i || (a.i == b.i && a.j < b.j);
  }

  class Cmp {
  public:
    bool operator()(const std::pair<uint64_t, uint64_t> &p,
                    const LdDataEntry &l) const {
      return p.first < l.i || (p.first == l.i && p.second < l.j);
    }
    bool operator()(const LdDataEntry &l,
                    const std::pair<uint64_t, uint64_t> &p) const {
      return l.i < p.first || (l.i == p.first && l.j < p.second);
    }
  };
};

/*----------------------------------------------------------------------------*/

class LdDataList {
public:
  void add(uint64_t bpa, uint64_t bpb, float v) {
    assert(bpa != bpb);
    m_list.push_back(LdDataEntry(bpa, bpb, v));
    m_list.push_back(LdDataEntry(bpb, bpa, v));
  }
  void finalize() {
    std::sort(m_list.begin(), m_list.end(), LdDataEntry::cmp);
  }

  void print() {
    for (uint32_t i = 0; i < std::min(m_list.size(), size_t(10)); i++) {
      printf("%lu %lu %f\n", m_list[i].i, m_list[i].j, m_list[i].v);
    }
  }

  float getLd(uint64_t bpa, uint64_t bpb) const {
    std::vector<LdDataEntry>::const_iterator it = std::lower_bound(
        m_list.begin(), m_list.end(), std::make_pair(bpa, bpb),
        LdDataEntry::Cmp());
    if (it == m_list.end()) {
      return R2_UNDEFINED;
    }
    /* lower_bound finds the first element not being smaller */
    if (it->i != bpa || it->j != bpb) {
      return R2_UNDEFINED;
    }
    return it->v;
  }

private:
  static float R2_UNDEFINED;
  std::vector<LdDataEntry> m_list;
};

float LdDataList::R2_UNDEFINED = -1.0f;

/*----------------------------------------------------------------------------*/

LdInfo::~LdInfo() {
  for (LdlMap::iterator it = m_lddat.begin(); it != m_lddat.end(); ++it) {
    delete it->second;
  }
}

/*----------------------------------------------------------------------------*/

void LdInfo::read(const char *fn) {
  ChrMap cm;
  const uint32_t skip = 1; /* skip header */
  FILE *f = fopen(fn, "r");
  if (f == NULL) {
    fprintf(stderr, "error: cannot open file '%s'\n", fn);
    exit(1);
  }
  char line[1024];

  uint64_t chra = 0;
  uint64_t bpa = 0;
  uint64_t chrb = 0;
  uint64_t bpb = 0;
  float    r2;
  uint32_t line_no = 0;
  uint64_t num_read = 0;
  while (fgets(line, sizeof(line)-1, f) != NULL) {
    line_no++;
    if (line_no <= skip) {
      continue;
    }
    /* fixed width file format */
    size_t len = strlen(line);
    READ_FIELD_ULL  (chra, line, len,  0, 6 );
    READ_FIELD_ULL  (bpa,  line, len,  6, 13);
    READ_FIELD_ULL  (chrb, line, len, 31, 6 );
    READ_FIELD_ULL  (bpb,  line, len, 38, 13);
    READ_FIELD_FLOAT(r2,   line, len, 63, 13);
    assert(chra == chrb && "CHR_A != CHR_B");
    //printf("%lu %lu %lu %lu %.6f\n", chra, bpa, chrb, bpb, r2);
    addEntry(chra, bpa, bpb, r2, cm);
    num_read++;
  }
  /* sort data */
  for (LdlMap::iterator it = m_lddat.begin(); it != m_lddat.end(); ++it) {
    it->second->finalize();
    //printf("chr %d\n", it->first);
    //it->second->print();
  }
  printf("LD entries read: %lu\n", num_read);
  fclose(f);
}

/*----------------------------------------------------------------------------*/

void LdInfo::addEntry(uint64_t chr, uint64_t bpa, uint64_t bpb, float r2,
    const ChrMap &cm) {
  ChrMap::ChrType c = cm.unifyChr(chr);
  LdlMap::iterator it = m_lddat.find(c);
    if (it == m_lddat.end()) {
      m_lddat.insert(std::make_pair(c, new LdDataList));
      it = m_lddat.find(c);
    }
    assert(it != m_lddat.end());
    it->second->add(bpa, bpb, r2);
  }

/*----------------------------------------------------------------------------*/

float LdInfo::getLd(const char* c, uint64_t bpa, uint64_t bpb) const {
  ChrMap cm;
  ChrMap::ChrType ct = cm.unifyChr(c);
  float ld = getLd(ct, bpa, bpb);
  printf("%s %lu %lu %f\n", c, bpa, bpb, ld);
  return ld;
}

/*----------------------------------------------------------------------------*/

float LdInfo::getLd(ChrMap::ChrType ct, uint64_t bpa, uint64_t bpb) const {
  const LdDataList* ldl = find(ct);
  if (ldl == NULL) {
    fprintf(stderr, "error: no ld info for chr '%d'\n", ct);
    exit(1);
  }
  return ldl->getLd(bpa, bpb);
}

/*----------------------------------------------------------------------------*/

LdDataList* LdInfo::find(ChrMap::ChrType c) {
  LdlMap::iterator it = m_lddat.find(c);
  return it->second;
}

/*----------------------------------------------------------------------------*/

const LdDataList* LdInfo::find(ChrMap::ChrType c) const {
  LdlMap::const_iterator it = m_lddat.find(c);
  return it->second;
}

/*----------------------------------------------------------------------------*/
#include "gcords.h"
void LdInfo::test(const GCords *s) const {
  uint64_t num_skipped = 0;
  uint64_t num_tested = 0;
  uint64_t num_found = 0;
  uint64_t num_snps = 0;
  const uint32_t num_neigbors = 1;
  for (uint32_t i = num_neigbors; i < s->cdata().size(); i++) {
    num_snps++;
    for (uint32_t j = 1; j <= num_neigbors; j++) {
      ChrMap::ChrType chra = s->cdata()[i-j].chr;
      uint64_t        bpa  = s->cdata()[i-j].s;
      ChrMap::ChrType chrb = s->cdata()[i].chr;
      uint64_t        bpb  = s->cdata()[i].s;
      if (chra != chrb) {
        continue;
      }
      float ld = getLd(chra, bpa, bpb);
      /* coutn if ld is -1 */
      num_skipped += (ld < 0 ? 1 : 0);
      num_found += (ld < 0 ? 0 : 1);
      num_tested++;
    }
    //printf("r2: chr %d %lu %lu %.5f\n", chra, bpa, bpb, ld);
  }
  printf("SNPs\tnum_snps=%lu\tnum_tested=%lu\tnum_found=%lu\tnum_skipped=%lu\n", num_snps, num_tested, num_found, num_skipped);
}
