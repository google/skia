describe('CanvasKit\'s Path Behavior', function() {
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

            let rrect = new CanvasKit.SkPath()
                               .addRoundRect(100, 10, 140, 62,
                                             10, 4, true);

            canvas.drawPath(rrect, paint);
            rrect.delete();

            surface.flush();

            path.delete();
            paint.delete();

            reportSurface(surface, 'path_api_example', done);
        }));
        // See PathKit for more tests, since they share implementation
    });

    it('can create a path from an SVG string', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            //.This is a parallelagram from
            // https://upload.wikimedia.org/wikipedia/commons/e/e7/Simple_parallelogram.svg
            let path = CanvasKit.MakePathFromSVGString('M 205,5 L 795,5 L 595,295 L 5,295 L 205,5 z');

            let cmds = path.toCmds();
            expect(cmds).toBeTruthy();
            // 1 move, 4 lines, 1 close
            // each element in cmds is an array, with index 0 being the verb, and the rest being args
            expect(cmds.length).toBe(6);
            expect(cmds).toEqual([[CanvasKit.MOVE_VERB, 205, 5],
                                  [CanvasKit.LINE_VERB, 795, 5],
                                  [CanvasKit.LINE_VERB, 595, 295],
                                  [CanvasKit.LINE_VERB, 5, 295],
                                  [CanvasKit.LINE_VERB, 205, 5],
                                  [CanvasKit.CLOSE_VERB]]);
            path.delete();
            done();
        }));
    });

    it('can create an SVG string from a path', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            let cmds = [[CanvasKit.MOVE_VERB, 205, 5],
                       [CanvasKit.LINE_VERB, 795, 5],
                       [CanvasKit.LINE_VERB, 595, 295],
                       [CanvasKit.LINE_VERB, 5, 295],
                       [CanvasKit.LINE_VERB, 205, 5],
                       [CanvasKit.CLOSE_VERB]];
            let path = CanvasKit.MakePathFromCmds(cmds);

            let svgStr = path.toSVGString();
            // We output it in terse form, which is different than Wikipedia's version
            expect(svgStr).toEqual('M205 5L795 5L595 295L5 295L205 5Z');
            path.delete();
            done();
        }));
    });

    it('uses offset to transform the path with dx,dy', function(done) {
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
            paint.setColor(CanvasKit.BLACK);

            canvas.clear(CanvasKit.WHITE);

            canvas.drawPath(path, paint);
            path.offset(80, 40);
            canvas.drawPath(path, paint);
            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'offset_path', done);
        }));
    });

    it('draws ovals', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();

            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(5.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.BLACK);

            canvas.clear(CanvasKit.WHITE);

            const path = new CanvasKit.SkPath();
            path.moveTo(5, 5);
            path.lineTo(10, 120);
            path.addOval(CanvasKit.LTRBRect(10, 20, 100, 200), false, 3);
            path.lineTo(300, 300);

            canvas.drawPath(path, paint);
            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'oval_path', done);
        }));
    });

    it('draws arcTo in a multitude of ways', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();

            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(5.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.BLACK);

            canvas.clear(CanvasKit.WHITE);

            const path = new CanvasKit.SkPath();
            //path.moveTo(5, 5);
            // takes 4, 5 or 7 args
            // - 5 x1, y1, x2, y2, radius
            path.arcTo(40, 0, 40, 40, 40);
            // - 4 oval (as Rect), startAngle, sweepAngle, forceMoveTo
            path.arcTo(CanvasKit.LTRBRect(90, 10, 120, 200), 30, 300, true);
            // - 7 rx, ry, xAxisRotate, useSmallArc, isCCW, x, y
            path.moveTo(5, 105);
            path.arcTo(24, 24, 45, true, false, 82, 156);

            canvas.drawPath(path, paint);
            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'arcto_path', done);
        }));
    });

    it('can draw a path using relative functions', function(done) {
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
            path.rMoveTo(20, 5)
                .rLineTo(10, 15)  // 30, 20
                .rLineTo(10, -5);  // 40, 10
            path.rLineTo(10, 10);  // 50, 20
            path.rLineTo(10, -20); // 60, 0
            path.rLineTo(-40, 5);  // 20, 5

            path.moveTo(20, 80)
                .rCubicTo(70, -70, 140, 70, 170, -70); // 90, 10, 160, 150, 190, 10

            path.moveTo(36, 148)
                .rQuadTo(30, 40, 84, -12) // 66, 188, 120, 136
                .lineTo(36, 148);

            path.moveTo(150, 180)
                .rArcTo(24, 24, 45, true, false, -68, -24); // 82, 156
            path.lineTo(160, 160);

            canvas.drawPath(path, paint);

            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'path_relative', done);
        }));
    });

    it('can measure a path', function(done) {
        LoadCanvasKit.then(catchException(done, () => {

            const path = new CanvasKit.SkPath();
            path.moveTo(10, 10)
                .lineTo(40, 50); // should be length 50 because of the 3/4/5 triangle rule

            path.moveTo(80, 0)
                .lineTo(80, 10)
                .lineTo(100, 5)
                .lineTo(80, 0);

            const meas = new CanvasKit.SkPathMeasure(path, false, 1);
            expect(meas.getLength()).toBeCloseTo(50.0, 3);
            const pt = meas.getPosTan(28.7); // arbitrary point
            expect(pt[0]).toBeCloseTo(27.22, 3); // x
            expect(pt[1]).toBeCloseTo(32.96, 3); // y
            expect(pt[2]).toBeCloseTo(0.6, 3);   // dy
            expect(pt[3]).toBeCloseTo(0.8, 3);   // dy
            const subpath = meas.getSegment(20, 40, true); // make sure this doesn't crash

            expect(meas.nextContour()).toBeTruthy();
            expect(meas.getLength()).toBeCloseTo(51.231, 3);

            expect(meas.nextContour()).toBeFalsy();

            path.delete();
            done();
        }));
    });

    it('can measure the contours of a path', function(done) {
        LoadCanvasKit.then(catchException(done, () => {

            const path = new CanvasKit.SkPath();
            path.moveTo(10, 10)
                .lineTo(40, 50); // should be length 50 because of the 3/4/5 triangle rule

            path.moveTo(80, 0)
                .lineTo(80, 10)
                .lineTo(100, 5)
                .lineTo(80, 0);

            const meas = new CanvasKit.SkContourMeasureIter(path, false, 1);
            let cont = meas.next();
            expect(cont).toBeTruthy();

            expect(cont.length()).toBeCloseTo(50.0, 3);
            const pt = cont.getPosTan(28.7); // arbitrary point
            expect(pt[0]).toBeCloseTo(27.22, 3); // x
            expect(pt[1]).toBeCloseTo(32.96, 3); // y
            expect(pt[2]).toBeCloseTo(0.6, 3);   // dy
            expect(pt[3]).toBeCloseTo(0.8, 3);   // dy
            const subpath = cont.getSegment(20, 40, true); // make sure this doesn't crash

            cont.delete();
            cont = meas.next();
            expect(cont).toBeTruthy()
            expect(cont.length()).toBeCloseTo(51.231, 3);

            cont.delete();
            expect(meas.next()).toBeFalsy();

            meas.delete();
            path.delete();
            done();
        }));
    });

    it('can draw a polygon', function(done) {
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

            const points = [[5, 5], [30, 20], [55, 5], [55, 50], [30, 30], [5, 50]];

            const mPoints = CanvasKit.Malloc(Float32Array, 6 * 2);
            mPoints.set([105, 105, 130, 120, 155, 105, 155, 150, 130, 130, 105, 150]);

            const path = new CanvasKit.SkPath();
            path.addPoly(points, true)
                .moveTo(100, 0)
                .addPoly(mPoints, true);

            canvas.drawPath(path, paint);

            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'drawpoly_path', done);
        }));
    });
});
