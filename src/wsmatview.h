
#ifndef WSMATVIEW_H_
#define WSMATVIEW_H_

#include "wsservice.h"


/*----------------------------------------------------------------------------*/
class WsMatViewPriv;
class WsMatView : public IWsServiceClient {
public:
  WsMatView(WsService* s, uint32_t clientId);
  virtual ~WsMatView();
  virtual void integrate(int64_t serviceTimeUsec);
protected:
  virtual void newData(const uint8_t* data, uint32_t len);
private:
  void sendStatus(char s);
  void sendTiny();
  void sendMain();
  void sendInfo();
  void sendYlab();
  void sendXlab();
  void renderMain();
  void renderXlab();
  void renderYlab();
  void renderTiny();
  Log m_log;
  int64_t m_nextupdate;
  WsService *m_srv;
  WsMatViewPriv *m;
};

/*----------------------------------------------------------------------------*/

class WsMatViewFactory : public IWsServiceClientFractory {
public:
  WsMatViewFactory() {}
  virtual ~WsMatViewFactory() {}
  virtual WsMatView* newWsServiceClient(WsService* s, uint32_t cid) {
    return new WsMatView(s, cid);
  };
};

/*----------------------------------------------------------------------------*/

#endif /* WSMATVIEW_H_ */
