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
