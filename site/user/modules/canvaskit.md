CanvasKit - Skia + WebAssembly
==============================

Skia now offers a WebAssembly build for easy deployment of our graphics APIs on
the web.

CanvasKit provides a playground for testing new Canvas and SVG platform APIs,
enabling fast-paced development on the web platform.
It can also be used as a deployment mechanism for custom web apps requiring
cutting-edge features, like Skia's [Lottie
animation](https://skia.org/user/modules/skottie) support.  


Features
--------

  - WebGL context encapsulated as an SkSurface, allowing for direct drawing to
    an HTML canvas
  - Core set of Skia canvas/paint/path/text APIs available, see bindings
  - Draws to a hardware-accelerated backend
  - Security tested with Skia's fuzzers

Samples
-------

<style>
  #demo canvas {
    border: 1px dashed #AAA;
  }

  #patheffect {
    width: 400px
    height: 400px;
  }

  #sk_drinks,#sk_party {
    width: 300px;
    height: 300px;
  }

  #sk_legos {
    width: 300px;
    height: 300px;
  }
</style>

<div id=demo>
  <h3>An Interactive Path (try mousing over)</h3>
  <canvas id=patheffect width=400 height=400></canvas>

  <h3>Skottie</h3>
  <canvas id=sk_legos width=600 height=600></canvas>
  <canvas id=sk_drinks width=500 height=500></canvas>
  <canvas id=sk_party width=800 height=800></canvas>
</div>

<script type="text/javascript" charset="utf-8">
(function() {
  // Tries to load the WASM version if supported, shows error otherwise
  let s = document.createElement('script');
  var locate_file = '';
  if (window.WebAssembly && typeof window.WebAssembly.compile === 'function') {
    console.log('WebAssembly is supported!');
    locate_file = 'https://storage.googleapis.com/skia-cdn/canvaskit-wasm/0.0.1/bin/';
  } else {
    console.log('WebAssembly is not supported (yet) on this browser.');
    document.getElementById('demo').innerHTML = "<div>WASM not supported by your browser. Try a recent version of Chrome, Firefox, Edge, or Safari.</div>";
    return;
  }
  s.src = locate_file + 'skia.js';
  s.onload = () => {
  var CanvasKit = null;
  var legoJSON = null;
  var drinksJSON = null;
  var confettiJSON = null;
  CanvasKitInit({
    locateFile: (file) => locate_file + file,
  }).then((CK) => {
    CanvasKit = CK;
    DrawingExample(CanvasKit);
    SkottieExample(CanvasKit, 'sk_legos', legoJSON);
    SkottieExample(CanvasKit, 'sk_drinks', drinksJSON);
    SkottieExample(CanvasKit, 'sk_party', confettiJSON);
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/lego_loader.json').then((resp) => {
    resp.text().then((str) => {
      legoJSON = str;
      SkottieExample(CanvasKit, 'sk_legos', legoJSON);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/drinks.json').then((resp) => {
    resp.text().then((str) => {
      drinksJSON = str;
      SkottieExample(CanvasKit, 'sk_drinks', drinksJSON);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/confetti.json').then((resp) => {
    resp.text().then((str) => {
      confettiJSON = str;
      SkottieExample(CanvasKit, 'sk_party', confettiJSON);
    });
  });

  function DrawingExample(CanvasKit) {
    const surface = CanvasKit.getWebGLSurface('patheffect');
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();

    const canvas = surface.getCanvas();

    const paint = new CanvasKit.SkPaint();

    const textPaint = new CanvasKit.SkPaint();
    textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
    textPaint.setStrokeWidth(2);
    textPaint.setTextSize(60);

    let i = 0;

    let X = 128;
    let Y = 128;

    function drawFrame() {
      const path = starPath(CanvasKit, X, Y);
      CanvasKit.setCurrentContext(context);
      const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], i/5);
      i++;

      paint.setPathEffect(dpe);
      paint.setStyle(CanvasKit.PaintStyle.STROKE);
      paint.setStrokeWidth(5.0 + -3 * Math.cos(i/30));
      paint.setAntiAlias(true);
      paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

      canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

      canvas.drawPath(path, paint);
      // Currently does not work.
      canvas.drawText('Some Text', 10, 10, textPaint);
      canvas.flush();
      dpe.delete();
      path.delete();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);

    // Make animation interactive
    document.getElementById('patheffect').addEventListener('mousemove', (e) => {
      X = e.offsetX;
      Y = e.offsetY;
    });

    // A client would need to delete this if it didn't go on for ever.
    //paint.delete();
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

  function SkottieExample(CanvasKit, id, jsonStr) {
    if (!CanvasKit || !jsonStr) {
      return;
    }
    const surface = CanvasKit.getWebGLSurface(id);
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();
    const canvas = surface.getCanvas();
    const animation = CanvasKit.MakeAnimation(jsonStr);

    let i = 0;
    function drawFrame() {
      CanvasKit.setCurrentContext(context);
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

Lottie files courtesy of the lottiefiles.com community:
[Lego Loader](https://www.lottiefiles.com/410-lego-loader), [I'm
thirsty](https://www.lottiefiles.com/77-im-thirsty),
[Confetti](https://www.lottiefiles.com/1370-confetti)


Test server
-----------
Test your code on our [CanvasKit Fiddle](https://jsfiddle.skia.org/canvaskit)

Download
--------
Work is underway on an npm download. Check back soon.
