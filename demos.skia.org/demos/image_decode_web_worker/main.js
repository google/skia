// Inspired by https://gist.github.com/ahem/d19ee198565e20c6f5e1bcd8f87b3408
const worker = new Worker('worker.js');

const canvasKitInitPromise =
    CanvasKitInit({locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.25.0/bin/full/'+file});

const bigImagePromise =
    fetch('https://upload.wikimedia.org/wikipedia/commons/3/30/Large_Gautama_Buddha_statue_in_Buddha_Park_of_Ravangla%2C_Sikkim.jpg')
    .then((response) => response.blob());

Promise.all([
    canvasKitInitPromise,
    bigImagePromise
]).then(([
    CanvasKit,
    imageBlob
]) => {
    const surface = CanvasKit.MakeWebGLCanvasSurface('main-thread-canvas', null);
    if (!surface) {
        throw 'Could not make main thread canvas surface';
    }

    const paint = new CanvasKit.Paint();
    paint.setColor(CanvasKit.RED);

    let decodedImage;
    // This animation draws a red circle oscillating from the center of the canvas.
    // It is there to show the lag introduced by decoding the image on the main
    // thread.
    const drawFrame = (canvas) => {
        canvas.clear(CanvasKit.WHITE);

        if (decodedImage) {
            canvas.drawImageRect(decodedImage,
                CanvasKit.LTRBRect(0, 0, 3764, 5706), // original size of the image
                CanvasKit.LTRBRect(0, 0, 500, 800), // scaled down
                null); // no paint needed
        }
        canvas.drawCircle(250, 250, 200 * Math.abs(Math.sin(Date.now() / 1000)), paint);
        surface.requestAnimationFrame(drawFrame);
    };
    surface.requestAnimationFrame(drawFrame);


    document.getElementById('load-button-main').addEventListener('click', () => {
        if (decodedImage) {
            decodedImage.delete();
            decodedImage = null;
        }
        const imgBitmapPromise = createImageBitmap(imageBlob);
        imgBitmapPromise.then((imgBitmap) => {
            decodedImage = CanvasKit.MakeImageFromCanvasImageSource(imgBitmap);
        });
    });

    document.getElementById('load-button-web').addEventListener('click', () => {
        if (decodedImage) {
            decodedImage.delete();
            decodedImage = null;
        }
        worker.postMessage(imageBlob);
    });
    worker.addEventListener('message', (e) => {
        const decodedBuffer = e.data.decodedArrayBuffer;
        const pixels = new Uint8Array(decodedBuffer);
        decodedImage = CanvasKit.MakeImage({
            width: e.data.width,
            height: e.data.height,
            alphaType: CanvasKit.AlphaType.Unpremul,
            colorType: CanvasKit.ColorType.RGBA_8888,
            colorSpace: CanvasKit.ColorSpace.SRGB
        }, pixels, 4 * e.data.width);
    });
    document.getElementById('clear-button').addEventListener('click', () => {
        if (decodedImage) {
            decodedImage.delete();
            decodedImage = null;
        }
    });
});


