#include "wsservice.h"
#include "bitops_def.h"
#include "buffer.h"
#include <memory.h>
#include <assert.h>


/*
 * Websocket spec: https://tools.ietf.org/html/rfc6455#page-38
 *
 */

/*----------------------------------------------------------------------------*/

#define WS_OPCODE_TEXTFAME 0x01

/*----------------------------------------------------------------------------*/

WsService::~WsService() {
  delete m_dhfac;
  for (auto cl = m_clients.begin(); cl != m_clients.end(); ++cl) {
    delete cl->second;
  }
}

/*----------------------------------------------------------------------------*/

struct WebSockHeaderUnpacked {
  bool      fin;
  bool      rsv1;
  bool      rsv2;
  bool      rsv3;
  uint8_t   opcode;
  bool      mask;
  uint8_t   payload_len;
  uint64_t  ext_payload_len;
  union {
    uint32_t  masking_key;
    uint8_t   masking_key_u8[4];
  };
};

/*----------------------------------------------------------------------------*/

class WebSockHeaderPacked {
public:
  WebSockHeaderPacked() {
    memset(m_hdata, 0, sizeof(m_hdata));
    m_hlen = 10;
    setFin(true);
    setOpcode(WS_OPCODE_TEXTFAME);
  }
  void setFin(bool b)        { BIT_CLEAR(m_hdata[0], BIT1(7)); BIT_SET1(m_hdata[0], BIT(7, b)); }
  void setOpcode(uint8_t oc) { BIT_CLEAR(m_hdata[0], 0xF);     BIT_SET1(m_hdata[0], oc); }
  void setPayloadLen(uint64_t len) {
    if (len < 126) {
      m_hlen = 2;
      BIT_SET1( m_hdata[1], len);
      return;
    }
    else if (len <= 0xFFFF) {
      BIT_SET1( m_hdata[1], 126);
      m_hlen = 4;
      uint8_t *plen8 = (uint8_t*)&len;
      m_hdata[2] = plen8[1];
      m_hdata[3] = plen8[0];
      return;
    }
    m_hlen = 10;
    BIT_SET1( m_hdata[1], 127);
    uint8_t *plen8 = (uint8_t*)&len;
    m_hdata[2] = plen8[7];
    m_hdata[3] = plen8[6];
    m_hdata[4] = plen8[5];
    m_hdata[5] = plen8[4];
    m_hdata[6] = plen8[3];
    m_hdata[7] = plen8[2];
    m_hdata[8] = plen8[1];
    m_hdata[9] = plen8[0];
  }
  const uint8_t* cdata() const {return m_hdata;}
  const uint32_t len()   const {return m_hlen;}
private:
  uint8_t m_hdata[14];
  uint8_t m_hlen;
};

/*----------------------------------------------------------------------------*/

