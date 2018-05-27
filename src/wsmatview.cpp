#include "wsmatview.h"
#include "buffer.h"
#include "hmmat.h"
#include <string.h>
#include <assert.h>

#include "helper.h"

/*----------------------------------------------------------------------------*/

#define JPEG_QUALITY 90

/*----------------------------------------------------------------------------*/

#define CFG_MAIN_WID_INT   "MAIN_WID"
#define CFG_MAIN_HEI_INT   "MAIN_HEI"
#define CFG_YLAB_WID_INT   "YLAB_WID"
#define CFG_XLAB_HEI_INT   "XLAB_HEI"
#define CFG_YDEN_WID_INT   "YDEN_WID"
#define CFG_XDEN_HEI_INT   "XDEN_HEI"
#define CFG_ZOOM_INT       "ZOOM"
#define CFG_POSX_INT       "POSX"
#define CFG_POSY_INT       "POSY"
#define CFG_LABTXT_HEI_INT "LABTXT_HEI"

class WsMatViewCfg {
public:
  WsMatViewCfg() {
    defineInt(CFG_YLAB_WID_INT, "50",  0, 4096);
    defineInt(CFG_XLAB_HEI_INT, "50",  0, 4096);
    defineInt(CFG_MAIN_HEI_INT, "200", 0, 4096);
    defineInt(CFG_MAIN_WID_INT, "300", 0, 4096);
    defineInt(CFG_XDEN_HEI_INT, "40",  0, 4096);
    defineInt(CFG_YDEN_WID_INT, "40",  0, 4096);
    defineInt(CFG_ZOOM_INT, "0", 0, 4096);
    defineInt(CFG_POSY_INT, "0", 0, 4096);
    defineInt(CFG_POSX_INT, "0", 0, 4096);
    defineInt(CFG_LABTXT_HEI_INT, "13", 0, 4096);
  }

  enum CfgType {
    CFGTYPE_INT
  };

  bool set(const char *name, const char *val) {
    m_cfg[name] = val;
    return true;
  }


  int32_t getInt(const char *s) const {
    auto it = m_cfg.find(s);
    assert(it != m_cfg.end());
    return atoi(it->second.c_str());
  }

  void setInt(const char *s, int32_t v) {
    auto it = m_cfg.find(s);
    assert(it != m_cfg.end());
    BufferDyn b(8);
    b.addf("%d", v);
    it->second.assign(b.cdata(), b.cdata() + b.len());
  }

private:
  std::map<std::string, std::string> m_cfg;


  void defineInt(const char *name, const char *v, int32_t min, int32_t max) {
    m_cfg[name] = v;
  }
};

/*----------------------------------------------------------------------------*/

#define cimg_use_jpeg
#define cimg_display 0
#include "../3rdparty/CImg.h"
namespace cimg = cimg_library;

/*----------------------------------------------------------------------------*/

class ImgBase {
public:
  ImgBase() {
    m_jpgbuf = NULL;
    m_jpgbufSize = 0;
  }
  virtual ~ImgBase() {
    if (m_jpgbuf) {
      free(m_jpgbuf);
    }
  }

  const BufferDyn& jpgB64() const {
    return m_jpgb64;
  }

  const cimg::CImg<unsigned char> & img() const {
    return m_img;
  }

protected:
  cimg::CImg<unsigned char> m_img;
  uint8_t   *m_jpgbuf;
  size_t    m_jpgbufSize;
  BufferDyn m_jpgb64;

  bool encode() {
    FILE *f = open_memstream((char**)&m_jpgbuf, &m_jpgbufSize);
    if (!f) {
      return false;
    }
    m_img.save_jpeg(f, JPEG_QUALITY);
    fclose(f);
    m_jpgb64.set(m_jpgbuf, m_jpgbufSize);
    m_jpgb64.toBase64();
    return true;
  }

  /* add canvas to match asp ratio*/
  static void fixAspRatio(cimg::CImg<unsigned char> & img, uint32_t w, uint32_t h) {
    const float a_new = float(w) / h;
    const int32_t wNew = img.height() * a_new;
    const int32_t hNew = img.width()  / a_new;
    if (wNew > img.width()) {
      /* add w */
      int32_t borderW = (wNew - img.width()) / 2;
      auto inew = cimg::CImg<unsigned char>(wNew, img.height(), 1, 3, 0xff);
      inew.draw_image(borderW, 0, 0, 0, img);
      img = inew;
    } else if (hNew > img.height()) {
      /* add h */
      int32_t borderN = (hNew - img.height()) / 2;
      auto inew = cimg::CImg<unsigned char>(img.width(), hNew, 1, 3, 0xff);
      inew.draw_image(0, borderN, 0, 0, img);
      img = inew;
    }
  }
};

