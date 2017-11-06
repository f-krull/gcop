#include "chrdef.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

ChrMap::ChrMap() {
  for (uint32_t i = CHRTYPE_1; i < CHRTYPE_NUMENTRIES; i++) {
    m_map[m_chrTypeStr[i]] = (ChrType)i;
  }
}

/*----------------------------------------------------------------------------*/

const char* ChrMap::m_chrTypeStr[] = {
#define ENUM_GET_NAME(name, str) str,
        ENUM_CHRTYPE(ENUM_GET_NAME)
        "undefined"
#undef ENUM_GET_NAME
};

/*----------------------------------------------------------------------------*/

ChrMap::ChrType ChrMap::unifyChr(const char * chr) const {
   std::map<std::string, ChrType>::const_iterator it = m_map.find(chr);
   if (it == m_map.end()) {
     fprintf(stderr, "error: unknown chr %s\n", chr);
     exit(1);
   }
   return it->second;
 }
