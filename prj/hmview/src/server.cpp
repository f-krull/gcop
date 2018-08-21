#include "server.h"
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define SOCK_DISCONNECTED -1

#define RX_BUF_LEN 4096

/*----------------------------------------------------------------------------*/

void ISocketService::write(uint32_t clientId, const uint8_t* data, uint32_t len) {
  m_srv->write(clientId, data, len);
}

/*----------------------------------------------------------------------------*/

void ISocketService::write(uint32_t clientId, const char *fmt, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  m_srv->write(clientId, (const uint8_t*)buffer, strlen(buffer));
}

/*----------------------------------------------------------------------------*/

ServerTcp::ServerTcp(ISocketService *s, const ServerTcpConfig &cfg) : Server(s) {
  m_config = cfg;
  m_server = SOCK_DISCONNECTED;
  m_lastClientId = 0;
  m_log.setPrefix("ServerTcp%u", m_config.port);
  m_rxbuf = new uint8_t[RX_BUF_LEN];
}

/*----------------------------------------------------------------------------*/

ServerTcp::~ServerTcp() {
  close();
  delete [] m_rxbuf;
}

/*----------------------------------------------------------------------------*/

bool ServerTcp::listen() {
  int reuse = 1;

  m_server = socket(PF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
  setsockopt(m_server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons(m_config.port);
  if (bind(m_server, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_in))
      < 0) {
    m_log.err("can not bind to socket");
    close();
    return false;
  }
  if (::listen(m_server, 8) < 0) {
    m_log.err("error, could not listen on server port");
    close();
    return false;
  }
  m_log.dbg("server listening on %u", m_config.port);
  return true;
}

/*----------------------------------------------------------------------------*/

void ServerTcp::integrate() {
  /* connected? */
  if (m_server == SOCK_DISCONNECTED) {
    return;
  }

  fd_set fds;
  int max = 0;
  struct timeval tv;

  FD_ZERO(&fds);
  FD_SET(m_server, &fds);
  if (m_server >= max) {
    max = m_server + 1;
  }

  for (std::map<uint32_t, Client*>::iterator it = m_clients.begin();
      it != m_clients.end(); ++it) {
    FD_SET(it->second->fd, &fds);
    if (it->second->fd >= max) {
      max = it->second->fd + 1;
    }
  }

  tv.tv_sec = 0;
  tv.tv_usec = 5000;
  if (select(max, &fds, NULL, NULL, &tv) > 0) {
    bool handleDiscon = false;
    /* new data? */
    if (FD_ISSET(m_server, &fds)) {
      /* server socket */
      uint32_t addrsize = sizeof(struct sockaddr_in);
      Client *cl = new Client();
      cl->fd = accept(m_server, (struct sockaddr *) &m_addr, &addrsize);
      cl->addr = inet_ntoa(m_addr.sin_addr);
      cl->id = m_lastClientId++;
      m_clients[cl->id] = cl;
      m_log.dbg("client %u (%s) : connected", cl->id, cl->addr.c_str());
    }
    for (std::map<uint32_t, Client*>::iterator cl = m_clients.begin();
        cl != m_clients.end(); ++cl) {
      /* client sockets */
      if (FD_ISSET(cl->second->fd, &fds)) {
        /* handle data on this connection */
        int32_t dsize = recvfrom(cl->second->fd, m_rxbuf, RX_BUF_LEN, 0, NULL, NULL);
        if (dsize > 0) {
          m_service->newData(cl->second->id, m_rxbuf, dsize);
        }
        /* disconnected */
        if (dsize == 0) {
          ::close(cl->second->fd);
          cl->second->fd = SOCK_DISCONNECTED;
          handleDiscon = true;
          m_service->disconnect(cl->second->id);
        }
      }
    }
    if (handleDiscon) {
      handleDisconnected();
    }
  }
}

/*----------------------------------------------------------------------------*/

bool ServerTcp::write(uint32_t clientId, const uint8_t* data, uint32_t len) {
  if (len == 0) {
    return false;
  }
  std::map<uint32_t, Client*>::iterator it = m_clients.find(clientId);
  if (it == m_clients.end()) {
    m_log.err("client %u not found", clientId);
    return false;
  }
  uint32_t num_written_sum = 0;
  while (num_written_sum < len) {
    const uint32_t num_written = ::write(it->second->fd, data, len - num_written_sum);
    if (num_written == 0) {
      return false;
    }
    num_written_sum += num_written;
  }

  return true;
}

/*----------------------------------------------------------------------------*/

bool ServerTcp::disconnect(uint32_t clientId) {
  std::map<uint32_t, Client*>::iterator cl = m_clients.find(clientId);
  if (cl == m_clients.end()) {
    return false;

  }
  return ::shutdown(cl->second->fd, 2);
}

/*----------------------------------------------------------------------------*/

bool ServerTcp::isListening() const {
  return m_server != SOCK_DISCONNECTED;
}

/*----------------------------------------------------------------------------*/

void ServerTcp::close() {
  /* close all clients */
  std::map<uint32_t, Client*>::iterator cl;
  for (cl = m_clients.begin(); cl != m_clients.end(); ++cl) {
    ::close(cl->second->fd);
    cl->second->fd = SOCK_DISCONNECTED;
    m_service->disconnect(cl->second->id);
  }
  handleDisconnected();
  ::close(m_server);
  m_server = SOCK_DISCONNECTED;
}

/*----------------------------------------------------------------------------*/

void ServerTcp::handleDisconnected() {
  std::map<uint32_t, Client*>::iterator cl;
  for (cl = m_clients.begin(); cl != m_clients.end();) {
    if (cl->second->fd == SOCK_DISCONNECTED) {
      m_log.dbg("client %u (%s) : disconnected ", cl->second->id,
          cl->second->addr.c_str());
      delete cl->second;
      m_clients.erase(cl++);
    } else {
      ++cl;
    }
  }
}