/*----------------------------------------------------------------------------*/

class ImgUnscaled : public ImgBase {
public:
  virtual ~ImgUnscaled() {}
  void update(const HmMat *hm) {
    const float mmax = hm->maxVal();
    const float mmin = hm->minVal();
    const float amax = fabs(mmin) > fabs(mmax) ? fabs(mmin) : fabs(mmax);
    m_img = cimg::CImg<unsigned char>(hm->ncol(), hm->nrow(), 1, 3, 0);
    for (uint32_t i = 0; i < hm->nrow(); i++) {
      for (uint32_t j = 0; j < hm->ncol(); j++) {
        const float v = hm->get(i, j);
        const uint8_t colMin[]  = { 0x11, 0x11, 0xff };
        const uint8_t colMax[]  = { 0xff, 0x11, 0x11 };
        const uint8_t colZero[] = { 0xf8, 0xf8, 0xf8 };
        const uint8_t *col0 = colZero;
        const uint8_t *col1 = v < 0 ? colMin : colMax;
        const float colScale = (amax !=0) ? (powf(fabs(v) / amax, 0.35)) : 0;
        assert(colScale <= 1);
        assert(colScale >= 0);
        m_img(j, i, 0) = (col0[0] * (1-colScale) + col1[0] * colScale);
        m_img(j, i, 1) = (col0[1] * (1-colScale) + col1[1] * colScale);
        m_img(j, i, 2) = (col0[2] * (1-colScale) + col1[2] * colScale);
      }
    }
    encode();
  }
};

class ImageStatusOk : public ImgBase {
public:
  ImageStatusOk() {
    const unsigned char colGreen[] = {0, 0xFF, 0};
    m_img = cimg::CImg<unsigned char>(30, 30, 1, 3, 0);
    m_img.draw_fill(0, 0, colGreen);
    encode();
  }
private:
};

/*----------------------------------------------------------------------------*/

class ImageStatusWarn : public ImgBase {
public:
  ImageStatusWarn() {
    const unsigned char colYellow[] = {0xFF, 0xFF, 0};
    m_img = cimg::CImg<unsigned char>(30, 30, 1, 3, 0);
    m_img.draw_fill(0, 0, colYellow);
    encode();
  }
private:
};

/*----------------------------------------------------------------------------*/

class ImgMain : public ImgBase {
public:
  ImgMain() {
    m_pxW = 0;
    m_pxH = 0;
    m_dispStartX = 0;
    m_dispStartY = 0;
    m_dispMatX0 = 0;
    m_dispMatY0 = 0;
    m_dispMatX1 = 0;
    m_dispMatY1 = 0;
    m_nCellW = 0;
    m_nCellH = 0;
    m_cellX = 0;
    m_cellY = 0;
  }
  void update(const ImgUnscaled &us, const WsMatViewCfg &cfg) {
    m_imgUs = us.img();
    const uint32_t dwid = cfg.getInt(CFG_MAIN_WID_INT);
    const uint32_t dhei = cfg.getInt(CFG_MAIN_HEI_INT);
    const uint32_t zoom = cfg.getInt(CFG_ZOOM_INT);
    /* determine pixel-size if we wanted to fit everything */
    const float pw = float(dwid) / m_imgUs.width();
    const float ph = float(dhei) / m_imgUs.height();
    const float pz = pw < ph ? pw : ph; /* min */
    /* new pixel size depending on zoom level - last term for high zoom vals */
    const float pz_new = pz * (zoom*0.125+1) + (zoom * zoom * 0.1);
    /* how many w and h pixels are we showing? */
    {
      int32_t nph = dhei / pz_new;
      int32_t npw = dwid / pz_new;
      npw = npw < m_imgUs.width()  ? npw : m_imgUs.width(); /* min */
      nph = nph < m_imgUs.height() ? nph : m_imgUs.height();
      m_nCellW = npw;
      m_nCellH = nph;
    }
    /* calc offsets */
    int32_t x0 = m_cellX - m_nCellW/2;
    int32_t y0 = m_cellY - m_nCellH/2;
    x0 = x0 < m_imgUs.width()  ? x0 : m_imgUs.width();
    y0 = y0 < m_imgUs.height() ? y0 : m_imgUs.height();
    x0 = x0+m_nCellW-1 < m_imgUs.width()  ? x0 : m_imgUs.width() -m_nCellW;
    y0 = y0+m_nCellH-1 < m_imgUs.height() ? y0 : m_imgUs.height()-m_nCellH;
    x0 = x0 > 0 ? x0 : 0;
    y0 = y0 > 0 ? y0 : 0;
    /* crop image */
    m_imgUSCrop = m_imgUs.get_crop(x0, y0, x0+m_nCellW-1, y0+m_nCellH-1); //TODO: check if negative
    /* add borders */
    m_imgUSCropAsp = m_imgUSCrop;
    fixAspRatio(m_imgUSCropAsp, dwid, dhei);
    m_img = m_imgUSCropAsp.get_resize(dwid, dhei);
    /* save info for pixel id calc */
    m_dispMatX0 = (m_imgUSCropAsp.width()  - m_imgUSCrop.width())  / 2 * pz_new;
    m_dispMatY0 = (m_imgUSCropAsp.height() - m_imgUSCrop.height()) / 2 * pz_new;
    m_dispMatX1 = m_img.width()  - m_dispMatX0;
    m_dispMatY1 = m_img.height() - m_dispMatY0;
    m_pxW         = float(m_img.width()  - m_dispMatX0 * 2) / m_nCellW;
    m_pxH         = float(m_img.height() - m_dispMatY0 * 2) / m_nCellH;
    m_dispStartX  = x0;
    m_dispStartY  = y0;
    encode();
  }

