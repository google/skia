PathKit - Skia in the Browser
=============================

Skia has made its [SkPath](../api/SkPath_Reference) object and many related methods
available to JS clients (e.g. Web Browsers) using WebAssembly and asm.js.

Download the library
--------------------

See [the npm page](https://www.npmjs.com/package/experimental-pathkit-wasm) for details on downloading
and getting started.

Features
--------

PathKit is still under rapid development, so the exact API is still changing.

The primary features are:

  - API compatibility (e.g. drop-in replacement) with [Path2D](https://developer.mozilla.org/en-US/docs/Web/API/Path2D)
  - Can output to SVG / Canvas / Path2D
  - Exposes a variety of path effects: <img width=800 src="./PathKit_effects.png"/>


Example code
------------
The best place to look for examples on how to use PathKit would be in the
[example.html](https://github.com/google/skia/blob/master/experimental/pathkit/npm-wasm/example.html#L45)
which comes in the npm package.