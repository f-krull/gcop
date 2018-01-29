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
    Interval(0, 0), chr(ChrMap::CHRTYPE_NUMENTRIES) {
  m_d.reserve(fts.size());
  for (uint32_t i = 0; i < fts.size(); i++) {
    //m_d.push_back(FieldValue(fts[i]));
    m_d.emplace_back(fts[i]);
  }
}

/*----------------------------------------------------------------------------*/

std::vector<GCord> GCords::getChr(ChrMap::ChrType ct) const {
  std::vector<GCord> cseg;
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
  ChrConv() : m_t(ChrMap::CHRTYPE_NUMENTRIES) {};
  virtual ~ChrConv() {};
  virtual void operator()(const char* s) {
    m_t = m_cm.unifyChr(s);
  }
  ChrMap::ChrType get() const {
    return m_t;
  }
private:
  ChrMap::ChrType m_t;
  ChrMap m_cm;
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
bool GCords::read(const char *filename, const char *fmt, uint32_t skip) {
  ChrMap chrmap;
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
  ChrConv cc;
  const uint32_t bufsize = 1024*64*8;
  char *buffer = new char[bufsize];

  uint64_t lineno = 0;
  while (f.gets(buffer, bufsize-1) != NULL) {
    lineno++;
    if (lineno <= skip) {
      continue;
    }
    m_d.emplace_back(ffmt.types());
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
    //m_d.push_back(r);
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
      printf("%s\t%lu\t%lu", chrmap.chrTypeStr(m_d[i].chr), m_d[i].s, m_d[i].e);
      for (uint32_t j = 0; j < ncols(); j++) {
        printf("\t%s", field2str(cdata()[i].d(j)).c_str());
      }
      printf("\n");
    }
  }
#endif
  delete [] buffer;
  return true;
}

#if 0
/*----------------------------------------------------------------------------*/

GCords::RowPtr GCords::get(uint64_t i) {
 return RowPtr(m_d[i], this, i);
}

/*----------------------------------------------------------------------------*/

float GCords::annotFloat(uint32_t annotIdx, uint64_t rowIdx) const {
  return m->annot.getFloat(annotIdx, rowIdx);
}

/*----------------------------------------------------------------------------*/

ChrMap::ChrType GCords::annotChr(uint32_t annotIdx, uint64_t rowIdx) const {
  return m->annot.getChr(annotIdx, rowIdx);
}

/*----------------------------------------------------------------------------*/

uint64_t GCords::annotUint(uint32_t annotIdx, uint64_t rowIdx) const {
  return m->annot.getUint(annotIdx, rowIdx);
}

/*----------------------------------------------------------------------------*/

const char* GCords::annotStr(uint32_t annotIdx, uint64_t rowIdx) const {
  return m->annot.str(annotIdx, rowIdx);
}

/*----------------------------------------------------------------------------*/

uint32_t GCords::numAnnot() const {
  return m->annot.num();
}
#endif

/*----------------------------------------------------------------------------*/

void GCords::intersect(const GCords* gca, const GCords* gcb, GCords* gci) {
  assert(gci != NULL);
  gci->clear();
  std::vector<char> annot(gca->cdata().size(), 0);
  ChrMap::ChrType chr_curr = ChrMap::CHRTYPE_NUMENTRIES;
  IntervalTree<GCord> *gcb_curr = NULL;
  for (uint32_t i = 0; i < gca->cdata().size(); i++) {
    if (chr_curr != gca->cdata()[i].chr) {
      chr_curr = gca->cdata()[i].chr;
      delete gcb_curr;
      gcb_curr = new IntervalTree<GCord>(gcb->getChr(chr_curr));
      printf("%s (%u)", ChrMap::chrTypeStr(chr_curr), gcb_curr->numNodes());
    }
    std::vector<char> hasOverLap;
    annot[i] = gcb_curr->overlaps(gca->cdata()[i]);
    if (i % 10000 == 0) {
      printf(".");
      fflush(stdout);
    }
  }
  delete gcb_curr;
  printf("\n");
  return;
}

/*
 * a = A  and B
 * b = A  and !B
 * c = !A and B
 * d = !B and !A
 *
 * forbes = a(a+b+c+d)/((a+b)(a+c))
 *
 */
