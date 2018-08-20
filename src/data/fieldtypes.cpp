#include "fieldtypes.h"





const char* fieldTypeStr[] = {
#define  ENUM_GET_STR(name, num, ch, str) str,
        ENUM_FIELDTYPE(ENUM_GET_STR)
        "undefined"
#undef ENUM_GET_NAME
};
