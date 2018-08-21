
#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>
#include <stdio.h>

/*----------------------------------------------------------------------------*/

class BufferDyn {
public:
  BufferDyn(uint32_t capacity = 2);
  BufferDyn(const uint8_t* data, uint32_t len);
  ~BufferDyn();
  void addf(const char *fmt, ...); /* will null-terminate buffer at [len] */
  void setf(const char *fmt, ...); /* will null-terminate buffer at [len] */
  void add(const uint8_t *d, uint32_t len);
  void add(char);
  void set(const uint8_t *d, uint32_t len);
  void swap(BufferDyn &b);
  void clear();
  uint8_t *data();
  const uint8_t *data() const;
  const uint8_t *cdata() const;
  const char* cstr() const;
  uint32_t len() const;
  uint32_t max() const;
  void toAsci();
  void toHexstr(uint32_t d = 2);
  void toBase64();
  void printinfo() const;
private:
  uint32_t m_max;
  uint32_t m_capacity;
  uint32_t m_len;
  uint8_t *m_buf;
  void requestSize(uint32_t sd);
  void setf(uint32_t pos, const char *fmt, va_list args);
  static char m_b64EncTab[];
  //BufferDyn & operator=(const BufferDyn &o);
};

#endif /* BUFFER_H_ */
