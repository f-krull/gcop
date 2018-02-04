#include "gcords.h"
#include "intervaltree.cpp"
#include "fieldformat.h"
#include "../util/tokenreader.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

GCord::GCord(std::vector<FieldType> fts) :
    Interval(0, 0), chr(ChrInfo::CTYPE_UNDEFINED) {
  m_d.reserve(fts.size());
  for (uint32_t i = 0; i < fts.size(); i++) {
#if __cplusplus >= 201103L
    m_d.emplace_back(fts[i]);
#else
    m_d.push_back(FieldValue(fts[i]));
#endif
  }
}

/*----------------------------------------------------------------------------*/

std::vector<GCord> GCords::getChr(ChrInfo::CType ct) const {
  std::vector<GCord> cseg; //TODO: optimize
  /* TODO: optimize */
  for (uint32_t i = 0; i < m_d.size(); i++) {
    if (m_d[i].chr == ct) {
      cseg.push_back(m_d[i]);
    }
  }
  return cseg;
}

/*----------------------------------------------------------------------------*/

class ChrConv : public TokenReader::IConv {
public:
  ChrConv(const ChrInfo *ci) : m_t(ChrInfo::CTYPE_UNDEFINED), m_ci(ci) {};
  virtual ~ChrConv() {};
  virtual void operator()(const char* s) {
    m_t = m_ci->str2type(s);
  }
  ChrInfo::CType get() const {
    return m_t;
  }
private:
  ChrInfo::CType m_t;
  const ChrInfo *m_ci;
};

/*----------------------------------------------------------------------------*/

char* read_token(char *pos, char delim, const FieldFormat::Entry &fte, GCord *s, ChrConv &cc) {
  assert(fte.idx <= s->nCols());
  switch (fte.type) {
    case FIELD_TYPE_GCCHR:
      {
         char *end = TokenReader::read_conv(pos, delim, &cc);
         s->chr = cc.get();
         return end;
      }
      break;
    case FIELD_TYPE_GCBPS:
      return TokenReader::read_uint64(pos, delim, &s->s);
      break;
    case FIELD_TYPE_GCBPE:
      return TokenReader::read_uint64(pos, delim, &s->e);
      break;
    case FIELD_TYPE_SKIP:
      return TokenReader::read_forget(pos, delim);
      break;
    case FIELD_TYPE_FLOAT:
      {
        char *end = TokenReader::read_float(pos, delim, &s->d(fte.idx).d().flt);
        return end;
      }
      break;
#if 0
    case FIELD_TYPE_CHR:
      {
        char *end = TokenReader::read_conv(pos, delim, &cc);
        a->push(ft.idx, cc.get());
        return end;
      }
      break;
    case FIELD_TYPE_UINT:
      {
        uint64_t v;
        char *end = TokenReader::read_uint64(pos, delim, &v);
        a->push(ft.idx, v);
        return end;
      }
      break;
    case FIELD_TYPE_STRING:
       {
         char *v;
         char *end = TokenReader::read_string(pos, delim, &v);
         a->push(ft.idx, v);
         return end;
       }
       break;
#endif
    default:
      assert(false);
      break;
  }
  return NULL;
}

/*----------------------------------------------------------------------------*/
#include <sstream>
std::string field2str(const FieldValue &f) {
  std::stringstream os;
  switch (f.type()) {
    case FIELD_TYPE_FLOAT:
      os << f.d().flt;
      break;
    default:
      fprintf(stderr, "error: fieldtype %s\n", fieldTypeStr[f.type()]);
      assert(false && "field type not implemented");
      exit(1);
      break;
  }
  return os.str();
}


#include "../util/file.h"
bool GCords::read(const char *filename, const char *fmt, uint32_t skip, const ChrInfo *chrinf) {
  const char delim = '\t';
  /* check format */
  const FieldFormat ffmt(fmt);
  const bool has_end = ffmt.hasField(FIELD_TYPE_GCBPE);
#if 0
  createFields(&m->annot, ft);
#endif
  ffmt.printFields();

  File f;
  if (!f.open(filename, "r", File::FILETYPE_AUTO)) {
    fprintf(stderr, "error: cannot open file %s\n", filename);
    exit(1);
  }
  ChrConv cc(chrinf);
  const uint32_t bufsize = 1024*64*8;
  char *buffer = new char[bufsize];

  uint64_t lineno = 0;
  while (f.gets(buffer, bufsize-1) != NULL) {
    lineno++;
    if (lineno <= skip) {
      continue;
    }
#if __cplusplus >= 201103L
    m_d.emplace_back(ffmt.types());
#else
    m_d.push_back(GCord(ffmt.types()));
#endif
    GCord &r = m_d.back();
    {
      /* change line end to delimiter to simplify parsing */
      assert(buffer[strlen(buffer)-1] == '\n');
      buffer[strlen(buffer)-1] = delim;
    }
    /* parse line */
    char *pos = buffer;
    for (uint32_t i = 0; i < ffmt.numEntries(); i++) {
      pos = read_token(pos, delim, ffmt.get(i), &r, cc);
    }
    /* no end? - assume we are reading points */
    if (!has_end) {
      r.e = r.s + 1;
    }
  }
  f.close();
  printf("read %lu genomic coordinate%s\n", m_d.size(), m_d.size() > 1 ? "s" : "");
#if 1
  {
    /* print header */
    printf("%s\t%s\t%s", "chr", "bps", "bpe");
    for (uint32_t j = 0; !cdata().empty() && j < cdata().front().nCols(); j++) {
      printf("\t%s", fieldTypeStr[cdata().front().d(j).type()]);
    }
    printf("\n");
    /* print head */
    const uint32_t num = 10;
    for (uint32_t i = 0; i < std::min(m_d.size(), (size_t)num); i++) {
      printf("%s\t%lu\t%lu", chrinf->ctype2str(m_d[i].chr), m_d[i].s, m_d[i].e);
      for (uint32_t j = 0; j < ncols(); j++) {
        printf("\t%s", field2str(cdata()[i].d(j)).c_str());
      }
      printf("\n");
    }
  }
#endif
  delete [] buffer;
  m_ci = *chrinf;
  return true;
}

