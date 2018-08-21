#ifndef MONSENDER_H_
#define MONSENDER_H_

#include <stdint.h>

class Client {
public:
   virtual ~Client() {};
   
   virtual bool send(const char* msg, ...) = 0;
   virtual bool sendData(uint8_t* data, uint32_t size) = 0;

private:
};

/*----------------------------------------------------------------------------*/

#include <arpa/inet.h> //inet_addr
#include <string>
#include "debuglog.h"

class ClientTcp  {
public:

   ClientTcp();
   virtual ~ClientTcp();

   struct Config {
      std::string server;
      uint32_t port;
      std::string name;
   };

   virtual bool send(const char* msg, ...);
   virtual bool sendData(uint8_t* data, uint32_t size);

   bool isConnected();

protected:
   bool connect();
   void disconnect();

private:
   Log m_log;
   int m_sock;
   struct sockaddr_in m_server;
   char m_buffer[1024];

   Config m_config;

};


#endif /* MONSENDER_H_ */
