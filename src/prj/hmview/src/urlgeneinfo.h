
#ifndef SRC_PRJ_HMVIEW_SRC_URLGENEINFO_H_
#define SRC_PRJ_HMVIEW_SRC_URLGENEINFO_H_

#include "httpservice.h"
#include "debuglog.h"

class UrlGeneInfo : public IUrlHandle {
public:
  UrlGeneInfo() : m_log("UrlGeneInfo") {}
  virtual ~UrlGeneInfo() {}
  virtual void handleUrl(uint32_t clientId, const char *url, const char *query, ISocketService* s);

private:
  Log m_log;
};


#endif /* SRC_PRJ_HMVIEW_SRC_URLGENEINFO_H_ */
