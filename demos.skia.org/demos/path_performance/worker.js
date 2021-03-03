importScripts('https://unpkg.com/canvaskit-wasm@0.25.0/bin/full/canvaskit.js');
importScripts('shared.js');

const CanvasKitPromise =
    CanvasKitInit({locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.25.0/bin/full/'+file});

const path2dAnimator = new Animator();
const canvasKitAnimator = new Animator();
addEventListener('message', async ({ data: {svgData, offscreenCanvas, type, switchMethod} }) => {
    // The worker expect to receive 2 messages after initialization: One with an offscreenCanvas
    // for Path2D rendering, and one with an offscreenCanvas for CanvasKit rendering.
    if (svgData && offscreenCanvas && type) {
        if (type === 'Path2D') {
            path2dAnimator.renderer =
                new Path2dRenderer(svgData, offscreenCanvas);
        }
        if (type === 'CanvasKit') {
            const CanvasKit = await CanvasKitPromise;
            canvasKitAnimator.renderer =
                new CanvasKitRenderer(svgData, offscreenCanvas, CanvasKit);
        }
    }
    // The worker receives a "switchMethod" message whenever the user clicks a rendering
    // method button on the web page.
    if (switchMethod) {
        // Pause other renderers and start correct renderer
        canvasKitAnimator.stop();
        path2dAnimator.stop();

        if (switchMethod === 'Path2D') {
            path2dAnimator.start();
        } else if (switchMethod === 'CanvasKit') {
            canvasKitAnimator.start();
        }
    }
});

// Report framerates of Path2D and CanvasKit rendering back to main.js
setInterval(() => {
    if (path2dAnimator.framesCount > 0) {
        postMessage({
            renderMethod: 'Path2D',
            framesCount: path2dAnimator.framesCount,
            totalFramesMs: path2dAnimator.totalFramesMs
        });
    }
    if (canvasKitAnimator.framesCount > 0) {
        postMessage({
            renderMethod: 'CanvasKit',
            framesCount: canvasKitAnimator.framesCount,
            totalFramesMs: canvasKitAnimator.totalFramesMs
        });
    }
}, 1000);
