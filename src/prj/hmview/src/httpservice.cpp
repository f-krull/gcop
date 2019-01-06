#include "l_buffer.h"
#include <assert.h>
#include <string.h>
#include "httpservice.h"

// https://tools.ietf.org/html/rfc2616

/*----------------------------------------------------------------------------*/

HttpHeader::HttpHeader() {
  connectionKeepAlive = false;
}

/*----------------------------------------------------------------------------*/

bool HttpHeader::setHeaderField(const char *name, const char* value) {
  if (strcmp(name, "Connection:") == 0) {
    connectionKeepAlive = strcmp(value, "keep-alive") == 0;
    return true;
  }
  return false;
}

/*----------------------------------------------------------------------------*/

HttpService::HttpService() : m_log("HttpServer") {
}

/*----------------------------------------------------------------------------*/

HttpService::~HttpService() {
  for (uint32_t i = 0; i < m_handles.size(); i++) {
    delete m_handles[i];
  }
  m_handles.clear();
}

/*----------------------------------------------------------------------------*/

class FileUrlHandle : public IUrlHandle {
public:
  FileUrlHandle(const char *path, HttpService::MimeType mt, Log *log) :
      m_path(path), m_mimetype(mt), m_log(log) {
  }
  virtual ~FileUrlHandle() {}
  virtual void handleUrl(uint32_t clientId, const char *url, ISocketService* s);
private:
  std::string m_path;
  HttpService::MimeType m_mimetype;
  Log *m_log;
};

/*----------------------------------------------------------------------------*/

void FileUrlHandle::handleUrl(uint32_t clientId, const char *url, ISocketService* s) {
  FILE *fin = fopen(m_path.c_str(), "rb");
  if (fin == NULL) {
    m_log->err("client %u - error opening file %s", m_path.c_str());
    s->write(clientId, "HTTP/1.1 %u\r\n", HttpService::HTTP_STATUS_INTERNALSERVERERROR);
    s->write(clientId, "\r\n");
    return;
  }
  /* get file size */
  fseek(fin, 0L, SEEK_END);
  const size_t flen = ftell(fin);
  rewind(fin);
  m_log->dbg("client %u <- sending file '%s' size=%zu", clientId, url, flen);
  s->write(clientId, "HTTP/1.1 200 OK\r\n");
  s->write(clientId, "Content-Type: %s\r\n", HttpService::MimeTypeStr[m_mimetype]);
  s->write(clientId, "Cache-Control: no-cache\r\n");
  s->write(clientId, "Content-Length: %zu\r\n", flen);
  s->write(clientId, "\r\n");
  {
    /* read file and send to client */
    uint8_t buf[16*1024];
    while (true) {
      const size_t len = fread(buf, 1, sizeof(buf), fin);
      if (len == 0) {
        break;
      }
      s->write(clientId, buf, len);
    }
  }
  fclose(fin);
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
  std::map<std::string, IUrlHandle*>::const_iterator it = m_rurls.find(inurl);
  if (it == m_rurls.end()) {
    m_log.dbg("client %u requested invalid url (%s)", clientId, inurl);
    write(clientId, "HTTP/1.1 %u\r\n", HTTP_STATUS_NOTFOUND);
    write(clientId, "\r\n");
    return;
  }
  it->second->handleUrl(clientId, inurl, this);
  if (!hd.connectionKeepAlive) {
    m_srv->disconnect(clientId);
  }
}

/*----------------------------------------------------------------------------*/

void HttpService::registerFile(const char *path, const char *url, MimeType mt) {
  IUrlHandle *h = new FileUrlHandle(path, mt, &m_log);
  registerUrl(h, url);
  /* delete later */
  m_handles.push_back(h);
}

/*----------------------------------------------------------------------------*/

void HttpService::registerUrl(IUrlHandle *h, const char *url) {
  m_rurls[url] = h;
}

/*----------------------------------------------------------------------------*/

