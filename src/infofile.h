#ifndef SRC_INFOFILE_H_
#define SRC_INFOFILE_H_

#include <vector>
#include <string>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class InfoFileEntry {
public:
#define ENUM_INFOFIELDTYPE(select_fun) \
    select_fun(FIELD_SNP      , "SNP"      ) \
    select_fun(FIELD_REF0     , "REF(0)"   ) \
    select_fun(FIELD_ALT1     , "ALT(1)"   ) \
    select_fun(FIELD_ALTFRQ   , "ALT_Frq"  ) \
    select_fun(FIELD_MAF      , "MAF"      ) \
    select_fun(FIELD_AVGCALL  , "AvgCall"  ) \
    select_fun(FIELD_RSQ      , "Rsq"      ) \
    select_fun(FIELD_GENOTYPED, "Genotyped") \
    select_fun(FIELD_LOOPRSQ  , "LooRsq"   ) \
    select_fun(FIELD_EMPR     , "EmpR"     ) \
    select_fun(FIELD_EMPRSQ   , "EmpRsq"   ) \
    select_fun(FIELD_DOSE0    , "Dose0"    ) \
    select_fun(FIELD_DOSE1    , "Dose1"    )

  enum InfoFieldType {
#define ENUM_GET_ENAME(name, str) name,
    ENUM_INFOFIELDTYPE(ENUM_GET_ENAME)
    INFOFIELDTPYE_NUMENTRIES
#undef ENUM_GET_ENAME
  };

  InfoFileEntry() { m_entries.resize(INFOFIELDTPYE_NUMENTRIES); }
  ~InfoFileEntry() { }
  const char* get(InfoFieldType t) const { return m_entries[t].c_str(); }
  void set(InfoFieldType t, const char *str) { m_entries[t] = str; }
  static const char* InfoFieldTypeStr[];

private:
  std::vector<std::string> m_entries;
};

/*----------------------------------------------------------------------------*/


class InfoFileFilter {
public:
  InfoFileFilter() {
    m_mafMin     = 0.01;
    m_avgCallMin = 0.99;
    m_rsqMin     = 0.90;
  }

  bool isOk(const InfoFileEntry &e) const {
    bool ret = true;
//    ret = ret && checkMaf(e);
//    ret = ret && checkAvgCall(e);
//    ret = ret && checkRsq(e);
    return ret;
  }
  float getMafMin() const { return m_mafMin; }
  float getAvgCallMin() const { return m_avgCallMin; }
  float getRsqMin() const { return m_rsqMin; }

  void setMafMin(float f) { m_mafMin = f; }
  void setAvgCallMin(float f) { m_avgCallMin = f; }
  void setRsqMin(float f) { m_rsqMin = f; }

private:
  bool checkMaf(const InfoFileEntry &e) const;
  bool checkAvgCall(const InfoFileEntry &e) const;
  bool checkRsq(const InfoFileEntry &e) const;

  float m_mafMin;
  float m_avgCallMin;
  float m_rsqMin;
};

/*----------------------------------------------------------------------------*/

class InfoFile {
public:
  InfoFile() : m_numVarOk(0) {}
  bool read(const char* fn, const InfoFileFilter &filter);

  const std::vector<InfoFileEntry> & variants() const { return m_variants; }
  const std::vector<char> & variantStatus() const { return m_variantStatus; }
  const size_t numVariants() const { return m_variants.size(); }
  const size_t numVariantsOk() const { return m_numVarOk; }
private:
  std::vector<InfoFileEntry> m_variants;
  std::vector<char> m_variantStatus;
  uint32_t m_numVarOk;
};


#endif /* SRC_INFOFILE_H_ */
