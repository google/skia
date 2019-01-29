const CanvasKitInit = require('./bin/canvaskit.js');
const fs = require('fs');
const path = require('path');

CanvasKitInit({
  locateFile: (file) => __dirname + '/bin/'+file,
}).ready().then((CanvasKit) => {
  let canvas = CanvasKit.MakeCanvas(300, 300);

  let img = fs.readFileSync(path.join(__dirname, 'test.png'));
  img = canvas.decodeImage(img);

  let fontData = fs.readFileSync(path.join(__dirname, './Roboto-Regular.woff'));
  canvas.loadFont(fontData, {
                                'family': 'Roboto',
                                'style': 'normal',
                                'weight': '400',
                              });

  let ctx = canvas.getContext('2d');
  ctx.font = '30px Roboto';
  ctx.rotate(.1);
  let text = ctx.measureText('Awesome');
  ctx.fillText('Awesome ', 50, 100);
  ctx.strokeText('Groovy!', 60+text.width, 100);

  // Draw line under Awesome
  ctx.strokeStyle = 'rgba(125,0,0,0.5)';
  ctx.beginPath();
  ctx.lineWidth = 6;
  ctx.lineTo(50, 102);
  ctx.lineTo(50 + text.width, 102);
  ctx.stroke();

  // squished vertically
  ctx.globalAlpha = 0.7
  ctx.imageSmoothingQuality = 'medium';
  ctx.drawImage(img, 150, 150, 150, 100);
  ctx.rotate(-.2);
  ctx.imageSmoothingEnabled = false;
  ctx.drawImage(img, 100, 150, 400, 350, 10, 200, 150, 100);

  console.log('<img src="' + canvas.toDataURL() + '" />');

  fancyAPI(CanvasKit);
});

function fancyAPI(CanvasKit) {
  let surface = CanvasKit.MakeSurface(300, 300);
  const canvas = surface.getCanvas();

  const paint = new CanvasKit.SkPaint();

  const fontMgr = CanvasKit.SkFontMgr.RefDefault();
  let robotoData = fs.readFileSync(path.join(__dirname, './Roboto-Regular.woff'));
  const roboto = fontMgr.MakeTypefaceFromData(robotoData);

  const textPaint = new CanvasKit.SkPaint();
  textPaint.setColor(CanvasKit.Color(40, 0, 0));
  textPaint.setAntiAlias(true);

  const textFont = new CanvasKit.SkFont(roboto, 30);

  const skpath = starPath(CanvasKit);
  const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], 1);

  paint.setPathEffect(dpe);
  paint.setStyle(CanvasKit.PaintStyle.Stroke);
  paint.setStrokeWidth(5.0);
  paint.setAntiAlias(true);
  paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

  canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

  canvas.drawPath(skpath, paint);
  canvas.drawText('Try Clicking!', 10, 280, textPaint, textFont);

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

  // These delete calls free up memeory in the C++ WASM memory block.
  dpe.delete();
  skpath.delete();
  textPaint.delete();
  paint.delete();
  roboto.delete();
  textFont.delete();

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