#include "gcords.h"
#include "intervaltree.cpp"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../util/tokenreader.h"

/*----------------------------------------------------------------------------*/

#define ENUM_FIELDTYPE(select_fun) \
    select_fun(FIELD_TYPE_SKIP  , 1, '.', "skip") \
    select_fun(FIELD_TYPE_GCCHR , 2, 'c', "chromosome") \
    select_fun(FIELD_TYPE_GCBPS , 3, 's', "BP start") \
    select_fun(FIELD_TYPE_GCBPE , 4, 'e', "BP end") \
    select_fun(FIELD_TYPE_FLOAT , 5, 'f', "float") \
    select_fun(FIELD_TYPE_CHR   , 6, 'c', "chromosome") \
    select_fun(FIELD_TYPE_UINT  , 7, 'u', "unsigned integer") \
    select_fun(FIELD_TYPE_STRING, 8, 's', "string")

enum FieldType {
#define ENUM_GET_ENAME(name, num, ch, str) name,
  ENUM_FIELDTYPE(ENUM_GET_ENAME)
  FIELD_TYPE_NUMENTRIES
#undef ENUM_GET_ENAME
};


constexpr char fieldTypeChars[] = {
#define  ENUM_GET_CHAR(name, num, ch, str) ch,
        ENUM_FIELDTYPE(ENUM_GET_CHAR)
        '-'
#undef ENUM_GET_NAME
};


const char* fieldTypeStr[] = {
#define  ENUM_GET_STR(name, num, ch, str) str,
        ENUM_FIELDTYPE(ENUM_GET_STR)
        "undefined"
#undef ENUM_GET_NAME
};


/*----------------------------------------------------------------------------*/

GCords::RowPtr::RowPtr(const GCord &g, GCords *p, uint64_t idx) :
    GCord(g), m_p(p), m_ridx(idx) {
}

/*----------------------------------------------------------------------------*/

float GCords::RowPtr::getFloat(uint32_t i) {
  return m_p->annotFloat(i, m_ridx);
}

/*----------------------------------------------------------------------------*/

ChrMap::ChrType GCords::RowPtr::getChr(uint32_t i) {
  return m_p->annotChr(i, m_ridx);
}

/*----------------------------------------------------------------------------*/

uint64_t GCords::RowPtr::getUint(uint32_t i) {
  return m_p->annotUint(i, m_ridx);
}

/*----------------------------------------------------------------------------*/

const char* GCords::RowPtr::str(uint32_t i) {
  return m_p->annotStr(i, m_ridx);
}

/*----------------------------------------------------------------------------*/

uint32_t GCords::RowPtr::numAnnot() const {
  return m_p->numAnnot();
}

/*----------------------------------------------------------------------------*/

class Annot {
public:
  virtual ~Annot() {};
  static const char* typestr() {return "null";}
  virtual const char* str(uint32_t idx) const {return "hi";};
private:
};

/*----------------------------------------------------------------------------*/

class AnnotFloat : public Annot {
public:
  static const char* typestr() {return "float";}
  const std::vector<float> & d() const {return m_d;}
  std::vector<float> & d() {return m_d;}
  virtual const char *str(uint32_t idx) const {
    static char buf[64];
    snprintf(buf, sizeof(buf)-1, "%f", d()[idx]);
    return buf;
  }
private:
  std::vector<float> m_d;
};

/*----------------------------------------------------------------------------*/

class AnnotString : public Annot {
public:
  ~AnnotString() {
    for (uint32_t i = 0; i < m_d.size(); i++) {
      free(m_d[i]);
    }
  }
  static const char* typestr() {return "string";}
  const std::vector<char*> & d() const {return m_d;}
  std::vector<char*> & d() {return m_d;}
  virtual const char *str(uint32_t idx) const {return d()[idx];}
private:
  std::vector<char*> m_d;
};

/*----------------------------------------------------------------------------*/

class AnnotChr : public Annot {
public:
  static const char* typestr() {return "chr";}
  const std::vector<ChrMap::ChrType> & d() const {return m_d;}
  std::vector<ChrMap::ChrType> & d() {return m_d;}
  virtual const char *str(uint32_t idx) const {
      static char buf[64];
      snprintf(buf, sizeof(buf)-1, "%d", d()[idx]);
      return buf;
    }
private:
  std::vector<ChrMap::ChrType> m_d;
};

/*----------------------------------------------------------------------------*/

class AnnotUint : public Annot {
public:
  static const char* typestr() {return "uint";}
  const std::vector<uint64_t> & d() const {return m_d;}
  std::vector<uint64_t> & d() {return m_d;}
  virtual const char *str(uint32_t idx) const {
      static char buf[64];
      snprintf(buf, sizeof(buf)-1, "%lu", d()[idx]);
      return buf;
    }
private:
  std::vector<uint64_t> m_d;
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

  uint32_t num() const { return m_annots.size(); }

  float getFloat(uint32_t aIdx, uint64_t rowIdx) const {
    const AnnotFloat *a = getAnnot<AnnotFloat>(aIdx);
    assert(a != NULL && "check error handling");
    return a->d()[rowIdx];
  }
  ChrMap::ChrType getChr(uint32_t aIdx, uint64_t rowIdx) const {
    const AnnotChr *a = getAnnot<AnnotChr>(aIdx);
    assert(a != NULL && "check error handling");
    return a->d()[rowIdx];
  }
  uint64_t getUint(uint32_t aIdx, uint64_t rowIdx) const {
    const AnnotUint *a = getAnnot<AnnotUint>(aIdx);
    assert(a != NULL && "check error handling");
    return a->d()[rowIdx];
  }
  const char *str(uint32_t aIdx, uint64_t rowIdx) const {
    const Annot *a = getAnnot<Annot>(aIdx);
    assert(a != NULL && "check error handling");
    return a->str(rowIdx);
  }

