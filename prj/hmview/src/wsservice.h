#ifndef WSSERVICE_H_
#define WSSERVICE_H_

#include "server.h"

/*----------------------------------------------------------------------------*/

class IWsServiceClient {
public:
  virtual ~IWsServiceClient() {}
  IWsServiceClient(uint32_t cid) :
      m_id(cid), m_connstate(CONNSTATE_INIT) {}
  virtual void newData(const uint8_t* data, uint32_t len) = 0;
  enum ConnectionState {
    CONNSTATE_INIT,
    CONNSTATE_CONNECTED
  };
  uint32_t id() const {return m_id;}
  virtual void integrate(int64_t serviceTimeUsec) {}
  ConnectionState state() const {return m_connstate;}
  void setState(ConnectionState s) {m_connstate = s;}
  virtual void disconnect() {};
protected:
  uint32_t m_id;
  ConnectionState m_connstate;
};

/*----------------------------------------------------------------------------*/

class WsService;
class IWsServiceClientFractory {
public:
  virtual ~IWsServiceClientFractory() {}
  virtual IWsServiceClient* newWsServiceClient(WsService *s, uint32_t cid) = 0;
};

/*----------------------------------------------------------------------------*/

class WsService : public ISocketService {
public:
  enum SendFlags {
    SEND_BINARY = 0x02
  };

  WsService(IWsServiceClientFractory *f) : m_dhfac(f), m_log("WsService") {}
  virtual ~WsService();
  virtual void newData(uint32_t clientId, const uint8_t* data, uint32_t len);
  void sendData(uint32_t clientId, const uint8_t* data, uint32_t len, int32_t flags = 0);
  virtual void disconnect(uint32_t clientId);
  void integrate(int64_t serviceTimeUsec);
protected:
  std::map<uint32_t, IWsServiceClient*> m_clients;
  IWsServiceClientFractory *m_dhfac;
private:
  Log m_log;
};

#endif /* WSSERVICE_H_ */
