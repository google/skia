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
            <canvas width=600 height=600 id=test></canvas>`;
    });

    afterEach(function() {
        container.innerHTML = '';
    });

    describe('color string parsing', function() {
        function hex(s) {
            return parseInt(s, 16);
        }

        it('parses hex color strings', function(done) {
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
                done();
            }));
        });
        it('parses rgba color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                let parseColor = CanvasKit._testing.parseColor;
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
        it('parses named color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                let parseColor = CanvasKit._testing.parseColor;
                expect(parseColor('grey')).toEqual(
                    CanvasKit.Color(128, 128, 128, 1.0));
                expect(parseColor('blanchedalmond')).toEqual(
                    CanvasKit.Color(255, 235, 205, 1.0));
                expect(parseColor('transparent')).toEqual(
                    CanvasKit.Color(0, 0, 0, 0));
                done();
            }));
        });
    }); // end describe('color string parsing')

    function multipleCanvasTest(testname, done, test) {
        if (CanvasKit.gpu) {
            // TODO(kjlubick): add Software backend to GPU build skia:8548
            console.log(`SKIPPING ${testname} on GPU`);
            done();
            return;
        }
        const skcanvas = CanvasKit.MakeCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
        skcanvas._config = 'software_canvas';
        const realCanvas = document.getElementById('test');
        realCanvas._config = 'html_canvas';
        realCanvas.width = CANVAS_WIDTH;
        realCanvas.height = CANVAS_HEIGHT;

        let promises = [];

        for (let canvas of [skcanvas, realCanvas]) {
            test(canvas);
            // canvas has .toDataURL (even though skcanvas is not a real Canvas)
            // so this will work.
            promises.push(reportCanvas(canvas, 'all_path_operations', canvas._config));
        }
        Promise.all(promises).then(() => {
            skcanvas.dispose();
            done();
        }).catch(reportError(done));
    }

    describe('Path drawing API', function() {
        it('supports all the line types', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('all_line_drawing_operations', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    // TODO(kjlubick): scale doesn't work quite right yet, but putting
                    // it before all draws makes it consistent for now.
                    ctx.scale(3.0, 3.0);
                    ctx.moveTo(20, 5);
                    ctx.lineTo(30, 20);
                    ctx.lineTo(40, 10);
                    ctx.lineTo(50, 20);
                    ctx.lineTo(60, 0);
                    ctx.lineTo(20, 5);

                    ctx.moveTo(20, 80);
                    ctx.bezierCurveTo(90, 10, 160, 150, 190, 10);

                    ctx.moveTo(36, 148);
                    ctx.quadraticCurveTo(66, 188, 120, 136);
                    ctx.lineTo(36, 148);

                    ctx.rect(5, 170, 20, 25);

                    ctx.moveTo(150, 180);
                    ctx.arcTo(150, 100, 50, 200, 20);
                    ctx.lineTo(160, 160);

                    ctx.moveTo(20, 120);
                    ctx.arc(20, 120, 18, 0, 1.75 * Math.PI);
                    ctx.lineTo(20, 120);

                    ctx.moveTo(150, 5);
                    ctx.ellipse(130, 25, 30, 10, -1*Math.PI/8, Math.PI/6, 1.5*Math.PI)

                    ctx.lineWidth = 2;
                    ctx.stroke();
                });
            }));
        });
    }); // end describe('Path drawing API')


});
