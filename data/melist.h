#include <stdio.h>
#include <stdint.h>
#include <vector>

/*----------------------------------------------------------------------------*/

class MatEntryList {
public:
  struct MatEntry {
    uint32_t i;
    uint32_t j;
    uint8_t v;
    void print();
    bool operator < (const MatEntry& str) const;
  };

  MatEntryList(uint32_t n);
  static bool readMatPlinkTri(const char* fn, MatEntryList *mel);
  void addValue(uint32_t i, uint32_t j, float value);
  void push(uint32_t i, uint32_t j, float value);
  void close();
  void sort();
  void print();
  void printLen();
  const std::vector<MatEntry>& l() const;
  uint32_t n() const;
  bool writeList(const char *fn) const;
private:
  std::vector<MatEntry> m_list;
  uint32_t m_n;
};

/*----------------------------------------------------------------------------*/

class MatrixEntrySink {
public:
  MatrixEntrySink() : m_numE(0) {}
  virtual ~MatrixEntrySink(){};
  void addEntryF(uint32_t i, uint32_t j, float value);
  void addEntryB(uint32_t i, uint32_t j, uint8_t b);
  virtual bool newEntry(const MatEntryList::MatEntry &) = 0;
  uint64_t numEntries() const {return m_numE;}
private:
  uint64_t m_numE;
};

/*----------------------------------------------------------------------------*/

/* writes a file containing MatEntry */
class MatEntryWriter : public MatrixEntrySink {
public:
  MatEntryWriter();
  bool open(const char *fn);
  bool close(uint32_t n);
protected:
  virtual bool newEntry(const MatEntryList::MatEntry &);
private:
  FILE *m_f;
};

/*----------------------------------------------------------------------------*/

/* reads a file containing MatEntry */
class MatEntryReader {
public:
  MatEntryReader();
  bool open(const char *fn);
  bool read(MatEntryList::MatEntry *e);
  void close();
  uint32_t n() const { return m_n; }
  uint64_t numEntries() const { return m_numEntries; }

private:
  FILE *m_f;
  uint32_t m_n;
  uint64_t m_numEntries;
};
