
---
title: "Particles"
linkTitle: "Particles"

weight: 40

---


Skia’s particle module provides a way to quickly generate large numbers of
drawing primitives with dynamic, animated behavior. Particles can be used to
create effects like fireworks, spark trails, ambient “weather”, and much more.
Nearly all properties and behavior are controlled by scripts written in Skia’s
custom language, SkSL.

## Samples

<style>
  #demo canvas {
    border: 1px dashed #AAA;
    margin: 2px;
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
  <figure>
    <canvas id=trail width=400 height=400></canvas>
    <figcaption>
      Trail (Click and Drag!)
    </figcaption>
  </figure>
  <figure>
    <canvas id=cube width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/?nameOrHash=@cube"
         target=_blank rel=noopener>Cuboid</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=confetti width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/?nameOrHash=@confetti"
         target=_blank rel=noopener>Confetti</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=curves width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/?nameOrHash=@swirl"
         target=_blank rel=noopener>Curves</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=fireworks width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/?nameOrHash=@fireworks"
         target=_blank rel=noopener>Fireworks</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=text width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/?nameOrHash=@text"
         target=_blank rel=noopener>Text</a>
    </figcaption>
  </figure>

</div>

<script type="text/javascript" charset="utf-8">
(function() {
  // Tries to load the WASM version if supported, shows error otherwise
  let s = document.createElement('script');
  var locate_file = '';
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
  var CanvasKit = null;
  CanvasKitInit({
    locateFile: (file) => locate_file + file,
  }).then((CK) => {
    CanvasKit = CK;
    TrailExample(CanvasKit, 'trail', trail);
    ParticleExample(CanvasKit, 'confetti', confetti, 200, 200);
    ParticleExample(CanvasKit, 'curves', curves, 200, 300);
    ParticleExample(CanvasKit, 'cube', cube, 200, 200);
    ParticleExample(CanvasKit, 'fireworks', fireworks, 200, 300);
    ParticleExample(CanvasKit, 'text', text, 75, 250);
  });

  function ParticleExample(CanvasKit, id, jsonData, cx, cy) {
    if (!CanvasKit || !jsonData) {
      return;
    }
    const surface = CanvasKit.MakeCanvasSurface(id);
    if (!surface) {
      console.error('Could not make surface');
      return;
    }
    const canvas = surface.getCanvas();
    canvas.translate(cx, cy);

    const particles = CanvasKit.MakeParticles(JSON.stringify(jsonData));
    particles.start(Date.now() / 1000.0, true);

    function drawFrame(canvas) {
      particles.update(Date.now() / 1000.0);

      canvas.clear(CanvasKit.WHITE);
      particles.draw(canvas);
      surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);
  }

const confetti ={
   "MaxCount": 200,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 8
   },
   "Code": [
     "void effectSpawn(inout Effect effect) {",
     "  effect.lifetime = 2;",
     "}",
     "",
     "void effectUpdate(inout Effect effect) {",
     "  if (effect.age < 0.25 || effect.age > 0.75) { effect.rate = 0; }",
     "  else { effect.rate = 200; }",
     "}",
     "",
      "void spawn(inout Particle p) {",
      "  int idx = int(rand(p.seed) * 4);",
      "  p.color.rgb = (idx == 0) ? float3(0.87, 0.24, 0.11)",
      "              : (idx == 1) ? float3(1.00, 0.90, 0.20)",
      "              : (idx == 2) ? float3(0.44, 0.73, 0.24)",
      "              :              float3(0.38, 0.54, 0.95);",
      "",
      "  p.lifetime = (1 - effect.age) * effect.lifetime;",
      "  p.scale = mix(0.6, 1, rand(p.seed));",
      "}",
      "",
      "void update(inout Particle p) {",
      "  p.color.a = 1 - p.age;",
      "",
      "  float a = radians(rand(p.seed) * 360);",
      "  float invAge = 1 - p.age;",
      "  p.vel = float2(cos(a), sin(a)) * mix(250, 550, rand(p.seed)) * invAge * invAge;",
      "}",
      ""
   ],
   "Bindings": []
};

const cube = {
  "MaxCount": 2000,
  "Drawable": {
    "Type": "SkCircleDrawable",
    "Radius": 4
  },
  "Code": [
    "void effectSpawn(inout Effect effect) {",
    "  effect.lifetime = 2;",
    "  effect.rate = 200;",
    "}",
    "",
    "void spawn(inout Particle p) {",
    "  p.lifetime = 10;",
    "}",
    "",
    "float4x4 rx(float rad) {",
    "  float c = cos(rad);",
    "  float s = sin(rad);",
    "  return float4x4(1, 0,  0, 0,",
    "                  0, c, -s, 0,",
    "                  0, s,  c, 0,",
    "                  0, 0,  0, 1);",
    "}",
    "",
    "float4x4 ry(float rad) {",
    "  float c = cos(rad);",
    "  float s = sin(rad);",
    "  return float4x4(c, 0, -s, 0,",
    "                  0, 1,  0, 0,",
    "                  s, 0,  c, 0,",
    "                  0, 0,  0, 1);",
    "}",
    "",
    "float4x4 rz(float rad) {",
    "  float c = cos(rad);",
    "  float s = sin(rad);",
    "  return float4x4( c, s, 0, 0,",
    "                  -s, c, 0, 0,",
    "                   0, 0, 1, 0,",
    "                   0, 0, 0, 1);",
    "}",
    "",
    "void update(inout Particle p) {",
    "  float3 pos = float3(rand(p.seed), rand(p.seed), rand(p.seed));",
    "  if (rand(p.seed) < 0.33) {",
    "    if (pos.x > 0.5) {",
    "      pos.x = 1;",
    "      p.color.rgb = float3(1, 0.2, 0.2);",
    "    } else {",
    "      pos.x = 0;",
    "      p.color.rgb = float3(0.2, 1, 1);",
    "    }",
    "  } else if (rand(p.seed) < 0.5) {",
    "    if (pos.y > 0.5) {",
    "      pos.y = 1;",
    "      p.color.rgb = float3(0.2, 0.2, 1);",
    "    } else {",
    "      pos.y = 0;",
    "      p.color.rgb = float3(1, 1, 0.2);",
    "    }",
    "  } else {",
    "    if (pos.z > 0.5) {",
    "      pos.z = 1;",
    "      p.color.rgb = float3(0.2, 1, 0.2);",
    "    } else {",
    "      pos.z = 0;",
    "      p.color.rgb = float3(1, 0.2, 1);",
    "    }",
    "  }",
    "",
    "  float s = effect.age * 2 - 1;",
    "  s = s < 0 ? -s : s;",
    "",
    "  pos = pos * 2 - 1;",
    "  pos = mix(pos, normalize(pos), s);",
    "  pos = pos * 100;",
    "",
    "  float age = float(effect.loop) + effect.age;",
    "  float4x4 mat = rx(age * radians(60))",
    "               * ry(age * radians(70))",
    "               * rz(age * radians(80));",
    "  pos = (mat * float4(pos, 1)).xyz;",
    "",
    "  p.pos.x = pos.x;",
    "  p.pos.y = pos.y;",
    "  p.scale = ((pos.z + 50) / 100 + 0.5) / 2;",
    "}",
    ""
  ],
  "Bindings": []
};

const curves = {
   "MaxCount": 1000,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 2
   },
   "Code": [
     "void effectSpawn(inout Effect effect) {",
     "  effect.rate = 200;",
     "  effect.color = float4(1, 0, 0, 1);",
     "}",
     "",
      "void spawn(inout Particle p) {",
      "  p.lifetime = 3 + rand(p.seed);",
      "  p.vel.y = -50;",
      "}",
      "",
      "void update(inout Particle p) {",
      "  float w = mix(15, 3, p.age);",
      "  p.pos.x = sin(radians(p.age * 320)) * mix(25, 10, p.age) + mix(-w, w, rand(p.seed));",
      "  if (rand(p.seed) < 0.5) { p.pos.x = -p.pos.x; }",
      "",
      "  p.color.g = (mix(75, 220, p.age) + mix(-30, 30, rand(p.seed))) / 255;",
      "}",
      ""
   ],
   "Bindings": []
};