/*----------------------------------------------------------------------------*/

void GCords::intersect(const GCords* gca, const GCords* gcb, GCords* gci) {
  assert(gci != NULL);
  gci->clear();
  std::vector<char> annot(gca->cdata().size(), 0);
  ChrInfo::CType chr_curr = ChrInfo::CTYPE_UNDEFINED;
  IntervalTree<GCord> *gcb_curr = NULL;
  for (uint32_t i = 0; i < gca->cdata().size(); i++) {
    if (chr_curr != gca->cdata()[i].chr) {
      chr_curr = gca->cdata()[i].chr;
      delete gcb_curr;
      gcb_curr = new IntervalTree<GCord>(gcb->getChr(chr_curr));
      printf("%s (%u)", gca->chrinfo().ctype2str(chr_curr), gcb_curr->numNodes());
    }
    std::vector<char> hasOverLap;
    annot[i] = gcb_curr->overlapsInterval_(gca->cdata()[i]);
    if (i % 10000 == 0) {
      printf(".");
      fflush(stdout);
    }
  }
  delete gcb_curr;
  printf("\n");
  return;
}

/*----------------------------------------------------------------------------*/

/* forbes: A and B */
static uint64_t overlapLen(const GCord &a, const std::vector<GCord> &bs) {
  uint64_t n = 0;
  for (uint32_t i = 0; i < bs.size(); i++) {
    n += Interval::numOverlap(a, bs[i]);
  }
  return n;
}

/* forbes: A and !B */
static uint64_t intersectLen(const GCord &a, const std::vector<GCord> &bs) {
  uint64_t n = a.len();
  for (uint32_t i = 0; i < bs.size(); i++) {
    n -= Interval::numOverlap(a, bs[i]);
  }
  return n;
}

static uint64_t calcAvsB(const std::vector<GCord> &ac,
                         const std::vector<GCord> &bc,
                         const IntervalTree<GCord> &bit,
                         uint64_t(*fun)(const GCord &, const std::vector<GCord>&)
                        ) {
  uint64_t res = 0;
  std::vector<GCord> bovac;
  for (uint64_t j = 0; j < ac.size(); j++) {
    /* get all overlapping intervals */
    std::vector<uint32_t> bova_idx;
    bit.overlapsInterval(ac[j], &bova_idx);
    /* to collection of gcords */
    bovac.clear();
    for (uint32_t k = 0; k < bova_idx.size(); k++) {
      bovac.push_back(bc[bova_idx[k]]);
    }
    /* compute length of fun */
    res += fun(ac[j], bovac);
  }
  return res;
}

/*----------------------------------------------------------------------------*/

void GCords::forbes(const GCords* gca, const GCords* gcb) {
/*
 * a = A  and B
 * b = A  and !B
 * c = !A and B
 * d = !B and !A
 *
 * forbes = a(a+b+c+d)/((a+b)(a+c))
 *
 */

  assert(gca->chrinfo() == gcb->chrinfo());
  const ChrInfo & chrinfo = gca->chrinfo();
  uint64_t r_a = 0;
  uint64_t r_b = 0;
  uint64_t r_c = 0;
  uint64_t r_d = 0;
  /* by chromosome */
  for (uint32_t i = 0; i < gca->m_ci.chrs().size(); i++) {
    ChrInfo::CType chr_curr = gca->m_ci.chrs()[i];
    printf("%s\n", gca->m_ci.ctype2str(chr_curr));
    std::vector<GCord> ac = gca->getChr(chr_curr);
    std::vector<GCord> bc = gcb->getChr(chr_curr);
    if (ac.empty() && bc.empty()) {
      continue;
    }
    IntervalTree<GCord> ait(ac);
    IntervalTree<GCord> bit(bc);
    r_a += calcAvsB(ac, bc, bit, overlapLen);
    r_b += calcAvsB(ac, bc, bit, intersectLen);
    r_c += calcAvsB(bc, ac, ait, intersectLen);
    r_d += chrinfo.chrlen(chr_curr) - r_a - r_b - r_c;
  }
  printf("a: %lu\n", r_a);
  printf("b: %lu\n", r_b);
  printf("c: %lu\n", r_c);
  printf("d: %lu\n", r_d);

  float num = r_a * (r_a + r_b + r_c + r_d);
  float den = (r_a*r_b)*(r_a*r_c);
  printf("forbes: %f\n", den > 0 ? num/den : 0);
}