#include "../3rdparty/TinySHA1.hpp"
void WsService::newData(uint32_t clientId, const uint8_t* data, uint32_t len) {
  /* find client */
  IWsServiceClient *cl = NULL;
  while (true) {
    std::map<uint32_t, IWsServiceClient*>::iterator it = m_clients.find(clientId);
    if (it != m_clients.end()) {
      cl = it->second;
      break;
    }
    /* new client */
    cl = m_dhfac->newWsServiceClient(this, clientId);
    m_clients.insert(std::make_pair(clientId, cl));
    break;
  }
  assert(cl != NULL);
  /* client state? */
  switch (cl->state()) {
    case IWsServiceClient::CONNSTATE_INIT:
      {
        /* parse header for ws key */
        const char *ws_key = NULL;
        char *msg = strndup((char*)data, len);
        {
          char *cur = msg;
          while (cur != NULL) {
            /* search line feed */
            char * end = strchr(cur, '\r');
            if (end == NULL) {
              break;
            }
            /* terminate line end */
            end[0] = '\0';
            end++;
            /* skip newline */
            if (end == NULL || end[0] != '\n') {
              break;
            }
            end++;
            /* split into key+value */
            while ("cur = end") {
              const char *val = strchr(cur, ' ');
              //m_log.dbg("client %u line %s", clientId, cur);
              if (val == NULL || val[0] == '\0') {
                break;
              }
              val++;
              const char keyname[] = "Sec-WebSocket-Key: ";
              if (strncmp(cur, keyname, strlen(keyname)) != 0) {
                break;
              }
              ws_key = val;
              break;
            }
            cur = end;
          }
        }
        while ("free(msg)") {
          if (!ws_key) {
            m_log.dbg("client %u error - sent no key", clientId);
            break;
          }
          const char magic_str[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
          const uint32_t expected_key_len = 24;
          if (strlen(ws_key) != expected_key_len) {
            m_log.dbg("client %u error - invalid key length(%u, '%s')", clientId, strlen(ws_key), ws_key);
            break;
          }
          char sha1_inp[expected_key_len + strlen(magic_str)];
          strcpy(sha1_inp, ws_key);
          strcpy(sha1_inp+expected_key_len, magic_str);
          sha1::SHA1 sha;
          sha.processBytes(sha1_inp, strlen(sha1_inp));
          uint8_t digest[5*4];
          sha.getDigestBytes(digest);
          BufferDyn sha1b64(digest, sizeof(digest));
          sha1b64.toBase64();
          /* generate response */
          BufferDyn buf(4096);
          buf.addf("HTTP/1.1 101 Switching Protocols\r\n");
          buf.addf("Upgrade: websocket\r\n");
          buf.addf("Connection: Upgrade\r\n");
          buf.addf("Sec-WebSocket-Accept: %s\r\n", sha1b64.cdata());
          //buf.addf("Sec-WebSocket-Protocol: chat\r\n");
          buf.addf("\r\n");
          write(clientId, buf.cdata(), buf.len());
          //m_log.dbg("client %u sent:\n>%s", clientId, buf.data());
          cl->setState(IWsServiceClient::CONNSTATE_CONNECTED);
          m_log.dbg("client %u connected", clientId);
          break;
        }
        free(msg);
      }
      break;
    case IWsServiceClient::CONNSTATE_CONNECTED:
      while (len > 0) {
        if (len < 2) {
          m_log.dbg("client %u error - frame length %u < 2", clientId, len);
          return;
        }
        WebSockHeaderUnpacked ws_header = {};
        uint32_t p = 0;
        ws_header.fin  = BIT_READ(data[p], BIT1(7));
        ws_header.rsv1 = BIT_READ(data[p], BIT1(6));
        ws_header.rsv2 = BIT_READ(data[p], BIT1(5));
        ws_header.rsv3 = BIT_READ(data[p], BIT1(4));
        ws_header.opcode = data[p] & 0xF;
        p++;
        ws_header.mask        = BIT_READ(data[p], BIT1(7));
        ws_header.payload_len = data[p] & 0x7F;
#if 0
        m_log.dbg("client %u -> %x %x", clientId, data[0], data[1]);
        m_log.dbg("client %u -> fin: %u", clientId, ws_header.fin);
        m_log.dbg("client %u -> opcode: %x", clientId, ws_header.opcode);
        m_log.dbg("client %u -> mask: %u", clientId, ws_header.mask);
        m_log.dbg("client %u -> payload len: %u", clientId, ws_header.payload_len);
        {
          BufferDyn a(data, len);
          a.toAsci();
          m_log.dbg("client %u -> msg: >>%s", clientId, a.cdata());
          BufferDyn x(data, len);
          x.toHexstr();
          m_log.dbg("client %u         >>%s", clientId, x.cdata());
        }
#endif
        assert(ws_header.payload_len < 126 && "need to implement extended payload len");
        p++;
        if (!ws_header.mask) {
          m_log.dbg("client %u error - no masking bit set", clientId);
          return;
        }
        const uint32_t packet_len = p + 4 + ws_header.payload_len;
        if (len < packet_len) {
          m_log.dbg("client %u error - len (%u) < expected len (%u)", clientId, len, packet_len);
          return;
        }
        ws_header.masking_key = ((uint32_t*)&data[p])[0];
        p+=4;
#if 0
        m_log.dbg("client %u -> masking key: %x", clientId, ws_header.masking_key);
#endif

        /* decode message */
        const char *msg = NULL;
        BufferDyn msgb(data+p, ws_header.payload_len);
        for (uint32_t i = 0; i < msgb.len(); i++) {
          msgb.data()[i] ^= ws_header.masking_key_u8[i%4];
        }
        msgb.toAsci();
        msg = (const char*)msgb.cdata();


        m_log.dbg("client %u -> pkt_size:%u,payload_size:%u%s%s", clientId, len, ws_header.payload_len, msg ? " " : "", msg ? msg : "");
        cl->newData(msgb.cdata(), msgb.len());
        /* update pointer to remaining data */
        data +=  packet_len;
        len  -= (packet_len);
      }
      break;
    default:
      {
        assert(false && "state not implemented");
      }
      break;
  }
}

/*----------------------------------------------------------------------------*/

void WsService::disconnect(uint32_t clientId) {
  std::map<uint32_t, IWsServiceClient*>::iterator cl = m_clients.find(clientId);
  if (cl != m_clients.end()) {
    delete cl->second;
    m_clients.erase(cl);
    m_log.dbg("client %u disconnect", clientId);
  }
}

/*----------------------------------------------------------------------------*/

void WsService::integrate(int64_t sericeTimeUsec) {
  for (auto cl = m_clients.begin(); cl != m_clients.end(); ++cl) {
    cl->second->integrate(sericeTimeUsec);
  }
}

/*----------------------------------------------------------------------------*/

void WsService::sendData(uint32_t clientId, const uint8_t* data, uint32_t len) {
  WebSockHeaderPacked wsh;
  wsh.setPayloadLen(len);
  BufferDyn pkt(wsh.len() + len);
  pkt.add(wsh.cdata(), wsh.len());
  pkt.add(data, len);
  m_srv->write(clientId, pkt.cdata(), pkt.len());
  m_log.dbg("client %u <- pkt_size:%u,payload_size:%u", clientId, pkt.len(), len);
#if 0
  {
    /* show packet */
    Buffer out(len+1);
    for (uint32_t i = 0; i < len; i++) {
      out.addf("%c", data[i]);
    }
    m_log.dbg("client %u msg(%u,%u): > %s", cl->id, len, out.len(), out.cdata());
  }
#endif
}