const fireworks = {
   "MaxCount": 300,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 3
   },
   "Code": [
     "void effectSpawn(inout Effect effect) {",
     "  // Phase one: Launch",
     "  effect.lifetime = 4;",
     "  effect.rate = 120;",
     "  float a = radians(mix(-20, 20, rand(effect.seed)) - 90);",
     "  float s = mix(200, 220, rand(effect.seed));",
     "  effect.vel.x = cos(a) * s;",
     "  effect.vel.y = sin(a) * s;",
     "  effect.color.rgb = float3(rand(effect.seed), rand(effect.seed), rand(effect.seed));",
     "  effect.pos.x = 0;",
     "  effect.pos.y = 0;",
     "  effect.scale = 0.25;  // Also used as particle behavior flag",
     "}",
     "",
     "void effectUpdate(inout Effect effect) {",
     "  if (effect.age > 0.5 && effect.rate > 0) {",
     "    // Phase two: Explode",
     "    effect.rate = 0;",
     "    effect.burst = 50;",
     "    effect.scale = 1;",
     "  } else {",
     "    effect.vel.y += dt * 90;",
     "  }",
     "}",
     "",
      "void spawn(inout Particle p) {",
      "  bool explode = p.scale == 1;",
      "",
      "  p.lifetime = explode ? (2 + rand(p.seed) * 0.5) : 0.5;",
      "  float a = radians(rand(p.seed) * 360);",
      "  float s = explode ? mix(90, 100, rand(p.seed)) : mix(5, 10, rand(p.seed));",
      "  p.vel.x = cos(a) * s;",
      "  p.vel.y = sin(a) * s;",
      "}",
      "",
      "void update(inout Particle p) {",
      "  p.color.a = 1 - p.age;",
      "  if (p.scale == 1) {",
      "    p.vel.y += dt * 50;",
      "  }",
      "}",
      ""
   ],
   "Bindings": []
};

