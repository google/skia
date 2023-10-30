describe('Path Behavior', () => {
    let container;

    beforeEach(async () => {
        await EverythingLoaded;
        container = document.createElement('div');
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    gm('path_api_example', (canvas) => {
        const paint = new CanvasKit.Paint();
        paint.setStrokeWidth(1.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const path = new CanvasKit.Path();
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
        path.arcToTangent(150, 100, 50, 200, 20);
        path.lineTo(160, 160);

        path.moveTo(20, 120);
        path.lineTo(20, 120);

        path.transform([2, 0, 0,
                        0, 2, 0,
                        0, 0, 1 ]);

        canvas.drawPath(path, paint);

        const rrect = CanvasKit.RRectXY([100, 10, 140, 62], 10, 4);

        const rrectPath = new CanvasKit.Path().addRRect(rrect, true);

        canvas.drawPath(rrectPath, paint);

        rrectPath.delete();
        path.delete();
        paint.delete();
        // See PathKit for more tests, since they share implementation
    });

    it('can create a path from an SVG string', () => {
        //.This is a parallelogram from
        // https://upload.wikimedia.org/wikipedia/commons/e/e7/Simple_parallelogram.svg
        const path = CanvasKit.Path.MakeFromSVGString(
          'M 205,5 L 795,5 L 595,295 L 5,295 L 205,5 z');

        const cmds = path.toCmds();
        expect(cmds).toBeTruthy();
        // 1 move, 4 lines, 1 close
        // each element in cmds is an array, with index 0 being the verb, and the rest being args
        expect(cmds).toEqual(Float32Array.of(
            CanvasKit.MOVE_VERB, 205, 5,
            CanvasKit.LINE_VERB, 795, 5,
            CanvasKit.LINE_VERB, 595, 295,
            CanvasKit.LINE_VERB, 5, 295,
            CanvasKit.LINE_VERB, 205, 5,
            CanvasKit.CLOSE_VERB));
        path.delete();
    });

    it('can create a path by combining two other paths', () => {
        // Get the intersection of two overlapping squares and verify that it is the smaller square.
        const pathOne = new CanvasKit.Path();
        pathOne.addRect([10, 10, 20, 20]);

        const pathTwo = new CanvasKit.Path();
        pathTwo.addRect([15, 15, 30, 30]);

        const path = CanvasKit.Path.MakeFromOp(pathOne, pathTwo, CanvasKit.PathOp.Intersect);
        const cmds = path.toCmds();
        expect(cmds).toBeTruthy();
        expect(cmds).toEqual(Float32Array.of(
            CanvasKit.MOVE_VERB, 15, 15,
            CanvasKit.LINE_VERB, 20, 15,
            CanvasKit.LINE_VERB, 20, 20,
            CanvasKit.LINE_VERB, 15, 20,
            CanvasKit.CLOSE_VERB));
        path.delete();
        pathOne.delete();
        pathTwo.delete();
    });

    it('can create an SVG string from a path', () => {
        const cmds = [CanvasKit.MOVE_VERB, 205, 5,
                   CanvasKit.LINE_VERB, 795, 5,
                   CanvasKit.LINE_VERB, 595, 295,
                   CanvasKit.LINE_VERB, 5, 295,
                   CanvasKit.LINE_VERB, 205, 5,
                   CanvasKit.CLOSE_VERB];
        const path = CanvasKit.Path.MakeFromCmds(cmds);

        const svgStr = path.toSVGString();
        // We output it in terse form, which is different than Wikipedia's version
        expect(svgStr).toEqual('M205 5L795 5L595 295L5 295L205 5Z');
        path.delete();
    });

    it('can create a path with malloced verbs, points, weights', () => {
        const mVerbs = CanvasKit.Malloc(Uint8Array, 6);
        const mPoints = CanvasKit.Malloc(Float32Array, 18);
        const mWeights = CanvasKit.Malloc(Float32Array, 1);
        mVerbs.toTypedArray().set([CanvasKit.MOVE_VERB, CanvasKit.LINE_VERB,
            CanvasKit.QUAD_VERB, CanvasKit.CONIC_VERB, CanvasKit.CUBIC_VERB, CanvasKit.CLOSE_VERB
        ]);

        mPoints.toTypedArray().set([
          1,2, // moveTo
          3,4, // lineTo
          5,6,7,8, // quadTo
          9,10,11,12, // conicTo
          13,14,15,16,17,18, // cubicTo
        ]);

        mWeights.toTypedArray().set([117]);

        let path = CanvasKit.Path.MakeFromVerbsPointsWeights(mVerbs, mPoints, mWeights);

        let cmds = path.toCmds();
        expect(cmds).toEqual(Float32Array.of(
            CanvasKit.MOVE_VERB, 1, 2,
            CanvasKit.LINE_VERB, 3, 4,
            CanvasKit.QUAD_VERB, 5, 6, 7, 8,
            CanvasKit.CONIC_VERB, 9, 10, 11, 12, 117,
            CanvasKit.CUBIC_VERB, 13, 14, 15, 16, 17, 18,
            CanvasKit.CLOSE_VERB,
        ));
        path.delete();

        // If given insufficient points, it stops early (but doesn't read out of bounds).
        path = CanvasKit.Path.MakeFromVerbsPointsWeights(mVerbs, mPoints.subarray(0, 10), mWeights);

        cmds = path.toCmds();
        expect(cmds).toEqual(Float32Array.of(
            CanvasKit.MOVE_VERB, 1, 2,
            CanvasKit.LINE_VERB, 3, 4,
            CanvasKit.QUAD_VERB, 5, 6, 7, 8,
        ));
        path.delete();
        CanvasKit.Free(mVerbs);
        CanvasKit.Free(mPoints);
        CanvasKit.Free(mWeights);
    });

    it('can create and update a path with verbs and points (no weights)', () => {
        const path = CanvasKit.Path.MakeFromVerbsPointsWeights(
          [CanvasKit.MOVE_VERB, CanvasKit.LINE_VERB],
          [1,2, 3,4]);
        let cmds = path.toCmds();
        expect(cmds).toEqual(Float32Array.of(
            CanvasKit.MOVE_VERB, 1, 2,
            CanvasKit.LINE_VERB, 3, 4
        ));

        path.addVerbsPointsWeights(
          [CanvasKit.QUAD_VERB, CanvasKit.CLOSE_VERB],
          [5,6,7,8],
        );

        cmds = path.toCmds();
        expect(cmds).toEqual(Float32Array.of(
            CanvasKit.MOVE_VERB, 1, 2,
            CanvasKit.LINE_VERB, 3, 4,
            CanvasKit.QUAD_VERB, 5, 6, 7, 8,
            CanvasKit.CLOSE_VERB
        ));
        path.delete();
    });


    it('can add points to a path in bulk', () => {
        const mVerbs = CanvasKit.Malloc(Uint8Array, 6);
        const mPoints = CanvasKit.Malloc(Float32Array, 18);
        const mWeights = CanvasKit.Malloc(Float32Array, 1);
        mVerbs.toTypedArray().set([CanvasKit.MOVE_VERB, CanvasKit.LINE_VERB,
            CanvasKit.QUAD_VERB, CanvasKit.CONIC_VERB, CanvasKit.CUBIC_VERB, CanvasKit.CLOSE_VERB
        ]);

        mPoints.toTypedArray().set([
            1,2, // moveTo
            3,4, // lineTo
            5,6,7,8, // quadTo
            9,10,11,12, // conicTo
            13,14,15,16,17,18, // cubicTo
        ]);

        mWeights.toTypedArray().set([117]);

        const path = new CanvasKit.Path();
        path.lineTo(77, 88);
        path.addVerbsPointsWeights(mVerbs, mPoints, mWeights);

        let cmds = path.toCmds();
        expect(cmds).toEqual(Float32Array.of(
            CanvasKit.MOVE_VERB, 0, 0,
            CanvasKit.LINE_VERB, 77, 88,
            CanvasKit.MOVE_VERB, 1, 2,
            CanvasKit.LINE_VERB, 3, 4,
            CanvasKit.QUAD_VERB, 5, 6, 7, 8,
            CanvasKit.CONIC_VERB, 9, 10, 11, 12, 117,
            CanvasKit.CUBIC_VERB, 13, 14, 15, 16, 17, 18,
            CanvasKit.CLOSE_VERB,
        ));

        path.rewind();
        cmds = path.toCmds();
        expect(cmds).toEqual(new Float32Array(0));

        path.delete();
        CanvasKit.Free(mVerbs);
        CanvasKit.Free(mPoints);
        CanvasKit.Free(mWeights);
    });

    it('can retrieve points from a path', () => {
        const path = new CanvasKit.Path();
        path.addRect([10, 15, 20, 25]);

        let pt = path.getPoint(0);
        expect(pt[0]).toEqual(10);
        expect(pt[1]).toEqual(15);

        path.getPoint(2, pt);
        expect(pt[0]).toEqual(20);
        expect(pt[1]).toEqual(25);

        path.getPoint(1000, pt); // off the end returns (0, 0) as per the docs.
        expect(pt[0]).toEqual(0);
        expect(pt[1]).toEqual(0);

        path.delete();
    });

    gm('offset_path', (canvas) => {
        const path = starPath(CanvasKit);

        const paint = new CanvasKit.Paint();
        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(5.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.BLACK);

        canvas.drawPath(path, paint);
        path.offset(80, 40);
        canvas.drawPath(path, paint);

        path.delete();
        paint.delete();
    });

    gm('oval_path', (canvas) => {
        const paint = new CanvasKit.Paint();

        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(5.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.BLACK);

        const path = new CanvasKit.Path();
        path.moveTo(5, 5);
        path.lineTo(10, 120);
        path.addOval(CanvasKit.LTRBRect(10, 20, 100, 200), false, 3);
        path.lineTo(300, 300);

        canvas.drawPath(path, paint);

        path.delete();
        paint.delete();
    });

    gm('bounds_path', (canvas) => {
        const paint = new CanvasKit.Paint();

        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(5.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.BLACK);

        const path = new CanvasKit.Path();
        // Arbitrary points to make an interesting curve.
        path.moveTo(97, 225);
        path.cubicTo(20, 400, 404, 75, 243, 271);

        canvas.drawPath(path, paint);

        const bounds = new Float32Array(4);
        path.getBounds(bounds);

        paint.setColor(CanvasKit.BLUE);
        paint.setStrokeWidth(3.0);
        canvas.drawRect(bounds, paint);

        path.computeTightBounds(bounds);
        paint.setColor(CanvasKit.RED);
        paint.setStrokeWidth(3.0);
        canvas.drawRect(bounds, paint);

        path.delete();
        paint.delete();
    });

    gm('arcto_path', (canvas) => {
        const paint = new CanvasKit.Paint();

        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(5.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.BLACK);

        const path = new CanvasKit.Path();

        // - x1, y1, x2, y2, radius
        path.arcToTangent(40, 0, 40, 40, 40);
        // - oval (as Rect), startAngle, sweepAngle, forceMoveTo
        path.arcToOval(CanvasKit.LTRBRect(90, 10, 120, 200), 30, 300, true);
        // - rx, ry, xAxisRotate, useSmallArc, isCCW, x, y
        path.moveTo(5, 105);
        path.arcToRotated(24, 24, 45, true, false, 82, 156);

        canvas.drawPath(path, paint);

        path.delete();
        paint.delete();
    });

    gm('path_relative', (canvas) => {
        const paint = new CanvasKit.Paint();
        paint.setStrokeWidth(1.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const path = new CanvasKit.Path();
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

        path.delete();
        paint.delete();
    });

    it('can measure the contours of a path',  () => {
        const path = new CanvasKit.Path();
        path.moveTo(10, 10)
            .lineTo(40, 50); // should be length 50 because of the 3/4/5 triangle rule

        path.moveTo(80, 0)
            .lineTo(80, 10)
            .lineTo(100, 5)
            .lineTo(80, 0);

        const meas = new CanvasKit.ContourMeasureIter(path, false, 1);
        let cont = meas.next();
        expect(cont).toBeTruthy();

        expect(cont.length()).toBeCloseTo(50.0, 3);
        const pt = cont.getPosTan(28.7); // arbitrary point
        expect(pt[0]).toBeCloseTo(27.22, 3); // x
        expect(pt[1]).toBeCloseTo(32.96, 3); // y
        expect(pt[2]).toBeCloseTo(0.6, 3);   // dy
        expect(pt[3]).toBeCloseTo(0.8, 3);   // dy

        pt.set([-1, -1, -1, -1]); // fill with sentinel values.
        cont.getPosTan(28.7, pt); // arbitrary point again, passing in an array to copy into.
        expect(pt[0]).toBeCloseTo(27.22, 3); // x
        expect(pt[1]).toBeCloseTo(32.96, 3); // y
        expect(pt[2]).toBeCloseTo(0.6, 3);   // dy
        expect(pt[3]).toBeCloseTo(0.8, 3);   // dy

        const subpath = cont.getSegment(20, 40, true); // make sure this doesn't crash

        cont.delete();
        cont = meas.next();
        expect(cont).toBeTruthy();
        expect(cont.length()).toBeCloseTo(51.231, 3);

        cont.delete();
        expect(meas.next()).toBeFalsy();

        meas.delete();
        path.delete();
    });

    gm('drawpoly_path', (canvas) => {
        const paint = new CanvasKit.Paint();
        paint.setStrokeWidth(1.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const points = [5, 5,  30, 20,  55, 5,  55, 50,  30, 30,  5, 50];

        const pointsObj = CanvasKit.Malloc(Float32Array, 6 * 2);
        const mPoints = pointsObj.toTypedArray();
        mPoints.set([105, 105, 130, 120, 155, 105, 155, 150, 130, 130, 105, 150]);

        const path = new CanvasKit.Path();
        path.addPoly(points, true)
            .moveTo(100, 0)
            .addPoly(mPoints, true);

        canvas.drawPath(path, paint);
        CanvasKit.Free(pointsObj);

        path.delete();
        paint.delete();
    });

    // Test trim, adding paths to paths, and a bunch of other path methods.
    gm('trim_path', (canvas) => {

        const paint = new CanvasKit.Paint();
        paint.setStrokeWidth(1.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const arcpath = new CanvasKit.Path();
        arcpath.arc(400, 400, 100, 0, -90, false) // x, y, radius, startAngle, endAngle, ccw
               .dash(3, 1, 0)
               .conicTo(10, 20, 30, 40, 5)
               .rConicTo(60, 70, 80, 90, 5)
               .trim(0.2, 1, false);

        const path = new CanvasKit.Path();
        path.addArc(CanvasKit.LTRBRect(10, 20, 100, 200), 30, 300)
            .addRect(CanvasKit.LTRBRect(200, 200, 300, 300)) // test single arg, default cw
            .addRect(CanvasKit.LTRBRect(240, 240, 260, 260), true) // test two arg, true means ccw
            .addRect([260, 260, 290, 290], true) // test five arg, true means ccw
            .addRRect([300, 10, 500, 290, // Rect in LTRB order
                       60, 60, 60, 60, 60, 60, 60, 60], // all radii are the same
                       false) // ccw
            .addRRect(CanvasKit.RRectXY([350, 60, 450, 240], 20, 80), true) // Rect, rx, ry, ccw
            .addPath(arcpath)
            .transform(0.54, -0.84,  390.35,
                       0.84,  0.54, -114.53,
                          0,     0,       1);

        canvas.drawPath(path, paint);

        path.delete();
        paint.delete();
    });

    gm('winding_example', (canvas) => {
        // Inspired by https://fiddle.skia.org/c/@Path_FillType_a
        const path = new CanvasKit.Path();
        // Draw overlapping rects on top
        path.addRect(CanvasKit.LTRBRect(10, 10, 30, 30), false);
        path.addRect(CanvasKit.LTRBRect(20, 20, 40, 40), false);
        // Draw overlapping rects on bottom, with different direction lines.
        path.addRect(CanvasKit.LTRBRect(10, 60, 30, 80), false);
        path.addRect(CanvasKit.LTRBRect(20, 70, 40, 90), true);

        expect(path.getFillType()).toEqual(CanvasKit.FillType.Winding);

        // Draw the two rectangles on the left side.
        const paint = new CanvasKit.Paint();
        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        canvas.drawPath(path, paint);

        const clipRect = CanvasKit.LTRBRect(0, 0, 51, 100);
        paint.setStyle(CanvasKit.PaintStyle.Fill);

        for (const fillType of [CanvasKit.FillType.Winding, CanvasKit.FillType.EvenOdd]) {
            canvas.translate(51, 0);
            canvas.save();
            canvas.clipRect(clipRect, CanvasKit.ClipOp.Intersect, false);
            path.setFillType(fillType);
            canvas.drawPath(path, paint);
            canvas.restore();
        }

        path.delete();
        paint.delete();
    });

    gm('as_winding', (canvas) => {
        const evenOddPath = new CanvasKit.Path();
        // Draw overlapping rects
        evenOddPath.addRect(CanvasKit.LTRBRect(10, 10, 70, 70), false);
        evenOddPath.addRect(CanvasKit.LTRBRect(30, 30, 50, 50), false);
        evenOddPath.setFillType(CanvasKit.FillType.EvenOdd);

        const evenOddCmds = evenOddPath.toCmds();
        expect(evenOddCmds).toEqual(Float32Array.of(
          CanvasKit.MOVE_VERB, 10, 10,
          CanvasKit.LINE_VERB, 70, 10,
          CanvasKit.LINE_VERB, 70, 70,
          CanvasKit.LINE_VERB, 10, 70,
          CanvasKit.CLOSE_VERB,
          CanvasKit.MOVE_VERB, 30, 30, // This contour is drawn
          CanvasKit.LINE_VERB, 50, 30, // clockwise, as specified.
          CanvasKit.LINE_VERB, 50, 50,
          CanvasKit.LINE_VERB, 30, 50,
          CanvasKit.CLOSE_VERB
        ));

        const windingPath = evenOddPath.makeAsWinding();

        expect(windingPath.getFillType()).toBe(CanvasKit.FillType.Winding);
        const windingCmds = windingPath.toCmds();
        expect(windingCmds).toEqual(Float32Array.of(
          CanvasKit.MOVE_VERB, 10, 10,
          CanvasKit.LINE_VERB, 70, 10,
          CanvasKit.LINE_VERB, 70, 70,
          CanvasKit.LINE_VERB, 10, 70,
          CanvasKit.CLOSE_VERB,
          CanvasKit.MOVE_VERB, 30, 50, // This contour has been
          CanvasKit.LINE_VERB, 50, 50, // re-drawn counter-clockwise
          CanvasKit.LINE_VERB, 50, 30, // so that it covers the same
          CanvasKit.LINE_VERB, 30, 30, // area, but with the winding fill type.
          CanvasKit.CLOSE_VERB
        ));

        const paint = new CanvasKit.Paint();
        paint.setStyle(CanvasKit.PaintStyle.Fill);
        const font = new CanvasKit.Font(CanvasKit.Typeface.GetDefault(), 20);

        canvas.drawText('Original path (even odd)', 5, 20, paint, font);
        canvas.translate(0, 50);
        canvas.drawPath(evenOddPath, paint);

        canvas.translate(300, 0);
        canvas.drawPath(windingPath, paint);

        canvas.translate(0, -50);
        canvas.drawText('makeAsWinding path', 5, 20, paint, font);

        evenOddPath.delete();
        windingPath.delete();
    });
});