  void zoomin(int32_t x, int32_t y, WsMatViewCfg *cfg) {
    const int32_t z = cfg->getInt(CFG_ZOOM_INT);
    cfg->setInt(CFG_ZOOM_INT, z + 1);
    /* calc x and y */
    m_cellX = (clippedPx2CellX(x) + clippedPx2CellX(cfg->getInt(CFG_MAIN_WID_INT)/2)/2);
    m_cellY = (clippedPx2CellY(y) + clippedPx2CellY(cfg->getInt(CFG_MAIN_HEI_INT)/2)/2);
    m_cellX = clippedPx2CellX(x);
    m_cellY = clippedPx2CellY(y);
#if 0
    const int32_t posx = cfg.getInt(CFG_POSX_INT);
    const int32_t posy = cfg.getInt(CFG_POSY_INT);
    m_cellX = (posx + calcClippedPosX(dwid/2))/2; /* TODO: set these in a zoomin(x,y) function */
    m_cellY = (posy + calcClippedPosY(dhei/2))/2;
#endif
  }

  void zoomout(int32_t x, int32_t y, WsMatViewCfg *cfg) {
    const int32_t z = cfg->getInt(CFG_ZOOM_INT);
    cfg->setInt(CFG_ZOOM_INT, z > 0 ? z - 1 : z);
    m_cellX = clippedPx2CellX(cfg->getInt(CFG_MAIN_WID_INT)/2);
    m_cellY = clippedPx2CellY(cfg->getInt(CFG_MAIN_HEI_INT)/2);
  }

  void pan(int32_t dPxX, int32_t dPxY) {
    move(round(-1*double(dPxX)/m_pxW), round(-1*double(dPxY)/m_pxH));
  }

  void move(const char* direction) {
    const float dscale = 0.1;
    const int32_t deltaX = int32_t(m_nCellW * dscale) > 1 ? (m_nCellW * dscale) : 1;
    const int32_t deltaY = int32_t(m_nCellH * dscale) > 1 ? (m_nCellH * dscale) : 1;
    switch (direction[0]) {
      case 'U':
        move(0, deltaY*-1);
        break;
      case 'D':
        move(0, deltaY);
        break;
      case 'L':
        move(deltaX*-1, 0);
        break;
      case 'R':
        move(deltaX, 0);
        break;
      default:
        break;
    }
  }

  void move(int32_t cellDeltaX, int32_t cellDeltaY) {
    {
      int32_t cellX_new = m_cellX + cellDeltaX;
      cellX_new = std::max(cellX_new, m_nCellW / 2);
      cellX_new = std::min(cellX_new, m_imgUs.width() - m_nCellW / 2);
      m_cellX = clipCellX(cellX_new);
    }
    {
      int32_t cellY_new = m_cellY + cellDeltaY;
      cellY_new = std::max(cellY_new, m_nCellH / 2);
      cellY_new = std::min(cellY_new, m_imgUs.height() - m_nCellH / 2);
      m_cellY = clipCellY(cellY_new);
    }
  }

  int32_t px2cellX(int32_t x) const {
    return (x - m_dispMatX0) / m_pxW + m_dispStartX;
  }

  int32_t px2CellY(int32_t y) const {
    return (y - m_dispMatY0) / m_pxH + m_dispStartY;
  }

