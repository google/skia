A WASM version of Skia's Canvas API.

See https://skia.org/user/modules/canvaskit for more background information.

# Getting Started

## Browser
To use the library, run `npm install canvaskit-wasm` and then simply include it:

    <script src="/node_modules/canvaskit-wasm/bin/canvaskit.js"></script>
    CanvasKitInit({
        locateFile: (file) => '/node_modules/canvaskit-wasm/bin/'+file,
    }).ready().then((CanvasKit) => {
        // Code goes here using CanvasKit
    });

As with all npm packages, there's a freely available CDN via unpkg.com:

    <script src="https://unpkg.com/canvaskit-wasm@0.3.0/bin/canvaskit.js"></script>
    CanvasKitInit({
         locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.3.0/bin/'+file,
    }).ready().then(...)

## Node
To use CanvasKit in Node, it's similar to the browser:

    const CanvasKitInit = require('/node_modules/canvaskit-wasm/bin/canvaskit.js');
    CanvasKitInit({
        locateFile: (file) => __dirname + '/bin/'+file,
    }).ready().then((CanvasKit) => {
        // Code goes here using CanvasKit
    });

With node, you also need to supply the `--expose-wasm` flag.

## WebPack

WebPack's support for WASM is still somewhat experimental, but CanvasKit can be
used with a few configuration changes.

In the JS code, use require():

    const CanvasKitInit = require('canvaskit-wasm/bin/canvaskit.js')
    CanvasKitInit().ready().then((CanvasKit) => {
        // Code goes here using CanvasKit
    });

Since WebPack does not expose the entire `/node_modules/` directory, but instead
packages only the needed pieces, we have to copy canvaskit.wasm into the build directory.
One such solution is to use [CopyWebpackPlugin](https://github.com/webpack-contrib/copy-webpack-plugin).
For example, add the following plugin:

    config.plugins.push(
        new CopyWebpackPlugin([
            { from: 'node_modules/canvaskit-wasm/bin/canvaskit.wasm' }
        ])
    );

If webpack gives an error similar to:

    ERROR in ./node_modules/canvaskit-wasm/bin/canvaskit.js
    Module not found: Error: Can't resolve 'fs' in '...'

Then, add the following configuration change to the node section of the config:

    config.node = {
        fs: 'empty'
    };


# Using the CanvasKit API

See `example.html` and `node.example.js` for demos of how to use the API.

More detailed docs will be coming soon.

## Drop-in Canvas2D replacement
For environments where an HTML canvas is not available (e.g. Node, headless servers),
CanvasKit has an optional API (included by default) that mirrors the HTML canvas.

    let skcanvas = CanvasKit.MakeCanvas(600, 600);

    let ctx = skcanvas.getContext('2d');
    let rgradient = ctx.createRadialGradient(200, 300, 10, 100, 100, 300);

    // Add three color stops
    rgradient.addColorStop(0, 'red');
    rgradient.addColorStop(0.7, 'white');
    rgradient.addColorStop(1, 'blue');

    ctx.fillStyle = rgradient;
    ctx.globalAlpha = 0.7;
    ctx.fillRect(0, 0, 600, 600);

    let imgData = skcanvas.toDataURL();
    // imgData is now a base64 encoded image.

See more examples in `example.html` and `node.example.js`.


# Filing bugs

Please file bugs at [skbug.com](skbug.com).
It may be convenient to use [our online fiddle](jsfiddle.skia.org/canvaskit) to demonstrate any issues encountered.

See CONTRIBUTING.md for more information on sending pull requests.