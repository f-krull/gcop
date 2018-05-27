#include "server.h"
#include "httpfileservice.h"
#include "wsmatview.h"
#include <time.h>

/*----------------------------------------------------------------------------*/

int64_t getTimeUsec() {
  struct timespec tms;
  clock_gettime(CLOCK_REALTIME,&tms);
  return (int64_t)(tms.tv_sec) * (int64_t)1000000000 + (int64_t)(tms.tv_nsec);
}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
  /* create HTTP server dor static files */
  HttpFileService * hfs = new HttpFileService;
  hfs->registerFile("data/index.html", "/index.html", HttpFileService::MIMETYPE_TEXT_HTML);
  hfs->registerFile("data/hmview.css", "/hmview.css", HttpFileService::MIMETYPE_TEXT_CSS);
  hfs->registerFile("data/hmview.js",  "/hmview.js",  HttpFileService::MIMETYPE_TEXT_JAVASCRIPT);
  ServerTcp srv_http(hfs, ServerTcpConfig{11380});
  srv_http.listen();
  /* create WebSocket server with matrix viewer service */
  WsService *wss = new WsService(new WsMatViewFactory());
  ServerTcp srv_webs(wss, ServerTcpConfig{11381});
  srv_webs.listen();

  const int64_t t_start = getTimeUsec();
  while (true) {
    srv_http.integrate();
    srv_webs.integrate();
    const int64_t t_cur = getTimeUsec() - t_start;
    wss->integrate(t_cur);
#if 1
    if (t_cur > 20 * 1e9) {
      break;
    }
#endif
  }
  return 0;
}