  int32_t clipCellX(int32_t posx) const {
    posx = posx > 0 ? posx : 0;
    return posx < m_imgUs.width() ? posx : (m_imgUs.width()-1);
  }

  int32_t clipCellY(int32_t posy) const {
    posy = posy > 0 ? posy : 0;
    return posy < m_imgUs.height() ? posy : (m_imgUs.height()-1);
  }

  int32_t clippedPx2CellX(int32_t x) const {
    return clipCellX(px2cellX(x));
  }

  int32_t clippedPx2CellY(int32_t y) const {
    return clipCellY(px2CellY(y));
  }

  int32_t numPxH() const {return m_nCellH;}
  int32_t numPxW() const {return m_nCellW;}
  int32_t dispStartX() const {return m_dispStartX;}
  int32_t dispStartY() const {return m_dispStartY;}
  int32_t dispMatX0() const {return m_dispMatX0;}
  int32_t dispMatY0() const {return m_dispMatY0;}
  int32_t dispMatX1() const {return m_dispMatX1;}
  int32_t dispMatY1() const {return m_dispMatY1;}


protected:
  cimg::CImg<unsigned char> m_imgUs;
  cimg::CImg<unsigned char> m_imgUSCrop;
  cimg::CImg<unsigned char> m_imgUSCropAsp;

  uint32_t m_dispStartX;
  uint32_t m_dispStartY;
  int32_t  m_dispMatX0; /* top-left corner of matrix; != 0 if img has border */
  int32_t  m_dispMatY0;
  int32_t  m_dispMatX1; /* bottom-right corner of matrix, already outside */
  int32_t  m_dispMatY1;
  int32_t  m_nCellW;
  int32_t  m_nCellH;
  float    m_pxW; /* cell width in px */
  float    m_pxH; /* cell height in px */
  int32_t m_cellX;
  int32_t m_cellY;
};


/*----------------------------------------------------------------------------*/

class ImgTiny : public ImgBase {
public:
  void update(const ImgUnscaled &us, const ImgMain &main, const WsMatViewCfg &cfg) {
    const uint32_t dw = cfg.getInt(CFG_YLAB_WID_INT);
    const uint32_t dh = cfg.getInt(CFG_XLAB_HEI_INT);
    m_img = us.img();
    fixAspRatio(m_img, dw, dh);
    /* border info needed for rect */
    const int32_t borderE = (m_img.width()  - us.img().width())  / 2;
    const int32_t borderN = (m_img.height() - us.img().height()) / 2;
    m_img.resize(dw, dh);
    /* if not zoomed in , draw rect */
    if (main.numPxW() != us.img().width() || main.numPxH() != us.img().height()) {
      const unsigned char red[] = { 255, 0, 0 };
      const float scalex = float(m_img.width())  / us.img().width();
      const float scaley = float(m_img.height()) / us.img().height();
      const float scale = scalex < scaley ? scalex : scaley;
      const int32_t x0 = (borderE + main.dispStartX()) * scale;
      const int32_t y0 = (borderN + main.dispStartY()) * scale;
      const int32_t x1 = (borderE + main.dispStartX() + main.numPxW()) * scale;
      const int32_t y1 = (borderN + main.dispStartY() + main.numPxH()) * scale;
      //printf("x0=%d y0=%d x1=%d y1=%d scale=%f borderE=%d borderN=%d usw%d ush=%d %d%d\n", x0, y0, x1, y1, scale, borderE, borderN, us.img().width(), us.img().height(), main.numPxW(), main.numPxH());
      m_img.draw_rectangle(x0, y0, x1-1, y1-1, red,1.0f,~0U);
    }
    encode();
  }
};

/*----------------------------------------------------------------------------*/

class ImgYlab : public ImgBase {
public:
  void update(const ImgMain &main, const HmMat *hmm, const WsMatViewCfg &cfg) {
    unsigned char blvl = 0xff;
    const uint32_t dw = cfg.getInt(CFG_YLAB_WID_INT);
    const uint32_t dh = cfg.getInt(CFG_MAIN_HEI_INT);
    m_img = cimg::CImg<unsigned char>(dw, dh, 1, 3, blvl);
    const unsigned char bgCol[] = {blvl, blvl, blvl};
    const unsigned char fgCol[] = {0x00, 0x00, 0x00};
    const int32_t tickW = 2 < m_img.width() ? 2 : m_img.width();
    const int32_t axBgn = main.dispMatY0();
    const int32_t axEnd = main.dispMatY1();
    const int32_t textHeight = cfg.getInt(CFG_LABTXT_HEI_INT);
    float pxHei = float(axEnd - axBgn) / main.numPxH();
    int32_t posprev = 0;
    for (int32_t i = 0; i < main.numPxH(); i++) {
      int32_t currpos = axBgn + pxHei * i + (pxHei / 2) - (float(textHeight) / 2);
      if (currpos - posprev >  textHeight) {
        m_img.draw_line(0, currpos+float(textHeight)/2, tickW, currpos+float(textHeight)/2, fgCol, 1.f);
        m_img.draw_text(tickW+3, currpos, "%s", fgCol, bgCol, 1, textHeight, hmm->ylab(main.dispStartY()+ i));
        posprev = currpos;
      }
    }
    encode();
  }
};

