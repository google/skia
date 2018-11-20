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
    margin: 2px;
  }

  #patheffect, #ink {
    width: 400px;
    height: 400px;
  }

  #sk_legos, #sk_drinks, #sk_party, #sk_onboarding {
    width: 300px;
    height: 300px;
  }

  figure {
    display: inline-block;
    margin: 0;
  }

  figcaption > a {
    margin: 2px 10px;
  }

</style>

<div id=demo>
  <h3>An Interactive Path</h3>
  <figure>
    <canvas id=patheffect width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/bb98ad306a0c826b6dc8e8b8f709fca75fee95210c529679def945f0be7ed90c"
          target=_blank rel=noopener>
        Star JSFiddle</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=ink width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/43475699d6d7d3d7dad1004c29f84015752a6a6dee2bb90f2e891b53e31d45cc"
          target=_blank rel=noopener>
        Ink JSFiddle</a>
    </figcaption>
  </figure>

  <h3>Skottie (click for fiddles)</h3>
  <a href="https://jsfiddle.skia.org/canvaskit/092690b273b41076d2f00f0d43d004893d6bb9992c387c0385efa8e6f6bc83d7"
     target=_blank rel=noopener>
    <canvas id=sk_legos width=300 height=300></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/e7ac983d9859f89aff1b6d385190919202c2eb53d028a79992892cacceffd209"
     target=_blank rel=noopener>
    <canvas id=sk_drinks width=500 height=500></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/0e06547181759731e7369d3e3613222a0826692f48c41b16504ed68d671583e1"
     target=_blank rel=noopener>
    <canvas id=sk_party width=500 height=500></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/be3fc1c5c351e7f43cc2840033f80b44feb3475925264808f321bb9e2a21174a"
     target=_blank rel=noopener>
    <canvas id=sk_onboarding width=500 height=500></canvas>
  </a>

  <h3>Nima (click for fiddle)</h3>
  <a href="https://jsfiddle.skia.org/canvaskit/8f72aa124b91d28c77ef38c849ad7b3b0a4c207010ecca6dccba2b273329895c"
     target=_blank rel=noopener>
    <canvas id=nima_robot width=300 height=300></canvas>
  </a>

</div>

