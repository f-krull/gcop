// helper functions
function assert(condition, message) {
  if (!condition) {
    throw message || "Assertion failed";
  }
}

//------------------------------------------------------------------------------

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
        g_imgmain.style.left = 0 + 'px';
        g_imgmain.style.top  = 0 + 'px';
        g_imgxlab.style.left = 0 + 'px';
        g_imgylab.style.top  = 0 + 'px';
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
  if (!this._dragging) {
    return;
  }
  this._coordX = x;
  this._coordY = y;
  g_imgmain.style.left = (this._coordX - this._startX) + 'px';
  g_imgmain.style.top  = (this._coordY - this._startY) + 'px';
  g_imgxlab.style.left = (this._coordX - this._startX) + 'px';
  g_imgylab.style.top  = (this._coordY - this._startY) + 'px';
}
DragInfo.prototype.stop = function(x, y) {
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
  var xden_hei = document.querySelector('tr.rowxden').offsetHeight;
  var yden_wid = document.querySelector('td.colyden').offsetWidth;
  g_mvWs.send("SET MAIN_WID "   + main_wid);
  g_mvWs.send("SET MAIN_HEI "   + main_hei);
  g_mvWs.send("SET XLAB_HEI "   + xlab_hei);
  g_mvWs.send("SET YLAB_WID "   + ylab_wid);
  g_mvWs.send("SET XDEN_HEI "   + xden_hei);
  g_mvWs.send("SET YDEN_WID "   + yden_wid);
  g_mvWs.send("SET LABTXT_HEI " + 13);
  console.log("main_wid " + main_wid);
  console.log("main_hei " + main_hei);
  console.log("xlab_hei " + xlab_hei);
  console.log("ylab_wid " + ylab_wid);
  g_imgmain = document.getElementById("main");
  g_imgxlab = document.getElementById("xlab");
  g_imgylab = document.getElementById("ylab");
}

//------------------------------------------------------------------------------

var g_mvWs = new MatViewWs();
var g_dragInf = new DragInfo();
var g_imgmain = null;
var g_imgxlab = null;
var g_imgylab = null;

document.getElementById("main").addEventListener("wheel", function(evt){
  if (evt.deltaY > 0) {
    g_mvWs.send("ZOOMIN "  + evt.offsetX + " " + evt.offsetY);
  } else if (evt.deltaY < 0) {
    g_mvWs.send("ZOOMOUT " + evt.offsetX + " " + evt.offsetY);
  }
});
document.getElementById("oclusslx").addEventListener("click", function(evt){
  g_mvWs.send("OCLUSSLX");
});
document.getElementById("oclussly").addEventListener("click", function(evt){
  g_mvWs.send("OCLUSSLY");
});
document.getElementById("oclusclx").addEventListener("click", function(evt){
  g_mvWs.send("OCLUSCLX");
});
document.getElementById("ocluscly").addEventListener("click", function(evt){
  g_mvWs.send("OCLUSCLY");
});
document.getElementById("onamey").addEventListener("click", function(evt){
  g_mvWs.send("ONAMEY");
});
document.getElementById("onamex").addEventListener("click", function(evt){
  g_mvWs.send("ONAMEX");
});
document.getElementById("orandy").addEventListener("click", function(evt){
  g_mvWs.send("ORANDY");
});
document.getElementById("orandx").addEventListener("click", function(evt){
  g_mvWs.send("ORANDX");
});
document.getElementById("transpose").addEventListener("click", function(evt){
  g_mvWs.send("TRANSPMAT");
});
document.getElementById("load1").addEventListener("click", function(evt){
  g_mvWs.send("LOADMAT data/disreg_matrix.txt");
});
document.getElementById("load2").addEventListener("click", function(evt){
  g_mvWs.send("LOADMAT data/disreg_matrix_10x8.txt");
});
document.getElementById("load3").addEventListener("click", function(evt){
  g_mvWs.send("LOADMAT data/disreg_matrix_half.txt");
});
document.getElementById('main').ondragstart = function(evt){
  g_mvWs.send("TEST " + evt.clientX + " " + evt.clientY);
  g_dragInf.start(evt.clientX, evt.clientY, evt.target ? evt.target : evt.srcElement);
  return false;
};
document.getElementById('main').onmousemove = function(evt){
  g_dragInf.move(evt.clientX, evt.clientY);
  return false;
};
document.onmouseup = function(evt){
  if (g_dragInf._dragging) {
    g_dragInf.stop();
    g_mvWs.send("PAN " + g_dragInf._deltaX + " " + g_dragInf._deltaY);
  }
  return false;
};

document.onkeydown = function(e){
  var keyup    = '38';
  var keydown  = '40';
  var keyleft  = '37';
  var keyright = '39';
  if (e.keyCode == keyup) {
    g_mvWs.send("MOVE UP");
    return false;
  }
  if (e.keyCode == keydown) {
    g_mvWs.send("MOVE DOWN");
    return false;
  }
  if (e.keyCode == keyleft) {
    g_mvWs.send("MOVE LEFT");
    return false;
  }
  if (e.keyCode == keyright) {
    g_mvWs.send("MOVE RIGHT");
    return false;
  }
  return true;
}

window.addEventListener('resize', function(event){
  requestSize();
});

function Start() {
  g_mvWs.connect();
  //alert(document.querySelector('td.xlab').offsetWidth)
}
Start()
