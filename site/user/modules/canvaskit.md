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

  #patheffect, #ink, #shaping {
    width: 400px;
    height: 400px;
  }

  #sk_legos, #sk_drinks, #sk_party, #sk_onboarding, #shader1 {
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
  <h3>Go beyond the HTML Canvas2D</h3>
  <figure>
    <canvas id=patheffect width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/43b38b83ca77dabe47f18f31cafe83f3018b3a24e569db27fe711c70bc3f7d62"
          target=_blank rel=noopener>
        Star JSFiddle</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=ink width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/ad0a5454db3ac757684ed2fa8ce9f1f0175f1c043d2cbe33597d81481cdb4baa"
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

  <h3>SkParagraph (using ICU and Harfbuzz)</h3>
  <figure>
    <canvas id=shaping width=500 height=500></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/56cb197c724dfdfad0c3d8133d4fcab587e4c4e7f31576e62c17251637d3745c"
          target=_blank rel=noopener>
        SkParagraph JSFiddle</a>
    </figcaption>
  </figure>

  <h3>SKSL for writing custom shaders</h3>
  <a href="https://jsfiddle.skia.org/canvaskit/33ff9bed883cd5742b4770169da0b36fb0cbc18fd395ddd9563213e178362d30"
    target=_blank rel=noopener>
    <canvas id=shader1 width=512 height=512></canvas>
  </a>

</div>

