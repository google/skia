A WASM version of Skia's PathOps toolkit.

To use the library, run `npm install experimental-pathkit-wasm` and then simply include it:

    <script src="/node_modules/experimental-pathkit-wasm/bin/pathkit.js"></script>
    PathKitInit({
        locateFile: (file) => '/node_modules/experimental-pathkit-wasm/bin/'+file,
    }).then((PathKit) => {
        // Code goes here using PathKit
    });

See example.html for a fuller example of how to use the library.

Using PathKit and WebPack
-------------------------

WebPack's support for WASM is still somewhat experimental, but PathKit can be
used with a few configuration changes.

In the JS code, use require():

    const PathKitInit = require('experimental-pathkit-wasm/bin/pathkit.js')
    PathKitInit().then((PathKit) => {
        // Code goes here using PathKit
    })

Since WebPack does not expose the entire `/node_modules/` directory, but instead
packages only the needed pieces, we have to copy pathkit.wasm into the build directory.
One such solution is to use [CopyWebpackPlugin](https://github.com/webpack-contrib/copy-webpack-plugin)
For example, add the following plugin:

    config.plugins.push(
        new CopyWebpackPlugin([
            { from: 'node_modules/experimental-pathkit-wasm/bin/pathkit.wasm' }
        ])
    );

If webpack gives an error similar to:

    ERROR in ./node_modules/experimental-pathkit-wasm/bin/pathkit.js
    Module not found: Error: Can't resolve 'fs' in '...'

Then, add the following configuration change to the node section of the config:

    config.node = {
        fs: 'empty'
    };