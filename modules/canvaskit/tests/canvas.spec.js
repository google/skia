describe('CanvasKit\'s Canvas Behavior', function() {
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

    it('can draw directly to a canvas', function(done) {
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
            paint.setStrokeWidth(2.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
            paint.setStyle(CanvasKit.PaintStyle.Stroke);

            canvas.drawLine(3, 10, 30, 15, paint);
            canvas.drawRoundRect(CanvasKit.LTRBRect(5, 35, 45, 80), 15, 10, paint);

            canvas.drawOval(CanvasKit.LTRBRect(5, 35, 45, 80), paint);

            canvas.drawArc(CanvasKit.LTRBRect(55, 35, 95, 80), 15, 270, true, paint);

            const font = new CanvasKit.SkFont(null, 20);
            canvas.drawText('this is ascii text', 5, 100, paint, font);

            const blob = CanvasKit.SkTextBlob.MakeFromText('Unicode chars 💩 é É ص', font);
            canvas.drawTextBlob(blob, 5, 130, paint);

            surface.flush();
            font.delete();
            blob.delete();
            paint.delete();

            reportSurface(surface, 'canvas_api_example', done);
        }));
        // See canvas2d for more API tests
    });

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
            textPaint.setAntiAlias(true);

            const textFont = new CanvasKit.SkFont(null, 30);

            const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], 1);

            paint.setPathEffect(dpe);
            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(5.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

            canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

            canvas.drawPath(path, paint);
            canvas.drawText('This is text', 10, 280, textPaint, textFont);
            surface.flush();

            dpe.delete();
            path.delete();
            paint.delete();
            textFont.delete();
            textPaint.delete();

            reportSurface(surface, 'effect_and_text_example', done);
        }));
    });

    it('returns the depth of the save state stack', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            expect(canvas.getSaveCount()).toEqual(1);
            canvas.save();
            canvas.save();
            canvas.restore();
            canvas.save();
            canvas.save();
            expect(canvas.getSaveCount()).toEqual(4);
            // does nothing, by the SkCanvas API
            canvas.restoreToCount(500);
            expect(canvas.getSaveCount()).toEqual(4);
            canvas.restore();
            expect(canvas.getSaveCount()).toEqual(3);
            canvas.save();
            canvas.restoreToCount(2);
            expect(canvas.getSaveCount()).toEqual(2);

            surface.delete();

            done();
        }));
    });

    it('draws circles', function(done) {
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

            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(5.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.CYAN);

            canvas.clear(CanvasKit.WHITE);

            canvas.drawCircle(30, 50, 15, paint);

            paint.setStyle(CanvasKit.PaintStyle.Fill);
            paint.setColor(CanvasKit.RED);
            canvas.drawCircle(130, 80, 60, paint);
            canvas.drawCircle(20, 150, 60, paint);

            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'circle_canvas', done);
        }));
    });

    it('draws simple rrects', function(done) {
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

            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(3.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.BLACK);

            canvas.clear(CanvasKit.WHITE);

            canvas.drawRRect(CanvasKit.RRectXY(
                CanvasKit.LTRBRect(10, 10, 50, 50), 5, 10), paint);

            canvas.drawRRect(CanvasKit.RRectXY(
                CanvasKit.LTRBRect(60, 10, 110, 50), 10, 5), paint);

            canvas.drawRRect(CanvasKit.RRectXY(
                CanvasKit.LTRBRect(10, 60, 210, 260), 0, 30), paint);

            canvas.drawRRect(CanvasKit.RRectXY(
                CanvasKit.LTRBRect(50, 90, 160, 210), 30, 30), paint);

            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'rrect_canvas', done);
        }));
    });

    it('draws complex rrects', function(done) {
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

            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(3.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.BLACK);

            canvas.clear(CanvasKit.WHITE);

            canvas.drawRRect({
              rect: CanvasKit.LTRBRect(10, 10, 210, 210),
              rx1: 10, // top left corner, going clockwise
              ry1: 30,
              rx2: 30,
              ry2: 10,
              rx3: 50,
              ry3: 75,
              rx4: 120,
              ry4: 120,
            }, paint);

            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'rrect_8corners_canvas', done);
        }));
    });

    it('draws between two rrects', function(done) {
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

            paint.setStyle(CanvasKit.PaintStyle.Fill);
            paint.setStrokeWidth(3.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.BLACK);

            canvas.clear(CanvasKit.WHITE);

            const outer = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 10, 5);
            const inner = CanvasKit.RRectXY(CanvasKit.LTRBRect(50, 90, 160, 210), 30, 30);

            canvas.drawDRRect(outer, inner, paint);

            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'drawDRRect_canvas', done);
        }));
    });

    it('draws with color filters', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();

            const blue = CanvasKit.SkColorFilter.MakeBlend(
                CanvasKit.BLUE, CanvasKit.BlendMode.SrcIn);
            const red =  CanvasKit.SkColorFilter.MakeBlend(
                CanvasKit.Color(255, 0, 0, 0.8), CanvasKit.BlendMode.SrcOver);
            const lerp = CanvasKit.SkColorFilter.MakeLerp(0.6, red, blue);

            paint.setStyle(CanvasKit.PaintStyle.Fill);
            paint.setAntiAlias(true);

            canvas.clear(CanvasKit.Color(230, 230, 230));

            paint.setColorFilter(blue)
            canvas.drawRect(CanvasKit.LTRBRect(10, 10, 60, 60), paint);
            paint.setColorFilter(lerp)
            canvas.drawRect(CanvasKit.LTRBRect(50, 10, 100, 60), paint);
            paint.setColorFilter(red)
            canvas.drawRect(CanvasKit.LTRBRect(90, 10, 140, 60), paint);

            const r = CanvasKit.SkColorMatrix.rotated(0, .707, -.707);
            const b = CanvasKit.SkColorMatrix.rotated(2, .5, .866);
            const s = CanvasKit.SkColorMatrix.scaled(0.9, 1.5, 0.8, 0.8);
            let cm = CanvasKit.SkColorMatrix.concat(r, s);
            cm = CanvasKit.SkColorMatrix.concat(cm, b);
            CanvasKit.SkColorMatrix.postTranslate(cm, 20, 0, -10, 0);

            const mat = CanvasKit.SkColorFilter.MakeMatrix(cm);
            const final = CanvasKit.SkColorFilter.MakeCompose(mat, lerp);

            paint.setColorFilter(final)
            canvas.drawRect(CanvasKit.LTRBRect(10, 70, 140, 120), paint);

            surface.flush();
            paint.delete();
            blue.delete();
            red.delete();
            lerp.delete();
            final.delete();

            reportSurface(surface, 'colorfilters_canvas', done);
        }));
    });

    it('can use Malloc to save a copy', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();

            const src = [
                 0.8,   0.45,      2,   0,  20,
                0.53, -0.918,  0.566,   0,   0,
                0.53, -0.918, -0.566,   0, -10,
                   0,      0,      0, 0.8,   0,
            ]
            const cm = new CanvasKit.Malloc(Float32Array, 20);
            for (i in src) {
                cm[i] = src[i];
            }
            const final = CanvasKit.SkColorFilter.MakeMatrix(cm);

            paint.setColorFilter(final)
            canvas.drawRect(CanvasKit.LTRBRect(10, 70, 140, 120), paint);

            surface.flush()
            paint.delete();
            final.delete();

            reportSurface(surface, 'colorfilters_malloc_canvas', done);
        }));
    });

    it('can clip using rrect and path', function(done) {
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
            paint.setColor(CanvasKit.BLUE);
            const rrect = CanvasKit.RRectXY(CanvasKit.LTRBRect(300, 300, 500, 500), 40, 40);

            canvas.save();
            // draw magenta around the outside edge of an rrect.
            canvas.clipRRect(rrect, CanvasKit.ClipOp.Difference, true);
            canvas.drawColor(CanvasKit.Color(250, 30, 240, 0.9), CanvasKit.BlendMode.SrcOver);
            canvas.restore();

            // draw grey inside of a star pattern, then the blue star on top
            canvas.clipPath(path, CanvasKit.ClipOp.Intersect, false);
            canvas.drawColor(CanvasKit.Color(200, 200, 200, 1.0), CanvasKit.BlendMode.SrcOver);
            canvas.drawPath(path, paint);

            surface.flush();
            path.delete();

            reportSurface(surface, 'clips_canvas', done);
        }));
    });

    it('can save layer with SaveLayerRec-like things', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            // Note: fiddle.skia.org quietly draws a white background before doing
            // other things, which is noticed in cases like this where we use saveLayer
            // with the rec struct.
            canvas.clear(CanvasKit.WHITE);
            canvas.scale(8, 8);
            const redPaint = new CanvasKit.SkPaint();
            redPaint.setColor(CanvasKit.RED);
            redPaint.setAntiAlias(true);
            canvas.drawCircle(21, 21, 8, redPaint);

            const bluePaint = new CanvasKit.SkPaint();
            bluePaint.setColor(CanvasKit.BLUE);
            canvas.drawCircle(31, 21, 8, bluePaint);

            const blurIF = CanvasKit.SkImageFilter.MakeBlur(8, 0.2, CanvasKit.TileMode.Decal, null);

            const count = canvas.saveLayer(null, blurIF, 0);
            expect(count).toEqual(1);
            canvas.scale(1/4, 1/4);
            canvas.drawCircle(125, 85, 8, redPaint);
            canvas.restore();

            surface.flush();
            blurIF.delete();
            redPaint.delete();
            bluePaint.delete();

            reportSurface(surface, 'savelayerrec_canvas', done);
        }));
    });

    it('can drawPoints', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();
            paint.setAntiAlias(true);
            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(10);
            paint.setColor(CanvasKit.Color(153, 204, 162, 0.82));

            const points = [[32, 16], [48, 48], [16, 32]];

            const caps = [CanvasKit.StrokeCap.Round, CanvasKit.StrokeCap.Square,
                          CanvasKit.StrokeCap.Butt];
            const joins = [CanvasKit.StrokeJoin.Round, CanvasKit.StrokeJoin.Miter,
                           CanvasKit.StrokeJoin.Bevel];
            const modes = [CanvasKit.PointMode.Points, CanvasKit.PointMode.Lines,
                           CanvasKit.PointMode.Polygon];

            for (let i = 0; i < caps.length; i++) {
                paint.setStrokeCap(caps[i]);
                paint.setStrokeJoin(joins[i]);

                for (const m of modes) {
                    canvas.drawPoints(m, points, paint);
                    canvas.translate(64, 0);
                }
                // Try with the malloc approach. Note that the drawPoints
                // will free the pointer when done.
                const mPoints = CanvasKit.Malloc(Float32Array, 3*2);
                mPoints.set([32, 16, 48, 48, 16, 32]);
                canvas.drawPoints(CanvasKit.PointMode.Polygon, mPoints, paint);
                canvas.translate(-192, 64);
            }

            surface.flush();
            paint.delete();

            reportSurface(surface, 'drawpoints_canvas', done);
        }));
    });

    it('can stretch an image with drawImageNine', function(done) {
        const imgPromise = fetch('/assets/mandrill_512.png')
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const pngData = values[0];
            expect(pngData).toBeTruthy();
            catchException(done, () => {
                let img = CanvasKit.MakeImageFromEncoded(pngData);
                expect(img).toBeTruthy();
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface')
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                canvas.clear(CanvasKit.WHITE);
                const paint = new CanvasKit.SkPaint();

                canvas.drawImageNine(img, {
                    fLeft: 40,
                    fTop: 40,
                    fRight: 400,
                    fBottom: 300,
                }, CanvasKit.LTRBRect(5, 5, 300, 650), paint);
                surface.flush();
                paint.delete();
                img.delete();

                reportSurface(surface, 'drawImageNine_canvas', done);
            })();
        });
    });
});
