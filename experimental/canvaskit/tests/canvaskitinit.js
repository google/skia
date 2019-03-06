// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

let CanvasKit = null;
const LoadCanvasKit = new Promise(function(resolve, reject) {
    CanvasKitInit({
        locateFile: (file) => '/canvaskit/'+file,
    }).ready().then((loaded) => {
        CanvasKit = loaded;
        resolve();
    });
});