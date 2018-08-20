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

typedef std::vector<std::vector<float>> SampleBuf;

class HapDoseReaderPriv {
public:
  HapDoseReader::SampleInfo sampleInfo;
  gzFile f;
  uint32_t buflen;
  char* buf;
  SampleBuf sampleBuf;
};

/*----------------------------------------------------------------------------*/

HapDoseReader::HapDoseReader() {
  m = new HapDoseReaderPriv;
  m->f = NULL;
  m->buflen = LINE_LENGTH_INIT;
  m->buf = new char[m->buflen];
  m->sampleBuf.resize(HAPLINDEX_NUMENRIES);
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

bool HapDoseReader::open(const char* fn, uint32_t numVar) {
  m->f = gzopen(fn, "r");
  if (!m->f) {
    fprintf(stderr, "error - cannot open '%s'\n", fn);
    return false;
  }
  return true;
}

/*----------------------------------------------------------------------------*/

const std::vector<float> & HapDoseReader::getHapDose1() const {
  return m->sampleBuf[HAPLINDEX_1];
}

/*----------------------------------------------------------------------------*/

const std::vector<float> & HapDoseReader::getHapDose2() const {
  return m->sampleBuf[HAPLINDEX_2];
}

/*----------------------------------------------------------------------------*/

const HapDoseReader::SampleInfo & HapDoseReader::getSampleInfo() const {
  return m->sampleInfo;
}

/*----------------------------------------------------------------------------*/

bool HapDoseReader::nextSample() {
  assert(m->f && "hap file not opened");
  m->sampleBuf[HAPLINDEX_1].clear();
  m->sampleBuf[HAPLINDEX_2].clear();
  uint32_t lineCount = 0;
  while (lineCount < HAPLINDEX_NUMENRIES) {
    const HaplIndexType hapIdx = (HaplIndexType)(lineCount % HAPLINDEX_NUMENRIES);
    z_off_t p_cur = gztell(m->f);
    char *line = gzgets(m->f, m->buf, m->buflen);
    /* end of file? */
    if (line == NULL) {
        return false;
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
      fprintf(stderr, "error - expected newline at %zu (hapdosefile line %u)\n",
          strlen(m->buf) - 1, lineCount);
      return false;
    }
    /* parse id and fid */
    char *fid  = line;
    char *id = gettoken(fid, '-'); /* "fid->id" */
    if (id[0] == '>') {
      id++;
    }
    char *hap = gettoken(id, DELIMITER);
    if (hapIdx == HAPLINDEX_1) {
      m->sampleInfo.id       = id;
      m->sampleInfo.familyid = fid;
    } else {
      assert(hapIdx == HAPLINDEX_2);
      bool err = false;
      err |= (m->sampleInfo.id       != id);
      err |= (m->sampleInfo.familyid != fid);
      if (err) {
        fprintf(stderr, "error - hapdose file line %u unexpected ID\n", lineCount+1);
        return false;
      }
    }
    char *dosage = gettoken(hap, DELIMITER);
    char *dosage_next = gettoken(dosage, DELIMITER);
    while (dosage[0] != '\0') {
      m->sampleBuf[hapIdx].push_back(atof(dosage));
      dosage      = dosage_next;
      dosage_next = gettoken(dosage, DELIMITER);
    }
    lineCount++;
  }
  if (m->sampleBuf[HAPLINDEX_1].size() != m->sampleBuf[HAPLINDEX_2].size()) {
    fprintf(stderr, "error - hapdose1 != hapdose2 for sample XY\n");
    return false;
  }
  if (m->sampleBuf[HAPLINDEX_1].empty()) {
    fprintf(stderr, "error - hapdose1 has no variants\n");
    return false;
  }
  return true;
}

/*----------------------------------------------------------------------------*/

void HapDoseReader::close() {
  if (m->f) {
    gzclose(m->f);
    m->f = NULL;
  }
}
