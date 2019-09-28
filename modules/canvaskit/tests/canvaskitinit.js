// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

let CanvasKit = null;
const LoadCanvasKit = new Promise(function(resolve, reject) {
    console.log('canvaskit loading', new Date());
    CanvasKitInit({
        locateFile: (file) => '/canvaskit/'+file,
    }).ready().then((loaded) => {
        console.log('canvaskit loaded', new Date());
        CanvasKit = loaded;
        resolve();
    }).catch((e) => {
        console.error('canvaskit failed to load', new Date(), e);
        reject();
    });
});