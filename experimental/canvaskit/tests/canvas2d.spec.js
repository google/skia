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

    describe('color strings', function() {
        function hex(s) {
            return parseInt(s, 16);
        }

        it('parses hex color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const parseColor = CanvasKit._testing.parseColor;
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
                const parseColor = CanvasKit._testing.parseColor;
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
                const parseColor = CanvasKit._testing.parseColor;
                expect(parseColor('grey')).toEqual(
                    CanvasKit.Color(128, 128, 128, 1.0));
                expect(parseColor('blanchedalmond')).toEqual(
                    CanvasKit.Color(255, 235, 205, 1.0));
                expect(parseColor('transparent')).toEqual(
                    CanvasKit.Color(0, 0, 0, 0));
                done();
            }));
        });

        it('properly produces color strings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const colorToString = CanvasKit._testing.colorToString;

                expect(colorToString(CanvasKit.Color(102, 51, 153, 1.0))).toEqual('#663399');

                expect(colorToString(CanvasKit.Color(255, 235, 205, 0.5))).toEqual(
                                               'rgba(255, 235, 205, 0.50196078)');

                done();
            }));
        });

        it('can multiply colors by alpha', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const multiplyByAlpha = CanvasKit.multiplyByAlpha;

                const testCases = [
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 1.0),
                        inAlpha:  1.0,
                        outColor: CanvasKit.Color(102, 51, 153, 1.0),
                    },
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 1.0),
                        inAlpha:  0.8,
                        outColor: CanvasKit.Color(102, 51, 153, 0.8),
                    },
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 0.8),
                        inAlpha:  0.7,
                        outColor: CanvasKit.Color(102, 51, 153, 0.56),
                    },
                    {
                        inColor:  CanvasKit.Color(102, 51, 153, 0.8),
                        inAlpha:  1000,
                        outColor: CanvasKit.Color(102, 51, 153, 1.0),
                    },
                ];

                for (let tc of testCases) {
                    // Print out the test case if the two don't match.
                    expect(multiplyByAlpha(tc.inColor, tc.inAlpha))
                          .toBe(tc.outColor, JSON.stringify(tc));
                }

                done();
            }));
        });
    }); // end describe('color string parsing')

    function multipleCanvasTest(testname, done, test) {
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
            promises.push(reportCanvas(canvas, testname, canvas._config));
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

        it('handles all the transforms as specified', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('all_matrix_operations', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    ctx.rect(10, 10, 20, 20);

                    ctx.scale(2.0, 4.0);
                    ctx.rect(30, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.rotate(Math.PI / 3);
                    ctx.rect(50, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.translate(30, -2);
                    ctx.rect(70, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.translate(60, 0);
                    ctx.rotate(Math.PI / 6);
                    ctx.transform(1.5, 0, 0, 0.5, 0, 0, 0); // effectively scale
                    ctx.rect(90, 10, 20, 20);
                    ctx.resetTransform();

                    ctx.setTransform(2, 0, -.5, 2.5, -40, 120);
                    ctx.rect(110, 10, 20, 20);
                    ctx.lineTo(110, 0);
                    ctx.resetTransform();
                    ctx.lineTo(220, 120);

                    ctx.scale(3.0, 3.0);
                    ctx.font = '6pt Arial';
                    ctx.fillText('This text should be huge', 10, 80);
                    ctx.resetTransform();

                    ctx.strokeStyle = 'black';
                    ctx.lineWidth = 2;
                    ctx.stroke();

                    ctx.beginPath();
                    ctx.moveTo(250, 30);
                    ctx.lineTo(250, 80);
                    ctx.scale(3.0, 3.0);
                    ctx.lineTo(280/3, 90/3);
                    ctx.closePath();
                    ctx.strokeStyle = 'black';
                    ctx.lineWidth = 5;
                    ctx.stroke();
                });
            }));
        });

        it('properly saves and restores states, even when drawing shadows', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('shadows_and_save_restore', done, (canvas) => {
                  let ctx = canvas.getContext('2d');
                  ctx.strokeStyle = '#000';
                  ctx.fillStyle = '#CCC';
                  ctx.shadowColor = 'rebeccapurple';
                  ctx.shadowBlur = 1;
                  ctx.shadowOffsetX = 3;
                  ctx.shadowOffsetY = -8;
                  ctx.rect(10, 10, 30, 30);

                  ctx.save();
                  ctx.strokeStyle = '#C00';
                  ctx.fillStyle = '#00C';
                  ctx.shadowBlur = 0;
                  ctx.shadowColor = 'transparent';

                  ctx.stroke();

                  ctx.restore();
                  ctx.fill();

                  ctx.beginPath();
                  ctx.moveTo(36, 148);
                  ctx.quadraticCurveTo(66, 188, 120, 136);
                  ctx.closePath();
                  ctx.stroke();

                  ctx.beginPath();
                  ctx.shadowColor = '#993366AA';
                  ctx.shadowOffsetX = 8;
                  ctx.shadowBlur = 5;
                  ctx.setTransform(2, 0, -.5, 2.5, -40, 120);
                  ctx.rect(110, 10, 20, 20);
                  ctx.lineTo(110, 0);
                  ctx.resetTransform();
                  ctx.lineTo(220, 120);
                  ctx.stroke();

                  ctx.fillStyle = 'green';
                  ctx.font = '16pt Arial';
                  ctx.fillText('This should be shadowed', 20, 80);
                });
            }));
        });

        it('fills/strokes rects and supports some global settings', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                multipleCanvasTest('global_dashed_rects', done, (canvas) => {
                    let ctx = canvas.getContext('2d');
                    ctx.scale(1.1, 1.1);
                    ctx.translate(10, 10);
                    // Shouldn't impact the fillRect calls
                    ctx.setLineDash([5, 3]);

                    ctx.fillStyle = 'rgba(200, 0, 100, 0.81)';
                    ctx.fillRect(20, 30, 100, 100);

                    ctx.globalAlpha = 0.81;
                    ctx.fillStyle = 'rgba(200, 0, 100, 1.0)';
                    ctx.fillRect(120, 30, 100, 100);
                    // This shouldn't do anything
                    ctx.globalAlpha = 0.1;

                    ctx.fillStyle = 'rgba(200, 0, 100, 0.9)';
                    ctx.globalAlpha = 0.9;
                    // Intentional no-op to check ordering
                    ctx.clearRect(220, 30, 100, 100);
                    ctx.fillRect(220, 30, 100, 100);

                    ctx.fillRect(320, 30, 100, 100);
                    ctx.clearRect(330, 40, 80, 80);

                    ctx.strokeStyle = 'blue';
                    ctx.lineWidth = 3;
                    ctx.setLineDash([5, 3]);
                    ctx.strokeRect(20, 150, 100, 100);
                    ctx.setLineDash([50, 30]);
                    ctx.strokeRect(125, 150, 100, 100);
                    ctx.lineDashOffset = 25;
                    ctx.strokeRect(230, 150, 100, 100);
                    ctx.setLineDash([2, 5, 9]);
                    ctx.strokeRect(335, 150, 100, 100);

                    ctx.setLineDash([5, 2]);
                    ctx.moveTo(336, 400);
                    ctx.quadraticCurveTo(366, 488, 120, 450);
                    ctx.lineTo(300, 400);
                    ctx.stroke();

                    ctx.font = '36pt Arial';
                    ctx.strokeText('Dashed', 20, 350);
                    ctx.fillText('Not Dashed', 20, 400);
                });
            }));
        });

        it('can read default properties', function(done) {
            LoadCanvasKit.then(catchException(done, () => {
                const skcanvas = CanvasKit.MakeCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
                const realCanvas = document.getElementById('test');
                realCanvas.width = CANVAS_WIDTH;
                realCanvas.height = CANVAS_HEIGHT;

                const skcontext = skcanvas.getContext('2d');
                const realContext = realCanvas.getContext('2d');

                const toTest = ['font', 'lineWidth', 'strokeStyle', 'lineCap',
                                'lineJoin', 'miterLimit', 'shadowOffsetY',
                                'shadowBlur', 'shadowColor', 'shadowOffsetX',
                                'globalAlpha', 'globalCompositeOperation',
                                'lineDashOffset'];

                // Compare all the default values of the properties of skcanvas
                // to the default values on the properties of a real canvas.
                for( let attr of toTest) {
                    expect(skcontext[attr]).toBe(realContext[attr], attr);
                }

                skcanvas.dispose();
                done();
            }));
        });
    }); // end describe('Path drawing API')


});
