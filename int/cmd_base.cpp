#include "cmdparam.h"
#include "command.h"

/*----------------------------------------------------------------------------*/

class CmdLoadSnp : public GcCommand {
public:
  CmdLoadSnp() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT_STR, GcCmdParam::PARAM_STRING, "cbe"));
    addParam(GcCmdParam(PARAM_SKIP_INT,   GcCmdParam::PARAM_INT,    "0"));
  }
  const char* name() const {
    return "load_snp";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
  static std::string PARAM_FORMAT_STR;
  static std::string PARAM_SKIP_INT;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadSnp::PARAM_DST_STR    = "dst";
std::string CmdLoadSnp::PARAM_FILE_STR   = "file";
std::string CmdLoadSnp::PARAM_FORMAT_STR = "format";
std::string CmdLoadSnp::PARAM_SKIP_INT   = "skip";



#include <stdio.h>
#include "../snpdata.h"
#include "objs.h"
void CmdLoadSnp::executeChild(const char *, GcObjSpace *os) {
  GcObjSnpData *snps = new GcObjSnpData();
  snps->data()->read(getParam(PARAM_FILE_STR)->valStr().c_str(),
                     getParam(PARAM_FORMAT_STR)->valStr().c_str(),
                     getParam(PARAM_SKIP_INT)->valInt());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), snps);
}

/*----------------------------------------------------------------------------*/

class CmdLoadSeg : public GcCommand {
public:
  CmdLoadSeg() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT_STR, GcCmdParam::PARAM_STRING, "cbe"));
    addParam(GcCmdParam(PARAM_SKIP_INT,   GcCmdParam::PARAM_INT,    "0"));
  }
  const char* name() const {
    return "load_seg";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
  static std::string PARAM_FORMAT_STR;
  static std::string PARAM_SKIP_INT;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadSeg::PARAM_DST_STR    = "dst";
std::string CmdLoadSeg::PARAM_FILE_STR   = "file";
std::string CmdLoadSeg::PARAM_FORMAT_STR = "format";
std::string CmdLoadSeg::PARAM_SKIP_INT   = "skip";



#include <stdio.h>
#include "../segdata.h"
void CmdLoadSeg::executeChild(const char *, GcObjSpace *os) {
  GcObjSegData *seg = new   GcObjSegData();
  seg->data()->read(getParam(PARAM_FILE_STR)->valStr().c_str(),
                    getParam(PARAM_FORMAT_STR)->valStr().c_str(),
                    getParam(PARAM_SKIP_INT)->valInt());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), seg);
}

/*----------------------------------------------------------------------------*/

class CmdSnpInfo : public GcCommand {
public:
  CmdSnpInfo() {
    addParam(GcCmdParam(PARAM_SRC_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "snp_info";
  }
  static std::string PARAM_SRC_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdSnpInfo::PARAM_SRC_STR = "src";



#include <stdio.h>
#include "../snpdata.h"
#include "objs.h"
void CmdSnpInfo::executeChild(const char *, GcObjSpace *os) {
  const char *src = getParam(PARAM_SRC_STR)->valStr().c_str();
  GcObjSnpData *snps = os->getObj<GcObjSnpData>(src);
  printf("%s: number of snps %lu\n", src, snps->data()->data().size());
}


