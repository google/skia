---
title: 'CanvasKit - Skia + WebAssembly'
linkTitle: 'CanvasKit - Skia + WebAssembly'

weight: 20
---

Skia now offers a WebAssembly build for easy deployment of our graphics APIs on
the web.

CanvasKit provides a playground for testing new Canvas and SVG platform APIs,
enabling fast-paced development on the web platform. It can also be used as a
deployment mechanism for custom web apps requiring cutting-edge features, like
Skia's [Lottie animation](https://skia.org/docs/user/modules/skottie) support.

## Features

- WebGL context encapsulated as an SkSurface, allowing for direct drawing to an
  HTML canvas
- Core set of Skia canvas/paint/path/text APIs available, see bindings
- Draws to a hardware-accelerated backend
- Security tested with Skia's fuzzers

## Samples

<style>
  #demo canvas {
    border: 1px dashed #AAA;
    margin: 2px;
  }

  #patheffect, #ink, #shaping, #shader1 {
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
  <h3>Paragraph shaping, custom shaders, and perspective transformation</h3>
  <figure>
    <canvas id=shaping width=500 height=500></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/48c67bde53f66a2f1e578e14b88a85bd062fdcf80c143c5eb92071233d4d86ae"
          target=_blank rel=noopener>
        SkParagraph JSFiddle</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=shader1 width=512 height=512></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/b9a8d33dc9853e491d5e464bc3b212560d9c546c44d71f00c9db15180ab6d5b8"
          target=_blank rel=noopener>
        Shader JSFiddle</a>
    </figcaption>
  </figure>

  <h3>Play back bodymovin lottie files with skottie (click for fiddles)</h3>
  <a href="https://jsfiddle.skia.org/canvaskit/6f4540a485ecbb8f0663b5ab3e04d9f3626a45234595d65f1b87942b90678aff"
     target=_blank rel=noopener>
    <canvas id=sk_legos width=300 height=300></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/2fdbd163e5a4ec55e38e84b6c3e069738d3b12fd858362265192109917f9dd2c"
     target=_blank rel=noopener>
    <canvas id=sk_drinks width=500 height=500></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/73624637ee6d96a8ce8ff8d523b90acdae3ec75b3fe16ddf3040990544c870ec"
     target=_blank rel=noopener>
    <canvas id=sk_party width=500 height=500></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/d63a544b87ccbe6bf8f15c772355d1c6dbc5eafc355bcd27457eca41da843cb5"
     target=_blank rel=noopener>
    <canvas id=sk_onboarding width=500 height=500></canvas>
  </a>

  <h3>Go beyond the HTML Canvas2D</h3>
  <figure>
    <canvas id=patheffect width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/97d4b9ce527a7ffb0f4bed77d9b7f01f889c8dbe68ac053d56739a5122c65b53"
          target=_blank rel=noopener>
        Star JSFiddle</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=ink width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/07c83d37d8115413442fda2c8b7f1bc56823d1c597473970638480e95cba42ee"
          target=_blank rel=noopener>
        Ink JSFiddle</a>
    </figcaption>
  </figure>

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
    locate_file = 'https://particles.skia.org/dist/';
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
  let fullBounds = [0, 0, 500, 500];
  const ckLoaded = CanvasKitInit({
    locateFile: (file) => locate_file + file,
  });

  ckLoaded.then((CK) => {
    CanvasKit = CK;
    DrawingExample(CanvasKit);
    InkExample(CanvasKit);
    ShapingExample(CanvasKit);
     // Set bounds to fix the 4:3 resolution of the legos
    SkottieExample(CanvasKit, 'sk_legos', legoJSON, [-183, -100, 483, 400]);
    // Re-size to fit
    SkottieExample(CanvasKit, 'sk_drinks', drinksJSON, fullBounds);
    SkottieExample(CanvasKit, 'sk_party', confettiJSON, fullBounds);
    SkottieExample(CanvasKit, 'sk_onboarding', onboardingJSON, fullBounds);
    ShaderExample1(CanvasKit);
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/lego_loader.json').then((resp) => {
    resp.text().then((str) => {
      legoJSON = str;
      SkottieExample(CanvasKit, 'sk_legos', legoJSON, [-183, -100, 483, 400]);
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

    const paint = new CanvasKit.Paint();

    const textPaint = new CanvasKit.Paint();
    textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
    textPaint.setAntiAlias(true);

    const textFont = new CanvasKit.Font(null, 30);

    let i = 0;

    let X = 200;
    let Y = 200;

    function drawFrame() {
      const path = starPath(CanvasKit, X, Y);
      CanvasKit.setCurrentContext(context);
      const dpe = CanvasKit.PathEffect.MakeDash([15, 5, 5, 10], i/5);
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

    let paint = new CanvasKit.Paint();
    paint.setAntiAlias(true);
    paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    paint.setStrokeWidth(4.0);
    // This effect smooths out the drawn lines a bit.
    paint.setPathEffect(CanvasKit.PathEffect.MakeCorner(50));

    // Draw I N K
    let path = new CanvasKit.Path();
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
        path = new CanvasKit.Path();
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

    const font = new CanvasKit.Font(null, 18);
    const fontPaint = new CanvasKit.Paint();
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
        const fontMgr = CanvasKit.FontMgr.FromData([robotoData, emojiData]);

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
    let p = new CanvasKit.Path();
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
    const paint = new CanvasKit.Paint();

    const prog = `
uniform float rad_scale;
uniform float2 in_center;
uniform float4 in_colors0;
uniform float4 in_colors1;

half4 main(float2 p) {
    float2 pp = p - in_center;
    float radius = sqrt(dot(pp, pp));
    radius = sqrt(radius);
    float angle = atan(pp.y / pp.x);
    float t = (angle + 3.1415926/2) / (3.1415926);
    t += radius * rad_scale;
    t = fract(t);
    return half4(mix(in_colors0, in_colors1, t));
}
`;

    // If there are multiple contexts on the screen, we need to make sure
    // we switch to this one before we draw.
    const context = CanvasKit.currentContext();
    const fact = CanvasKit.RuntimeEffect.Make(prog);
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

## Test server

Test your code on our [CanvasKit Fiddle](https://jsfiddle.skia.org/canvaskit)

## Download

Get [CanvasKit on NPM](https://www.npmjs.com/package/canvaskit-wasm).
Documentation and Typescript definitions are available in the `types/` subfolder
of the npm package or from the
[Skia repo](https://github.com/google/skia/tree/master/modules/canvaskit/npm_build/types).

Check out the [quickstart guide](../quickstart) as well.
