var NumTeeth = 24;
var Delta = Math.PI/90;
var NumGears = 60;
var FaceColors = ["#000099", "#006600", "#990000", "#EEEE00"];
var SideColors = ["#0000FF", "#009900", "#FF0000", "#CCCC00"];


function drawGear(ctx, r) {
  var dT = Math.PI*2/NumTeeth;
  var dTq = dT/4;
  var outer = r;
  var inner = 0.7 * r;
  ctx.beginPath();
  for (var i=0; i<NumTeeth; i+=2) {
    ctx.lineTo(Math.sin(dT*i-dTq)*outer, Math.cos(dT*i-dTq)*outer);
    ctx.lineTo(Math.sin(dT*i+dTq)*inner, Math.cos(dT*i+dTq)*inner);
    ctx.lineTo(Math.sin(dT*(i+1)-dTq)*inner, Math.cos(dT*(i+1)-dTq)*inner);
    ctx.lineTo(Math.sin(dT*(i+1)+dTq)*outer, Math.cos(dT*(i+1)+dTq)*outer);
  }
  ctx.closePath();
  ctx.stroke();
};

function draw3DGear(ctx, angle, faceColor, sideColor, r) {
  ctx.fillStyle = faceColor;
  ctx.strokeStyle = sideColor;
  ctx.rotate(angle);
  drawGear(ctx, r);
  for (var i=0; i < 20; i++) {
    ctx.rotate(-angle);
    ctx.translate(0.707, 0.707);
    ctx.rotate(angle);
    drawGear(ctx, r);
  }
  ctx.fill()
  ctx.rotate(-angle);
}

function draw3DGearAt(ctx, x, y, r, angle, faceColor, sideColor) {
  ctx.save();
  ctx.translate(x, y);
  draw3DGear(ctx, angle, faceColor, sideColor, r);
  ctx.restore();
}

var anim = function() {
  var canvas = document.getElementById('gears');
  var ctx = canvas.getContext("2d");
  var ticks=0;
  var rotation = 0;
  var gears = [];

  for (var i=0; i<NumGears; i++) {
    color = Math.floor(Math.random()*FaceColors.length);
    gears.push({
        x: Math.random()*500,
        y: Math.random()*500,
        r: Math.random()*100+5,
        faceColor: FaceColors[color],
        sideColor: SideColors[color]
    });
  }

  function draw() {
    ctx.resetTransform();
    ctx.fillStyle = 'white';
    ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);

    rotation += Delta;
    if (rotation >= Math.PI*2) {
      rotation = 0;
    }

    for (var i=0; i < gears.length; i++) {
      draw3DGearAt(ctx, gears[i].x,  gears[i].y, gears[i].r, rotation,
          gears[i].faceColor, gears[i].sideColor); }

    ticks++;
    requestAnimationFrame(draw);
  }

  requestAnimationFrame(draw);


  function fps() {
    console.log(ticks);
    ticks = 0;
  }

  setInterval(fps, 1000);
}();


