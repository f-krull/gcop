#include "gcords.h"
#include "intervaltree.cpp"
#include "fieldformat.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tokenreader.h"

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

const std::vector<GCord> & GCords::getChr(ChrInfo::CType ct) const {
  return m_d[ct];
}

/*----------------------------------------------------------------------------*/

class ChrConv : public TokenReader::IConv {
public:
  ChrConv(const ChrInfo *ci, bool allowUndefChr) :
      m_t(ChrInfo::CTYPE_UNDEFINED), m_ci(ci), m_allowUndefined(allowUndefChr) {
  }
  virtual ~ChrConv() {};
  virtual void operator()(const char* s) {
    m_t = m_ci->str2type(s);
    if (!m_allowUndefined && m_t == m_ci->CTYPE_UNDEFINED) {
      fprintf(stderr, "error: chromosome \'%s\' not defined\n", s);
      m_ci->print();
      exit(1);
    }
  }
  ChrInfo::CType get() const {
    return m_t;
  }
private:
  ChrInfo::CType m_t;
  const ChrInfo *m_ci;
  const bool m_allowUndefined;
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


#include "../shared/file.h"
bool GCords::read(const char *filename, const char *fmt, uint32_t skip, const ChrInfo *chrinf, bool allowUndefChr) {
  m_ci = *chrinf;
  const char delim = '\t';
  /* check format */
  const FieldFormat ffmt(fmt);
  const bool has_end = ffmt.hasField(FIELD_TYPE_GCBPE);
#if 0
  createFields(&m->annot, ft);
#endif
  ffmt.printFields();

  m_d.resize(chrinf->numchrs());

  File f;
  if (!f.open(filename, "r", File::FILETYPE_AUTO)) {
    fprintf(stderr, "error: cannot open file %s\n", filename);
    exit(1);
  }
  ChrConv cc(chrinf, allowUndefChr);
  const uint32_t bufsize = 1024*64*8;
  char *buffer = new char[bufsize];

  uint64_t lineno = 0;
  uint64_t num_ignored = 0;
  while (f.gets(buffer, bufsize-1) != NULL) {
    lineno++;
    if (lineno <= skip) {
      continue;
    }
    GCord r(ffmt.types());
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
    if (r.chr == m_ci.CTYPE_UNDEFINED) {
      num_ignored++;
      continue;
    }
    m_d[r.chr].push_back(r);
  }
  f.close();
#if 0
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
#else
#endif
  delete [] buffer;
  rebuild();
  printf("read %lu genomic coordinate%s from %s\n", numgc(), numgc() > 1 ? "s" : "", filename);
  if (num_ignored) {
    printf("warning - ignoring %lu coordinates on undefined chromosomes\n", num_ignored);
  }
  const uint32_t num = 10;
  write(stdout, std::min(m_d.size(), (size_t)num));
  return true;
}

/*----------------------------------------------------------------------------*/

bool GCords::write(FILE *f, uint64_t maxlines) const {
  const char sep = '\t';
  uint64_t n = 0;
  for (auto chr_it = begin(); chr_it != end(); ++chr_it) {
    for (auto gc_it = chr_it->begin(); gc_it != chr_it->end(); ++gc_it) {
      if (maxlines != 0 && n >= maxlines) {
        break;
      }
      const GCord &g = *gc_it;
      fprintf(f, "%s%c%lu%c%lu", m_ci.ctype2str(g.chr), sep, g.s, sep, g.e);
      for (uint32_t j = 0; j < ncols(); j++) {
        fprintf(f, "\t%s", field2str(g.d(j)).c_str());
      }
      fprintf(f, "\n");
      n++;
    }
  }
  return true;
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
#if 0
/* forbes: A and !B */
static uint64_t intersectLen(const GCord &a, const std::vector<GCord> &bs) {
  uint64_t n = a.len();
  for (uint32_t i = 0; i < bs.size(); i++) {
    n -= Interval::numOverlap(a, bs[i]);
  }
  return n;
}
#endif

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

static uint64_t calcSum(const std::vector<GCord> &ac) {
  uint64_t sum = 0;
  for (uint32_t i = 0; i < ac.size(); i++) {
    sum += ac[i].len();
  }
  return sum;
}

/*----------------------------------------------------------------------------*/

void GCords::forbes(const GCords* gca, const GCords* gcb) {
 /*
  * forbes = N * |A and B| / ( |A| * |B| )
  */
  assert(gca->chrinfo() == gcb->chrinfo());
  const ChrInfo & chrinfo = gca->chrinfo();
  uint64_t n_ab = 0;
  uint64_t n_a = 0;
  uint64_t n_b = 0;
  uint64_t n_c = 0;
  /* by chromosome */
  for (uint32_t i = 0; i < gca->m_ci.chrs().size(); i++) {
    ChrInfo::CType chr_curr = gca->m_ci.chrs()[i];
    printf("%s\n", gca->m_ci.ctype2str(chr_curr));
    std::vector<GCord> ac = gca->getChr(chr_curr);
    std::vector<GCord> bc = gcb->getChr(chr_curr);
    n_c += chrinfo.chrlen(chr_curr);
    if (ac.empty() && bc.empty()) {
      continue;
    }
    IntervalTree<GCord> bit(bc);
    n_ab += calcAvsB(ac, bc, bit, overlapLen);
    n_a += calcSum(ac);
    n_b += calcSum(bc);
  }
  printf("ab: %lu\n", n_ab);
  printf("a: %lu\n", n_a);
  printf("b: %lu\n", n_b);

  float num = n_c * n_ab;
  float den = n_a * n_b;
  float ret = den > 0 ? num/den : 0;
  printf("forbes: %f\n", ret);
}

/*----------------------------------------------------------------------------*/

void GCords::expand(uint64_t len) {
  if (!len) {
    return;
  }
  for (auto chr_it = begin(); chr_it != end(); ++chr_it) {
    for (auto gc_it = chr_it->begin(); gc_it != chr_it->end(); ++gc_it) {
      GCord &g = *gc_it;
      const uint64_t chrlen = chrinfo().chrlen(g.chr);
      g.s = (g.s > len) ? g.s - len : 0;
      g.e = std::min(chrlen, g.e + len);
    }
  }
}

/*----------------------------------------------------------------------------*/

void GCords::flatten() {
  /* for each chromosome */
  for (auto chr_it = begin(); chr_it != end(); ++chr_it) {
    std::vector<GCord> gcs_new;
    std::vector<char> merged(chr_it->size(), false);
    const std::vector<GCord> &gcs = *chr_it;
    /* for each unmerged gc */
    for (uint32_t i = 0; i < gcs.size(); i++) {
      if (merged[i] == true) {
        continue;
      }
      GCord gm = gcs[i];
      /* is next start inside merged? */
      for (uint32_t j = i+1; j < gcs.size(); j++) {
        /* no - abort */
        if (gcs[j].s >= gm.e) {
          break;
        }
        /* yes - merge too */
        gm.e = std::max(gm.e, gcs[j].e);
        merged[j] = true;
      }
      /* add merged */
      gcs_new.push_back(gm);
    }
    /* update chr */
    chr_it->swap(gcs_new);
  }
  rebuild();
}

/*----------------------------------------------------------------------------*/

void GCords::toPoints() {
  /* for each chromosome */
  for (auto chr_it = begin(); chr_it != end(); ++chr_it) {
    std::vector<GCord> &gcs = *chr_it;
    /* for each gc */
    for (uint32_t i = 0; i < gcs.size(); i++) {
      const uint64_t mid = (gcs[i].e + gcs[i].s) / 2;
      gcs[i].s = mid;
      gcs[i].e = mid + 1;
    }
  }
}

/*----------------------------------------------------------------------------*/


void GCords::rebuild() {
  for (auto it = begin(); it != end(); ++it) {
    /* sort by starts */
    std::sort(it->begin(), it->end());
  }
}

/*----------------------------------------------------------------------------*/

uint64_t GCords::numgc() const {
  uint64_t sum = 0;
  for (auto it = begin(); it != end(); ++it) {
    sum += it->size();
  }
  return sum;
}
