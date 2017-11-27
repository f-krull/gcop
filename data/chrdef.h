
#ifndef CHRDEF_H_
#define CHRDEF_H_

/*----------------------------------------------------------------------------*/

#define ENUM_CHRTYPE(select_fun) \
    select_fun(CHRTYPE_1,  "chr1", 1) \
    select_fun(CHRTYPE_2,  "chr2", 2) \
    select_fun(CHRTYPE_3,  "chr3", 3) \
    select_fun(CHRTYPE_4,  "chr4", 4) \
    select_fun(CHRTYPE_5,  "chr5", 5) \
    select_fun(CHRTYPE_6,  "chr6", 6) \
    select_fun(CHRTYPE_7,  "chr7", 7) \
    select_fun(CHRTYPE_8,  "chr8", 8) \
    select_fun(CHRTYPE_9,  "chr9", 9) \
    select_fun(CHRTYPE_10, "chr10",10) \
    select_fun(CHRTYPE_11, "chr11",11) \
    select_fun(CHRTYPE_12, "chr12",12) \
    select_fun(CHRTYPE_13, "chr13",13) \
    select_fun(CHRTYPE_14, "chr14",14) \
    select_fun(CHRTYPE_15, "chr15",15) \
    select_fun(CHRTYPE_16, "chr16",16) \
    select_fun(CHRTYPE_17, "chr17",17) \
    select_fun(CHRTYPE_18, "chr18",18) \
    select_fun(CHRTYPE_19, "chr19",19) \
    select_fun(CHRTYPE_20, "chr20",20) \
    select_fun(CHRTYPE_21, "chr21",21) \
    select_fun(CHRTYPE_22, "chr22",22) \
    select_fun(CHRTYPE_X,  "chrX", 23) \
    select_fun(CHRTYPE_Y,  "chrY", 24)

#include <map>
#include <string>
#include <stdint.h>

class ChrMap {
public:
  enum ChrType {
#define ENUM_GET_ENAME(name, str, num ) name,
    ENUM_CHRTYPE(ENUM_GET_ENAME)
          CHRTYPE_NUMENTRIES
#undef ENUM_GET_ENAME
  };

  ChrMap();

  static const char* chrTypeStr(ChrType c) {
    return m_chrTypeStr[c];
  }

  static uint32_t chrTypeNum(ChrType c) {
    return m_chrTypeNum[c];
  }

  ChrType unifyChr(const char * chr) const;
  ChrType unifyChr(uint32_t chr) const;

private:
  std::map<std::string, ChrType> m_str2ct;
  std::map<uint32_t, ChrType>    m_num2ct;
  static const char* m_chrTypeStr[];
  static uint32_t    m_chrTypeNum[];
};



#endif /* CHRDEF_H_ */
