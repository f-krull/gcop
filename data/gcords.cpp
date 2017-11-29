#include "gcords.h"
#include "tokenreader.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

#define CHAR_CHROM 'c'
#define CHAR_START 's'
#define CHAR_END   'e'
#define CHAR_SKIP  '.'

#define FIELD_TYPE_SKIP  0x0000
#define FIELD_TYPE_CHR   0x0001
#define FIELD_TYPE_BPS   0x0002
#define FIELD_TYPE_BPE   0x0003
#define FIELD_TYPE_FLOAT 0x0004

/*----------------------------------------------------------------------------*/

GCords::RowPtr::RowPtr(const GCord &g, GCords *p, uint64_t idx) :
    GCord(g), m_p(p), m_ridx(idx) {
}

/*----------------------------------------------------------------------------*/

float GCords::RowPtr::getFloat(uint32_t i) {
  return m_p->annotFloat(i, m_ridx);
}

/*----------------------------------------------------------------------------*/

class Annot {
public:
  virtual ~Annot() {};
  static const char* typestr() {return "null";}
private:
};

/*----------------------------------------------------------------------------*/

class AnnotFloat : public Annot {
public:
  static const char* typestr() {return "float";}
  const std::vector<float> & d() const {return m_d;}
  std::vector<float> & d() {return m_d;}
private:
  std::vector<float> m_d;
};

/*----------------------------------------------------------------------------*/

class Annots {
public:
  ~Annots() {
    for (std::map<uint32_t, Annot*>::iterator it = m_annots.begin();
        it != m_annots.end(); ++it) {
      delete it->second;
    }
  }


  float getFloat(uint32_t aIdx, uint64_t rowIdx) const {
    const AnnotFloat *a = getAnnot<AnnotFloat>(aIdx);
    assert(a != NULL && "check error handling");
    return a->d()[rowIdx];
  }

  void create(uint32_t idx, uint32_t type) {
    switch (type) {
      case FIELD_TYPE_FLOAT:
        m_annots.insert(std::make_pair(idx, new AnnotFloat));
        break;
      default:
        fprintf(stderr, "error: cannot create field of type %x(%u)", type, type);
        exit(1);
        break;
    }
  }

  void push(uint32_t aIdx, float v) {
    AnnotFloat *a = getAnnot<AnnotFloat>(aIdx);
    assert(a != NULL);
    a->d().push_back(v);
  }

private:
  template <typename T>
  const T* getAnnot(uint32_t aIdx) const {
    AnnotMap::const_iterator it = m_annots.find(aIdx);
    if (it == m_annots.end()) {
      fprintf(stderr, "error: can not find column %d\n", aIdx);
      exit(1);
    }
    T *o = dynamic_cast<T*>(it->second);
    if (o == NULL) {
      fprintf(stderr, "error: column %d is not of type %s\n", aIdx, T::typestr());
      exit(1);
    }
    return o;
  }

  template <typename T>
  T* getAnnot(uint32_t aIdx) {
    AnnotMap::iterator it = m_annots.find(aIdx);
    if (it == m_annots.end()) {
      fprintf(stderr, "error: can not find column %d\n", aIdx);
      exit(1);
    }
    T *o = dynamic_cast<T*>(it->second);
    if (o == NULL) {
      fprintf(stderr, "error: column %d is not of type %s\n", aIdx, T::typestr());
      exit(1);
    }
    return o;
  }

  typedef std::map<uint32_t, Annot*> AnnotMap;
  AnnotMap m_annots;
};



/*----------------------------------------------------------------------------*/

class GCordsPriv {
public:
  Annots annot;
};

/*----------------------------------------------------------------------------*/

GCords::GCords() {
  m = new GCordsPriv;
}

/*----------------------------------------------------------------------------*/