<script type="text/javascript" charset="utf-8">
(function() {
  // Tries to load the WASM version if supported, shows error otherwise
  let s = document.createElement('script');
  var locate_file = '';
  if (window.WebAssembly && typeof window.WebAssembly.compile === 'function') {
    console.log('WebAssembly is supported!');
    locate_file = 'https://storage.googleapis.com/skia-cdn/canvaskit-wasm/0.2.1/bin/';
  } else {
    console.log('WebAssembly is not supported (yet) on this browser.');
    document.getElementById('demo').innerHTML = "<div>WASM not supported by your browser. Try a recent version of Chrome, Firefox, Edge, or Safari.</div>";
    return;
  }
  s.src = locate_file + 'canvaskit.js';
  s.onload = () => {
  var CanvasKit = null;
  var legoJSON = null;
  var drinksJSON = null;
  var confettiJSON = null;
  var onboardingJSON = null;
  var nimaFile = null;
  var nimaTexture = null;
  var fullBounds = {fLeft: 0, fTop: 0, fRight: 500, fBottom: 500};
  CanvasKitInit({
    locateFile: (file) => locate_file + file,
  }).then((CK) => {
    CK.initFonts();
    CanvasKit = CK;
    DrawingExample(CanvasKit);
    InkExample(CanvasKit);
     // Set bounds to fix the 4:3 resolution of the legos
    SkottieExample(CanvasKit, 'sk_legos', legoJSON, {fLeft: -50, fTop: 0, fRight: 350, fBottom: 300});
    // Re-size to fit
    SkottieExample(CanvasKit, 'sk_drinks', drinksJSON, fullBounds);
    SkottieExample(CanvasKit, 'sk_party', confettiJSON, fullBounds);
    SkottieExample(CanvasKit, 'sk_onboarding', onboardingJSON, fullBounds);

    NimaExample(CanvasKit, nimaFile, nimaTexture);
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/lego_loader.json').then((resp) => {
    resp.text().then((str) => {
      legoJSON = str;
      SkottieExample(CanvasKit, 'sk_legos', legoJSON, {fLeft: -50, fTop: 0, fRight: 350, fBottom: 300});
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/drinks.json').then((resp) => {
    resp.text().then((str) => {
      drinksJSON = str;
      SkottieExample(CanvasKit, 'sk_drinks', drinksJSON, fullBounds);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/confetti.json').then((resp) => {
    resp.text().then((str) => {
      confettiJSON = str;
      SkottieExample(CanvasKit, 'sk_party', confettiJSON, fullBounds);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/onboarding.json').then((resp) => {
    resp.text().then((str) => {
      onboardingJSON = str;
      SkottieExample(CanvasKit, 'sk_onboarding', onboardingJSON, fullBounds);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/robot.nima').then((resp) => {
    resp.blob().then((blob) => {
      let reader = new FileReader();
      reader.addEventListener('loadend', function() {
          nimaFile = reader.result;
          NimaExample(CanvasKit, nimaFile, nimaTexture);
      });
      reader.readAsArrayBuffer(blob);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/robot.nima.png').then((resp) => {
    resp.blob().then((blob) => {
      let reader = new FileReader();
      reader.addEventListener('loadend', function() {
          nimaTexture = reader.result;
          NimaExample(CanvasKit, nimaFile, nimaTexture);
      });
      reader.readAsArrayBuffer(blob);
    });
  });

  function preventScrolling(canvas) {
    canvas.addEventListener('touchmove', (e) => {
      // Prevents touch events in the canvas from scrolling the canvas.
      e.preventDefault();
      e.stopPropagation();
    });
  }

  function DrawingExample(CanvasKit) {
    const surface = CanvasKit.MakeCanvasSurface('patheffect');
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();

    const canvas = surface.getCanvas();

    const paint = new CanvasKit.SkPaint();

    const textPaint = new CanvasKit.SkPaint();
    textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
    textPaint.setTextSize(30);
    textPaint.setAntiAlias(true);

    let i = 0;

    let X = 200;
    let Y = 200;

    function drawFrame() {
      const path = starPath(CanvasKit, X, Y);
      CanvasKit.setCurrentContext(context);
      const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], i/5);
      i++;

      paint.setPathEffect(dpe);
      paint.setStyle(CanvasKit.PaintStyle.Stroke);
      paint.setStrokeWidth(5.0 + -3 * Math.cos(i/30));
      paint.setAntiAlias(true);
      paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

      canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

      canvas.drawPath(path, paint);
      canvas.drawText('Try Clicking!', 10, 380, textPaint);
      canvas.flush();
      dpe.delete();
      path.delete();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);

    // Make animation interactive
    let interact = (e) => {
      if (!e.buttons) {
        return;
      }
      X = e.offsetX;
      Y = e.offsetY;
    };
    document.getElementById('patheffect').addEventListener('pointermove', interact);
    document.getElementById('patheffect').addEventListener('pointerdown', interact);
    preventScrolling(document.getElementById('patheffect'));

    // A client would need to delete this if it didn't go on for ever.
    //paint.delete();
  }

  function InkExample(CanvasKit) {
    const surface = CanvasKit.MakeCanvasSurface('ink');
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();

    const canvas = surface.getCanvas();

    let paint = new CanvasKit.SkPaint();
    paint.setAntiAlias(true);
    paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    paint.setStrokeWidth(4.0);
    // This effect smooths out the drawn lines a bit.
    paint.setPathEffect(CanvasKit.MakeSkCornerPathEffect(50));

    // Draw I N K
    let path = new CanvasKit.SkPath();
    path.moveTo(80, 30);
    path.lineTo(80, 80);

    path.moveTo(100, 80);
    path.lineTo(100, 15);
    path.lineTo(130, 95);
    path.lineTo(130, 30);

    path.moveTo(150, 30);
    path.lineTo(150, 80);
    path.moveTo(170, 30);
    path.lineTo(150, 55);
    path.lineTo(170, 80);

    let paths = [path];
    let paints = [paint];

    function drawFrame() {
      CanvasKit.setCurrentContext(context);

      for (let i = 0; i < paints.length && i < paths.length; i++) {
        canvas.drawPath(paths[i], paints[i]);
      }
      canvas.flush();

      window.requestAnimationFrame(drawFrame);
    }

    let hold = false;
    let interact = (e) => {
      let type = e.type;
      if (type === 'lostpointercapture' || type === 'pointerup' || !e.pressure ) {
        hold = false;
        return;
      }
      if (hold) {
        path.lineTo(e.offsetX, e.offsetY);
      } else {
        paint = paint.copy();
        paint.setColor(CanvasKit.Color(Math.random() * 255, Math.random() * 255, Math.random() * 255, Math.random() + .2));
        paints.push(paint);
        path = new CanvasKit.SkPath();
        paths.push(path);
        path.moveTo(e.offsetX, e.offsetY);
      }
      hold = true;
    };
    document.getElementById('ink').addEventListener('pointermove', interact);
    document.getElementById('ink').addEventListener('pointerdown', interact);
    document.getElementById('ink').addEventListener('lostpointercapture', interact);
    document.getElementById('ink').addEventListener('pointerup', interact);
    preventScrolling(document.getElementById('ink'));
    window.requestAnimationFrame(drawFrame);
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

  function SkottieExample(CanvasKit, id, jsonStr, bounds) {
    if (!CanvasKit || !jsonStr) {
      return;
    }
    const animation = CanvasKit.MakeAnimation(jsonStr);
    const duration = animation.duration() * 1000;
    const size = animation.size();
    let c = document.getElementById(id);
    bounds = bounds || {fLeft: 0, fTop: 0, fRight: size.w, fBottom: size.h};

    const surface = CanvasKit.MakeCanvasSurface(id);
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();
    const canvas = surface.getCanvas();

    let firstFrame = new Date().getTime();

    function drawFrame() {
      let now = new Date().getTime();
      let seek = ((now - firstFrame) / duration) % 1.0;
      CanvasKit.setCurrentContext(context);
      animation.seek(seek);

      animation.render(canvas, bounds);
      canvas.flush();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);
    //animation.delete();
  }

  function NimaExample(CanvasKit, nimaFile, nimaTexture) {
    if (!CanvasKit || !nimaFile || !nimaTexture) {
      return;
    }
    const animation = CanvasKit.MakeNimaActor(nimaFile, nimaTexture);
    if (!animation) {
      console.error('could not make animation');
      return;
    }

    const surface = CanvasKit.MakeCanvasSurface('nima_robot');
    if (!surface) {
      console.error('Could not make surface');
      return;
    }

    const context = CanvasKit.currentContext();
    const canvas = surface.getCanvas();
    canvas.translate(125, 275);
    canvas.scale(0.4, -0.4);

    let firstFrame = Date.now();
    animation.setAnimationByName('attack');

    function drawFrame() {
      let seek = ((Date.now() - firstFrame) / 1000.0);
      CanvasKit.setCurrentContext(context);
      canvas.clear(CanvasKit.Color(255, 255, 255, 0.0));
      animation.seek(seek);
      animation.render(canvas);
      surface.flush();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);
  }
  }
  document.head.appendChild(s);
})();
</script>

Lottie files courtesy of the lottiefiles.com community:
[Lego Loader](https://www.lottiefiles.com/410-lego-loader),
[I'm thirsty](https://www.lottiefiles.com/77-im-thirsty),
[Confetti](https://www.lottiefiles.com/1370-confetti),
[Onboarding](https://www.lottiefiles.com/1134-onboarding-1)

Nima files courtesy of 2dimensions.com:
[Robot](https://www.2dimensions.com/s/281-robot)


Test server
-----------
Test your code on our [CanvasKit Fiddle](https://jsfiddle.skia.org/canvaskit)

Download
--------
Work is underway on an npm download. Check back soon.
