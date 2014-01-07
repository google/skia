var NumTeeth = 24;
var NumGears = 60;
var DeltaTheta = Math.PI/90;
var FaceColors = ["#000099", "#006600", "#990000", "#EEEE00"];
var SideColors = ["#0000FF", "#009900", "#FF0000", "#CCCC00"];

function gearPath(r) {
  var outer = r;
  var inner = 0.7 * r;
  var dT = Math.PI*2/NumTeeth;
  var dTq = dT/4;
  p = new Path();
  p.moveTo(Math.sin(-2*dTq)*outer, Math.cos(-2*dTq)*outer);
  for (var i=0; i<NumTeeth; i+=2) {
    p.lineTo(Math.sin(dT*i-dTq)*outer, Math.cos(dT*i-dTq)*outer);
    p.lineTo(Math.sin(dT*i+dTq)*inner, Math.cos(dT*i+dTq)*inner);
    p.lineTo(Math.sin(dT*(i+1)-dTq)*inner, Math.cos(dT*(i+1)-dTq)*inner);
    p.lineTo(Math.sin(dT*(i+1)+dTq)*outer, Math.cos(dT*(i+1)+dTq)*outer);
  }
  p.close();
  return p;
}

function draw3DGear(ctx, angle, faceColor, sideColor, path) {
  ctx.strokeStyle = sideColor;
  ctx.fillStyle = faceColor;
  ctx.rotate(angle);
  ctx.stroke(path);
  for (var i=0; i < 20; i++) {
    ctx.rotate(-angle);
    ctx.translate(0.707, 0.707);
    ctx.rotate(angle);
    ctx.stroke(path);
  }
  ctx.fill(path)
  ctx.rotate(-angle);
}

function draw3DGearAt(ctx, x, y, angle, path, faceColor, sideColor) {
  ctx.save();
  ctx.translate(x, y);
  draw3DGear(ctx, angle, faceColor, sideColor, path);
  ctx.restore();
}

var onDraw = function() {
  var ticks=0;
  var rotation = 0;
  var gears = [];

  for (var i=0; i<NumGears; i++) {
    color = Math.floor(Math.random()*FaceColors.length);
    gears.push({
        x: Math.random()*500,
        y: Math.random()*500,
        path: gearPath(Math.random()*100+5),
        faceColor: FaceColors[color],
        sideColor: SideColors[color]
    });
  }

  function draw(ctx) {
    ctx.resetTransform();

    rotation += DeltaTheta;
    if (rotation >= Math.PI*2) {
      rotation = 0;
    }

    for (var i=0; i < gears.length; i++) {
      draw3DGearAt(ctx, gears[i].x,  gears[i].y, rotation, gears[i].path,
          gears[i].faceColor, gears[i].sideColor);
    }

    ticks++;
    inval();
  };

  function fps() {
    print(ticks);
    ticks = 0;
    setTimeout(fps, 1000);
  };

  setTimeout(fps, 1000);

  return draw;
}();
