#include "hapdosereader.h"
#include "l_gettoken.h"
#include "l_linereader.h"
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
  LineReader lr;
  SampleBuf sampleBuf;
};

/*----------------------------------------------------------------------------*/

HapDoseReader::HapDoseReader() {
  m = new HapDoseReaderPriv;
  m->sampleBuf.resize(HAPLINDEX_NUMENRIES);
}

/*----------------------------------------------------------------------------*/

HapDoseReader::~HapDoseReader() {
  close();
  delete m;
}

/*----------------------------------------------------------------------------*/

#define RETURN_ERR gzclose(m->f); \
                   m->f = NULL; \
                   return false;

bool HapDoseReader::open(const char* fn) {
  return m->lr.open(fn);
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
  assert(m->lr.isOpen() && "hap file not opened");
  m->sampleBuf[HAPLINDEX_1].clear();
  m->sampleBuf[HAPLINDEX_2].clear();
  uint32_t lineCount = 0;
  while (lineCount < HAPLINDEX_NUMENRIES) {
    const HaplIndexType hapIdx = (HaplIndexType)(lineCount % HAPLINDEX_NUMENRIES);
    char *line = m->lr.readLine();
    /* end of file? */
    if (line == NULL) {
        return false;
    }
    /* patch line end */
    if (line[strlen(line) - 1] != '\n') {
      fprintf(stderr, "error - expected newline at %zu (hapdosefile line %u)\n",
          strlen(line) - 1, lineCount);
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
  m->lr.close();
}