/*----------------------------------------------------------------------------*/

class ImgXlab : public ImgBase {
public:
  void update(const ImgMain &main, const HmMat *hmm, const WsMatViewCfg &cfg) {
    unsigned char blvl = 0xff;
    const uint32_t dw = cfg.getInt(CFG_XLAB_HEI_INT);
    const uint32_t dh = cfg.getInt(CFG_MAIN_WID_INT);
    m_img = cimg::CImg<unsigned char>(dw, dh, 1, 3, blvl);
    const unsigned char bgCol[] = {blvl, blvl, blvl};
    const unsigned char fgCol[] = {0x00, 0x00, 0x00};
    const int32_t tickW = 2 < m_img.height() ? 2 : m_img.height();
    const int32_t axBgn = main.dispMatX0();
    const int32_t axEnd = main.dispMatX1();
    const int32_t textHeight = cfg.getInt(CFG_LABTXT_HEI_INT);
    float pxWid = float(axEnd - axBgn) / main.numPxW();
    int32_t posprev = 0;
    for (int32_t i = 0; i < main.numPxW(); i++) {
      int32_t currpos = axBgn + pxWid * i + (pxWid / 2) - (float(textHeight) / 2);
      if (currpos - posprev >  textHeight) {
        m_img.draw_line(0, currpos+float(textHeight)/2, tickW, currpos+float(textHeight)/2, fgCol, 1.f);
        m_img.draw_text(tickW+3, currpos, "%s", fgCol, bgCol, 1, textHeight, hmm->xlab(main.dispStartX()+ i));
        posprev = currpos;
      }
    }
    m_img.rotate(-90);
    encode();
  }
};

/*----------------------------------------------------------------------------*/

#define DEN_MIN_DIST 3

#include "dendrogram.h"

class ImgYden : public ImgBase {
public:
  void update(const ImgMain &imain, const ImgUnscaled &ius, const HmMat *hmm, const WsMatViewCfg &cfg) {
    unsigned char blvl = 0xFF;
    const uint32_t dw = cfg.getInt(CFG_YDEN_WID_INT);
    const uint32_t dh = cfg.getInt(CFG_MAIN_HEI_INT);
    m_img = cimg::CImg<unsigned char>(dw, dh, 1, 3, blvl);
    while (true) { /* dummy */
      const Dendrogram *dend = hmm->getDendY();
      if (!dend) {
        break;
      }
      int32_t i0 = imain.dispStartY();
      int32_t i1 = imain.dispStartY() + imain.numPxH() - 1;

      assert(imain.dispMatY0() >= 0);
      assert(imain.dispMatY1() >= 0);
      std::vector<const DgNode*> roots = dend->getRoots(i0, i1);
      double distmax = 0;
      double distmin = DBL_MAX;
      for (uint32_t i = 0; i < roots.size(); i++) {
        distmax = std::max(distmax, roots[i]->distAbs());
        distmin = std::min(distmin, roots[i]->distAbsMin());
      }
      for (uint32_t i = 0; i < roots.size(); i++) {
        drawNode(NULL, roots[i], imain, ius, 0, distmin, distmax);
      }
      break;
    }
    encode();
  }
private:
  void drawNode(const DgNode* parent, const DgNode *node, const ImgMain &imain, const ImgUnscaled &ius, int32_t x0, double distmin, double distmax) {
    if (!node) {
      return;
    }

    const unsigned char fgCol[] = {0x00, 0x00, 0x00};
    const int32_t axBgn = imain.dispMatY0();
    const int32_t axEnd = imain.dispMatY1();
    float pxHei = float(axEnd - axBgn) / imain.numPxH();
    int32_t y0 = pxHei * (node->hPos() * ius.img().height() - imain.dispStartY()) + axBgn + (pxHei / 2);
    int32_t x1 = (m_img.width()-1) * (1 - (node->distAbs() - distmin) / (distmax-distmin));
    if (x1 - x0 < DEN_MIN_DIST) {
      x1 = x0;
    }
    m_img.draw_line(x0, y0, x1, y0, fgCol, 1.f);
    if (parent) {
      int32_t y1 = pxHei * (parent->hPos() * ius.img().height() - imain.dispStartY()) + axBgn + (pxHei / 2);
      m_img.draw_line(x0, y0, x0, y1, fgCol, 1.f);
    }
    drawNode(node, node->getLo(), imain, ius, x1, distmin, distmax);
    drawNode(node, node->getHi(), imain, ius, x1, distmin, distmax);
  }
};

