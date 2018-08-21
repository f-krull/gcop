#include "client.h"
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>

#define SOCK_DISCONNECTED -1

/*----------------------------------------------------------------------------*/

ClientTcp::ClientTcp() {
   m_sock = SOCK_DISCONNECTED;

   m_config.server = "127.0.0.1";
   m_config.port = 11380;
}

/*----------------------------------------------------------------------------*/

ClientTcp::~ClientTcp() {
   disconnect();
}

/*----------------------------------------------------------------------------*/

bool ClientTcp::connect() {
   m_log.dbg("connecting to %s %u", m_config.server.c_str(), m_config.port);
   m_sock = socket(AF_INET, SOCK_STREAM, 0);
   if (m_sock == -1) {
      m_log.err("%p could not create socket", (void*)this);
      return false;
   }
   m_server.sin_addr.s_addr = inet_addr(m_config.server.c_str());
   m_server.sin_family = AF_INET;
   m_server.sin_port = htons(m_config.port);
   if (::connect(m_sock, (struct sockaddr *) &m_server, sizeof(m_server)) < 0) {
      m_log.err("Could not create socket");
      disconnect();
      return false;
   }
   m_log.dbg("%p connected", (void*)this);
   return true;
}

/*----------------------------------------------------------------------------*/

void ClientTcp::disconnect() {
   close(m_sock);
   m_sock = SOCK_DISCONNECTED;
}

/*----------------------------------------------------------------------------*/

bool ClientTcp::isConnected() {
   return m_sock != SOCK_DISCONNECTED;
}

/*----------------------------------------------------------------------------*/

bool ClientTcp::send(const char* msg, ...) {
   va_list args;
   va_start(args, msg);
   vsnprintf(m_buffer, sizeof(m_buffer), msg, args);
   va_end(args);
   return sendData((uint8_t*)m_buffer, strlen(m_buffer));
}

/*----------------------------------------------------------------------------*/
#include <errno.h>
bool ClientTcp::sendData(uint8_t *data, uint32_t size) {
   if (isConnected() == false && connect() == false) {
      return false;
   }
   int ret = ::send(m_sock, data, size, MSG_NOSIGNAL); 
   if (ret > 0) {
      return true;
   } 
   if (errno == EPIPE) {
      m_log.dbg("%p disconnected", (void*)this);
      disconnect();
      return false;
   }
   m_log.err("%p send failed; data size %u (code %d: %s)",(void*)this, size, errno, strerror(errno));      
   disconnect();
   return false;
}

