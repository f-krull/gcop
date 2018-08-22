#ifndef DATA_FIELDTYPES_H_
#define DATA_FIELDTYPES_H_


#define ENUM_FIELDTYPE(select_fun) \
    select_fun(FIELD_TYPE_SKIP  , 1, '.', "skip") \
    select_fun(FIELD_TYPE_GCCHR , 2, 'c', "chromosome") \
    select_fun(FIELD_TYPE_GCBPS , 3, 's', "BP start") \
    select_fun(FIELD_TYPE_GCBPE , 4, 'e', "BP end") \
    select_fun(FIELD_TYPE_FLOAT , 5, 'f', "float") \
    select_fun(FIELD_TYPE_CHR   , 6, 'c', "chromosome") \
    select_fun(FIELD_TYPE_UINT  , 7, 'u', "unsigned integer") \
    select_fun(FIELD_TYPE_STRING, 8, 's', "string")

enum FieldType {
#define ENUM_GET_ENAME(name, num, ch, str) name,
  ENUM_FIELDTYPE(ENUM_GET_ENAME)
  FIELD_TYPE_NUMENTRIES
#undef ENUM_GET_ENAME
};


constexpr char fieldTypeChars[] = {
#define  ENUM_GET_CHAR(name, num, ch, str) ch,
        ENUM_FIELDTYPE(ENUM_GET_CHAR)
        '-'
#undef ENUM_GET_NAME
};

extern const char* fieldTypeStr[];



#endif /* DATA_FIELDTYPES_H_ */
