CanvasKit - Quickstart
==============================

CanvasKit is a wasm module that uses Skia to draw to canvas elements a more advance feature set than the canvas API.

Minimal application
-------------------

This example is a minimal Canvaskit application that draws a rounded rect for one frame.
It pulls the wasm binary from unpkg.com but you can also build and host it yourself.

<!--?prettify?-->
``` js
<canvas id=foo width=300 height=300></canvas>

<script type="text/javascript"
  src="https://unpkg.com/canvaskit-wasm@0.19.0/bin/canvaskit.js"></script>
<script type="text/javascript">
  const ckLoaded = CanvasKitInit({
    locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.19.0/bin/'+file});
  ckLoaded.then((CanvasKit) => {
    const surface = CanvasKit.MakeCanvasSurface('foo');

    const paint = new CanvasKit.Paint();
    paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    paint.setAntiAlias(true);
    const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);

    function draw(canvas) {
      canvas.clear(CanvasKit.WHITE);
      canvas.drawRRect(rr, paint);
    }
    surface.drawOnce(draw);
  });
</script>
```

<canvas id=foo width=300 height=300></canvas>

<script type="text/javascript"
  src="https://unpkg.com/canvaskit-wasm@0.19.0/bin/canvaskit.js"></script>
<script type="text/javascript">
  const ckLoaded = CanvasKitInit({
    locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.19.0/bin/'+file});
  ckLoaded.then((CanvasKit) => {
    const surface = CanvasKit.MakeCanvasSurface('foo');

    const paint = new CanvasKit.Paint();
    paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    paint.setAntiAlias(true);
    const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);

    function draw(canvas) {
      canvas.clear(CanvasKit.WHITE);
      canvas.drawRRect(rr, paint);
    }
    surface.drawOnce(draw);
  });
</script>

Let's break it down into parts and explain what they are doing:

`<canvas id=foo width=300 height=300></canvas>` Creates the canvas to which CanvasKit will draw.
This element is where we control the width and height of the drawing buffer, while it's css style
would control any scaling applied after drawing to those pixels. Despite using a canvas element,
CanvasKit isn't calling the HTML canvas's own draw methods. It is using this canvas element to
get a WebGL2 context and performing most of the drawing work in C++ code compiled to WebAssembly,
then sending commands to the GPU at the end of each frame.

<!--?prettify?-->
``` html
<script type="text/javascript"
  src="https://unpkg.com/canvaskit-wasm@0.19.0/bin/canvaskit.js"></script>
```
and

<!--?prettify?-->
``` js
const ckLoaded = CanvasKitInit({
  locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.19.0/bin/'+file});
ckLoaded.then((CanvasKit) => {
```
are loading the canvaskit helper js and wasm binary respectively. CanvasKitInit accepts a function
for allowing you to alter the path where it will try to find `canvaskit.wasm` and returns a promise
that resolves with the loaded module, which we typically name `CanvasKit`.

<!--?prettify?-->
``` js
const surface = CanvasKit.MakeCanvasSurface('foo');
```
Creates a Surface associated with the HTML canvas element above.
Hardware acceleration is the default behavior, but can be overridden by calling
`MakeSWCanvasSurface` instead. `MakeCanvasSurface` is also where alternative color spaces or gl
attrtributes can be specified.

<!--?prettify?-->
``` js
const paint = new CanvasKit.Paint();
paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
paint.setStyle(CanvasKit.PaintStyle.Stroke);
paint.setAntiAlias(true);
const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);
```
Creates a paint, a description of how to fill or stroke rects, paths, text and other geometry in
canvaskit. `rr` is a rounded rect, with corners having a radius of 25 in the x axis, and 15 pixels
in the y axis.

<!--?prettify?-->
``` js
function draw(canvas) {
  canvas.clear(CanvasKit.WHITE);
  canvas.drawRRect(rr, paint);
}
```
Defines a function that will draw our frame. The function is provided a Canvas object on which we
make draw calls. One to clear the entire canvas, and one to draw the rounded rect with the
paint from above.

We also delete the paint object. CanvasKit objects created with `new` or methods prefixed with
`make` must be deleted for the wasm memory to be released. Javascript's GC will not take care of
it automatically. `rr` is just an array, wasn't created with `new` and doesn't point to any WASM
memory, so we don't have to call delete on it.

<!--?prettify?-->
``` js
surface.drawOnce(draw);
paint.delete()
```
Hand the drawing function to `surface.drawOnce` which makes the calls and flushes the surface.
Upon flushing, Skia will batch and send WebGL commands, making visible changes appear onscreen.
This example draws once and disposes of the surface. As promised, it is is a minimal
application.

Basic Draw Loop
---------------

What if we need to redraw to our canvas every frame? This example
bounces a rounded rect around like a 90s screensaver.

<!--?prettify?-->
``` js
ckLoaded.then((CanvasKit) => {
  const surface = CanvasKit.MakeCanvasSurface('foo2');

  const paint = new CanvasKit.Paint();
  paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
  paint.setStyle(CanvasKit.PaintStyle.Stroke);
  paint.setAntiAlias(true);
  // const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);
  const w = 100; // size of rect
  const h = 60;
  let x = 10; // initial position of top left corner.
  let y = 60;
  let dirX = 1; // box is always moving at a constant speed in one of the four diagonal directions
  let dirY = 1;

  function drawFrame(canvas) {
    // boundary check
    if (x < 0 || x+w > 300) {
      dirX *= -1; // reverse x direction when hitting side walls
    }
    if (y < 0 || y+h > 300) {
      dirY *= -1; // reverse y direction when hitting top and bottom walls
    }
    // move
    x += dirX;
    y += dirY;

    canvas.clear(CanvasKit.WHITE);
    const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(x, y, x+w, y+h), 25, 15);
    canvas.drawRRect(rr, paint);
    surface.requestAnimationFrame(drawFrame);
  }
  surface.requestAnimationFrame(drawFrame);
});
```

<canvas id=foo2 width=300 height=300></canvas>

<script type="text/javascript">
  ckLoaded.then((CanvasKit) => {
    const surface = CanvasKit.MakeCanvasSurface('foo2');

    const paint = new CanvasKit.Paint();
    paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    paint.setAntiAlias(true);
    // const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);
    const w = 100; // size of rect
    const h = 60;
    let x = 10; // initial position of top left corner.
    let y = 60;
    // The box is always moving at a constant speed in one of the four diagonal directions
    let dirX = 1;
    let dirY = 1;

    function drawFrame(canvas) {
      // boundary check
      if (x < 0 || x+w > 300) {
        dirX *= -1; // reverse x direction when hitting side walls
      }
      if (y < 0 || y+h > 300) {
        dirY *= -1; // reverse y direction when hitting top and bottom walls
      }
      // move
      x += dirX;
      y += dirY;

      canvas.clear(CanvasKit.WHITE);
      const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(x, y, x+w, y+h), 25, 15);
      canvas.drawRRect(rr, paint);
      surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);
  });
