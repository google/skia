var IS_SKV8 = typeof document == "undefined";
var HAS_PATH = typeof Path2D != "undefined";
var HAS_DISPLAY_LIST = typeof DisplayList != "undefined";

var NumTeeth = 24;
var NumGears = 60;
var DeltaTheta = Math.PI/90;
var FaceColors = ["#000099", "#006600", "#990000", "#EEEE00"];
var SideColors = ["#0000FF", "#009900", "#FF0000", "#CCCC00"];

function makeGear(pathLike, r) {
  var dT = Math.PI*2/NumTeeth;
  var dTq = dT/4;
  var outer = r;
  var inner = 0.7 * r;
  pathLike.moveTo(Math.sin(-2*dTq)*outer, Math.cos(-2*dTq)*outer);
  for (var i=0; i<NumTeeth; i+=2) {
    pathLike.lineTo(Math.sin(dT*i-dTq)*outer, Math.cos(dT*i-dTq)*outer);
    pathLike.lineTo(Math.sin(dT*i+dTq)*inner, Math.cos(dT*i+dTq)*inner);
    pathLike.lineTo(Math.sin(dT*(i+1)-dTq)*inner, Math.cos(dT*(i+1)-dTq)*inner);
    pathLike.lineTo(Math.sin(dT*(i+1)+dTq)*outer, Math.cos(dT*(i+1)+dTq)*outer);
  }
}

function gearPath(r) {
  if (HAS_PATH) {
    p = new Path2D();
    makeGear(p, r)
    p.closePath();
    return p;
  } else {
    return null;
  }
}

function gearDisplayListStroke(r, color) {
  if (HAS_DISPLAY_LIST) {
    p = new Path2D();
    makeGear(p, r)
    p.closePath();
    var dl = new DisplayList();
    dl.strokeStyle = color;
    dl.stroke(p);
    dl.finalize()
    return dl;
  } else {
    return null;
  }
}

function gearDisplayListFill(r, color) {
  if (HAS_DISPLAY_LIST) {
    p = new Path2D();
    makeGear(p, r)
    p.closePath();
    var dl = new DisplayList();
    dl.fillStyle = color;
    dl.fill(p);
    dl.finalize()
    return dl;
  } else {
    return null;
  }
}

function strokeGear(ctx, gear) {
  if (HAS_PATH) {
    ctx.stroke(gear.path);
  } else {
    ctx.beginPath();
    makeGear(ctx, gear.r);
    ctx.closePath();
    ctx.stroke();
  }
}

function fillGear(ctx) {
  if (HAS_PATH) {
    ctx.fill(gear.path);
  } else {
    ctx.beginPath();
    makeGear(ctx, gear.r);
    ctx.closePath();
    ctx.fill();
  }
}

function draw3DGear(ctx, angle, gear) {
  ctx.strokeStyle = gear.sideColor;
  ctx.fillStyle = gear.faceColor;
  ctx.rotate(angle);
  strokeGear(ctx, gear);
  for (var i=0; i < 20; i++) {
    ctx.rotate(-angle);
    ctx.translate(0.707, 0.707);
    ctx.rotate(angle);
    if (HAS_DISPLAY_LIST) {
        ctx.draw(gear.gearStroke);
    } else {
        strokeGear(ctx, gear);
    }
  }
  if (HAS_DISPLAY_LIST) {
      ctx.draw(gear.gearFill);
  } else {
      fillGear(ctx, gear);
  }
  ctx.rotate(-angle);
}

function draw3DGearAt(ctx, angle, gear) {
  ctx.save();
  ctx.translate(gear.x, gear.y);
  draw3DGear(ctx, angle, gear);
  ctx.restore();
}

var onDraw = function() {
  var ticks=0;
  var rotation = 0;
  var gears = [];

  for (var i=0; i<NumGears; i++) {
    color = Math.floor(Math.random()*FaceColors.length);
    r = Math.random()*100+5;
    gears.push({
        x: Math.random()*500,
        y: Math.random()*500,
        path: gearPath(r),
        gearFill: gearDisplayListFill(r, FaceColors[color]),
        gearStroke: gearDisplayListStroke(r, SideColors[color]),
        r: r,
        faceColor: FaceColors[color],
        sideColor: SideColors[color]
    });
  }

  function draw(ctx) {
    ctx.resetTransform();

    ctx.fillStyle = "#FFFFFF";
    ctx.fillRect(0, 0, 499, 499);

    rotation += DeltaTheta;
    if (rotation >= Math.PI*2) {
      rotation = 0;
    }

    for (var i=0; i < gears.length; i++) {
      gear = gears[i];
      draw3DGearAt(ctx, rotation, gear);
    }

    ticks++;
    if (IS_SKV8) {
      inval();
    }
  };

  function fps() {
    console.log(ticks);
    ticks = 0;
    setTimeout(fps, 1000);
  };

  setTimeout(fps, 1000);

  return draw;
}();

if (!IS_SKV8) {
  window.onload = function(){
    var canvas = document.getElementById("gears");
    var ctx = canvas.getContext("2d");
    function drawCallback() {
      onDraw(ctx);
      setTimeout(drawCallback, 1);
    }
    setTimeout(drawCallback, 1);
  }
}

console.log("HAS_PATH: " + HAS_PATH);