<script type="text/javascript" charset="utf-8">
(function() {
  // Tries to load the WASM version if supported, shows error otherwise
  let s = document.createElement('script');
  let locate_file = '';
  // Hey, if you are looking at this code for an example of how to do it yourself, please use
  // an actual CDN, such as https://unpkg.com/canvaskit-wasm - it will have better reliability
  // and niceties like brotli compression.
  if (window.WebAssembly && typeof window.WebAssembly.compile === 'function') {
    console.log('WebAssembly is supported!');
    locate_file = 'https://particles.skia.org/static/';
  } else {
    console.log('WebAssembly is not supported (yet) on this browser.');
    document.getElementById('demo').innerHTML = "<div>WASM not supported by your browser. Try a recent version of Chrome, Firefox, Edge, or Safari.</div>";
    return;
  }
  s.src = locate_file + 'canvaskit.js';
  s.onload = () => {
  let CanvasKit = null;
  let legoJSON = null;
  let drinksJSON = null;
  let confettiJSON = null;
  let onboardingJSON = null;
  let fullBounds = {fLeft: 0, fTop: 0, fRight: 500, fBottom: 500};
  CanvasKitInit({
    locateFile: (file) => locate_file + file,
  }).ready().then((CK) => {
    CanvasKit = CK;
    DrawingExample(CanvasKit);
    InkExample(CanvasKit);
    ShapingExample(CanvasKit);
     // Set bounds to fix the 4:3 resolution of the legos
    SkottieExample(CanvasKit, 'sk_legos', legoJSON, {fLeft: -50, fTop: 0, fRight: 350, fBottom: 300});
    // Re-size to fit
    SkottieExample(CanvasKit, 'sk_drinks', drinksJSON, fullBounds);
    SkottieExample(CanvasKit, 'sk_party', confettiJSON, fullBounds);
    SkottieExample(CanvasKit, 'sk_onboarding', onboardingJSON, fullBounds);
    ShaderExample1(CanvasKit);
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
    textPaint.setAntiAlias(true);

    const textFont = new CanvasKit.SkFont(null, 30);

    let i = 0;

    let X = 200;
    let Y = 200;

    function drawFrame() {
      const path = starPath(CanvasKit, X, Y);
      CanvasKit.setCurrentContext(context);
      const dpe = CanvasKit.SkPathEffect.MakeDash([15, 5, 5, 10], i/5);
      i++;

      paint.setPathEffect(dpe);
      paint.setStyle(CanvasKit.PaintStyle.Stroke);
      paint.setStrokeWidth(5.0 + -3 * Math.cos(i/30));
      paint.setAntiAlias(true);
      paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

      canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

      canvas.drawPath(path, paint);
      canvas.drawText('Try Clicking!', 10, 380, textPaint, textFont);
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

    // A client would need to delete this if it didn't go on forever.
    // font.delete();
    // paint.delete();
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
    paint.setPathEffect(CanvasKit.SkPathEffect.MakeCorner(50));

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

  function ShapingExample(CanvasKit) {
    const surface = CanvasKit.MakeCanvasSurface('shaping');
    if (!surface) {
      console.log('Could not make surface');
      return;
    }
    let robotoData = null;
    fetch('https://storage.googleapis.com/skia-cdn/google-web-fonts/Roboto-Regular.ttf').then((resp) => {
      resp.arrayBuffer().then((buffer) => {
        robotoData = buffer;
        requestAnimationFrame(drawFrame);
      });
    });

    let emojiData = null;
    fetch('https://storage.googleapis.com/skia-cdn/misc/NotoColorEmoji.ttf').then((resp) => {
      resp.arrayBuffer().then((buffer) => {
        emojiData = buffer;
        requestAnimationFrame(drawFrame);
      });
    });

    const skcanvas = surface.getCanvas();

    const font = new CanvasKit.SkFont(null, 18);
    const fontPaint = new CanvasKit.SkPaint();
    fontPaint.setStyle(CanvasKit.PaintStyle.Fill);
    fontPaint.setAntiAlias(true);

    skcanvas.drawText(`Fetching Font data...`, 5, 450, fontPaint, font);
    surface.flush();

    const context = CanvasKit.currentContext();

    let paragraph = null;
    let X = 10;
    let Y = 10;
    const str = 'The quick brown fox 🦊 ate a zesty hamburgerfons 🍔.\nThe 👩‍👩‍👧‍👧 laughed.';

    function drawFrame() {
      if (robotoData && emojiData && !paragraph) {
        const fontMgr = CanvasKit.SkFontMgr.FromData([robotoData, emojiData]);

        const paraStyle = new CanvasKit.ParagraphStyle({
          textStyle: {
            color: CanvasKit.BLACK,
            fontFamilies: ['Roboto', 'Noto Color Emoji'],
            fontSize: 50,
          },
          textAlign: CanvasKit.TextAlign.Left,
          maxLines: 7,
          ellipsis: '...',
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText(str);
        paragraph = builder.build();
      }
      if (!paragraph) {
        requestAnimationFrame(drawFrame);
        return;
      }
      CanvasKit.setCurrentContext(context);
      skcanvas.clear(CanvasKit.WHITE);

      const wrapTo = 350 + 150 * Math.sin(Date.now() / 2000);
      paragraph.layout(wrapTo);
      skcanvas.drawParagraph(paragraph, 0, 0);
      skcanvas.drawLine(wrapTo, 0, wrapTo, 400, fontPaint);

      let posA = paragraph.getGlyphPositionAtCoordinate(X, Y);
      const cp = str.codePointAt(posA.pos);
      if (cp) {
        const glyph = String.fromCodePoint(cp);
        skcanvas.drawText(`At (${X.toFixed(2)}, ${Y.toFixed(2)}) glyph is '${glyph}'`, 5, 450, fontPaint, font);
      }

      surface.flush();
      requestAnimationFrame(drawFrame);
    }

    // Make animation interactive
    let interact = (e) => {
      // multiply by 4/5 to account for the difference in the canvas width and the CSS width.
      // The 10 accounts for where the mouse actually is compared to where it is drawn.
      X = (e.offsetX * 4/5) - 10;
      Y = e.offsetY * 4/5;
    };
    document.getElementById('shaping').addEventListener('pointermove', interact);
    document.getElementById('shaping').addEventListener('pointerdown', interact);
    document.getElementById('shaping').addEventListener('lostpointercapture', interact);
    document.getElementById('shaping').addEventListener('pointerup', interact);
    preventScrolling(document.getElementById('shaping'));
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

  function ShaderExample1(CanvasKit) {
    if (!CanvasKit) {
      return;
    }
    const surface = CanvasKit.MakeCanvasSurface('shader1');
    if (!surface) {
      throw 'Could not make surface';
    }
    const skcanvas = surface.getCanvas();
    const paint = new CanvasKit.SkPaint();

    const prog = `
uniform float rad_scale;
uniform float2 in_center;
uniform float4 in_colors0;
uniform float4 in_colors1;

void main(float2 p, inout half4 color) {
    float2 pp = p - in_center;
    float radius = sqrt(dot(pp, pp));
    radius = sqrt(radius);
    float angle = atan(pp.y / pp.x);
    float t = (angle + 3.1415926/2) / (3.1415926);
    t += radius * rad_scale;
    t = fract(t);
    color = half4(mix(in_colors0, in_colors1, t));
}
`;

    // If there are multiple contexts on the screen, we need to make sure
    // we switch to this one before we draw.
    const context = CanvasKit.currentContext();
    const fact = CanvasKit.SkRuntimeEffect.Make(prog);
    function drawFrame() {
      CanvasKit.setCurrentContext(context);
      skcanvas.clear(CanvasKit.WHITE);
      const shader = fact.makeShader([
        Math.sin(Date.now() / 2000) / 5,
        256, 256,
        1, 0, 0, 1,
        0, 1, 0, 1],
        true/*=opaque*/);

      paint.setShader(shader);
      skcanvas.drawRect(CanvasKit.LTRBRect(0, 0, 512, 512), paint);
      surface.flush();
      requestAnimationFrame(drawFrame);
      shader.delete();
    }
    requestAnimationFrame(drawFrame);
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

Test server
-----------
Test your code on our [CanvasKit Fiddle](https://jsfiddle.skia.org/canvaskit)

Download
--------
Get [CanvasKit on NPM](https://www.npmjs.com/package/canvaskit-wasm)
