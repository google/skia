// Set up worker and send it a message with the canvas to draw to.
const offscreenCanvas = document.getElementById('offscreen-canvas').transferControlToOffscreen();
const worker = new Worker('worker.js');
worker.postMessage({ offscreenCanvas }, [offscreenCanvas]);

const canvasKitInitPromise =
    CanvasKitInit({locateFile: (file) => '../../../out/canvaskit_wasm/'+file});
const skottieJsonPromise =
    fetch('https://storage.googleapis.com/skia-cdn/misc/lego_loader.json')
    .then((response) => response.text());

Promise.all([
    canvasKitInitPromise,
    skottieJsonPromise
]).then(([
    CanvasKit,
    jsonStr
]) => {
    const onscreenCanvas = document.getElementById('onscreen-canvas').transferControlToOffscreen();
    CanvasKit.GetWebGLContext(onscreenCanvas, {antialias: true});
    const surface = CanvasKit.MakeWebGLCanvasSurface(onscreenCanvas, null);
    if (!surface) {
        throw 'Could not make canvas surface';
    }

    SkottieExample(CanvasKit, surface, jsonStr);
});

document.getElementById('busy-button').addEventListener('click', () => {
    const startTime = performance.now();
    while (performance.now() - startTime < 600) {
    }
});