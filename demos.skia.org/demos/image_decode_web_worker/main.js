// Inspired by https://gist.github.com/ahem/d19ee198565e20c6f5e1bcd8f87b3408
const worker = new Worker('worker.js');

const canvasKitInitPromise =
    CanvasKitInit({locateFile: (file) => 'https://particles.skia.org/static/'+file});

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
    const mtSurface = CanvasKit.MakeWebGLCanvasSurface('main-thread-canvas', null);
    if (!mtSurface) {
        throw 'Could not make main thread canvas surface';
    }

    const mtPaint = new CanvasKit.SkPaint();
    mtPaint.setColor(CanvasKit.RED);

    let decodedImage;
    const drawMTFrame = (canvas) => {
        canvas.clear(CanvasKit.WHITE);

        if (decodedImage) {
            canvas.drawImageRect(decodedImage, CanvasKit.LTRBRect(0, 0, 3764, 5706), CanvasKit.LTRBRect(0, 0, 500, 800), null);
        }
        canvas.drawCircle(250, 250, 200 * Math.abs(Math.sin(Date.now() / 1000)), mtPaint);
        mtSurface.requestAnimationFrame(drawMTFrame);
    };
    mtSurface.requestAnimationFrame(drawMTFrame);


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
        decodedImage = CanvasKit.MakeImage(pixels, e.data.width, e.data.height,
            CanvasKit.AlphaType.Unpremul, CanvasKit.ColorType.RGBA_8888, CanvasKit.SkColorSpace.SRGB);
    });
    document.getElementById('clear-button').addEventListener('click', () => {
        if (decodedImage) {
            decodedImage.delete();
            decodedImage = null;
        }
    });
});


