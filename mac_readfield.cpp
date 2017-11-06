
#define READ_C(pos, delim, chr) \
    { \
      const char *s = pos; \
      assert(pos != NULL); \
      pos = strchr(pos, delim); \
      pos[0] = '\0'; \
      assert(pos - s == 4 || pos - s == 5); \
      chr[0] = s[3]; \
      chr[1] = s[4]; \
      pos++; \
    }

#define READ_CHR(pos, delim, chr, cm) \
    { \
      const char *s = pos; \
      assert(pos != NULL); \
      pos = strchr(pos, delim); \
      pos[0] = '\0'; \
      chr = cm.unifyChr(s); \
      pos++; \
    }


#define READ_BP(pos, delim, bp) \
    { \
      const char *s = pos; \
      assert(pos != NULL); \
      pos = strchr(pos, delim); \
      pos[0] = '\0'; \
      bp = atol(s); \
      pos++; \
    }

#define READ_FLOAT(pos, delim, f) \
    { \
      const char *s = pos; \
      assert(pos != NULL); \
      pos = strchr(pos, delim); \
      pos[0] = '\0'; \
      f = atof(s); \
      pos++; \
    }
#define SKIP(pos, delim) \
    { \
      assert(pos != NULL); \
      pos = strchr(pos, delim); \
      pos++; \
    }
