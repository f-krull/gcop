#include "objs.h"
#include <string>

class GcObjPriv {
public:
  std::string name;
private:
};

GcObj::GcObj(const char *name) {
  m = new GcObjPriv;
  m->name = name;
}

GcObj::~GcObj() {
  delete m;
}

const char * GcObj::name() const {
  return m->name.c_str();
}

#include "../snpdata.h"

GcObjSnpData::GcObjSnpData() : GcObj("SnpData") {
  m_snp = new SnpData();
}

GcObjSnpData::~GcObjSnpData() {
  delete m_snp;
}


SnpData* GcObjSnpData::data() {
  return m_snp;
}

const SnpData* GcObjSnpData::data() const {
  return m_snp;
}




#include "../segdata.h"

GcObjSegData::GcObjSegData() : GcObj("SegData") {
  m_seg = new SimpleSegData();
}

GcObjSegData::~GcObjSegData() {
  delete m_seg;
}


ISegData* GcObjSegData::data() {
  return m_seg;
}

const ISegData* GcObjSegData::data() const {
  return m_seg;
}
