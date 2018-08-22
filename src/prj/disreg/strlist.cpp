#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/

class StrList {
public:
  void readList(const char * filename, char comment = '#') {
    std::ifstream file;
    std::string line;

    file.open(filename);
    if (!file) {
      fprintf(stderr, "error - cannot open file %s\n", filename);
      exit(1);
    }
    while (getline((file), line)) {
       if ((line.length() > 0) && (line.at(0) != comment)) {
         m_l.push_back(line);
       }
    }
    file.close();
  }

  const std::vector<std::string> & get() const {
    return m_l;
  }
private:
  std::vector<std::string> m_l;
};

/*----------------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

class StrTable {
public:
  void read(const char * filename, bool header = false, uint32_t maxlines = 0, char sep = '\t') {
    char buffer[1024];
    FILE *f = fopen(filename, "r");
    assert(f);
    /* get number of columns */
    assert(fgets(buffer, sizeof(buffer) - 1, f) && "empty file?");
    uint32_t ncol = 0;
    {
      const char *s = buffer;
      while (s[0] != '\0') {
        ncol += s[0] == sep  ? 1 : 0;
        ncol += s[0] == '\n' ? 1 : 0;
        s++;
      }
    }
    rewind(f);
    maxlines += (maxlines && header) ? 1 : 0; /* do not count any header */
    uint32_t line = 0;
    while (fgets(buffer, sizeof(buffer) - 1, f) != NULL) {
      line++;
      std::vector<std::string> row;
      char *s = buffer;
      char *e = s;
      while (e[0] != '\0') {
        if (e[0] != sep && e[0] != '\n') {
          e++;
          continue;
        }
        e[0] = '\0';
        row.push_back(s);
        e++;
        s = e;
      }
      if (row.size() != ncol) {
        fprintf(stderr, "error: %s, line %u has %lu elements - expected %u\n",
            filename, line, row.size(), ncol);
        exit(1);
      }
      if (line == 1 && header == true) {
        m_h = row;
      } else {
        m_b.push_back(row);
      }
      if (maxlines && line >= maxlines) {
        break;
      }
    }
    fclose(f);
    printf("nrows:%u ncols:%u\n", nrows(), ncols());
  }

  const std::vector<std::vector<std::string>> & body() const {
    return m_b;
  }

  const std::vector<std::string> & header() const {
    return m_h;
  }

  uint32_t ncols() const {
    return m_b.empty() ? 0 : m_b.front().size();
  }
  uint32_t nrows() const {
    return m_b.size();
  }

private:
  std::vector<std::vector<std::string>> m_b;
  std::vector<std::string> m_h;
};
