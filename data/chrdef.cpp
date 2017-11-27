#include "chrdef.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

ChrMap::ChrMap() {
  for (uint32_t i = CHRTYPE_1; i < CHRTYPE_NUMENTRIES; i++) {
    m_str2ct[m_chrTypeStr[i]] = (ChrType)i;
  }
  for (uint32_t i = CHRTYPE_1; i < CHRTYPE_NUMENTRIES; i++) {
    m_num2ct[m_chrTypeNum[i]] = (ChrType)i;
  }
}

/*----------------------------------------------------------------------------*/

const char* ChrMap::m_chrTypeStr[] = {
#define ENUM_GET_NAME(name, str, num) str,
        ENUM_CHRTYPE(ENUM_GET_NAME)
        "undefined"
#undef ENUM_GET_NAME
};

/*----------------------------------------------------------------------------*/

uint32_t ChrMap::m_chrTypeNum[] = {
#define ENUM_GET_NAME(name, str, num) num,
        ENUM_CHRTYPE(ENUM_GET_NAME)
        0
#undef ENUM_GET_NAME
};


/*----------------------------------------------------------------------------*/

ChrMap::ChrType ChrMap::unifyChr(const char * chr) const {
   std::map<std::string, ChrType>::const_iterator it = m_str2ct.find(chr);
   if (it == m_str2ct.end()) {
     fprintf(stderr, "error: unknown chr %s\n", chr);
     exit(1);
   }
   return it->second;
 }

/*----------------------------------------------------------------------------*/

ChrMap::ChrType ChrMap::unifyChr(uint32_t chr) const {
   std::map<uint32_t, ChrType>::const_iterator it = m_num2ct.find(chr);
   if (it == m_num2ct.end()) {
     fprintf(stderr, "error: unknown chr %u\n", chr);
     exit(1);
   }
   return it->second;
 }