/*----------------------------------------------------------------------------*/

class WsMatViewPriv {
public:
  WsMatViewCfg cfg;
  bool sendUpdate;
  ImageStatusOk   imgStatusOk;
  ImageStatusWarn imgStatusWarn;
  HmMat *mat;
  ImgUnscaled imgUnscaled;
  ImgMain     imgMain;
  ImgTiny     imgTiny;
  ImgYlab     imgYlab;
  ImgXlab     imgXlab;
  ImgYden     imgYden;
};

/*----------------------------------------------------------------------------*/

WsMatView::WsMatView(WsService* s, uint32_t clientId) :
    IWsServiceClient(clientId),
    m_log("WsMatView%u", clientId),
    m_nextupdate(0),
    m_srv(s) {
  m = new WsMatViewPriv;
  m->mat = new HmMat();
  //m->mat->read("data/disreg_matrix_half.txt");
  loadMat("data/disreg_matrix_10x8.txt");
  //loadMat("data/disreg_matrix.txt");
}

/*----------------------------------------------------------------------------*/

WsMatView::~WsMatView() {
  delete m->mat;
  delete m;
}

/*----------------------------------------------------------------------------*/

#define CMD_PFX_SET          "SET "
#define CMD_PFX_ZOOMIN       "ZOOMIN "
#define CMD_PFX_ZOOMOUT      "ZOOMOUT "
#define CMD_PFX_CLICK        "CLICK "
#define CMD_PFX_PAN          "PAN "
#define CMD_PFX_MOVE         "MOVE "
#define CMD_PFX_OCLUSSLX "OCLUSSLX"
#define CMD_PFX_OCLUSSLY "OCLUSSLY"
#define CMD_PFX_OCLUSCLX "OCLUSCLX"
#define CMD_PFX_OCLUSCLY "OCLUSCLY"
#define CMD_PFX_ONAMEX   "ONAMEX"
#define CMD_PFX_ONAMEY   "ONAMEY"
#define CMD_PFX_ORANDX   "ORANDX"
#define CMD_PFX_ORANDY   "ORANDY"
#define CMD_PFX_LOADMAT    "LOADMAT "
#define CMD_PFX_TRANSPMAT  "TRANSPMAT"

/*----------------------------------------------------------------------------*/

void WsMatView::loadMat(const char *fn) {
  m->mat->read(fn);
  m->mat->order(HmMat::ORDER_HCLUSTER_SL_X);
  m->mat->order(HmMat::ORDER_HCLUSTER_SL_Y);
  m->imgUnscaled.update(m->mat);
  m->cfg.setInt(CFG_ZOOM_INT, 0);
  m->sendUpdate = true;
}

/*----------------------------------------------------------------------------*/

