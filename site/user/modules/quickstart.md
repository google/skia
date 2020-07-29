CanvasKit - Quickstart
==============================

Minimal application
-------------------

A minimal Canvaskit application that draws to the screen once alooks like the following.
This pulls the wasm binary from unpkg.com but you can also build and host it yourself.

```js
<canvas id=foo width=300 height=300></canvas>

<script type="text/javascript" src="https://unpkg.com/canvaskit-wasm@0.16.0/bin/canvaskit.js"></script>
<script type="text/javascript">
  const ckLoaded = CanvasKitInit({locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.16.0/bin/'+file});
  ckLoaded.then((CanvasKit) => {
    const surface = CanvasKit.MakeCanvasSurface('foo');
    const canvas = surface.getCanvas();

    const paint = new CanvasKit.SkPaint();
    paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    paint.setAntiAlias(true);
    const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);

    canvas.clear(CanvasKit.WHITE);
    canvas.drawRRect(rr, paint);
    surface.flush();
  });
</script>
```

<canvas id=foo width=300 height=300></canvas>

<script type="text/javascript" src="https://unpkg.com/canvaskit-wasm@0.16.0/bin/canvaskit.js"></script>
<script type="text/javascript">
  const ckLoaded = CanvasKitInit({locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.16.0/bin/'+file});
  ckLoaded.then((CanvasKit) => {
    const surface = CanvasKit.MakeCanvasSurface('foo');
    const canvas = surface.getCanvas();

    const paint = new CanvasKit.SkPaint();
    paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    paint.setAntiAlias(true);
    const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);

    canvas.clear(CanvasKit.WHITE);
    canvas.drawRRect(rr, paint);
    surface.flush();
  });
</script>

let's break it down into parts and explain what they are doing:

`<canvas id=foo width=300 height=300></canvas>` Creates a canvas with a particular id.
this element is where your drawings end up, and it's width and height control the resolution
you are drawing in, while it's css style would control any scaling applied after drawing to
those pixels. Despite using a canvas element, CanvasKit isn't calling the HTML canvas's own
draw methods, it is using this canvas element to get a WebGL2 context and performing most of
the drawing work in C++ code compiled to WebAssembly, then sending commands to the GPU at
the end of each frame.

```html
<script type="text/javascript" src="https://unpkg.com/canvaskit-wasm@0.16.0/bin/canvaskit.js"></script>
```
and 
```js
const ckLoaded = CanvasKitInit({locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.16.0/bin/'+file});
ckLoaded.then((CanvasKit) => {
```
are loading the canvaskit helper js and wasm binary respectively. CanvasKitInit accepts a function
for allowing you to alter the path where it will try to find `canvaskit.wasm` and returns a promise
that resolves with the loaded module, which we typically name `CanvasKit`.

```js
const surface = CanvasKit.MakeCanvasSurface('foo');
const canvas = surface.getCanvas();
```
Creates an SkSurface associated with the HTML canvas element above, and an SkCanvas from that SkSurface.
`canvas` is the object we will make draw calls on.
```js
const paint = new CanvasKit.SkPaint();
paint.setColor(CanvasKit.Color4f(0.9, 0, 0, 1.0));
paint.setStyle(CanvasKit.PaintStyle.Stroke);
paint.setAntiAlias(true);
const rr = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 25, 15);
```
Creates a paint, a description of how to fill or stroke rects, paths, text and other geometry in canvaskit.
`rr` is a rounded rect, with corners having a radius of 25 in the x axis, and 15 pixels in the y axis.
```js
canvas.clear(CanvasKit.WHITE);
canvas.drawRRect(rr, paint);
```
Make two draw calls on our canvas. One to clear to white, and one to draw the rounded rect with the paint from above.
Nothing happens until we call
```js
surface.flush();
```
Which causes Skia to process the commands and send them via WebGL to the GPU, making visible changes appear onscreen.
This example draws once. As promised, it is is a minimal application.

Basic Draw Loop
---------------

What if we need to redraw to our canvas every frame? This example
bounces a rounded rect around like a 90s screensaver.

``` js
<canvas id=foo2 width=300 height=300></canvas>

<script type="text/javascript" src="https://unpkg.com/canvaskit-wasm@0.16.0/bin/canvaskit.js"></script>
<script type="text/javascript">
  const ckLoaded = CanvasKitInit({locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.16.0/bin/'+file});
  ckLoaded.then((CanvasKit) => {
    const surface = CanvasKit.MakeCanvasSurface('foo2');

    const paint = new CanvasKit.SkPaint();
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
</script>
```

<canvas id=foo2 width=300 height=300></canvas>

<script type="text/javascript" src="https://unpkg.com/canvaskit-wasm@0.16.0/bin/canvaskit.js"></script>
<script type="text/javascript">
  const ckLoaded = CanvasKitInit({locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.16.0/bin/'+file});
  ckLoaded.then((CanvasKit) => {
    const surface = CanvasKit.MakeCanvasSurface('foo2');

    const paint = new CanvasKit.SkPaint();
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
</script>

The main difference here is that we define a function to be called before each frame is drawn, and pass it to `surface.requestAnimationFrame(drawFrame);`
That callback is handed a `canvas` and flushing is taken care of.

```js
function drawFrame(canvas) {
  canvas.clear(CanvasKit.WHITE);
  // code to update and draw the frame goes here
  surface.requestAnimationFrame(drawFrame);
}
surface.requestAnimationFrame(drawFrame);
````

Creates a function to serve as our main drawing loop. Each time a frame is about to be rendered
(the browser will target 60fps), our function is called, we clear the canvas with white, redraw the round rect,
and call `surface.requestAnimationFrame(drawFrame)` registering the function to be called again before the next frame.

`surface.requestAnimationFrame(drawFrame)` combines window.requestAnimationFrame with `surface.flush()` and should be
used in all the same ways. If for example your application would only make visible changes as a result of mouse events,
don't call `surface.requestAnimationFrame` at the end of your drawFrame function, call it only after handling mouse input.
