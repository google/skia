PathKit - Skia in the Browser
=============================

Skia has made its [SkPath](../api/SkPath_Reference) object and many related methods
available to JS clients (e.g. Web Browsers) using WebAssembly and asm.js.

Download the library
--------------------

See the the npm page for either the [WebAssembly](https://www.npmjs.com/package/experimental-pathkit-wasm)
or [asm.js](https://www.npmjs.com/package/experimental-pathkit-asmjs) version
for details on downloading and getting started.

Features
--------

PathKit is still under rapid development, so the exact API is still changing.

The primary features are:

  - API compatibility (e.g. drop-in replacement) with [Path2D](https://developer.mozilla.org/en-US/docs/Web/API/Path2D)
  - Can output to SVG / Canvas / Path2D
  - Exposes a variety of path effects:

<style>
  svg, canvas {
    border: 1px dashed #AAA;
  }

  canvas.big {
    width: 300px;
    height: 300px;
  }

</style>

<canvas class=big id=canvas1></canvas>
<canvas class=big id=canvas2></canvas>
<canvas class=big id=canvas3></canvas>
<canvas class=big id=canvas4></canvas>
<canvas class=big id=canvas5></canvas>
<canvas class=big id=canvas6></canvas>

<script src="https://unpkg.com/experimental-pathkit-asmjs@0.2.0/bin/pathkit.js"></script>
<script>
PathKitInit({
    locateFile: (file) => 'https://unpkg.com/experimental-pathkit-asmjs@0.2.0/bin/'+file,
}).then((PathKit) => {
    // Code goes here using PathKit
    console.log('Hello Pathkit', PathKit.usingWasm, PathKit.SkPath);
});
</script>


Example Code
------------
The best place to look for examples on how to use PathKit would be in the
[example.html](https://github.com/google/skia/blob/master/experimental/pathkit/npm-wasm/example.html#L45)
which comes in the npm package.


API
----

The primary feature of the library is the `SkPath` object. It can be created:

 - From the SVG string of a path `PathKit.FromSVGString(str)`
 - From a 2D array of verbs and arguments `PathKit.FromCmds(cmds)`
 - From `PathKit.NewPath()` (It will be blank)
 - As a copy of an existing `SkPath` with `path.copy()` or `PathKit.NewPath(path)`

It can be exported as:

 - An SVG string `path.ToSVGString()`
 - A [Path2D](https://developer.mozilla.org/en-US/docs/Web/API/Path2D) object `path.ToPath2D()`
 - Directly to a canvas 2D context `path.toCanvas(ctx)`
 - A 2D array of verbs and arguments `path.toCmds()`

Once an SkPath object has been made, it can be interacted with in the following ways:

 - expanded by any of the Path2D operations (`moveTo`, `lineTo`, `rect`, `arc`, etc)
 - combined with other paths using `op` or `PathKit.MakeFromOp(p1, p2, op)`.  For example, `path1.op(path2, PathKit.PathOp.INTERSECT)` will set path1 to be the area represented by where path1 and path2 overlap (intersect). `PathKit.MakeFromOp(path1, path2, PathKit.PathOp.INTERSECT)` will do the same but returned as a new `SkPath` object.
 - adjusted with some of the effects (`trim`, `dash`, `stroke`, etc)


**Important**: Any objects (`SkPath`, `SkOpBuilder`, etc) that are created must be cleaned up with `path.delete()` when they
leave the scope to avoid leaking the memory in the WASM heap. This counts any of the constructors or
any function prefixed with "Make".


###PathKit###

###SkPath (object)###

###SkOpBuilder (object)###

###SkMatrix (struct)###

###StrokeOpts (struct)###

###PathOp (enum)###

###FillType (enum)###

###StrokeJoin (enum)###

###StrokeCap (enum)###

###Constants###

###Functions for testing only###