void WsMatView::newData(const uint8_t* _data, uint32_t _len) {
  if (_len == 0) {
    return;
  }
  /* copy data so we can null term tokens */
  BufferDyn msgbuf(_data, _len);
  msgbuf.addf("\0");
  char *msg = (char*)msgbuf.data();
  if (strncmp(msg, CMD_PFX_SET, strlen(CMD_PFX_SET)) == 0) {
    char *arg1 = gettoken(msg, ' ');
    char *arg2 = gettoken(arg1, ' ');
    m_log.dbg("CMD %s%s %s", CMD_PFX_SET, arg1, arg2);
    m->cfg.set(arg1, arg2); /* points to null char in worst case */
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_ZOOMIN, strlen(CMD_PFX_ZOOMIN)) == 0) {
    char *arg1 = gettoken(msg, ' ');
    char *arg2 = gettoken(arg1, ' ');
    m_log.dbg("CMD %s%s %s", CMD_PFX_ZOOMIN, arg1, arg2);
    m->imgMain.zoomin(atoi(arg1), atoi(arg2), &m->cfg);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_ZOOMOUT, strlen(CMD_PFX_ZOOMOUT)) == 0) {
    char *arg1 = gettoken(msg, ' ');
    char *arg2 = gettoken(arg1, ' ');
    m_log.dbg("CMD %s%s %s", CMD_PFX_ZOOMOUT, arg1, arg2);
    m->imgMain.zoomout(atoi(arg1), atoi(arg2), &m->cfg);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_PAN, strlen(CMD_PFX_PAN)) == 0) {
    char *arg1 = gettoken(msg, ' ');
    char *arg2 = gettoken(arg1, ' ');
    m_log.dbg("CMD %s%s %s", CMD_PFX_PAN, arg1, arg2);
    m->imgMain.pan(atoi(arg1), atoi(arg2));
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_CLICK, strlen(CMD_PFX_CLICK)) == 0) {
    char *arg1 = gettoken(msg, ' ');
    char *arg2 = gettoken(arg1, ' ');
    m_log.dbg("CMD %s%s %s", CMD_PFX_CLICK, arg1, arg2);
    uint32_t x = m->imgMain.clippedPx2CellX(atoi(arg1));
    uint32_t y = m->imgMain.clippedPx2CellY(atoi(arg2));
    BufferDyn out(1024);
    out.addf("info");
    out.addf("%d: %s<br>", x, m->mat->xlab(x));
    out.addf("%d: %s<br>", y, m->mat->ylab(y));
    out.addf("z-score: %f<br>", m->mat->get(y, x));
    m_srv->sendData(id(), out.cdata(), out.len());
    return;
  }
  if (strncmp(msg, CMD_PFX_ONAMEX, strlen(CMD_PFX_ONAMEX)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_ONAMEX);
    m->mat->order(HmMat::ORDER_ALPHABELIC_X);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_ONAMEY, strlen(CMD_PFX_ONAMEY)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_ONAMEY);
    m->mat->order(HmMat::ORDER_ALPHABELIC_Y);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_ORANDX, strlen(CMD_PFX_ORANDX)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_ORANDX);
    m->mat->order(HmMat::ORDER_RANDOM_X);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_ORANDY, strlen(CMD_PFX_ORANDY)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_ORANDY);
    m->mat->order(HmMat::ORDER_RANDOM_Y);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_OCLUSSLX, strlen(CMD_PFX_OCLUSSLX)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_OCLUSSLX);
    m->mat->order(HmMat::ORDER_HCLUSTER_SL_X);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_OCLUSSLY, strlen(CMD_PFX_OCLUSSLY)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_OCLUSSLY);
    m->mat->order(HmMat::ORDER_HCLUSTER_SL_Y);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }

  if (strncmp(msg, CMD_PFX_OCLUSCLX, strlen(CMD_PFX_OCLUSCLX)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_OCLUSCLX);
    m->mat->order(HmMat::ORDER_HCLUSTER_CL_X);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_OCLUSCLY, strlen(CMD_PFX_OCLUSCLY)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_OCLUSCLY);
    m->mat->order(HmMat::ORDER_HCLUSTER_CL_Y);
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }

  if (strncmp(msg, CMD_PFX_MOVE, strlen(CMD_PFX_MOVE)) == 0) {
    char *arg1 = gettoken(msg, ' ');
    gettoken(arg1, ' '); /* null term */
    m_log.dbg("CMD %s%s", CMD_PFX_MOVE, arg1);
    m->imgMain.move(arg1);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_LOADMAT, strlen(CMD_PFX_LOADMAT)) == 0) {
    char *arg1 = gettoken(msg, ' ');
    gettoken(arg1, ' '); /* null term */
    m_log.dbg("CMD %s%s", CMD_PFX_LOADMAT, arg1);
    loadMat(arg1);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
  if (strncmp(msg, CMD_PFX_TRANSPMAT, strlen(CMD_PFX_TRANSPMAT)) == 0) {
    m_log.dbg("CMD %s", CMD_PFX_TRANSPMAT);
    m->mat->transpose();
    m->imgUnscaled.update(m->mat);
    m->sendUpdate = true;
    sendStatus('w');
    return;
  }
}

/*----------------------------------------------------------------------------*/

void WsMatView::integrate(int64_t serviceTimeUsec) {
#if 0
  const uint32_t rate = 1 /* 1 per sec */;
  /* do update? */
  if (m_nextupdate < serviceTimeUsec) {
    m_nextupdate = serviceTimeUsec + (int64_t(1e9) / rate);
  }
#endif
  if (m->sendUpdate) {
    renderMain();
    renderTiny();
    renderYlab();
    renderXlab();
    renderYden();
    sendTiny();
    sendMain();
    sendYlab();
    sendXlab();
    sendYDen();
    sendInfo();
    m->sendUpdate = false;
    sendStatus('o');
  }
}

