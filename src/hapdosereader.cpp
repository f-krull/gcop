#include "hapdosereader.h"
#include "gettoken.h"
#include <zlib.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/*----------------------------------------------------------------------------*/

#define LINE_LENGTH_INIT (1 * 1024 * 1024)
#define DELIMITER '\t'

/*----------------------------------------------------------------------------*/

typedef std::vector<std::array<float, HAPLINDEX_NUMENRIES>> Samples;
typedef std::vector<Samples> VariantBuf;

class HapDoseReaderPriv {
public:
  std::vector<HapDoseReader::SampleInfo> sampleInfo;
  gzFile f;
  uint32_t buflen;
  char* buf;
  std::vector<ptrdiff_t> lineoffsets;

  VariantBuf variantBuf;
  int32_t varIdx;
};

/*----------------------------------------------------------------------------*/

HapDoseReader::HapDoseReader() {
  m = new HapDoseReaderPriv;
  m->f = NULL;
  m->buflen = LINE_LENGTH_INIT;
  m->buf = new char[m->buflen];
  m->varIdx = -1;
}

/*----------------------------------------------------------------------------*/

HapDoseReader::~HapDoseReader() {
  close();
  delete [] m->buf;
  delete m;
}

/*----------------------------------------------------------------------------*/

bool HapDoseReader::allocVarBuf(uint32_t _numVar, uint32_t numSamples) {
  uint32_t numBufVar = _numVar;
  /* try to alloc for all variants - use half if alloc fails */
  while (numBufVar >= 1) {
    try {
      std::array<float, 2> hinit = {0.f,0.f};
      Samples s(m->sampleInfo.size(), hinit);
      m->variantBuf.resize(numBufVar, s);
    } catch (const std::bad_alloc &) {
      numBufVar /= 2;
      continue;
    }
    return true;
  }
  return false;
}

/*----------------------------------------------------------------------------*/

#define RETURN_ERR gzclose(m->f); \
                   m->f = NULL; \
                   return false;

bool HapDoseReader::open(const char* fn, uint32_t numVar) {
  m->f = gzopen(fn, "r");
  uint32_t lineCount = 0;
  while (true) {
    const HaplIndexType hapIdx = (HaplIndexType)(lineCount % HAPLINDEX_NUMENRIES);
    z_off_t p_cur = gztell(m->f);
    char *line = gzgets(m->f, m->buf, m->buflen);
    if (line == NULL) {
      break;
    }
    /* buffer too small? */
    if (strlen(m->buf)+1 == m->buflen) {
      /* realloc and retry the same line */
      m->buflen = m->buflen * 2;
      printf("  realloc line buffer to %u bytes\n", m->buflen);
      delete [] m->buf;
      m->buf = new char[m->buflen];
      gzseek(m->f, p_cur, 0);
      continue;
    }
    /* patch line end */
    if (line[strlen(line) - 1] != '\n') {
      fprintf(stderr, "error - expected newline at %zu (file '%s', line %u)\n",
          strlen(m->buf) - 1, fn, lineCount);
      RETURN_ERR;
    }
    char *fid  = line;
    char *id = gettoken(fid, '-'); /* "fid->id" */
    if (id[0] == '>') {
      id++;
    }
    char *hap = gettoken(id, DELIMITER);
    if (hapIdx == HAPLINDEX_1) {
      m->sampleInfo.push_back(SampleInfo());
      m->sampleInfo.back().id       = id;
      m->sampleInfo.back().familyid = fid;
    } else {
      assert(hapIdx == HAPLINDEX_2);
      bool err = false;
      err |= (m->sampleInfo.back().id       != id);
      err |= (m->sampleInfo.back().familyid != fid);
      if (err) {
        fprintf(stderr, "error - file '%s' line %u unexpected ID\n", fn, lineCount+1);
        RETURN_ERR;
      }
    }
    char *dosage = gettoken(hap, DELIMITER);
    ptrdiff_t lineoffset = dosage - line;
    m->lineoffsets.push_back(lineoffset);
    lineCount++;
  }
  assert(m->sampleInfo.size() * HAPLINDEX_NUMENRIES == lineCount);
  return allocVarBuf(numVar, m->sampleInfo.size());
}

/*----------------------------------------------------------------------------*/

const std::vector<HapDoseReader::SampleInfo> HapDoseReader::sampleInfo() const {
  return m->sampleInfo;
}

/*----------------------------------------------------------------------------*/

const std::vector<std::array<float, HAPLINDEX_NUMENRIES>> & HapDoseReader::nextVar() {
  if (m->varIdx >= 0 && (uint32_t)m->varIdx+1 < m->variantBuf.size()) {
    m->varIdx++;
    return m->variantBuf[m->varIdx];
  }
  assert(m->f && "hap file not opened");
  /* rewind */
  gzseek(m->f, 0, 0);
  uint32_t lineCount = 0;
  printf("  buffering variants 0%%");
  while (true) {
    const HaplIndexType hapIdx = (HaplIndexType)(lineCount % HAPLINDEX_NUMENRIES);
    const uint32_t   sampleIdx =                 lineCount / HAPLINDEX_NUMENRIES;
    char *line = gzgets(m->f, m->buf, m->buflen);
    if (line == NULL) {
      break;
    }
    char *dosage      = line + m->lineoffsets[lineCount];
    char *dosage_next = gettoken(dosage, DELIMITER);

    uint32_t vars_read = 0;
    while (dosage[0] != '\0' && vars_read < m->variantBuf.size()) {
      m->variantBuf[vars_read][sampleIdx][hapIdx] = atof(dosage);
      m->lineoffsets[lineCount] =  dosage_next - line;
      dosage      = dosage_next;
      dosage_next = gettoken(dosage, DELIMITER);
      vars_read++;
    }
    /* point to next dosage in line for subsequent call */
    lineCount++;
    printf("\r  buffering %zu variants %.2f%%", m->variantBuf.size(),
      float(lineCount)/2/m->sampleInfo.size()*100);
  }
  printf("\n");
  m->varIdx = 0;
  return m->variantBuf[m->varIdx];
}

/*----------------------------------------------------------------------------*/

void HapDoseReader::close() {
  if (m->f) {
    gzclose(m->f);
    m->f = NULL;
  }
}
