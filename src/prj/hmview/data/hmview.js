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
    this._ws.binaryType = "arraybuffer";
    this._ws.onopen = function() {
      requestSize();
    };
    this._ws.onmessage = function (evt) {
      var received_msg = evt.data;
      var dst_len = 4;
      /* convert first 4 bytes to string */
      var dst = received_msg.slice(0, 4);
      if (typeof(dst) != "string") {
        dst = new TextDecoder("utf-8").decode(dst);
      }
      if (dst == "info") {
        /* transmitted as string */
        document.getElementById(dst).innerHTML = received_msg.slice(dst_len);
      } else {
        /* transmitted as binary */
        var bytes = new Uint8Array(received_msg.slice(dst_len));
        var blob = new Blob([bytes.buffer], {type: "image/jpeg"});
        var url = URL.createObjectURL(blob);
        document.getElementById(dst).src = url;
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

//------------------------------------------------------------------------------

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
  g_imgmain = document.getElementById("main");
  g_imgxlab = document.getElementById("xlab");
  g_imgylab = document.getElementById("ylab");
}

//------------------------------------------------------------------------------

var g_mvWs = new MatViewWs();
var g_dragInf   = new DragInfo();
var g_selectInf = new DragInfo();
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
document.getElementById("crop").addEventListener("click", function(evt){
  g_mvWs.send("CROPMAT");
});
document.getElementById("load1").addEventListener("click", function(evt){
  g_mvWs.send("LOADMAT data/disreg_matrix_1212x845.txt");
});
document.getElementById("load2").addEventListener("click", function(evt){
  g_mvWs.send("LOADMAT data/disreg_matrix_10x8.txt");
});
document.getElementById("load3").addEventListener("click", function(evt){
  g_mvWs.send("LOADMAT data/disreg_matrix_1212x845.txt");
});
document.getElementById("dist_euclidean").addEventListener("click", function(evt){
  g_mvWs.send("ODISTFUN euclidean");
});
document.getElementById("dist_pearson").addEventListener("click", function(evt){
  g_mvWs.send("ODISTFUN corrpearson");
  g_mvWs.send("SELECTY Schiz.*|Bipolar.*");
});
document.getElementById("dist_spears").addEventListener("click", function(evt){
  g_mvWs.send("ODISTFUN corrspears");
});
document.getElementById('main').ondragstart = function(evt){
  if (!evt.shiftKey) {
    g_dragInf.start(evt.clientX, evt.clientY, evt.target ? evt.target : evt.srcElement);
  } else {
    g_selectInf.start(evt.offsetX, evt.offsetY, evt.target ? evt.target : evt.srcElement);
  }
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
};
document.getElementById('main').onmouseup = function(evt){
  /* this might conflict with document.onmouseup! */
  if (g_selectInf._dragging) {
    g_mvWs.send("SELECT " + g_selectInf._startX + " " + g_selectInf._startY + " "
                          + evt.offsetX         + " " + evt.offsetY);
    g_selectInf.stop();
  } else if (!g_dragInf._dragging) {
    g_mvWs.send("CLICK " + evt.offsetX + " " + evt.offsetY);
  }
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
