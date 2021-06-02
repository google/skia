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

  #patheffect, #ink, #shaping, #shader1, #camera3d {
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
      <a href="https://jsfiddle.skia.org/canvaskit/6a5c211a8cb4a7752297674b3533f7e1bbc2a78dd37f117c29a77bcc68411f31"
          target=_blank rel=noopener>
        SkParagraph JSFiddle</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=shader1 width=512 height=512></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/b382d3b660c4f314eb6a6eae9c0f1e0aadc95c0a2747b707e0dbe3f65a8b0a14"
          target=_blank rel=noopener>
        Shader JSFiddle</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=camera3d width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/a5f9e976f1af65ef13bd978a5e265bdcb92110f5e64699fba5e8871c54be22b6"
          target=_blank rel=noopener>
        3D Cube JSFiddle</a>
    </figcaption>
  </figure>

  <h3>Play back bodymovin lottie files with skottie (click for fiddles)</h3>
  <a href="https://jsfiddle.skia.org/canvaskit/cb0b72eadb45f7e75b4015db7251e9da2cc202a2ce1cbec8eb2e453d83a619a6"
     target=_blank rel=noopener>
    <canvas id=sk_legos width=300 height=300></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/e77274c30d63645d3bb82fd366991e27c1e1c3df39def04e999b4fcce9f425a2"
     target=_blank rel=noopener>
    <canvas id=sk_drinks width=500 height=500></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/e42700132d80efd3470b0f08334556028490ac08d1938210fa618504c6109c99"
     target=_blank rel=noopener>
    <canvas id=sk_party width=500 height=500></canvas>
  </a>
  <a href="https://jsfiddle.skia.org/canvaskit/987b1f99f4703f9f44dbfb2f43a5ed107672334f68d6262cd53ba44ed7a09236"
     target=_blank rel=noopener>
    <canvas id=sk_onboarding width=500 height=500></canvas>
  </a>

  <h3>Go beyond the HTML Canvas2D</h3>
  <figure>
    <canvas id=patheffect width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/3588b3b0a7cc93f36d9fa4f08b397c38971dcb1f80a36107f9ad93c051f2cb28"
          target=_blank rel=noopener>
        Star JSFiddle</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=ink width=400 height=400></canvas>
    <figcaption>
      <a href="https://jsfiddle.skia.org/canvaskit/bd42c174a0dcb2f65ff1f3c803397df14014d1e66b92185e9980dc631a49f258"
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

  const loadBrickTex = fetch('https://storage.googleapis.com/skia-cdn/misc/brickwork-texture.jpg').then((response) => response.arrayBuffer());
  const loadBrickBump = fetch('https://storage.googleapis.com/skia-cdn/misc/brickwork_normal-map.jpg').then((response) => response.arrayBuffer());
  Promise.all([ckLoaded, loadBrickTex, loadBrickBump]).then((results) => {Camera3D(...results)});

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
    const paint = new CanvasKit.Paint();

    const textPaint = new CanvasKit.Paint();
    textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
    textPaint.setAntiAlias(true);

    const textFont = new CanvasKit.Font(null, 30);

    let i = 0;

    let X = 200;
    let Y = 200;

    function drawFrame(canvas) {
      const path = starPath(CanvasKit, X, Y);
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
      dpe.delete();
      path.delete();
      surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);

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

    function drawFrame(canvas) {
      canvas.clear(CanvasKit.WHITE);
      for (let i = 0; i < paints.length && i < paths.length; i++) {
        canvas.drawPath(paths[i], paints[i]);
      }
      surface.requestAnimationFrame(drawFrame);
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
    surface.requestAnimationFrame(drawFrame);
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
      });
    });

    let emojiData = null;
    fetch('https://storage.googleapis.com/skia-cdn/misc/NotoColorEmoji.ttf').then((resp) => {
      resp.arrayBuffer().then((buffer) => {
        emojiData = buffer;
      });
    });

    const font = new CanvasKit.Font(null, 18);
    const fontPaint = new CanvasKit.Paint();
    fontPaint.setStyle(CanvasKit.PaintStyle.Fill);
    fontPaint.setAntiAlias(true);

    let paragraph = null;
    let X = 250;
    let Y = 250;
    const str = 'The quick brown fox 🦊 ate a zesty hamburgerfons 🍔.\nThe 👩‍👩‍👧‍👧 laughed.';

    function drawFrame(canvas) {
      surface.requestAnimationFrame(drawFrame);
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
        canvas.drawText(`Fetching Font data...`, 5, 450, fontPaint, font);
        return;
      }
      canvas.clear(CanvasKit.WHITE);

      let wrapTo = 350 + 150 * Math.sin(Date.now() / 2000);
      paragraph.layout(wrapTo);
      canvas.drawParagraph(paragraph, 0, 0);
      canvas.drawLine(wrapTo, 0, wrapTo, 400, fontPaint);

      const posA = paragraph.getGlyphPositionAtCoordinate(X, Y);
      const cp = str.codePointAt(posA.pos);
      if (cp) {
        const glyph = String.fromCodePoint(cp);
        canvas.drawText(`At (${X.toFixed(2)}, ${Y.toFixed(2)}) glyph is '${glyph}'`, 5, 450, fontPaint, font);
      }
    }

    surface.requestAnimationFrame(drawFrame);
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
    surface.requestAnimationFrame(drawFrame);
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
    let firstFrame = new Date().getTime();

    function drawFrame(canvas) {
      let now = new Date().getTime();
      let seek = ((now - firstFrame) / duration) % 1.0;

      animation.seek(seek);
      animation.render(canvas, bounds);

      surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);
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

    const fact = CanvasKit.RuntimeEffect.Make(prog);
    function drawFrame(canvas) {
      canvas.clear(CanvasKit.WHITE);
      const shader = fact.makeShader([
        Math.sin(Date.now() / 2000) / 5,
        256, 256,
        1, 0, 0, 1,
        0, 1, 0, 1],
        true/*=opaque*/);

      paint.setShader(shader);
      canvas.drawRect(CanvasKit.LTRBRect(0, 0, 512, 512), paint);
      shader.delete();
      surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);
  }

  function Camera3D(canvas, textureImgData, normalImgData) {
    const surface = CanvasKit.MakeCanvasSurface('camera3d');
    if (!surface) {
      console.error('Could not make surface');
      return;
    }

    const sizeX = document.getElementById('camera3d').width;
    const sizeY = document.getElementById('camera3d').height;

    let clickToWorld = CanvasKit.M44.identity();
    let worldToClick = CanvasKit.M44.identity();
    // rotation of the cube shown in the demo
    let rotation = CanvasKit.M44.identity();
    // temporary during a click and drag
    let clickRotation = CanvasKit.M44.identity();

    // A virtual sphere used for tumbling the object on screen.
    const vSphereCenter = [sizeX/2, sizeY/2];
    const vSphereRadius = Math.min(...vSphereCenter);

    // The rounded rect used for each face
    const margin = vSphereRadius / 20;
    const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(margin, margin,
      vSphereRadius - margin, vSphereRadius - margin), margin*2.5, margin*2.5);

    const camAngle = Math.PI / 12;
    const cam = {
      'eye'  : [0, 0, 1 / Math.tan(camAngle/2) - 1],
      'coa'  : [0, 0, 0],
      'up'   : [0, 1, 0],
      'near' : 0.05,
      'far'  : 4,
      'angle': camAngle,
    };

    let mouseDown = false;
    let clickDown = [0, 0]; // location of click down
    let lastMouse = [0, 0]; // last mouse location

    // keep spinning after mouse up. Also start spinning on load
    let axis = [0.4, 1, 1];
    let totalSpin = 0;
    let spinRate = 0.1;
    let lastRadians = 0;
    let spinning = setInterval(keepSpinning, 30);

    const imgscale = CanvasKit.Matrix.scaled(2, 2);
    const textureShader = CanvasKit.MakeImageFromEncoded(textureImgData).makeShaderCubic(
      CanvasKit.TileMode.Clamp, CanvasKit.TileMode.Clamp, 1/3, 1/3, imgscale);
    const normalShader = CanvasKit.MakeImageFromEncoded(normalImgData).makeShaderCubic(
      CanvasKit.TileMode.Clamp, CanvasKit.TileMode.Clamp, 1/3, 1/3, imgscale);
    const children = [textureShader, normalShader];

    const prog = `
      uniform shader color_map;
      uniform shader normal_map;

      uniform float3   lightPos;
      uniform float4x4 localToWorld;
      uniform float4x4 localToWorldAdjInv;

      float3 convert_normal_sample(half4 c) {
        float3 n = 2 * c.rgb - 1;
        n.y = -n.y;
        return n;
      }

      half4 main(float2 p) {
        float3 norm = convert_normal_sample(sample(normal_map, p));
        float3 plane_norm = normalize(localToWorldAdjInv * float4(norm, 0)).xyz;

        float3 plane_pos = (localToWorld * float4(p, 0, 1)).xyz;
        float3 light_dir = normalize(lightPos - plane_pos);

        float ambient = 0.2;
        float dp = dot(plane_norm, light_dir);
        float scale = min(ambient + max(dp, 0), 1);

        return sample(color_map, p) * half4(float4(scale, scale, scale, 1));
      }
`;

    const fact = CanvasKit.RuntimeEffect.Make(prog);

    // properties of light
    let lightLocation = [...vSphereCenter];
    let lightDistance = vSphereRadius;
    let lightIconRadius = 12;
    let draggingLight = false;

    function computeLightWorldPos() {
      return CanvasKit.Vector.add(CanvasKit.Vector.mulScalar([...vSphereCenter, 0], 0.5),
        CanvasKit.Vector.mulScalar(vSphereUnitV3(lightLocation), lightDistance));
    }

    let lightWorldPos = computeLightWorldPos();

    function drawLight(canvas) {
      const paint = new CanvasKit.Paint();
      paint.setAntiAlias(true);
      paint.setColor(CanvasKit.WHITE);
      canvas.drawCircle(...lightLocation, lightIconRadius + 2, paint);
      paint.setColor(CanvasKit.BLACK);
      canvas.drawCircle(...lightLocation, lightIconRadius, paint);
    }

    // Takes an x and y rotation in radians and a scale and returns a 4x4 matrix used to draw a
    // face of the cube in that orientation.
    function faceM44(rx, ry, scale) {
      return CanvasKit.M44.multiply(
        CanvasKit.M44.rotated([0,1,0], ry),
        CanvasKit.M44.rotated([1,0,0], rx),
        CanvasKit.M44.translated([0, 0, scale]));
    }

    const faceScale = vSphereRadius/2
    const faces = [
      {matrix: faceM44(         0,         0, faceScale ), color:CanvasKit.RED}, // front
      {matrix: faceM44(         0,   Math.PI, faceScale ), color:CanvasKit.GREEN}, // back

      {matrix: faceM44( Math.PI/2,         0, faceScale ), color:CanvasKit.BLUE}, // top
      {matrix: faceM44(-Math.PI/2,         0, faceScale ), color:CanvasKit.CYAN}, // bottom

      {matrix: faceM44(         0, Math.PI/2, faceScale ), color:CanvasKit.MAGENTA}, // left
      {matrix: faceM44(         0,-Math.PI/2, faceScale ), color:CanvasKit.YELLOW}, // right
    ];

    // Returns a component of the matrix m indicating whether it faces the camera.
    // If it's positive for one of the matrices representing the face of the cube,
    // that face is currently in front.
    function front(m) {
      // Is this invertible?
      var m2 = CanvasKit.M44.invert(m);
      if (m2 === null) {
        m2 = CanvasKit.M44.identity();
      }
      // look at the sign of the z-scale of the inverse of m.
      // that's the number in row 2, col 2.
      return m2[10]
    }

    function setClickToWorld(canvas, matrix) {
      const l2d = canvas.getLocalToDevice();
      worldToClick = CanvasKit.M44.multiply(CanvasKit.M44.mustInvert(matrix), l2d);
      clickToWorld = CanvasKit.M44.mustInvert(worldToClick);
    }

    function normalMatrix(m) {
      m[3]  = 0;
      m[7]  = 0;
      m[11] = 0;
      m[12] = 0;
      m[13] = 0;
      m[14] = 0;
      m[15] = 1;
      return CanvasKit.M44.transpose(CanvasKit.M44.mustInvert(m));
    }

    function drawCubeFace(canvas, m, color) {
      const trans = new CanvasKit.M44.translated([vSphereRadius/2, vSphereRadius/2, 0]);
      const localToWorld = new CanvasKit.M44.multiply(m, CanvasKit.M44.mustInvert(trans));
      canvas.concat(CanvasKit.M44.multiply(trans, localToWorld));
      const znormal = front(canvas.getLocalToDevice());
      if (znormal < 0) {
        return; // skip faces facing backwards
      }
      const uniforms = [...lightWorldPos, ...localToWorld, ...normalMatrix(localToWorld)];
      const paint = new CanvasKit.Paint();
      paint.setAntiAlias(true);
      const shader = fact.makeShaderWithChildren(uniforms, true /*=opaque*/, children);
      paint.setShader(shader);
      canvas.drawRRect(rr, paint);
    }

    function drawFrame(canvas) {
      const clickM = canvas.getLocalToDevice();
      canvas.save();
      canvas.translate(vSphereCenter[0] - vSphereRadius/2, vSphereCenter[1] - vSphereRadius/2);
      // pass surface dimensions as viewport size.
      canvas.concat(CanvasKit.M44.setupCamera(
        CanvasKit.LTRBRect(0, 0, vSphereRadius, vSphereRadius), vSphereRadius/2, cam));
      setClickToWorld(canvas, clickM);
      for (let f of faces) {
        const saveCount = canvas.getSaveCount();
        canvas.save();
        drawCubeFace(canvas, CanvasKit.M44.multiply(clickRotation, rotation, f.matrix), f.color);
        canvas.restoreToCount(saveCount);
      }
      canvas.restore();  // camera
      canvas.restore();  // center the following content in the window

      // draw virtual sphere outline.
      const paint = new CanvasKit.Paint();
      paint.setAntiAlias(true);
      paint.setStyle(CanvasKit.PaintStyle.Stroke);
      paint.setColor(CanvasKit.Color(64, 255, 0, 1.0));
      canvas.drawCircle(vSphereCenter[0], vSphereCenter[1], vSphereRadius, paint);
      canvas.drawLine(vSphereCenter[0], vSphereCenter[1] - vSphereRadius,
                       vSphereCenter[0], vSphereCenter[1] + vSphereRadius, paint);
      canvas.drawLine(vSphereCenter[0] - vSphereRadius, vSphereCenter[1],
                       vSphereCenter[0] + vSphereRadius, vSphereCenter[1], paint);

      drawLight(canvas);
    }

    // convert a 2D point in the circle displayed on screen to a 3D unit vector.
    // the virtual sphere is a technique selecting a 3D direction by clicking on a the projection
    // of a hemisphere.
    function vSphereUnitV3(p) {
      // v = (v - fCenter) * (1 / fRadius);
      let v = CanvasKit.Vector.mulScalar(CanvasKit.Vector.sub(p, vSphereCenter), 1/vSphereRadius);

      // constrain the clicked point within the circle.
      let len2 = CanvasKit.Vector.lengthSquared(v);
      if (len2 > 1) {
          v = CanvasKit.Vector.normalize(v);
          len2 = 1;
      }
      // the closer to the edge of the circle you are, the closer z is to zero.
      const z = Math.sqrt(1 - len2);
      v.push(z);
      return v;
    }

    function computeVSphereRotation(start, end) {
      const u = vSphereUnitV3(start);
      const v = vSphereUnitV3(end);
      // Axis is in the scope of the Camera3D function so it can be used in keepSpinning.
      axis = CanvasKit.Vector.cross(u, v);
      const sinValue = CanvasKit.Vector.length(axis);
      const cosValue = CanvasKit.Vector.dot(u, v);

      let m = new CanvasKit.M44.identity();
      if (Math.abs(sinValue) > 0.000000001) {
          m = CanvasKit.M44.rotatedUnitSinCos(
            CanvasKit.Vector.mulScalar(axis, 1/sinValue), sinValue, cosValue);
          const radians = Math.atan(cosValue / sinValue);
          spinRate = lastRadians - radians;
          lastRadians = radians;
      }
      return m;
    }

    function keepSpinning() {
      totalSpin += spinRate;
      clickRotation = CanvasKit.M44.rotated(axis, totalSpin);
      spinRate *= .998;
      if (spinRate < 0.01) {
        stopSpinning();
      }
      surface.requestAnimationFrame(drawFrame);
    }

    function stopSpinning() {
        clearInterval(spinning);
        rotation = CanvasKit.M44.multiply(clickRotation, rotation);
        clickRotation = CanvasKit.M44.identity();
    }

    function interact(e) {
      const type = e.type;
      let eventPos = [e.offsetX, e.offsetY];
      if (type === 'lostpointercapture' || type === 'pointerup' || type == 'pointerleave') {
        if (draggingLight) {
          draggingLight = false;
        } else if (mouseDown) {
          mouseDown = false;
          if (spinRate > 0.02) {
            stopSpinning();
            spinning = setInterval(keepSpinning, 30);
          }
        } else {
          return;
        }
        return;
      } else if (type === 'pointermove') {
        if (draggingLight) {
          lightLocation = eventPos;
          lightWorldPos = computeLightWorldPos();
        } else if (mouseDown) {
          lastMouse = eventPos;
          clickRotation = computeVSphereRotation(clickDown, lastMouse);
        } else {
          return;
        }
      } else if (type === 'pointerdown') {
        // Are we repositioning the light?
        if (CanvasKit.Vector.dist(eventPos, lightLocation) < lightIconRadius) {
          draggingLight = true;
          return;
        }
        stopSpinning();
        mouseDown = true;
        clickDown = eventPos;
        lastMouse = eventPos;
      }
      surface.requestAnimationFrame(drawFrame);
    };

    document.getElementById('camera3d').addEventListener('pointermove', interact);
    document.getElementById('camera3d').addEventListener('pointerdown', interact);
    document.getElementById('camera3d').addEventListener('lostpointercapture', interact);
    document.getElementById('camera3d').addEventListener('pointerleave', interact);
    document.getElementById('camera3d').addEventListener('pointerup', interact);

    surface.requestAnimationFrame(drawFrame);
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
