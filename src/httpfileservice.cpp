#include "httpfileservice.h"
#include <assert.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

HttpFileService::HttpFileService() : m_log("HttpFileServer") {
}

/*----------------------------------------------------------------------------*/

void HttpFileService::newData(uint32_t clientId, const uint8_t* data, uint32_t len) {
  assert(len > 0 && len < 1e6);

  //TODO: check ^GET
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
  //m_log.dbg("HttpReceiver: client %u url %s", clientId, url);
  http_get(clientId, url);
  delete [] url;
}

/*----------------------------------------------------------------------------*/

void HttpFileService::registerFile(const char *path, const char *url, const char *mimetype) {
  RegFile r;
  r.mimetype = mimetype;
  r.path     = path;
  m_rfiles[url] = r;
}

/*----------------------------------------------------------------------------*/

void HttpFileService::http_get(uint32_t clientId, const char* url) {
  std::map<std::string, RegFile>::const_iterator it = m_rfiles.find(url);
  if (it == m_rfiles.end()) {
    m_log.dbg("HttpReceiver: client %u url: %s", clientId, url);
    write(clientId, "HTTP/1.1 400\r\n");
    write(clientId, "\r\n");
    return;
  }
  m_log.dbg("HttpReceiver: client %u file '%s' found", clientId, url);
  FILE *fin = fopen(it->second.path.c_str(), "rb");
  if (fin == NULL) {
    m_log.err("HttpReceiver: opening file %s", it->second.path.c_str());
    write(clientId, "HTTP/1.1 400\r\n");
    write(clientId, "\r\n");
    return;
  }
  write(clientId, "HTTP/1.1 200 OK\r\n");
  write(clientId, "Content-Type: %s\r\n", it->second.mimetype.c_str());
  //TODO: Content-Length: 4100
  write(clientId, "Cache-Control: no-cache\r\n");
  write(clientId, "\r\n");
  {
    /* read file and send to client */
    uint8_t buf[1024];
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
