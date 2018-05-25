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
  /* convenience functions */
protected:
  void write(uint32_t clientId, const uint8_t* data, uint32_t len);
  void write(uint32_t clientId, const char *msg, ...);
  Server *m_srv;
};

/*----------------------------------------------------------------------------*/
#include <set>
class Server {
public:
  virtual ~Server() {}
  virtual bool write(uint32_t clientId, const uint8_t* data, uint32_t len) = 0;
  virtual bool disconnect(uint32_t clientId) = 0;

  virtual void integrate() = 0;

  void addReader(ISocketService *r) {
    m_services.insert(r);
    r->setSource(this);
  }
protected:
  std::set<ISocketService*> m_services;
};

/*----------------------------------------------------------------------------*/
#include <string>
#include <map>

class ServerTcpConfig {
public:
  ServerTcpConfig() : port(11380) {}
  uint32_t port;
};

class ServerTcp : public Server {
public:


  ServerTcp(const ServerTcpConfig &cfg);
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
  void handleDisconnected();
};

#endif /* SERVER_H_ */
