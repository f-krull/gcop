#include "l_buffer.h"
#include <assert.h>
#include <string.h>
#include "httpservice.h"


// https://tools.ietf.org/html/rfc2616


/*----------------------------------------------------------------------------*/

#define HTTP_STATUS_OK                  200
#define HTTP_STATUS_BADREQUEST          400
#define HTTP_STATUS_NOTFOUND            404
#define HTTP_STATUS_INTERNALSERVERERROR 500

/*----------------------------------------------------------------------------*/

class HttpHeader {
public:
  bool connectionKeepAlive;

  HttpHeader() {
    connectionKeepAlive = false;
  }

  bool setHeaderField(const char *name, const char* value) {
    if (strcmp(name, "Connection:") == 0) {
      connectionKeepAlive = strcmp(value, "keep-alive") == 0;
      return true;
    }
    return false;
  }
private:
};

/*----------------------------------------------------------------------------*/

HttpService::HttpService() : m_log("HttpFileServer") {
}

/*----------------------------------------------------------------------------*/

#define HTTP_CMD_GET "GET"

/*----------------------------------------------------------------------------*/

const char* HttpService::MimeTypeStr[] = {
#define  ENUM_GET_STR(name, str) str,
        ENUM_MIMETYPE(ENUM_GET_STR)
        "undefined"
#undef ENUM_GET_NAME
};

/*----------------------------------------------------------------------------*/

#include "helper.h"
void HttpService::newData(uint32_t clientId, const uint8_t* _data, uint32_t _len) {
  if (_len == 0 || _len > 1e6) {
    m_log.err("client %u - received invalid size (%u bytes)", clientId, _len);
    return;
  }
  BufferDyn buf(_data, _len);
  buf.addf(""); /* null-term str */
  char *line1 = (char*)buf.data();
  char *lineN = gettoken(line1, '\n');
  char *inget = line1;
  char *inurl = gettoken(inget, ' ');
  char *inrem = gettoken(inurl, ' ');
  gettoken(inrem, '\r');
  if (strcmp(HTTP_CMD_GET, inget) != 0) {
    m_log.err("client %u - expected %s, received (%s)", clientId, HTTP_CMD_GET, inget);
    return;
  }
  if (inurl[0] == '\0') {
    m_log.err("client %u - didn't send url", HTTP_CMD_GET);
    write(clientId, "HTTP/1.1 %u\r\n", HTTP_STATUS_BADREQUEST);
    write(clientId, "\r\n");
    return;
  }
  m_log.dbg("client %u -> request %s %s %s ...", clientId, inget, inurl, inrem);
  /* get header fields */
  HttpHeader hd;
  while (lineN[0] != '\0' && lineN[0] != '\r') {
    char *hfname = lineN;
    lineN = gettoken(lineN, '\n');
    char *hfvalue = gettoken(hfname, ' ');
    gettoken(hfvalue, '\r');
    const bool knownHf = hd.setHeaderField(hfname, hfvalue);
    m_log.dbg("client %u -> %sheader field: %s %s", clientId, knownHf ? "" : "ignoring ", hfname, hfvalue);
  }
  http_get(clientId, inurl, &hd);
}

/*----------------------------------------------------------------------------*/

void HttpService::registerFile(const char *path, const char *url, MimeType mt) {
  RegUrl r;
  r.mimetype = mt;
  r.path     = path;
  m_rurls[url] = r;
}

/*----------------------------------------------------------------------------*/

void HttpService::http_get(uint32_t clientId, const char* url, const HttpHeader *header) {
  std::map<std::string, RegUrl>::const_iterator it = m_rurls.find(url);
  if (it == m_rurls.end()) {
    m_log.dbg("client %u requested invalid file (%s)", clientId, url);
    write(clientId, "HTTP/1.1 %u\r\n", HTTP_STATUS_NOTFOUND);
    write(clientId, "\r\n");
    return;
  }
  FILE *fin = fopen(it->second.path.c_str(), "rb");
  if (fin == NULL) {
    m_log.err("client %u - error opening file %s", it->second.path.c_str());
    write(clientId, "HTTP/1.1 %u\r\n", HTTP_STATUS_INTERNALSERVERERROR);
    write(clientId, "\r\n");
    return;
  }
  /* get file size */
  fseek(fin, 0L, SEEK_END);
  const size_t flen = ftell(fin);
  rewind(fin);
  m_log.dbg("client %u <- sending file '%s' size=%zu", clientId, url, flen);
  write(clientId, "HTTP/1.1 200 OK\r\n");
  write(clientId, "Content-Type: %s\r\n", MimeTypeStr[it->second.mimetype]);
  write(clientId, "Cache-Control: no-cache\r\n");
  write(clientId, "Content-Length: %zu\r\n", flen);
  write(clientId, "\r\n");
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
  if (!header->connectionKeepAlive) {
    m_srv->disconnect(clientId);
  }
}
