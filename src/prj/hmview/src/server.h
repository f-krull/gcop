#ifndef SERVER_H_
#define SERVER_H_

#include "debuglog.h"
#include <vector>
#include <stdint.h>
#include <sys/socket.h>  //socket
#include <arpa/inet.h>   //inet_addr

/*----------------------------------------------------------------------------*/
class Server;

class ISocketService {
public:
  ISocketService() : m_srv(NULL) {}
  virtual ~ISocketService() {}
  virtual void setSource(Server *s) {m_srv = s;}
  virtual void newData(uint32_t clientId, const uint8_t* data, uint32_t len) = 0;
  virtual void disconnect(uint32_t clientId) {}
protected:
  /* convenience functions */
  void write(uint32_t clientId, const uint8_t* data, uint32_t len);
  void write(uint32_t clientId, const char *msg, ...);
  Server *m_srv;
};

/*----------------------------------------------------------------------------*/

class Server {
public:
  Server(ISocketService *s) : m_service(s) {m_service->setSource(this);}
  virtual ~Server() {delete m_service;}
  virtual bool write(uint32_t clientId, const uint8_t* data, uint32_t len) = 0;
  virtual bool disconnect(uint32_t clientId) = 0;
  virtual void integrate() = 0;
  ISocketService * getService() {return m_service;}
protected:
  ISocketService* m_service;
};

/*----------------------------------------------------------------------------*/

#include "l_buffer.h"
#include <string>
#include <map>

struct ServerTcpConfig {
  uint32_t port;
};

class ServerTcp : public Server {
public:
  ServerTcp(ISocketService *s, const ServerTcpConfig &cfg);
  ~ServerTcp();

  virtual void integrate();
  virtual bool write(uint32_t clientId, const uint8_t* data, uint32_t len);
  virtual bool disconnect(uint32_t clientId);

  bool listen();
  bool isListening() const;
  void close();

private:
  ServerTcpConfig m_config;
  Log m_log;

  int m_server;
  struct sockaddr_in m_addr;

  class Client {
  public:
    int fd;
    std::string addr;
    uint32_t id;
  };
  std::map<uint32_t, Client*> m_clients;
  uint32_t m_lastClientId;
  uint8_t *m_rxbuf;
  void handleDisconnected();
};

#endif /* SERVER_H_ */
