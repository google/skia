var IS_SKV8 = typeof document == "undefined";
var HAS_PATH = typeof Path2D != "undefined";

function circlePath(r) {
  if (HAS_PATH) {
    var p = new Path2D();
    p.arc(0, 0, r, 0, 2*Math.PI);
    p.closePath();
    return p;
  } else {
    return null;
  }
}

var onDraw = function() {
  var W = 500;
  var H = 500;
  var NumParticles = 100;

  var angle = 0;
  var ticks = 0;
  var particles =[];

  for (var i = 0; i < NumParticles; i++) {
    particles[i] = {
      x:    Math.floor(Math.random()*W),
      y:    Math.floor(Math.random()*H),
      r:    Math.floor(Math.random()*7+1),
      path: circlePath(Math.random()*7+1),
    }
  }

  function draw(ctx) {
    ctx.fillStyle = "#ADD8E6";
    ctx.fillRect(0, 0, W-1, H-1);
    ctx.fillStyle = "#FFFFFF";

    angle += 0.0039;
    for (var i = 0; i < particles.length; i++) {
      var p = particles[i];
      p.x += Math.floor(Math.sin(angle)*5.0);
      p.y += 0.6*p.r;
      if (p.x > W) {
        p.x-=W;
      }
      if (p.x < 0) {
        p.x += W;
      }
      if(p.y>(H+1)){
        p.y = 0;
      }
      if (HAS_PATH) {
        ctx.save();
        ctx.translate(p.x, p.y);
        ctx.fill(p.path);
        ctx.restore();
      } else {
        ctx.beginPath();
        ctx.moveTo(p.x, p.y);
        ctx.arc(p.x, p.y, p.r, 0, 2*Math.PI, true);
        ctx.closePath();
        ctx.fill();
      }
    };

    ticks++;
    if (IS_SKV8) {
      inval();
    }
  }

  function fps() {
    console.log(ticks);
    ticks = 0;
    setTimeout(fps, 1000);
  }

  setTimeout(fps, 1000);

  return draw;
}();

if (!IS_SKV8) {
  window.onload = function(){
    var canvas = document.getElementById("snow");
    var ctx = canvas.getContext("2d");
    function drawCallback() {
      onDraw(ctx);
      setTimeout(drawCallback, 1);
    }
    setTimeout(drawCallback, 1);
  }
}

console.log("HAS_PATH: " + HAS_PATH);
