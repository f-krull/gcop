#ifndef HTTPFILESERVICE_H_
#define HTTPFILESERVICE_H_

#include "server.h"

/*----------------------------------------------------------------------------*/

#define ENUM_MIMETYPE(select_fun) \
    select_fun(MIMETYPE_TEXT_PLAIN      , "text/plain"       ) \
    select_fun(MIMETYPE_TEXT_HTML       , "text/html"        ) \
    select_fun(MIMETYPE_TEXT_CSS        , "text/css"         ) \
    select_fun(MIMETYPE_TEXT_JAVASCRIPT , "text/javascript"  ) \
    select_fun(MIMETYPE_TEXT_EVENTSTREAM, "text/event-stream") \
    select_fun(MIMETYPE_IMAGE_PNG       , "image/png"        )

class HttpHeader;

class HttpFileService: public ISocketService {
public:
  enum MimeType {
  #define ENUM_GET_ENAME(name, str) name,
    ENUM_MIMETYPE(ENUM_GET_ENAME)
    MIMETPYE_NUMENTRIES
  #undef ENUM_GET_ENAME
  };
  static const char* MimeTypeStr[];

  HttpFileService();
  virtual ~HttpFileService() {};
  virtual void newData(uint32_t clientId, const uint8_t* data, uint32_t len);
  void registerFile(const char *path, const char *url, MimeType mt);

protected:
  void http_get(uint32_t clientId, const char* url, const HttpHeader *);
  struct RegFile {
    MimeType mimetype;
    std::string path;
  };
  std::map<std::string, RegFile> m_rfiles;

private:
  Log m_log;
};

/*----------------------------------------------------------------------------*/

#endif /* HTTPFILESERVICE_H_ */
