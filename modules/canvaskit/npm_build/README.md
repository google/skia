A WASM version of Skia's Canvas API.

See https://skia.org/user/modules/canvaskit for more background information.

# Getting Started

## Browser

To use the library, run `npm install canvaskit-wasm` and then simply include it:

```html
<script src="/node_modules/canvaskit-wasm/bin/canvaskit.js"></script>
```
```javascript
CanvasKitInit({
    locateFile: (file) => '/node_modules/canvaskit-wasm/bin/'+file,
}).then((CanvasKit) => {
    // Code goes here using CanvasKit
});
```

As with all npm packages, there's a freely available CDN via unpkg.com:

```html
<script src="https://unpkg.com/canvaskit-wasm@latest/bin/canvaskit.js"></script>
```
```javascript
CanvasKitInit({
    locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@latest/bin/'+file,
}).then((CanvasKit) => {
    // Code goes here using CanvasKit
});
```

## Node
To use CanvasKit in Node, it's similar to the browser:

```javascript
const CanvasKitInit = require('canvaskit-wasm/bin/canvaskit.js');
CanvasKitInit({
    locateFile: (file) => __dirname + '/bin/'+file,
}).then((CanvasKit) => {
    // Code goes here using CanvasKit
});
```

## WebPack

WebPack's support for WASM is still somewhat experimental, but CanvasKit can be
used with a few configuration changes.

In the JS code, use require():

```javascript
const CanvasKitInit = require('canvaskit-wasm/bin/canvaskit.js')
CanvasKitInit().then((CanvasKit) => {
    // Code goes here using CanvasKit
});
```

Since WebPack does not expose the entire `/node_modules/` directory, but instead
packages only the needed pieces, we have to copy canvaskit.wasm into the build directory.
One such solution is to use [CopyWebpackPlugin](https://github.com/webpack-contrib/copy-webpack-plugin).
For example, add the following plugin:

```javascript
config.plugins.push(
    new CopyWebpackPlugin([
        { from: 'node_modules/canvaskit-wasm/bin/canvaskit.wasm' }
    ])
);
```

If webpack gives an error similar to:

```warn
ERROR in ./node_modules/canvaskit-wasm/bin/canvaskit.js
Module not found: Error: Can't resolve 'fs' in '...'
```

Then, add the following configuration change to the node section of the config:

```javascript
config.node = {
    fs: 'empty'
};
```


# Different canvaskit bundles

`canvaskit-wasm` includes 3 types of bundles:

* default `./bin/canvaskit.js` - Basic canvaskit functionality


```javascript
const InitCanvasKit = require('canvaskit-wasm/bin/canvaskit');
```

* full `./bin/full/canvaskit.js` - includes [Skottie](https://skia.org/docs/user/modules/skottie/) and other libraries

```javascript
const InitCanvasKit = require('canvaskit-wasm/bin/full/canvaskit');
```

* profiling `./bin/profiling/canvaskit.js` - the same as `full` but contains full names of wasm functions called internally

```javascript
const InitCanvasKit = require('canvaskit-wasm/bin/profiling/canvaskit');
```

# ES6 import and node entrypoints

This package also exposes [entrypoints](https://nodejs.org/api/packages.html#package-entry-points)

```javascript
import InitCanvasKit from 'canvaskit-wasm'; // default
```

```javascript
import InitCanvasKit from 'canvaskit-wasm/full';
```

```javascript
import InitCanvasKit from 'canvaskit-wasm/profiling';
```

If you use [typescript](https://www.typescriptlang.org/)

you need to enable [resolvePackageJsonExports](https://www.typescriptlang.org/tsconfig#resolvePackageJsonExports) in your `tsconfig.json`

```json
{
    "compilerOptions": {
        "resolvePackageJsonExports": true
    }
}
```

# Using the CanvasKit API

See `example.html` and `node.example.js` for demos of how to use the core API.

See `extra.html` for some optional add-ins like an animation player (Skottie).

See `types/index.d.ts` for a typescript definition file that contains all the
APIs and some documentation about them.

## Drop-in Canvas2D replacement
For environments where an HTML canvas is not available (e.g. Node, headless servers),
CanvasKit has an optional API (included by default) that mostly mirrors the [CanvasRenderingContext2D](https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D).

```javascript
const skcanvas = CanvasKit.MakeCanvas(600, 600);

const ctx = skcanvas.getContext('2d');
const rgradient = ctx.createRadialGradient(200, 300, 10, 100, 100, 300);

// Add three color stops
rgradient.addColorStop(0, 'red');
rgradient.addColorStop(0.7, 'white');
rgradient.addColorStop(1, 'blue');

ctx.fillStyle = rgradient;
ctx.globalAlpha = 0.7;
ctx.fillRect(0, 0, 600, 600);

const imgData = skcanvas.toDataURL();
// imgData is now a base64 encoded image.
```

See more examples in `example.html` and `node.example.js`.

### Known issues with Canvas2D Emulation layer
 - measureText returns width only and does no shaping. It is only sort of valid with ASCII letters.
 - textAlign is not supported.
 - textBaseAlign is not supported.
 - fillText does not support the width parameter.

# Filing bugs

Please file bugs at [https://skbug.com](skbug.com).
It may be convenient to use [our online fiddle](https://jsfiddle.skia.org/canvaskit) to demonstrate any issues encountered.

See CONTRIBUTING.md for more information on sending pull requests.

# Types and Documentation

There are Typescript types and associated API docs in [types/](./types/).