GCords::~GCords() {
  delete m;
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

static bool has_field(const char *fmt, char c) {
  return strchr(fmt, c) != NULL;
}

/*----------------------------------------------------------------------------*/

struct FieldType {
  uint32_t type;
  uint32_t idx;
  FieldType(uint32_t t, uint32_t i = 0) : type(t), idx(i) {
  }
};

std::vector<FieldType> getFields(const char *fmt) {
  std::vector<FieldType> fieldTypes;
  for (; fmt[0] != '\0'; fmt++) {
    switch (fmt[0]) {
    case CHAR_CHROM:
      fieldTypes.push_back(FieldType(FIELD_TYPE_CHR));
      break;
    case CHAR_START:
      fieldTypes.push_back(FieldType(FIELD_TYPE_BPS));
      break;
    case CHAR_END:
      fieldTypes.push_back(FieldType(FIELD_TYPE_BPE));
      break;
    case CHAR_SKIP:
      fieldTypes.push_back(FieldType(FIELD_TYPE_SKIP));
      break;
    default:
      {
        /* check if token is a number */
        char *end;
        uint64_t v = strtoul(fmt, &end, 10);
        if (fmt != end) {
          fmt = end;
          fieldTypes.push_back(FieldType(FIELD_TYPE_FLOAT, v));
          continue;
        }
        /* unknown field type */
        fprintf(stderr, "error: unknown format '%s'\n", fmt);
        exit(1);
      }
      break;
    }
  }
  return fieldTypes;
}

/*----------------------------------------------------------------------------*/

static void createFields(Annots* a, const std::vector<FieldType> & fields) {
  for (uint32_t i = 0; i < fields.size(); i++) {
    if (fields[i].type == FIELD_TYPE_FLOAT) {
      a->create(fields[i].idx, fields[i].type);
    }
  }
}

/*----------------------------------------------------------------------------*/

char* read_token(char *pos, char delim, const FieldType &ft, GCord *s, Annots *a, const TokenReader &tr) {
  switch (ft.type) {
    case FIELD_TYPE_CHR:
      return tr.read_chr(pos, delim, &s->chr);
      break;
    case FIELD_TYPE_BPS:
      return tr.read_uint64(pos, delim, &s->s);
      break;
    case FIELD_TYPE_BPE:
      return tr.read_uint64(pos, delim, &s->e);
      break;
    case FIELD_TYPE_SKIP:
      return tr.read_forget(pos, delim);
      break;
    case FIELD_TYPE_FLOAT:
      {
        float v;
        char *end = tr.read_float(pos, delim, &v);
        a->push(ft.idx, v);
        return end;
      }
      break;
    default:
      assert(false);
      break;
  }
  return NULL;
}

/*----------------------------------------------------------------------------*/
#include "../util/file.h"
bool GCords::read(const char *filename, const char *fmt, uint32_t skip) {
  ChrMap chrmap;
  const char delim = '\t';

  /* check format */
  const bool has_end = has_field(fmt, CHAR_END);
  std::vector<FieldType> ft = getFields(fmt);
  createFields(&m->annot, ft);

  //FILE *f = fopen(filename, "r");
  File f;
  if (!f.open(filename, "r", File::FILETYPE_AUTO)) {
    fprintf(stderr, "error: cannot open file %s\n", filename);
    exit(1);
  }
  TokenReader tr;
  char buffer[1024];
  uint64_t lineno = 0;
 // while (fgets(buffer, sizeof(buffer)-1, f) != NULL) {}
  while (f.gets(buffer, sizeof(buffer)-1) != NULL) {
    lineno++;
    if (lineno <= skip) {
      continue;
    }
    GCord r;
    /* parse line */
    char *pos = buffer;
    for (uint32_t i = 0; i < ft.size(); i++) {
      pos = read_token(pos, delim, ft[i], &r, &m->annot, tr);
    }
    /* no end? - assume we are reading points */
    if (!has_end) {
      r.e = r.s + 1;
    }
    m_d.push_back(r);
  }
  f.close();
  printf("read %lu genomic coordinates\n", m_d.size());
#if 0
  for (uint32_t i = 0; i < std::min(m_d.size(), (size_t)10); i++) {
    printf("%s %lu %lu %f\n", chrmap.chrTypeStr(m_d[i].chr), m_d[i].s, m_d[i].e, get(i).getFloat(1));
  }
#endif
  return true;
}

/*----------------------------------------------------------------------------*/

GCords::RowPtr GCords::get(uint64_t i) {
 return RowPtr(m_d[i], this, i);
}

/*----------------------------------------------------------------------------*/

float GCords::annotFloat(uint32_t annotIdx, float rowIdx) const {
  return m->annot.getFloat(annotIdx, rowIdx);
}

/*----------------------------------------------------------------------------*/
#include "../intervaltree.cpp"
void GCords::intersect(const GCords* gca, const GCords* gcb) {
  std::vector<char> annot(gca->data().size(), 0);
    ChrMap::ChrType chr_curr = ChrMap::CHRTYPE_NUMENTRIES;
    IntervalTree<GCord> *gcb_curr = NULL;
    for (uint32_t i = 0; i < gca->data().size(); i++) {
      if (chr_curr != gca->data()[i].chr) {
        chr_curr = gca->data()[i].chr;
        delete gcb_curr;
        gcb_curr = new IntervalTree<GCord>(gcb->getChr(chr_curr));
        printf("%s (%u)", ChrMap::chrTypeStr(chr_curr), gcb_curr->numNodes());
      }
      const uint64_t bp = gca->data()[i].s;
      annot[i] = gcb_curr->overlaps(bp);
      if (i % 10000 == 0) {
        printf(".");
        fflush(stdout);
      }
    }
    delete gcb_curr;
    printf("\n");
    return;
}
