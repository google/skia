// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

describe('CanvasKit\'s Canvas 2d Behavior', function() {
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

    describe('color string parsing', function() {
        function hex(s) {
            return parseInt(s, 16);
        }

        it('can draw a path', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                let parseColor = CanvasKit._testing.parseColor;
                expect(parseColor('#FED')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), 1));
                expect(parseColor('#FEDC')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), hex('CC')/255));
                expect(parseColor('#fed')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), 1));
                expect(parseColor('#fedc')).toEqual(
                    CanvasKit.Color(hex('FF'), hex('EE'), hex('DD'), hex('CC')/255));
                expect(parseColor('rgba(117, 33, 64, 0.75)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 0.75));
                expect(parseColor('rgb(117, 33, 64, 0.75)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 0.75));
                expect(parseColor('rgba(117,33,64)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 1.0));
                expect(parseColor('rgb(117,33, 64)')).toEqual(
                    CanvasKit.Color(117, 33, 64, 1.0));
                done();
            }));
        });
    }); //end describe('color string parsing')


});
