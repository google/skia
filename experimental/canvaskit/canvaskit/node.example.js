console.log('hello world');

const CanvasKitInit = require('./bin/canvaskit.js');

CanvasKitInit({
  locateFile: (file) => __dirname + '/bin/'+file,
}).then((CK) => {
  CanvasKit = CK;
  let canvas = CanvasKit.MakeCanvas(300, 300);

  let ctx = canvas.getContext('2d');
  ctx.font = '30px Impact'
  ctx.rotate(.1);
  let text = ctx.measureText('Awesome');
  ctx.fillText('Awesome ', 50, 100);
  ctx.strokeText('Groovy!', 60+text.width, 100);

  // Draw line under Awesome
  ctx.strokeStyle = 'rgba(125,0,0,0.5)';
  ctx.beginPath();
  ctx.lineWidth=6;
  ctx.lineTo(50, 102);
  ctx.lineTo(50 + text.width, 102);
  ctx.stroke();

  // TODO load an image from file
  console.log('<img src="' + canvas.toDataURL() + '" />');
});

function fancyAPI() {
  CanvasKit.initFonts();
  console.log('loaded');

  let surface = CanvasKit.MakeSurface(300, 300);
  const canvas = surface.getCanvas();

  const paint = new CanvasKit.SkPaint();

  const textPaint = new CanvasKit.SkPaint();
  textPaint.setColor(CanvasKit.Color(40, 0, 0));
  textPaint.setTextSize(30);
  textPaint.setAntiAlias(true);

  const path = starPath(CanvasKit);
  const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], 1);

  paint.setPathEffect(dpe);
  paint.setStyle(CanvasKit.PaintStyle.STROKE);
  paint.setStrokeWidth(5.0);
  paint.setAntiAlias(true);
  paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

  canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

  canvas.drawPath(path, paint);
  canvas.drawText('Try Clicking!', 10, 280, textPaint);

  surface.flush();

  const img = surface.makeImageSnapshot()
  if (!img) {
    console.error('no snapshot');
    return;
  }
  const png = img.encodeToData()
  if (!png) {
    console.error('encoding failure');
    return
  }
  const pngBytes = CanvasKit.getSkDataBytes(png);
  // See https://stackoverflow.com/a/12713326
  let b64encoded = Buffer.from(pngBytes).toString('base64');
  console.log(`<img src="data:image/png;base64,${b64encoded}" />`);

  dpe.delete();
  path.delete();
  canvas.delete();
  textPaint.delete();
  paint.delete();

  surface.dispose();
}

function starPath(CanvasKit, X=128, Y=128, R=116) {
  let p = new CanvasKit.SkPath();
  p.moveTo(X + R, Y);
  for (let i = 1; i < 8; i++) {
    let a = 2.6927937 * i;
    p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
  }
  return p;
}