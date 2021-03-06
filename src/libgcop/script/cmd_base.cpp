#include "cmdparam.h"
#include "command.h"
#include "objs.h"
#include "objspace.h"
#include "icmdsink.h"
#include <assert.h>

/*----------------------------------------------------------------------------*/
#if 0
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
#endif
/*----------------------------------------------------------------------------*/
#include "objs_base.h"
class CmdLoadGCords : public GcCommand {
public:
  CmdLoadGCords() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FORMAT_STR, GcCmdParam::PARAM_STRING, "cbe"));
    addParam(GcCmdParam(PARAM_SKIP_INT,   GcCmdParam::PARAM_INT,    "0"));
    addParam(GcCmdParam(PARAM_BUILD_STR,  GcCmdParam::PARAM_STRING, OBJ_CHRINFO_HG19));
  }
  const char* name() const {
    return "load_gcords";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
  static std::string PARAM_FORMAT_STR;
  static std::string PARAM_SKIP_INT;
  static std::string PARAM_BUILD_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadGCords::PARAM_DST_STR    = "dst";
std::string CmdLoadGCords::PARAM_FILE_STR   = "file";
std::string CmdLoadGCords::PARAM_FORMAT_STR = "format";
std::string CmdLoadGCords::PARAM_SKIP_INT   = "skip";
std::string CmdLoadGCords::PARAM_BUILD_STR  = "build";

#include <stdio.h>
#include "../gcords.h"
void CmdLoadGCords::executeChild(const char *, GcObjSpace *os) {
  const char *str_buid = getParam(PARAM_BUILD_STR)->valStr().c_str();
  GcObjChrInfo *gchr = os->getObj<GcObjChrInfo>(str_buid);
  GcObjGCords *gcs = new GcObjGCords();
  gcs->d()->read(getParam(PARAM_FILE_STR)->valStr().c_str(),
                    getParam(PARAM_FORMAT_STR)->valStr().c_str(),
                    getParam(PARAM_SKIP_INT)->valInt(),
                    gchr->d(), true);
  os->addObj(getParam(PARAM_DST_STR)->valStr(), gcs);
}

/*----------------------------------------------------------------------------*/

class CmdGCordsInfo : public GcCommand {
public:
  CmdGCordsInfo() {
    addParam(GcCmdParam(PARAM_SRC_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "gcords_info";
  }
  static std::string PARAM_SRC_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdGCordsInfo::PARAM_SRC_STR = "src";

#include <stdio.h>
#include "../gcords.h"
#include "objs.h"
void CmdGCordsInfo::executeChild(const char *, GcObjSpace *os) {
  const char *src = getParam(PARAM_SRC_STR)->valStr().c_str();
  GcObjGCords *snps = os->getObj<GcObjGCords>(src);
  const GCords *g = snps->d();
  printf("%s: number of genomic coordinates %lu\n", src, g->numgc());
}

/*----------------------------------------------------------------------------*/

class CmdLoadChrInfo : public GcCommand {
public:
  CmdLoadChrInfo() {
    addParam(GcCmdParam(PARAM_DST_STR,    GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_FILE_STR,   GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "load_chrinfo";
  }
  static std::string PARAM_DST_STR;
  static std::string PARAM_FILE_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdLoadChrInfo::PARAM_DST_STR    = "dst";
std::string CmdLoadChrInfo::PARAM_FILE_STR   = "file";

#include <stdio.h>
#include "../chrinfo.h"
void CmdLoadChrInfo::executeChild(const char *, GcObjSpace *os) {
  GcObjChrInfo *gci = new GcObjChrInfo();
  gci->d()->read(getParam(PARAM_FILE_STR)->valStr().c_str());
  os->addObj(getParam(PARAM_DST_STR)->valStr(), gci);
}

/*----------------------------------------------------------------------------*/

class CmdForbes : public GcCommand {
public:
  CmdForbes() {
    addParam(GcCmdParam(PARAM_SRCA_STR, GcCmdParam::PARAM_STRING, ""));
    addParam(GcCmdParam(PARAM_SRCB_STR, GcCmdParam::PARAM_STRING, ""));
  }
  const char* name() const {
    return "forbes";
  }
  static std::string PARAM_SRCA_STR;
  static std::string PARAM_SRCB_STR;
protected:
  void executeChild(const char *, GcObjSpace *os);
};

std::string CmdForbes::PARAM_SRCA_STR = "srca";
std::string CmdForbes::PARAM_SRCB_STR = "srcb";

#include <stdio.h>
#include "../gcords.h"
void CmdForbes::executeChild(const char *, GcObjSpace *os) {
  const char *srca = getParam(PARAM_SRCA_STR)->valStr().c_str();
  const char *srcb = getParam(PARAM_SRCB_STR)->valStr().c_str();
  GcObjGCords *gca = os->getObj<GcObjGCords>(srca);
  GcObjGCords *gcb = os->getObj<GcObjGCords>(srcb);
  GCords::forbes(gca->d(), gcb->d());
}

/*----------------------------------------------------------------------------*/

void cmd_base_add(ICmdSink *cs) {
  cs->addCmd(new CmdGCordsInfo);
#if 0
  cs->addCmd(new CmdLoadLdInfo);
  cs->addCmd(new CmdLdGet);
  cs->addCmd(new CmdLdTest);
#endif
  cs->addCmd(new CmdLoadGCords);
  cs->addCmd(new CmdLoadChrInfo);
  cs->addCmd(new CmdForbes);
}
