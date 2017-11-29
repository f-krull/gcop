
#ifndef UTIL_FILE_H_
#define UTIL_FILE_H_

/*----------------------------------------------------------------------------*/

class FilePriv;

class File {
public:
  enum FileType {
    FILETYPE_AUTO,
    FILETYPE_TXT,
    FILETYPE_GZ
  };

  File();
  ~File();
  bool open(const char *fn, const char *mode, FileType ft);
  char *gets(char *buf, int n);
  void close();

private:
  FilePriv *m;
};


#endif /* UTIL_FILE_H_ */
