#include "hapdosereader.h"
#include "gettoken.h"
#include <zlib.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/*----------------------------------------------------------------------------*/

#define LINE_LENGTH_INIT (16)
#define DELIMITER '\t'

/*----------------------------------------------------------------------------*/

class HapDoseReaderPriv {
public:
  std::vector<std::vector<float>> doseBuf;
  std::vector<HapDoseReader::SampleInfo> sampleInfo;
  gzFile f;
  uint32_t buflen;
  char* buf;
  std::vector<ptrdiff_t> lineoffsets;

  struct Samples {
    std::vector<std::array<float, HAPLINDEX_NUMENRIES>> hap;
  };
  struct VariantBuf {
    int32_t varIdx;
    Samples samples;
  } variantBuf;
};

/*----------------------------------------------------------------------------*/

HapDoseReader::HapDoseReader() {
  m = new HapDoseReaderPriv;
  m->doseBuf.resize(HAPLINDEX_NUMENRIES);
  m->f = NULL;
  m->buflen = LINE_LENGTH_INIT;
  m->buf = new char[m->buflen];
  m->variantBuf.varIdx = -1;
}

/*----------------------------------------------------------------------------*/

HapDoseReader::~HapDoseReader() {
  close();
  delete [] m->buf;
  delete m;
}

/*----------------------------------------------------------------------------*/

#define RETURN_ERR gzclose(m->f); \
                   m->f = NULL; \
                   return false;

bool HapDoseReader::open(const char* fn) {
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
      printf("realloc line buffer - %u bytes\n", m->buflen);
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
    //lineoffset = readDosage(m, line, lineoffset);
    m->lineoffsets.push_back(lineoffset);
    lineCount++;
  }
  assert(m->sampleInfo.size() * HAPLINDEX_NUMENRIES == lineCount);
  return true;
}

/*----------------------------------------------------------------------------*/

const std::vector<HapDoseReader::SampleInfo> HapDoseReader::sampleInfo() const {
  return m->sampleInfo;
}

/*----------------------------------------------------------------------------*/
#include <array>
const std::vector<std::array<float, HAPLINDEX_NUMENRIES>> & HapDoseReader::nextVar() {
  assert(m->f && "hap file not opened");
  /* rewind */
  gzseek(m->f, 0, 0);
  {
    std::array<float, 2> hinit = {0.f,0.f};
    m->variantBuf.samples.hap.resize(m->sampleInfo.size(),hinit );
  }
  uint32_t lineCount = 0;
  while (true) {
    const HaplIndexType hapIdx = (HaplIndexType)(lineCount % HAPLINDEX_NUMENRIES);
    const uint32_t   sampleIdx =                 lineCount / HAPLINDEX_NUMENRIES;
    char *line = gzgets(m->f, m->buf, m->buflen);
    if (line == NULL) {
      break;
    }
    char *dosage      = line + m->lineoffsets[lineCount];
    char *dosage_next = gettoken(dosage, DELIMITER);
    m->variantBuf.samples.hap[sampleIdx][hapIdx] = atof(dosage);
    /* point to next dosage in line for subsequent call */
    m->lineoffsets[lineCount] =  dosage_next - line;
    lineCount++;
  }
  m->variantBuf.varIdx++;
  return m->variantBuf.samples.hap;
}

/*----------------------------------------------------------------------------*/

void HapDoseReader::close() {
  if (m->f) {
    gzclose(m->f);
    m->f = NULL;
  }
}
