#include "bedwriter.h"
#include <assert.h>
#include <string.h>


#define HOMOZYGOUS_A1    0x00
#define MISSING_GENOTYPE 0x01
#define HETEROZYGOUS     0x02
#define HOMOZYGOUS_A2    0x03

#define SETHAP(o, p, t) ((o) |= ((t)<<((p)*2)))

/*----------------------------------------------------------------------------*/

BedWriter::BedWriter() {
  m_f = NULL;
  m_delta = 0.05;
  m_numStat.resize(4, 0);
}

/*----------------------------------------------------------------------------*/

BedWriter::~BedWriter() {
  close();
}

/*----------------------------------------------------------------------------*/

bool BedWriter::close() {
  if (!m_f) {
    return false;
  }
  if (m_outBuf.empty()) {
    return false;
  }
  bool ret = true;
  bool sameSize = true;
  const size_t samplBlockSize = m_outBuf.front().size();
  /* for all variants */
  for (uint32_t i = 0; i < m_outBuf.size() && sameSize == true; i++) {
    /* blocks of samples */
    sameSize = sameSize && m_outBuf[i].size() == samplBlockSize;
    ret = ret && fwrite(m_outBuf[i].data(), 1, m_outBuf[i].size(), m_f) == m_outBuf[i].size();
  }
  if (!sameSize) {
    fprintf(stderr, "error - missing some sample data\n");
  }
  if (!ret) {
    fprintf(stderr, "error - cannot write bed file\n");
  }
  fclose(m_f);
  m_f = NULL;
  printf("stats: \n");
  printf("  num HOMOZYGOUS_A1:    %lu\n", m_numStat[HOMOZYGOUS_A1]);
  printf("  num HETEROZYGOUS:     %lu\n", m_numStat[HETEROZYGOUS]);
  printf("  num HOMOZYGOUS_A2:    %lu\n", m_numStat[HOMOZYGOUS_A2]);
  printf("  num MISSING_GENOTYPE: %lu\n", m_numStat[MISSING_GENOTYPE]);
  return ret && sameSize;
};

/*----------------------------------------------------------------------------*/

bool BedWriter::write(uint64_t varIdx, uint32_t sampIdx, double hapdose1, double hapdose2) {
  if (varIdx >= m_outBuf.size()) {
    fprintf(stderr, "error - cannot write variant (idx=%lu,max=%zu)\n", varIdx, m_outBuf.size());
    return false;
  }
  const double d0max = 0 + m_delta;
  const double d1min = 1 - m_delta;
  const bool hap1is0 = hapdose1 <= d0max;
  const bool hap1is1 = hapdose1 >= d1min;
  const bool hap2is0 = hapdose2 <= d0max;
  const bool hap2is1 = hapdose2 >= d1min;
  int32_t hapt = MISSING_GENOTYPE;
  if (false) {
  } else if (hap1is0 && hap2is0) {
    hapt = HOMOZYGOUS_A1;
  } else if (hap1is1 && hap2is1) {
    hapt = HOMOZYGOUS_A2;
  } else if ((hap1is0 || hap1is1) && (hap2is0 || hap2is1)) {
    hapt = HETEROZYGOUS;
  }
  m_numStat[hapt]++;
  const uint32_t bytepos = sampIdx/4;
  if (bytepos >= m_outBuf[varIdx].size()) {
    try {
      m_outBuf[varIdx].resize(bytepos+1, 0x00);
    } catch (const std::bad_alloc &) {
      fprintf(stderr, "error - cannot allocate output buffer (%zu bytes)\n", m_outBuf[varIdx].capacity()*2*m_outBuf.size());
      return false;
    }
  }
  SETHAP(m_outBuf[varIdx].data()[bytepos], sampIdx%4, hapt);
  return true;
}

/*----------------------------------------------------------------------------*/

bool BedWriter::open(const char* fn, uint64_t numVar) {
  m_numStat.resize(4, 0);
  assert(!m_f);
  m_f = fopen(fn, "wb");
  if (m_f == NULL) {
    fprintf(stderr, "error - cannot open file '%s'\n", fn);
    return false;
  }
  const uint8_t bed_magic_num[] = { 0x6c, 0x1b, 0x01 };
  if (fwrite(bed_magic_num, sizeof(bed_magic_num), 1, m_f) != 1) {
    fclose(m_f);
    m_f = NULL;
    fprintf(stderr, "error - cannot write to file '%s'\n", fn);
    return false;
  }
  m_outBuf.resize(numVar);
#if 0
  try {
    m_outBuf.clear();
    m_outBuf.resize(numOutBytes, 0x00);
  } catch (const std::bad_alloc &) {
    fprintf(stderr, "error - cannot allocate output buffer (%zu bytes)\n", numOutBytes);
    return false;
  }
#endif
  return true;
}
