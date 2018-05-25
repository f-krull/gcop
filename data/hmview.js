// helper functions
function assert(condition, message) {
  if (!condition) {
    throw message || "Assertion failed";
  }
}

//------------------------------------------------------------------------------

function MVMsgPrefix() {
  this.MAT_HEIGHT = "MAT_HEIGHT";
  this.MAT_WIDTH  = "MAT_WIDTH";
};

function MatViewWs() {
  this._ws = null;
}
MatViewWs.prototype.send = function(msg) {
  if (this._ws == null) {
    return;
  }
  this._ws.send(msg)
}
MatViewWs.prototype.connect = function() {
  // already connected?
  if (this._ws != null) {
    return;
  }
  if ("WebSocket" in window) {
    this._ws = new WebSocket("ws://" + window.location.hostname + ":11381");
    this._ws.onopen = function() {
      requestSize();
    };
    this._ws.onmessage = function (evt) {
      var received_msg = evt.data;
      var dst_len = 4;
      var dst = received_msg.substring(0, dst_len);
      if (dst == "info") {
        document.getElementById(dst).innerHTML = received_msg.slice(dst_len);
      } else {
        document.getElementById(dst).src = received_msg.slice(dst_len);
      }
    };
    this._ws.onclose = function() {
      // websocket is closed.
      alert("Connection is closed...");
      this._ws = null;
    };
    } else {
      alert("WebSocket not supported by your browser!");
    }
}

//------------------------------------------------------------------------------

function DragInfo() {
  this._startX   = -1;
  this._startY   = -1;
  this._dragging = false;
  this._target   = null;
  this._coordX = 0;
  this._coordY = 0;
  this._deltaX = 0;
  this._deltaY = 0;
}
DragInfo.prototype.start = function(x, y, t) {
  console.log("start")
  this._startX = x;
  this._startY = y;
  this._dragging = true;
  this._target = t;
  this._coordX = x;
  this._coordY = y;
  this._deltaX = 0;
  this._deltaY = 0;
}
DragInfo.prototype.move = function(x, y) {
  if (!dragInf._dragging) {
    return;
  }
  this._coordX = x;
  this._coordY = y;
}
DragInfo.prototype.stop = function(x, y) {
  console.log("stop")
  this._dragging = false;
  this._deltaX = this._coordX - this._startX;
  this._deltaY = this._coordY - this._startY;
  this._startX = -1;
  this._startY = -1;
}

function requestSize() {
  var main_wid = document.querySelector('td.colxlab').offsetWidth;
  var main_hei = document.querySelector('tr.rowmain').offsetHeight;
  var xlab_hei = document.querySelector('tr.rowxlab').offsetHeight;
  var ylab_wid = document.querySelector('td.colstat').offsetWidth;
  mvWs.send("SET MAIN_WID "   + main_wid);
  mvWs.send("SET MAIN_HEI "   + main_hei);
  mvWs.send("SET XLAB_HEI "   + xlab_hei);
  mvWs.send("SET YLAB_WID "   + ylab_wid);
  mvWs.send("SET LABTXT_HEI " + 13);
  console.log("main_wid " + main_wid);
  console.log("main_hei " + main_hei);
  console.log("xlab_hei " + xlab_hei);
  console.log("ylab_wid " + ylab_wid);
}

//------------------------------------------------------------------------------

mvWs = new MatViewWs();
dragInf = new DragInfo();

document.getElementById("main").addEventListener("wheel", function(evt){
  if (evt.deltaY > 0) {
    mvWs.send("ZOOMIN "  + evt.offsetX + " " + evt.offsetY);
  } else if (evt.deltaY < 0) {
    mvWs.send("ZOOMOUT " + evt.offsetX + " " + evt.offsetY);
  }
});
document.getElementById("oclusx").addEventListener("click", function(evt){
  mvWs.send("OCLUSSLX");
});
document.getElementById("oclusy").addEventListener("click", function(evt){
  mvWs.send("OCLUSSLY");
});
document.getElementById("onamey").addEventListener("click", function(evt){
  mvWs.send("ONAMEY");
});
document.getElementById("onamex").addEventListener("click", function(evt){
  mvWs.send("ONAMEX");
});
document.getElementById("orandy").addEventListener("click", function(evt){
  mvWs.send("ORANDY");
});
document.getElementById("orandx").addEventListener("click", function(evt){
  mvWs.send("ORANDX");
});
document.getElementById('main').ondragstart = function(evt){
  mvWs.send("TEST " + evt.clientX + " " + evt.clientY);
  dragInf.start(evt.clientX, evt.clientY, evt.target ? evt.target : evt.srcElement);
  return false;
};
document.getElementById('main').onmousemove = function(evt){
  dragInf.move(evt.clientX, evt.clientY);
  return false;
};
document.onmouseup = function(evt){
  if (dragInf._dragging) {
    dragInf.stop();
    mvWs.send("PAN " + dragInf._deltaX + " " + dragInf._deltaY);
  }
  return false;
};

document.onkeydown = function(e){
  var keyup    = '38';
  var keydown  = '40';
  var keyleft  = '37';
  var keyright = '39';
  if (e.keyCode == keyup) {
    mvWs.send("MOVE UP");
    return false;
  }
  if (e.keyCode == keydown) {
    mvWs.send("MOVE DOWN");
    return false;
  }
  if (e.keyCode == keyleft) {
    mvWs.send("MOVE LEFT");
    return false;
  }
  if (e.keyCode == keyright) {
    mvWs.send("MOVE RIGHT");
    return false;
  }
  return true;
}

window.addEventListener('resize', function(event){
  requestSize();
});

function Start() {
  mvWs.connect();
  //alert(document.querySelector('td.xlab').offsetWidth)
}
Start()
