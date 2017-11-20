
class GcObj {
public:


private:
};

class GcPoints : public GcObj {
public:


private:
};

#include <string>
#include <stdint.h>
class GcScriptInfo {
public:
  GcScriptInfo(const std::string &fn, uint32_t line) :
      m_filename(fn), m_line(line) {}

private:
  std::string m_filename;
  uint32_t    m_line;
};


class GcObjSpace;




/*----------------------------------------------------------------------------*/


#include "scriptenv.h"
int main(int argc, char **argv) {
  GcScriptEnv e;
  e.run("load_seg dst=b file=data/wgEncodeUwDnaseCd20ro01778PkRep1.narrowPeak format=cse skip=0", 1);
  e.run("load_snp dst=a file=data/scz.txt format=1...2...3 skip=1", 1);
  e.run("snp_info src=a", 2);
  return 0;
}

/*
 * a = load_snp file=fn format=1..2.3 skip=1
 * b = load_seg file=fn format=1..2.3 skip=1
 * b = intersect a b
 *
 *
 * */
