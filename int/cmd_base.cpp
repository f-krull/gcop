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


/*----------------------------------------------------------------------------*/

class CmdIntersectPS : public GcCommand {
public:
  CmdIntersectPS() {
    addParam(GcCmdParam(PARAM_SRCP_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_SRCS_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "intersect";
  }
  static std::string PARAM_SRCP_STR;
  static std::string PARAM_SRCS_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdIntersectPS::PARAM_SRCP_STR = "srcp";
std::string CmdIntersectPS::PARAM_SRCS_STR = "srcs";

#include <stdio.h>
#include "../snpdata.h"
#include "objs.h"
#include "../segannot.h"
void CmdIntersectPS::executeChild(const char *, GcObjSpace *os) {
  const char *srcPoints   = getParam(PARAM_SRCP_STR)->valStr().c_str();
  const char *srcSegments = getParam(PARAM_SRCS_STR)->valStr().c_str();
  GcObjSnpData *snps = os->getObj<GcObjSnpData>(srcPoints);
  GcObjSegData *segs = os->getObj<GcObjSegData>(srcSegments);
  SegAnnot sa;
  sa.intersect(*segs->data(), *snps->data());
}

/*----------------------------------------------------------------------------*/

class CmdLoadLdInfo : public GcCommand {
public:
  CmdLoadLdInfo() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "load_ld";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadLdInfo::PARAM_DST_STR    = "dst";
std::string CmdLoadLdInfo::PARAM_FILE_STR   = "file";


#include <stdio.h>
#include "../data/ldinfo.h"
void CmdLoadLdInfo::executeChild(const char *, GcObjSpace *os) {
  GcObjLdInfo *ldi = new GcObjLdInfo();
  ldi->data()->read(getParam(PARAM_FILE_STR)->valStr().c_str());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), ldi);
}

/*----------------------------------------------------------------------------*/

class CmdLdGet : public GcCommand {
public:
  CmdLdGet() {
    addParam(GcCmdParam(PARAM_SRC_STR, GcCmdParam::PARAM_INT, ""));
    addParam(GcCmdParam(PARAM_CHR_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_BPA_NUM, GcCmdParam::PARAM_INT, ""));
    addParam(GcCmdParam(PARAM_BPB_NUM, GcCmdParam::PARAM_INT, ""));
  }
  const char* name() const {
    return "ld_get";
  }
  static std::string PARAM_SRC_STR;
  static std::string PARAM_CHR_STR;
  static std::string PARAM_BPA_NUM;
  static std::string PARAM_BPB_NUM;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLdGet::PARAM_SRC_STR   = "src";
std::string CmdLdGet::PARAM_CHR_STR   = "chr";
std::string CmdLdGet::PARAM_BPA_NUM   = "bpa";
std::string CmdLdGet::PARAM_BPB_NUM   = "bpb";


#include <stdio.h>
#include "../data/ldinfo.h"
void CmdLdGet::executeChild(const char *, GcObjSpace *os) {
  const char *srcLdi   = getParam(PARAM_SRC_STR)->valStr().c_str();
  GcObjLdInfo *ldi = os->getObj<GcObjLdInfo>(srcLdi);
  ldi->data()->getLd(
       getParam(PARAM_CHR_STR)->valStr().c_str()
      ,getParam(PARAM_BPA_NUM)->valInt()
      ,getParam(PARAM_BPB_NUM)->valInt()
  );
}
