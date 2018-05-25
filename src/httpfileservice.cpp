#include "httpfileservice.h"
#include "buffer.h"
#include <assert.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

HttpFileService::HttpFileService() : m_log("HttpFileServer") {
}

/*----------------------------------------------------------------------------*/

#define HTTP_CMD_GET "GET"

/*----------------------------------------------------------------------------*/

const char* HttpFileService::MimeTypeStr[] = {
#define  ENUM_GET_STR(name, str) str,
        ENUM_MIMETYPE(ENUM_GET_STR)
        "undefined"
#undef ENUM_GET_NAME
};

/*----------------------------------------------------------------------------*/

#include "helper.h"
void HttpFileService::newData(uint32_t clientId, const uint8_t* _data, uint32_t _len) {
  if (_len == 0 || _len > 1e6) {
    m_log.err("client %u - received invalid size (%u bytes)", clientId, _len);
    return;
  }
  BufferDyn buf(_data, _len);
  buf.addf(""); /* null-term str */
  char *inget = (char*)buf.data();
  char *inurl = gettoken(inget, ' ');
  char *inrem = gettoken(inurl, ' ');
  gettoken(inrem, '\r');
  if (strcmp(HTTP_CMD_GET, inget) != 0) {
    m_log.err("client %u - expected %s, received (%s)", clientId, HTTP_CMD_GET, inget);
    return;
  }
  if (inurl[0] == '\0') {
    m_log.err("client %u - didn't send url", HTTP_CMD_GET);
    return;
  }
#if 0
  //m_log.dbg("HttpReceiver: client %u:\n%s", clientId, data);
  const char *urls = strchr((char*)data, ' ');
  if (urls == NULL || urls - (char*)data > (int32_t)len) {
    m_log.err("line %d", __LINE__);
    return;
  }
  urls++;
  const char *urle = strchr((char*)urls, ' ');
  if (urle == NULL || urle - (char*)data > (int32_t)len) {
    m_log.err("line %d", __LINE__);
    return;
  }
  /* copy + null-terminate sting */
  char* url = new char[urle-urls+1];
  memcpy(url, urls, urle-urls);
  url[urle-urls] = '\0';
#endif
  m_log.dbg("client %u -> request %s %s %s ...", clientId, inget, inurl, inrem);
  http_get(clientId, inurl);
}

/*----------------------------------------------------------------------------*/

void HttpFileService::registerFile(const char *path, const char *url, MimeType mt) {
  RegFile r;
  r.mimetype = mt;
  r.path     = path;
  m_rfiles[url] = r;
}

/*----------------------------------------------------------------------------*/

void HttpFileService::http_get(uint32_t clientId, const char* url) {
  std::map<std::string, RegFile>::const_iterator it = m_rfiles.find(url);
  if (it == m_rfiles.end()) {
    m_log.dbg("client %u requested invalid file (%s)", clientId, url);
    write(clientId, "HTTP/1.1 400\r\n");
    write(clientId, "\r\n");
    return;
  }
  FILE *fin = fopen(it->second.path.c_str(), "rb");
  if (fin == NULL) {
    m_log.err("client %u - error opening file %s", it->second.path.c_str());
    write(clientId, "HTTP/1.1 400\r\n");
    write(clientId, "\r\n");
    return;
  }
  m_log.dbg("client %u <- sending file '%s'", clientId, url);
  write(clientId, "HTTP/1.1 200 OK\r\n");
  write(clientId, "Content-Type: %s\r\n", MimeTypeStr[it->second.mimetype]);
  write(clientId, "Cache-Control: no-cache\r\n");
  write(clientId, "\r\n"); //TODO: Content-Length: 4100
  {
    /* read file and send to client */
    uint8_t buf[16*1024];
    while (true) {
      const size_t len = fread(buf, 1, sizeof(buf), fin);
      if (len == 0) {
        break;
      }
      write(clientId, buf, len);
    }
  }
  fclose(fin);
  m_srv->disconnect(clientId);
}
