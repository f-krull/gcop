/**
 * Opens a gz or plain-text file, and reads it line by line.
 * If the line buffer is too small, reallocates line buffer transparently.
 *
 */

#ifndef SRC_SHARED_LINEREADER_H_
#define SRC_SHARED_LINEREADER_H_

#include <stdint.h>

/*----------------------------------------------------------------------------*/

class LineReaderPriv;

/*----------------------------------------------------------------------------*/

class LineReader {
public:
  LineReader(uint32_t sinit = 0);
  ~LineReader();
  bool open(const char *fn);
  char* readLine();
  void close();
  bool isOpen() const;

private:
  LineReaderPriv *m;
};


#endif /* SRC_SHARED_LINEREADER_H_ */
