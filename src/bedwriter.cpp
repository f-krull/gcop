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
  m_sampleBuf.resize(HAPLINDEX_NUMENRIES);
  m_outBuf = NULL;
  m_outBufLen = 0;
  m_delta = 0.05;
  m_num.resize(4, 0);
}

/*----------------------------------------------------------------------------*/

BedWriter::~BedWriter() {
  delete [] m_outBuf;
}

/*----------------------------------------------------------------------------*/

bool BedWriter::write(HaplIndexType h, double d) {
  m_sampleBuf[h].push_back(d);
  return true;
}

/*----------------------------------------------------------------------------*/

void BedWriter::close() {
  assert(m_f);
  fclose(m_f);
  m_f = NULL;
  printf("stats: \n");
  printf("  num HOMOZYGOUS_A1:    %lu\n", m_num[HOMOZYGOUS_A1]);
  printf("  num HETEROZYGOUS:     %lu\n", m_num[HETEROZYGOUS]);
  printf("  num HOMOZYGOUS_A2:    %lu\n", m_num[HOMOZYGOUS_A2]);
  printf("  num MISSING_GENOTYPE: %lu\n", m_num[MISSING_GENOTYPE]);
};

/*----------------------------------------------------------------------------*/

bool BedWriter::closeVariant() {
  if (!m_f) {
    return false;
  }
  assert(m_sampleBuf.size() == HAPLINDEX_NUMENRIES);
  assert(m_sampleBuf[HAPLINDEX_1].size() == m_sampleBuf[HAPLINDEX_2].size());
  if (!m_outBuf) {
    m_outBufLen = (m_sampleBuf[HAPLINDEX_1].size()+3) / 4; /* round up */
    //m_outBufLen = (m_sampleBuf[HAPLINDEX_1].size());
    m_outBuf = new uint8_t[m_outBufLen];
  }
  assert(m_outBufLen >= m_sampleBuf[HAPLINDEX_1].size()/4);
  memset(m_outBuf, 0, m_outBufLen);
  const double d0max = 0 + m_delta;
  const double d1min = 1 - m_delta;
  for (uint32_t i = 0; i < m_sampleBuf[HAPLINDEX_1].size(); i++) {
    const bool hap1is0 = m_sampleBuf[HAPLINDEX_1][i] <= d0max;
    const bool hap1is1 = m_sampleBuf[HAPLINDEX_1][i] >= d1min;
    const bool hap2is0 = m_sampleBuf[HAPLINDEX_2][i] <= d0max;
    const bool hap2is1 = m_sampleBuf[HAPLINDEX_2][i] >= d1min;
    int32_t hapt = MISSING_GENOTYPE;
    if (false) {
    } else if (hap1is0 && hap2is0) {
      hapt = HOMOZYGOUS_A1;
    } else if (hap1is1 && hap2is1) {
      hapt = HOMOZYGOUS_A2;
    } else if ((hap1is0 || hap1is1) && (hap2is0 || hap2is1)) {
      hapt = HETEROZYGOUS;
    }
    m_num[hapt]++;
//    printf("%f %f h1_0=%d h1_1=%d h2_0=%d h2_1=%d -> 0x%02x\n",
//        m_sampleBuf[HAPLINDEX_1][i], m_sampleBuf[HAPLINDEX_2][i], hap1is0,
//        hap1is1, hap2is0, hap2is1, hapt);
    SETHAP(m_outBuf[i/4], i%4, hapt);
    //SETHAP(m_outBuf[i], 0, hapt);
  }
  const bool ret = fwrite(m_outBuf, 1, m_outBufLen, m_f) == m_outBufLen;
  m_sampleBuf[HAPLINDEX_1].clear();
  m_sampleBuf[HAPLINDEX_2].clear();
  return ret;
}

/*----------------------------------------------------------------------------*/

bool BedWriter::open(const char* fn) {
  m_num.resize(4, 0);
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
  return true;
}
