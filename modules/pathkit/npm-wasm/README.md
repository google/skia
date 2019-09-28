A WASM version of Skia's PathOps toolkit.

To use the library, run `npm install pathkit-wasm` and then simply include it:

    <script src="/node_modules/pathkit-wasm/bin/pathkit.js"></script>
    PathKitInit({
        locateFile: (file) => '/node_modules/pathkit-wasm/bin/'+file,
    }).ready().then((PathKit) => {
        // Code goes here using PathKit
    });

PathKit comes in two parts, a JS loader and the actual WASM code. The JS loader creates
a global `PathKitInit` that can be called to load the WASM code. The `locateFile` function
is used to tell the JS loader where to find the .wasm file. By default, it will
look for /pathkit.wasm, so if this is not the case, use `locateFile` to configure
this properly.
The `PathKit` object returned upon resolution of the .ready() Promise is fully loaded and ready to use.

See the [API page](https://skia.org/user/modules/pathkit) and
[example.html](https://github.com/google/skia/blob/master/modules/pathkit/npm-wasm/example.html)
for details on how to use the library.

Using PathKit and WebPack
-------------------------

WebPack's support for WASM is still somewhat experimental, but PathKit can be
used with a few configuration changes.

In the JS code, use require():

    const PathKitInit = require('pathkit-wasm/bin/pathkit.js')
    PathKitInit().ready().then((PathKit) => {
        // Code goes here using PathKit
    })

Since WebPack does not expose the entire `/node_modules/` directory, but instead
packages only the needed pieces, we have to copy pathkit.wasm into the build directory.
One such solution is to use [CopyWebpackPlugin](https://github.com/webpack-contrib/copy-webpack-plugin).
For example, add the following plugin:

    config.plugins.push(
        new CopyWebpackPlugin([
            { from: 'node_modules/pathkit-wasm/bin/pathkit.wasm' }
        ])
    );

If webpack gives an error similar to:

    ERROR in ./node_modules/pathkit-wasm/bin/pathkit.js
    Module not found: Error: Can't resolve 'fs' in '...'

Then, add the following configuration change to the node section of the config:

    config.node = {
        fs: 'empty'
    };