/*----------------------------------------------------------------------------*/

void WsMatView::sendStatus(char s) {
  ImgBase *img = ((s == 'o') ? ((ImgBase*)&m->imgStatusOk) : ((ImgBase*)&m->imgStatusWarn));
  BufferDyn out(1024*1024);
  out.addf("statdata:image/jpeg;base64,%s", img->jpgB64().cdata());
  m_srv->sendData(id(), out.cdata(), out.len());
}

/*----------------------------------------------------------------------------*/

void WsMatView::sendTiny() {
  BufferDyn out(1024*1024);
  out.addf("tinydata:image/jpeg;base64,%s", m->imgTiny.jpgB64().cdata());
  m_srv->sendData(id(), out.cdata(), out.len());
}

/*----------------------------------------------------------------------------*/

void WsMatView::sendMain() {
  BufferDyn out(1024*1024);
  out.addf("maindata:image/jpeg;base64,%s", m->imgMain.jpgB64().cdata());
  m_srv->sendData(id(), out.cdata(), out.len());
}


/*----------------------------------------------------------------------------*/

void WsMatView::sendXlab() {
  BufferDyn out(1024*1024);
  out.addf("xlabdata:image/jpeg;base64,%s", m->imgXlab.jpgB64().cdata());
  m_srv->sendData(id(), out.cdata(), out.len());
}

/*----------------------------------------------------------------------------*/

void WsMatView::sendYlab() {
  BufferDyn out(1024*1024);
  out.addf("ylabdata:image/jpeg;base64,%s", m->imgYlab.jpgB64().cdata());
  m_srv->sendData(id(), out.cdata(), out.len());
}

/*----------------------------------------------------------------------------*/

void WsMatView::sendYDen() {
  BufferDyn out(1024*1024);
  out.addf("ydendata:image/jpeg;base64,%s", m->imgYden.jpgB64().cdata());
  m_srv->sendData(id(), out.cdata(), out.len());
}

/*----------------------------------------------------------------------------*/

void WsMatView::sendInfo() {
  BufferDyn out(1024);
  out.addf("info");
  out.addf("%s: %d<br>", CFG_MAIN_HEI_INT, m->cfg.getInt(CFG_MAIN_HEI_INT));
  out.addf("%s: %d<br>", CFG_MAIN_WID_INT, m->cfg.getInt(CFG_MAIN_WID_INT));
  out.addf("%s: %d<br>", CFG_XLAB_HEI_INT, m->cfg.getInt(CFG_XLAB_HEI_INT));
  out.addf("%s: %d<br>", CFG_YLAB_WID_INT, m->cfg.getInt(CFG_YLAB_WID_INT));
  out.addf("%s: %d<br>", CFG_XDEN_HEI_INT, m->cfg.getInt(CFG_XDEN_HEI_INT));
  out.addf("%s: %d<br>", CFG_YDEN_WID_INT, m->cfg.getInt(CFG_YDEN_WID_INT));
  out.addf("%s: %d<br>", CFG_POSX_INT, m->cfg.getInt(CFG_POSX_INT));
  out.addf("%s: %d<br>", CFG_POSY_INT, m->cfg.getInt(CFG_POSY_INT));
  out.addf("%s: %d<br>", CFG_ZOOM_INT, m->cfg.getInt(CFG_ZOOM_INT));
  out.addf("%s: %d<br>", CFG_LABTXT_HEI_INT, m->cfg.getInt(CFG_LABTXT_HEI_INT));
  m_srv->sendData(id(), out.cdata(), out.len());
}

/*----------------------------------------------------------------------------*/

void WsMatView::renderTiny() {
  m->imgTiny.update(m->imgUnscaled, m->imgMain, m->cfg);
}

/*----------------------------------------------------------------------------*/

void WsMatView::renderMain() {
  m->imgMain.update(m->imgUnscaled, m->cfg);
}

/*----------------------------------------------------------------------------*/

void WsMatView::renderXlab() {
  m->imgXlab.update(m->imgMain, m->mat, m->cfg);
}
/*----------------------------------------------------------------------------*/

void WsMatView::renderYlab() {
  m->imgYlab.update(m->imgMain, m->mat, m->cfg);
}

/*----------------------------------------------------------------------------*/

void WsMatView::renderYden() {
  m->imgYden.update(m->imgMain, m->imgUnscaled, m->mat, m->cfg);
}


// img drag:
// http://jsfiddle.net/gigyme/YNMEX/132/