const text = {
   "MaxCount": 2000,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 1
   },
   "Code": [
     "void effectSpawn(inout Effect effect) {",
     "  effect.rate = 1000;",
     "}",
     "",
      "void spawn(inout Particle p) {",
      "  p.lifetime = mix(1, 3, rand(p.seed));",
      "  float a = radians(mix(250, 290, rand(p.seed)));",
      "  float s = mix(10, 30, rand(p.seed));",
      "  p.vel.x = cos(a) * s;",
      "  p.vel.y = sin(a) * s;",
      "  p.pos = text(rand(p.seed)).xy;",
      "}",
      "",
      "void update(inout Particle p) {",
      "  float4 startColor = float4(1, 0.196, 0.078, 1);",
      "  float4 endColor   = float4(1, 0.784, 0.078, 1);",
      "  p.color = mix(startColor, endColor, p.age);",
      "}",
      ""
   ],
   "Bindings": [
      {
         "Type": "SkTextBinding",
         "Name": "text",
         "Text": "SKIA",
         "FontSize": 96
      }
   ]
};

  function preventScrolling(canvas) {
    canvas.addEventListener('touchmove', (e) => {
      // Prevents touch events in the canvas from scrolling the canvas.
      e.preventDefault();
      e.stopPropagation();
    });
  }

  function TrailExample(CanvasKit, id, jsonData) {
    if (!CanvasKit || !jsonData) {
      return;
    }
    const surface = CanvasKit.MakeCanvasSurface(id);
    if (!surface) {
      console.error('Could not make surface');
      return;
    }
    const canvas = surface.getCanvas();

    const particles = CanvasKit.MakeParticles(JSON.stringify(jsonData));
    particles.start(Date.now() / 1000.0, true);

    function drawFrame(canvas) {
      particles.update(Date.now() / 1000.0);

      canvas.clear(CanvasKit.WHITE);
      particles.draw(canvas);
      surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);

    let interact = (e) => {
      particles.setPosition([e.offsetX, e.offsetY]);
      particles.setRate(e.pressure * 1000);
    };
    document.getElementById('trail').addEventListener('pointermove', interact);
    document.getElementById('trail').addEventListener('pointerdown', interact);
    document.getElementById('trail').addEventListener('pointerup', interact);
    preventScrolling(document.getElementById('trail'));
  }

const trail = {
   "MaxCount": 2000,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 4
   },
   "Code": [
      "void spawn(inout Particle p) {",
      "  p.lifetime = 2 + rand(p.seed);",
      "  float a = radians(rand(p.seed) * 360);",
      "  p.vel = float2(cos(a), sin(a)) * mix(5, 15, rand(p.seed));",
      "  p.scale = mix(0.25, 0.75, rand(p.seed));",
      "}",
      "",
      "void update(inout Particle p) {",
      "  p.color.r = p.age;",
      "  p.color.g = 1 - p.age;",
      "}",
      ""
   ],
   "Bindings": []
};

  }
  document.head.appendChild(s);
})();
</script>