  void create(uint32_t idx, FieldType type) {
    switch (type) {
      case FIELD_TYPE_FLOAT:
        m_annots.insert(std::make_pair(idx, new AnnotFloat));
        break;
      case FIELD_TYPE_CHR:
        m_annots.insert(std::make_pair(idx, new AnnotChr));
        break;
      case FIELD_TYPE_UINT:
        m_annots.insert(std::make_pair(idx, new AnnotUint));
        break;
      case FIELD_TYPE_STRING:
        m_annots.insert(std::make_pair(idx, new AnnotString));
        break;
      default:
        fprintf(stderr, "error: cannot create field of type %c(%s)", fieldTypeChars[type], fieldTypeStr[type]);
        exit(1);
        break;
    }
  }

  void push(uint32_t aIdx, float v) {
    AnnotFloat *a = getAnnot<AnnotFloat>(aIdx);
    assert(a != NULL);
    a->d().push_back(v);
  }
  void push(uint32_t aIdx, ChrMap::ChrType v) {
    AnnotChr *a = getAnnot<AnnotChr>(aIdx);
    assert(a != NULL);
    a->d().push_back(v);
  }
  void push(uint32_t aIdx, uint64_t v) {
    AnnotUint *a = getAnnot<AnnotUint>(aIdx);
    assert(a != NULL);
    a->d().push_back(v);
  }
  void push(uint32_t aIdx, char *v) {
    AnnotString *a = getAnnot<AnnotString>(aIdx);
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

struct FieldTypeIdx {
  FieldType type;
  uint32_t idx;
  FieldTypeIdx(FieldType t, uint32_t i = 0) : type(t), idx(i) {
  }
};

static std::vector<FieldTypeIdx> getFields(const char *fmt) {
  std::vector<FieldTypeIdx> fieldTypes;
  for (; fmt[0] != '\0'; fmt++) {
    switch (fmt[0]) {
    case fieldTypeChars[FIELD_TYPE_GCCHR]:
      fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_GCCHR));
      break;
    case fieldTypeChars[FIELD_TYPE_GCBPS]:
      fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_GCBPS));
      break;
    case fieldTypeChars[FIELD_TYPE_GCBPE]:
      fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_GCBPE));
      break;
    case fieldTypeChars[FIELD_TYPE_SKIP]:
      fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_SKIP));
      break;
    default:
      {
        /* check if token is a number */
        char *end;
        uint64_t v = strtoul(fmt, &end, 10);
        if (fmt != end) {
          fmt = end;
          switch (fmt[0]) {
            case fieldTypeChars[FIELD_TYPE_FLOAT]:
              fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_FLOAT, v));
              break;
            case fieldTypeChars[FIELD_TYPE_CHR]:
              fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_CHR, v));
              break;
            case fieldTypeChars[FIELD_TYPE_UINT]:
              fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_UINT, v));
              break;
            case fieldTypeChars[FIELD_TYPE_STRING]:
              fieldTypes.push_back(FieldTypeIdx(FIELD_TYPE_STRING, v));
              break;
            default:
              fprintf(stderr, "error: unknown type at %lu in format '%s'\n", v, fmt);
              exit(1);
              break;
          }
          /* fmt++ is done by for */
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

static void printFields(const std::vector<FieldTypeIdx> &fs) {
  printf("input format:\n");
  for (uint32_t i = 0; i < fs.size(); i++) {
    printf("column %u : %s %u\n", i, fieldTypeStr[fs[i].type], fs[i].idx);
  }
}

/*----------------------------------------------------------------------------*/

static void createFields(Annots* a, const std::vector<FieldTypeIdx> & fields) {
  for (uint32_t i = 0; i < fields.size(); i++) {
    if (fields[i].type > FIELD_TYPE_GCBPE) {
      a->create(fields[i].idx, fields[i].type);
    }
  }
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

char* read_token(char *pos, char delim, const FieldTypeIdx &ft, GCord *s, Annots *a, ChrConv &cc) {
  switch (ft.type) {
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
        float v;
        char *end = TokenReader::read_float(pos, delim, &v);
        a->push(ft.idx, v);
        return end;
      }
      break;
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
  const bool has_end = has_field(fmt, fieldTypeChars[FIELD_TYPE_GCBPE]);
  std::vector<FieldTypeIdx> ft = getFields(fmt);
  createFields(&m->annot, ft);
  printFields(ft);

  //FILE *f = fopen(filename, "r");
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
    GCord r;
    /* parse line */
    char *pos = buffer;
    for (uint32_t i = 0; i < ft.size(); i++) {
      pos = read_token(pos, delim, ft[i], &r, &m->annot, cc);
    }
    /* no end? - assume we are reading points */
    if (!has_end) {
      r.e = r.s + 1;
    }
    m_d.push_back(r);
  }
  f.close();
  printf("read %lu genomic coordinates\n", m_d.size());
#if 1
  {
    const uint32_t n = numAnnot();
    for (uint32_t i = 0; i < std::min(m_d.size(), (size_t)10); i++) {
      printf("%s\t%lu\t%lu", chrmap.chrTypeStr(m_d[i].chr), m_d[i].s, m_d[i].e);
      for (uint32_t j = 0; j < n; j++) {
        printf("\t%s", get(i).str(j));
      }
      printf("\n");
    }
  }
#endif
  delete [] buffer;
  return true;
}

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

/*----------------------------------------------------------------------------*/

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
      annot[i] = gcb_curr->overlaps(gca->data()[i]);
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
