#ifndef HTTPSERVICE_H_
#define HTTPSERVICE_H_

#include "server.h"

/*----------------------------------------------------------------------------*/

#define ENUM_HTTPSTATUS(select_fun) \
  select_fun(HTTP_STATUS_OK                 , 200) \
  select_fun(HTTP_STATUS_BADREQUEST         , 400) \
  select_fun(HTTP_STATUS_NOTFOUND           , 404) \
  select_fun(HTTP_STATUS_INTERNALSERVERERROR, 500)

#define ENUM_MIMETYPE(select_fun) \
    select_fun(MIMETYPE_TEXT_PLAIN      , "text/plain"       ) \
    select_fun(MIMETYPE_TEXT_HTML       , "text/html"        ) \
    select_fun(MIMETYPE_TEXT_CSS        , "text/css"         ) \
    select_fun(MIMETYPE_TEXT_JAVASCRIPT , "text/javascript"  ) \
    select_fun(MIMETYPE_TEXT_EVENTSTREAM, "text/event-stream") \
    select_fun(MIMETYPE_IMAGE_PNG       , "image/png"        )

class HttpHeader;
class IUrlHandle;

/*----------------------------------------------------------------------------*/

class HttpService: public ISocketService {
public:
  enum MimeType {
#define ENUM_GET_ENAME(name, str) name,
    ENUM_MIMETYPE(ENUM_GET_ENAME)
    MIMETPYE_NUMENTRIES
#undef ENUM_GET_ENAME
  };
  static const char* MimeTypeStr[];

  enum HttpStatus {
#define ENUM_GET_ENAME(name, str) name,
    ENUM_HTTPSTATUS(ENUM_GET_ENAME)
    HTTP_STATUS_NUMENTRIES
#undef ENUM_GET_ENAME
  };

  HttpService();
  virtual ~HttpService() {};
  virtual void newData(uint32_t clientId, const uint8_t* data, uint32_t len);
  void registerFile(const char *path, const char *url, MimeType mt);
  void registerUrl(IUrlHandle *      , const char *url, MimeType mt);

protected:
  void http_get(uint32_t clientId, const char* url, const HttpHeader *);
  struct RegUrl {
    MimeType mimetype;
    std::string path;
    IUrlHandle *urlHandle;
  };
  std::map<std::string, RegUrl> m_rurls;

private:
  Log m_log;
};

/*----------------------------------------------------------------------------*/

#endif /* HTTPSERVICE_H_ */
