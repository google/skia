// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

describe('CanvasKit\'s Path Behavior', function() {
    // Note, don't try to print the CanvasKit object - it can cause Karma/Jasmine to lock up.
    var CanvasKit = null;
    const LoadCanvasKit = new Promise(function(resolve, reject) {
        if (CanvasKit) {
            resolve();
        } else {
            CanvasKitInit({
                locateFile: (file) => '/canvaskit/'+file,
            }).then((_CanvasKit) => {
                CanvasKit = _CanvasKit;
                CanvasKit.initFonts();
                resolve();
            });
        }
    });

    let container = document.createElement('div');
    document.body.appendChild(container);
    const CANVAS_WIDTH = 600;
    const CANVAS_HEIGHT = 600;

    beforeEach(function() {
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
    });

    afterEach(function() {
        container.innerHTML = '';
    });

    function reportSurface(surface, testname, done) {
        // In docker, the webgl canvas is blank, but the surface has the pixel
        // data. So, we copy it out and draw it to a normal canvas to take a picture.
        // To be consistent across CPU and GPU, we just do it for all configurations
        // (even though the CPU canvas shows up after flush just fine).
        let pixelLen = CANVAS_WIDTH * CANVAS_HEIGHT * 4; // 4 bytes for r,g,b,a
        let pixelPtr = CanvasKit._malloc(pixelLen);
        let success = surface._readPixels(CANVAS_WIDTH, CANVAS_HEIGHT, pixelPtr);
        if (!success) {
            done();
            expect(success).toBeFalsy('could not read pixels');
            return;
        }
        let pixels = new Uint8ClampedArray(CanvasKit.buffer, pixelPtr, pixelLen);
        var imageData = new ImageData(pixels, CANVAS_WIDTH, CANVAS_HEIGHT);

        let reportingCanvas =  document.getElementById('report');
        reportingCanvas.getContext('2d').putImageData(imageData, 0, 0);
        CanvasKit._free(pixelPtr);
        reportCanvas(reportingCanvas, testname).then(() => {
            done();
        }).catch(reportError(done));
    }

    it('can draw a path', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            // This is taken from example.html
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();
            paint.setStrokeWidth(1.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
            paint.setStyle(CanvasKit.PaintStyle.Stroke);

            const path = new CanvasKit.SkPath();
            path.moveTo(20, 5);
            path.lineTo(30, 20);
            path.lineTo(40, 10);
            path.lineTo(50, 20);
            path.lineTo(60, 0);
            path.lineTo(20, 5);

            path.moveTo(20, 80);
            path.cubicTo(90, 10, 160, 150, 190, 10);

            path.moveTo(36, 148);
            path.quadTo(66, 188, 120, 136);
            path.lineTo(36, 148);

            path.moveTo(150, 180);
            path.arcTo(150, 100, 50, 200, 20);
            path.lineTo(160, 160);

            path.moveTo(20, 120);
            path.lineTo(20, 120);

            path.transform([2, 0, 0,
                            0, 2, 0,
                            0, 0, 1 ])

            canvas.drawPath(path, paint);
            surface.flush();

            path.delete();
            paint.delete();

            reportSurface(surface, 'path_api_example', done);
        }));
        // See CanvasKit for more tests, since they share implementation
    });

    function starPath(CanvasKit, X=128, Y=128, R=116) {
        let p = new CanvasKit.SkPath();
        p.moveTo(X + R, Y);
        for (let i = 1; i < 8; i++) {
          let a = 2.6927937 * i;
          p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
        }
        return p;
      }

    it('can apply an effect and draw text', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const path = starPath(CanvasKit);

            const paint = new CanvasKit.SkPaint();

            const textPaint = new CanvasKit.SkPaint();
            textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
            textPaint.setTextSize(30);
            textPaint.setAntiAlias(true);

            const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], 1);

            paint.setPathEffect(dpe);
            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(5.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

            canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

            canvas.drawPath(path, paint);
            canvas.drawText('This is text', 10, 280, textPaint);
            surface.flush();
            dpe.delete();
            path.delete();

            reportSurface(surface, 'effect_and_text_example', done);
        }));
    });
});