</script>

The main difference here is that we define a function to be called before each frame is drawn and
pass it to `surface.requestAnimationFrame(drawFrame);` That callback is handed a `canvas` and
flushing is taken care of.

<!--?prettify?-->
``` js
function drawFrame(canvas) {
  canvas.clear(CanvasKit.WHITE);
  // code to update and draw the frame goes here
  surface.requestAnimationFrame(drawFrame);
}
surface.requestAnimationFrame(drawFrame);
```

Creates a function to serve as our main drawing loop. Each time a frame is about to be rendered
(the browser will typically target 60fps), our function is called, we clear the canvas with white,
redraw the round rect, and call `surface.requestAnimationFrame(drawFrame)` registering the
function to be called again before the next frame.

`surface.requestAnimationFrame(drawFrame)` combines window.requestAnimationFrame with
`surface.flush()` and should be used in all the same ways. If your application would only make
visible changes as a result of mouse events,
don't call `surface.requestAnimationFrame` at the end of your drawFrame function. Call it only
after handling mouse input.

Text Shaping
------------

One of the biggest features that CanvasKit offers over the HTML Canvas API is paragraph shaping.
To use text your applicatoin, supply a font file and use Promise.all to run your code when both
CanvasKit and the font file are ready.

<!--?prettify?-->
``` js
const loadFont = fetch('https://storage.googleapis.com/skia-cdn/misc/Roboto-Regular.ttf')
  .then((response) => response.arrayBuffer());

Promise.all([ckLoaded, loadFont]).then(([CanvasKit, robotoData]) => {
  const surface = CanvasKit.MakeCanvasSurface('foo3');
  const canvas = surface.getCanvas();
  canvas.clear(CanvasKit.Color4f(0.9, 0.9, 0.9, 1.0));

  const fontMgr = CanvasKit.FontMgr.FromData([robotoData]);
  const paraStyle = new CanvasKit.ParagraphStyle({
    textStyle: {
      color: CanvasKit.BLACK,
      fontFamilies: ['Roboto'],
      fontSize: 28,
    },
    textAlign: CanvasKit.TextAlign.Left,
  });
  const text = 'Any sufficiently entrenched technology is indistinguishable from Javascript';
  const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
  builder.addText(text);
  const paragraph = builder.build();
  paragraph.layout(290); // width in pixels to use when wrapping text
  canvas.drawParagraph(paragraph, 10, 10);
  surface.flush();
});
```

