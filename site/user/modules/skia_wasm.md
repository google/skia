Skia + WebAssembly + WebGL
==========================

We are experimenting with building Skia for Web Assembly.
An SkSurface encapsulates a WebGL context, allowing for
direct drawing to an html canvas.

<style>
  #demo canvas {
    border: 1px dashed #AAA;
  }

  #demo,#paths,#sk_drinks,#sk_party {
    width: 300px;
    height: 300px;
  }

  #sk_legos {
    width: 400px;
    height: 300px;
  }
</style>

<div id=demo>
  <h2>Skia draws Paths to WebGL</h2>
  <canvas id=patheffect width=300 height=300></canvas>
  <canvas id=paths width=200 height=200></canvas>

  <h2>Skottie</h2>
  <canvas id=sk_legos width=800 height=600></canvas>
  <canvas id=sk_drinks width=500 height=500></canvas>
  <canvas id=sk_party width=800 height=800></canvas>
</div>

<script type="text/javascript" charset="utf-8">
(function() {
  // Tries to load the WASM version if supported, then falls back to asmjs
  let s = document.createElement('script');
  var locate_file = '';
  if (window.WebAssembly && typeof window.WebAssembly.compile === 'function') {
    console.log('WebAssembly is supported!');
    locate_file = 'https://storage.googleapis.com/skia-cdn/skia-wasm/0.0.1/bin/';
  } else {
    console.log('WebAssembly is not supported (yet) on this browser. Using the asmjs version of PathKit');
    document.getElementById('demo').innerHTML = "WASM not supported by your browser. Try a recent version of Chrome or Firefox";
    return;
  }
  s.src = locate_file + 'skia.js';
  s.onload = () => {
  var SkiaWASM = null;
  var legoJSON = null;
  var drinksJSON = null;
  var confettiJSON = null;
  SkiaInit({
    locateFile: (file) => locate_file + file,
  }).then((Skia) => {
    SkiaWASM = Skia;
    DrawingExample(Skia);
    PathExample(Skia);
    SkottieExample(SkiaWASM, 'sk_legos', legoJSON);
    SkottieExample(SkiaWASM, 'sk_drinks', drinksJSON);
    SkottieExample(SkiaWASM, 'sk_party', confettiJSON);
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/lego_loader.json').then((resp) => {
    resp.text().then((str) => {
      legoJSON = str;
      SkottieExample(SkiaWASM, 'sk_legos', legoJSON);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/drinks.json').then((resp) => {
    resp.text().then((str) => {
      drinksJSON = str;
      SkottieExample(SkiaWASM, 'sk_drinks', drinksJSON);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/confetti.json').then((resp) => {
    resp.text().then((str) => {
      confettiJSON = str;
      SkottieExample(SkiaWASM, 'sk_party', confettiJSON);
    });
  });

  function DrawingExample(Skia) {
    const surface = Skia.getWebGLSurface('patheffect');
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = Skia.currentContext();

    const canvas = surface.getCanvas();
    const path = starPath(Skia);
    const paint = new Skia.SkPaint();

    const textPaint = new Skia.SkPaint();
    textPaint.setColor(Skia.Color(40, 0, 0, 1.0));
    textPaint.setStrokeWidth(2);
    textPaint.setTextSize(60);

    let i = 0;

    function drawFrame() {
      Skia.setCurrentContext(context);
      const dpe = Skia.MakeSkDiscretePathEffect(10.0, 4.0, i/20);
      i++;

      paint.setPathEffect(dpe);
      paint.setStyle(Skia.PaintStyle.STROKE);
      paint.setStrokeWidth(2.0);
      paint.setAntiAlias(true);
      paint.setColor(Skia.Color(66, 129, 164, 1.0));

      canvas.clear(Skia.Color(255, 255, 255, 1.0));

      canvas.drawPath(path, paint);
      // Currently does not work.
      canvas.drawText('Some Text', 10, 10, textPaint);
      canvas.flush();
      dpe.delete();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);

    // A client would need to delete this if it didn't go on for ever.
    //path.delete();
    //paint.delete();
  }

  function PathExample(Skia) {
    const surface = Skia.getWebGLSurface('paths');
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = Skia.currentContext();

    const canvas = surface.getCanvas();

    function drawFrame() {
      Skia.setCurrentContext(context);
      const paint = new Skia.SkPaint();
      paint.setStrokeWidth(1.0);
      paint.setAntiAlias(true);
      paint.setColor(Skia.Color(0, 0, 0, 1.0));
      paint.setStyle(Skia.PaintStyle.STROKE);

      const path = new Skia.SkPath();
      path.moveTo(20, 5);
      path.lineTo(30, 20);
      path.lineTo(40, 10);
      path.lineTo(50, 20);
      path.lineTo(60, 0);
      path.lineTo(20, 5);

      path.moveTo(20, 80);
      path.cubicTo(90, 10, 160, 150, 190, 10);

      path.moveTo(36, 148);
      path.quadTo(66, 188, 120, 136);
      path.lineTo(36, 148);

      path.moveTo(150, 180);
      path.arcTo(150, 100, 50, 200, 20);
      path.lineTo(160, 160);

      path.moveTo(20, 120);
      path.lineTo(20, 120);

      canvas.drawPath(path, paint);

      canvas.flush();

      path.delete();
      paint.delete();
      // Intentionally just draw frame once
    }
    window.requestAnimationFrame(drawFrame);
  }

  function starPath(Skia) {
    const R = 115.2;
    const C = 128.0;
    let p = new Skia.SkPath();
    p.moveTo(C + R, C);
    for (let i = 1; i < 8; i++) {
      let a = 2.6927937 * i;
      p.lineTo(C + R * Math.cos(a), C + R * Math.sin(a));
    }
    return p;
  }

  function SkottieExample(Skia, id, jsonStr) {
    if (!Skia || !jsonStr) {
      return;
    }
    const surface = Skia.getWebGLSurface(id);
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = Skia.currentContext();
    const canvas = surface.getCanvas();
    const animation = Skia.MakeAnimation(jsonStr);

    let i = 0;
    function drawFrame() {
      Skia.setCurrentContext(context);
      animation.seek(i);
      i += 1/300.0;
      if (i > 1.0) {
        i = 0;
      }
      animation.render(canvas);
      canvas.flush();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);

    //animation.delete();
  }
  }
  document.head.appendChild(s);
})();
</script>

Download from npm
-----------------

This isn't quite ready for prime time yet.

Check back soon.