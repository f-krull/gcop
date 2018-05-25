#ifndef HTTPFILESERVICE_H_
#define HTTPFILESERVICE_H_

#include "server.h"

/*----------------------------------------------------------------------------*/

//TODO: move this to public enum in HttpFileServer
#define MIME_TEXT_PLAIN        "text/plain"
#define MIME_TEXT_HTML         "text/html"
#define MIME_TEXT_CSS          "text/css"
#define MIME_TEXT_JAVASCRIPT   "text/javascript"
#define MIME_TEXT_EVENTSTREAM  "text/event-stream"
#define MIME_IMAGE_PNG         "image/png"

/*----------------------------------------------------------------------------*/

class HttpFileService: public ISocketService {
public:
  HttpFileService();
  virtual ~HttpFileService() {};
  virtual void newData(uint32_t clientId, const uint8_t* data, uint32_t len);
  void registerFile(const char *path, const char *url, const char *mimetype);
protected:
  void http_get(uint32_t clientId, const char* url);
  struct RegFile {
    std::string mimetype;
    std::string path;
  };
  std::map<std::string, RegFile> m_rfiles;
private:
  Log m_log;
};



#endif /* HTTPFILESERVICE_H_ */
