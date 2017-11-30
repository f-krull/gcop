#include "cmdparam.h"
#include "command.h"
#include "objs.h"
#include "objspace.h"
#include "icmdsink.h"

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
  ldi->d()->read(getParam(PARAM_FILE_STR)->valStr().c_str());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), ldi);
}

/*----------------------------------------------------------------------------*/

class CmdLdGet : public GcCommand {
public:
  CmdLdGet() {
    addParam(GcCmdParam(PARAM_SRC_STR, GcCmdParam::PARAM_STRING, ""));
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
  ldi->d()->getLd(
       getParam(PARAM_CHR_STR)->valStr().c_str()
      ,getParam(PARAM_BPA_NUM)->valInt()
      ,getParam(PARAM_BPB_NUM)->valInt()
  );
}

/*----------------------------------------------------------------------------*/

class CmdLdTest : public GcCommand {
public:
  CmdLdTest() {
    addParam(GcCmdParam(PARAM_SRC_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_SRCSNP_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "ld_test";
  }
  static std::string PARAM_SRC_STR;
  static std::string PARAM_SRCSNP_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLdTest::PARAM_SRC_STR     = "src";
std::string CmdLdTest::PARAM_SRCSNP_STR  = "srcsnp";

#include <stdio.h>
#include "../data/ldinfo.h"
void CmdLdTest::executeChild(const char *, GcObjSpace *os) {
  const char *srcLdi    = getParam(PARAM_SRC_STR)->valStr().c_str();
  const char *srcsnps   = getParam(PARAM_SRCSNP_STR)->valStr().c_str();
  GcObjLdInfo  *ldi  = os->getObj<GcObjLdInfo>(srcLdi);
  GcObjGCords  *snps = os->getObj<GcObjGCords>(srcsnps);
  ldi->d()->test(snps->d());
}

/*----------------------------------------------------------------------------*/

class CmdLoadGCords : public GcCommand {
public:
  CmdLoadGCords() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT_STR, GcCmdParam::PARAM_STRING, "cbe"));
    addParam(GcCmdParam(PARAM_SKIP_INT,   GcCmdParam::PARAM_INT,    "0"));
  }
  const char* name() const {
    return "load_gcords";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
  static std::string PARAM_FORMAT_STR;
  static std::string PARAM_SKIP_INT;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadGCords::PARAM_DST_STR    = "dst";
std::string CmdLoadGCords::PARAM_FILE_STR   = "file";
std::string CmdLoadGCords::PARAM_FORMAT_STR = "format";
std::string CmdLoadGCords::PARAM_SKIP_INT   = "skip";

#include <stdio.h>
#include "../data/gcords.h"
void CmdLoadGCords::executeChild(const char *, GcObjSpace *os) {
  GcObjGCords *gcs = new GcObjGCords();
  gcs->d()->read(getParam(PARAM_FILE_STR)->valStr().c_str(),
                    getParam(PARAM_FORMAT_STR)->valStr().c_str(),
                    getParam(PARAM_SKIP_INT)->valInt());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), gcs);
}

/*----------------------------------------------------------------------------*/

class CmdIntersectGc : public GcCommand {
public:
  CmdIntersectGc() {
    addParam(GcCmdParam(PARAM_SRCA_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_SRCB_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "intersect_gc";
  }
  static std::string PARAM_SRCA_STR;
  static std::string PARAM_SRCB_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdIntersectGc::PARAM_SRCA_STR = "srca";
std::string CmdIntersectGc::PARAM_SRCB_STR = "srcb";

#include <stdio.h>
#include "../data/gcords.h"
void CmdIntersectGc::executeChild(const char *, GcObjSpace *os) {
  const char *srca = getParam(PARAM_SRCA_STR)->valStr().c_str();
  const char *srcb = getParam(PARAM_SRCB_STR)->valStr().c_str();
  GcObjGCords *gca = os->getObj<GcObjGCords>(srca);
  GcObjGCords *gcb = os->getObj<GcObjGCords>(srcb);
  GCords::intersect(gca->d(), gcb->d());
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
#include "../data/gcords.h"
#include "objs.h"
void CmdSnpInfo::executeChild(const char *, GcObjSpace *os) {
  const char *src = getParam(PARAM_SRC_STR)->valStr().c_str();
  GcObjGCords *snps = os->getObj<GcObjGCords>(src);
  printf("%s: number of snps %lu\n", src, snps->d()->data().size());
}

/*----------------------------------------------------------------------------*/

void cmd_base_add(ICmdSink *cs) {
  cs->addCmd(new CmdSnpInfo);
  cs->addCmd(new CmdLoadLdInfo);
  cs->addCmd(new CmdLdGet);
  cs->addCmd(new CmdLdTest);
  cs->addCmd(new CmdLoadGCords);
  cs->addCmd(new CmdIntersectGc);
}
