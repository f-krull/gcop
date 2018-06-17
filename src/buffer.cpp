#include "buffer.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/

BufferDyn::BufferDyn(uint32_t capacity) : m_max(UINT32_MAX), m_capacity(capacity), m_len(0), m_buf(NULL) {
  m_buf = (uint8_t*)malloc(m_capacity);
  assert(m_buf && "malloc failed");
}

/*----------------------------------------------------------------------------*/

BufferDyn::BufferDyn(const uint8_t *data, uint32_t len) : m_max(UINT32_MAX), m_capacity(len), m_len(len), m_buf(NULL) {
  m_buf = (uint8_t*)malloc(m_capacity);
  assert(m_buf && "malloc failed");
  memcpy(m_buf, data, len);
}
/*----------------------------------------------------------------------------*/

BufferDyn::BufferDyn(const char *fmt, ...) : m_max(UINT32_MAX), m_capacity(strlen(fmt)), m_len(0), m_buf(NULL) {
  m_buf = (uint8_t*)malloc(m_capacity);
  va_list args;
  va_start(args, fmt);
  setf(0, fmt, args);
  va_end(args);
}

/*----------------------------------------------------------------------------*/

BufferDyn::~BufferDyn() {
  if (m_buf) {
    free(m_buf);
  }
}

/*----------------------------------------------------------------------------*/

void BufferDyn::addf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  setf(m_len, fmt, args);
  va_end(args);
}

/*----------------------------------------------------------------------------*/

void BufferDyn::setf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  setf(0, fmt, args);
  va_end(args);
}

/*----------------------------------------------------------------------------*/

void BufferDyn::add(const uint8_t *d, uint32_t len) {
  requestSize(len);
  /* how much can be copied? */
  memcpy(m_buf+m_len, d, len);
  m_len += len;
}

/*----------------------------------------------------------------------------*/

void BufferDyn::add(char c) {
  add((const uint8_t*)&c, 1);
}

/*----------------------------------------------------------------------------*/

void BufferDyn::set(const uint8_t *d, uint32_t len) {
  requestSize(len);
  memcpy(m_buf, d, len);
  m_len = len;
}

/*----------------------------------------------------------------------------*/

void BufferDyn::clear() {
  m_len = 0;
}

/*----------------------------------------------------------------------------*/

uint8_t *BufferDyn::data() {
  return m_buf;
}

/*----------------------------------------------------------------------------*/

const uint8_t *BufferDyn::data() const {
  return m_buf;
}

/*----------------------------------------------------------------------------*/

const uint8_t *BufferDyn::cdata() const {
  return m_buf;
}

/*----------------------------------------------------------------------------*/

const char *BufferDyn::cstr() const {
  return (char*)m_buf;
}

/*----------------------------------------------------------------------------*/

char *BufferDyn::str() {
  return (char*)m_buf;
}

/*----------------------------------------------------------------------------*/

uint32_t BufferDyn::len() const {
  return m_len;
}

/*----------------------------------------------------------------------------*/

uint32_t BufferDyn::max() const {
  return m_max;
}

/*----------------------------------------------------------------------------*/

void BufferDyn::requestSize(uint32_t sdelta) {
  const uint32_t lenNeeded = m_len + sdelta;
  if (lenNeeded <= m_capacity) {
    return;
  }
  /* compute size needed */
  uint32_t capacity_new = m_capacity > 0 ? m_capacity : 4;
  while ((capacity_new < lenNeeded) && (capacity_new < (m_max / 2))) {
    capacity_new *= 2;
  }
  if (capacity_new < m_max) {
    m_capacity = capacity_new;
    m_buf = (uint8_t*)realloc(m_buf, m_capacity);
    return;
  }
  assert(false && "requested len is bigger than max");
}

/*----------------------------------------------------------------------------*/

void BufferDyn::setf(uint32_t pos, const char *fmt, va_list args) {
  while (true) {
    va_list argscp;
    /* va_list can't be reused - always make a copy TODO: inline above */
    va_copy(argscp , args);
    assert(pos <= m_capacity);
    const uint32_t remaining = m_capacity - pos;
    const int32_t n = vsnprintf((char*)m_buf+pos, remaining, fmt, argscp);
    va_end(argscp);
    if (n < (int32_t)remaining) {
      break;
    }
    requestSize(n+1);
  }
  //TODO: does not work when mixing str and bin
  m_len = strlen((char*)m_buf);
}

/*----------------------------------------------------------------------------*/

void BufferDyn::swap(BufferDyn &o) {
  BufferDyn tmp = *this;
  *this = o;
  o = tmp;
  tmp.m_buf = NULL;
}

/*----------------------------------------------------------------------------*/

void BufferDyn::toAsci() {
  BufferDyn b(4);
  for (uint32_t i = 0; i < m_len; i++) {
    const char cin  = m_buf[i];
    const char cout = (cin >= 32 && cin <= 126) ? cin : '.';
    b.addf("%c", cout);
  }
  swap(b);
}

/*----------------------------------------------------------------------------*/

void BufferDyn::toHexstr(uint32_t d) {
  BufferDyn b;
  for (uint32_t i = 0; i < m_len; i++) {
    b.addf("%02x%s", m_buf[i], ((i+1) % d == 0) ? " " : "" );
  }
  swap(b);
}

/*----------------------------------------------------------------------------*/


char BufferDyn::m_b64EncTab[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                 'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                 '4', '5', '6', '7', '8', '9', '+', '/'};

/*----------------------------------------------------------------------------*/

void BufferDyn::toBase64() {
  BufferDyn b(4 * ((len() + 2) / 3));
  for (uint32_t i = 0; i < len(); i+=3) {
    const uint32_t octet_a = i+0 < len() ? (unsigned char)data()[i+0] : 0;
    const uint32_t octet_b = i+1 < len() ? (unsigned char)data()[i+1] : 0;
    const uint32_t octet_c = i+2 < len() ? (unsigned char)data()[i+2] : 0;
    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
    b.add(m_b64EncTab[(triple >> 3 * 6) & 0x3F]);
    b.add(m_b64EncTab[(triple >> 2 * 6) & 0x3F]);
    b.add(m_b64EncTab[(triple >> 1 * 6) & 0x3F]);
    b.add(m_b64EncTab[(triple >> 0 * 6) & 0x3F]);
  }
  uint32_t mod_table[] = {0, 2, 1};
  for (uint32_t i = 0; i < mod_table[len() % 3]; i++)
      b.data()[b.len() - 1 - i] = '=';
  b.addf(""); /* null-term string */
  swap(b);
}

/*----------------------------------------------------------------------------*/

void BufferDyn::printinfo() const {
  printf("max=%u\n", m_max);
  printf("len=%u\n", m_len);
  printf("capacity=%u\n", m_capacity);
  printf("buf=%p\n", m_buf);
}
