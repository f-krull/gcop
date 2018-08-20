#ifndef SRC_FAMWRITER_H_
#define SRC_FAMWRITER_H_

#include <string>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class FamWriter {
public:
  struct Entry {
    std::string fid;
    std::string id;
    std::string fatherId;
    std::string motherId;
    std::string sex;
    std::string phenotype;

    Entry(const char *_id) {
      fid = _id;
      id = _id;
      fatherId = "0";
      motherId = "0";
      sex = SEXCODE_NA;
      phenotype = PHENOTYPE_NA;
    }
  };

  static const char SEXCODE_MALE[];
  static const char SEXCODE_FEMALE[];
  static const char SEXCODE_NA[];

  static const char PHENOTYPE_CONTROL[];
  static const char PHENOTYPE_CASE[];
  static const char PHENOTYPE_NA[];

  FamWriter();
  bool open(const char *fn);
  bool write(const Entry &e);
  uint32_t numWritten() const { return m_numWritten; } /* call before close() */
  void close();

private:
  FILE *m_f;
  std::string m_fn;
  uint32_t m_numWritten;
};

#endif /* SRC_FAMWRITER_H_ */
