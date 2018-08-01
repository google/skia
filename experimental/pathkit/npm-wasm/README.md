A WASM version of Skia's PathOps toolkit.

To use the library, run `npm install experimental-pathkit-wasm` and then simply include it:

    <script src="/node_modules/experimental-pathkit-wasm/bin/pathkit.js"></script>
    PathKitInit({
        locateFile: (file) => '/node_modules/experimental-pathkit-wasm/bin/'+file,
    }).then((PathKit) => {
        // Code goes here using PathKit
    });

See example.html for a fuller example of how to use the library.