<canvas id=foo3 width=300 height=300></canvas>

<script type="text/javascript">
const loadFont = fetch('https://storage.googleapis.com/skia-cdn/misc/Roboto-Regular.ttf')
  .then((response) => response.arrayBuffer());

Promise.all([ckLoaded, loadFont]).then(([CanvasKit, robotoData]) => {
  const surface = CanvasKit.MakeCanvasSurface('foo3');
  const canvas = surface.getCanvas();
  canvas.clear(CanvasKit.Color4f(0.9, 0.9, 0.9, 1.0));

  const fontMgr = CanvasKit.FontMgr.FromData([robotoData]);
  const paraStyle = new CanvasKit.ParagraphStyle({
    textStyle: {
      color: CanvasKit.BLACK,
      fontFamilies: ['Roboto'],
      fontSize: 28,
    },
    textAlign: CanvasKit.TextAlign.Left,
  });
  const text = 'Any sufficiently entrenched technology is indistinguishable from Javascript';
  const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
  builder.addText(text);
  const paragraph = builder.build();
  paragraph.layout(290); // width in pixels to use when wrapping text
  canvas.drawParagraph(paragraph, 10, 10);
  surface.flush();
});
</script>

<!--?prettify?-->
``` js
const fontMgr = CanvasKit.FontMgr.FromData([robotoData]);
```
Creates an object that provides fonts by name to various text facilities in CanvasKit. You could
load more than one font in this statement if needed.

<!--?prettify?-->
``` js
const paraStyle = new CanvasKit.ParagraphStyle({
  textStyle: {
    color: CanvasKit.BLACK,
    fontFamilies: ['Roboto'],
    fontSize: 28,
  },
  textAlign: CanvasKit.TextAlign.Left,
});
```
Specifies the style of the text. The font's name, Roboto, will be used to fetch it from the font
manager. You can specify either (color) or (foregroundColor and backgroundColor) in order to have
a highlight. For the full documentation of the API, check out the Typescript definitions in the
`types/` subfolder of the npm package or in the
[Skia repo](https://github.com/google/skia/tree/master/modules/canvaskit/canvaskit/types).

<!--?prettify?-->
``` js
const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
builder.addText(text);
const paragraph = builder.build();
```
Next, we create a `ParagraphBuilder` with a style, add some text, and finalize it with `build()`.
Alternatively, we could use multiple `TextStyle`s in one paragraph with

<!--?prettify?-->
``` js
const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
builder.addText(text1);
const boldTextStyle = CanvasKit.TextStyle({
    color: CanvasKit.BLACK,
    fontFamilies: ['Roboto'],
    fontSize: 28,
    fontStyle: {'weight': CanvasKit.FontWeight.Bold},
})
builder.pushStyle(boldTextStyle);
builder.addText(text2);
builder.pop();
builder.addText(text3);
const paragraph = builder.build();
```
Finally, we *layout* the paragraph, meaning wrap the text to a particular width, and draw it to
the canvas with

<!--?prettify?-->
``` js
paragraph.layout(290); // width in pixels to use when wrapping text
canvas.drawParagraph(paragraph, 10, 10); // (x, y) position of left top corner of paragraph.
```
