#include "server.h"
#include "httpfileservice.h"
#include "wsmatview.h"
#include <time.h>
#include <signal.h>

/*----------------------------------------------------------------------------*/

volatile sig_atomic_t g_stop = 0;

static void signalHandler(int signum) {
  g_stop = 1;
}

/*----------------------------------------------------------------------------*/

int64_t getTimeUsec() {
  struct timespec tms;
  clock_gettime(CLOCK_REALTIME,&tms);
  return (int64_t)(tms.tv_sec) * (int64_t)1000000000 + (int64_t)(tms.tv_nsec);
}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv) {
  signal(SIGINT, signalHandler);
  Log log("main");

  /* create HTTP server for static files */
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
  /* start servers */
  const int64_t t_start = getTimeUsec();
  while (!g_stop) {
    srv_http.integrate();
    srv_webs.integrate();
    const int64_t t_cur = getTimeUsec() - t_start;
    wss->integrate(t_cur);
#if 0
    if (t_cur > 20 * 1e9) {
      break;
    }
#endif
  }
  log.dbg("shutdown");
  return 0;
}