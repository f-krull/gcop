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
  ServerTcpConfig scfg;
  scfg.port = 11380;
  ServerTcp s(scfg);
  s.listen();

  HttpFileService hfs;
  hfs.registerFile("data/index.html", "/index.html", HttpFileService::MIMETYPE_TEXT_HTML);
  hfs.registerFile("data/hmview.css", "/hmview.css", HttpFileService::MIMETYPE_TEXT_CSS);
  hfs.registerFile("data/hmview.js",  "/hmview.js",  HttpFileService::MIMETYPE_TEXT_JAVASCRIPT);
  s.addReader(&hfs);


  ServerTcpConfig swcfg;
  swcfg.port = 11381;
  ServerTcp sw(swcfg);
  sw.listen();
  WsService ws(new WsMatViewFactory());
  sw.addReader(&ws);

  const int64_t t_start = getTimeUsec();
  while (true) {
    s.integrate();
    sw.integrate();
    const int64_t t_cur = getTimeUsec() - t_start;
    ws.integrate(t_cur);
#if 0
    if (t_cur > 20 * 1e9) {
      break;
    }
#endif
  }
  return 0;
}
