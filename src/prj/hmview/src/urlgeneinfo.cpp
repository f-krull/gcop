#include "urlgeneinfo.h"
#include "l_gettoken.h"

/*----------------------------------------------------------------------------*/

class QueryConfig {
public:
  void declare(const char *name) {
    Parameter p;
    p.name = name;
    p.parsed = false;
    m_params[p.name] = p;
  }

  void parse(const char *q) {
    char *urlin = strdup(q);
    /* skip first part */
    char *param = urlin;
    while (param[0] != '\0') {
      char *value = gettoken(param, '=');
      /* we do not test value[0] != '\0' - an empty string is allowed */
      char *next = gettoken(value, '&');
      setParam(param, value);
      param = next;
    }
    free(urlin);
  }

  bool setParam(const char* name, const char *value) {
    auto it = m_params.find(name);
    if (it == m_params.end()) {
      return false;
    }
    it->second.parsed = true;
    it->second.value  = value;
    return true;
  }

  const char *getParam(const char *name) {
    auto it = m_params.find(name);
    if (it == m_params.end() || it->second.parsed == false) {
      return "";
    }
    return it->second.value.c_str();
  }

private:
  struct Parameter {
    std::string name;
    std::string value;
    bool parsed;
  };

  std::map<std::string, Parameter> m_params;
};

/*----------------------------------------------------------------------------*/

void UrlGeneInfo::handleUrl(uint32_t clientId, const char *, const char *query, ISocketService* s) {
  QueryConfig q;
  q.declare("x0");
  q.declare("x1");
  q.declare("y0");
  q.declare("y1");
  q.parse(query);


  m_log.dbg("(%s,%s), (%s,%s)", q.getParam("x0"), q.getParam("y0"), q.getParam("x1"), q.getParam("y1"));

  /* parse url */
  s->write(clientId, "HTTP/1.1 200 OK\r\n");
  s->write(clientId, "Content-Type: %s\r\n", HttpService::MimeTypeStr[HttpService::MIMETYPE_TEXT_PLAIN]);
  s->write(clientId, "Cache-Control: no-cache\r\n");
  s->write(clientId, "Content-Length: %zu\r\n", 8);
  s->write(clientId, "\r\n");
  s->write(clientId, "hi there");
}
