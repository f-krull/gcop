
#ifndef CHRDEF_H_
#define CHRDEF_H_



/*----------------------------------------------------------------------------*/

#define ENUM_CHRTYPE(select_fun) \
    select_fun(CHRTYPE_1,  "chr1") \
    select_fun(CHRTYPE_2,  "chr2") \
    select_fun(CHRTYPE_3,  "chr3") \
    select_fun(CHRTYPE_4,  "chr4") \
    select_fun(CHRTYPE_5,  "chr5") \
    select_fun(CHRTYPE_6,  "chr6") \
    select_fun(CHRTYPE_7,  "chr7") \
    select_fun(CHRTYPE_8,  "chr8") \
    select_fun(CHRTYPE_9,  "chr9") \
    select_fun(CHRTYPE_10, "chr10") \
    select_fun(CHRTYPE_11, "chr11") \
    select_fun(CHRTYPE_12, "chr12") \
    select_fun(CHRTYPE_13, "chr13") \
    select_fun(CHRTYPE_14, "chr14") \
    select_fun(CHRTYPE_15, "chr15") \
    select_fun(CHRTYPE_16, "chr16") \
    select_fun(CHRTYPE_17, "chr17") \
    select_fun(CHRTYPE_18, "chr18") \
    select_fun(CHRTYPE_19, "chr19") \
    select_fun(CHRTYPE_20, "chr20") \
    select_fun(CHRTYPE_21, "chr21") \
    select_fun(CHRTYPE_22, "chr22") \
    select_fun(CHRTYPE_X,  "chrX") \
    select_fun(CHRTYPE_Y,  "chrY")

#include <map>
#include <string>

class ChrMap {
public:
  enum ChrType {
#define ENUM_GET_ENAME(name, str) name,
    ENUM_CHRTYPE(ENUM_GET_ENAME)
          CHRTYPE_NUMENTRIES
#undef ENUM_GET_ENAME
  };

  ChrMap();

  static const char* chrTypeStr(ChrType c) {
    return m_chrTypeStr[c];
  }

  ChrType unifyChr(const char * chr) const;

private:
  std::map<std::string, ChrType> m_map;
  static const char* m_chrTypeStr[];
};



#endif /* CHRDEF_H_ */
