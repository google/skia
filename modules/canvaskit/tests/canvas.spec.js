describe('Canvas Behavior', () => {
    let container;

    beforeEach(async () => {
        await LoadCanvasKit;
        container = document.createElement('div');
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    gm('canvas_api_example', (canvas) => {
        const paint = new CanvasKit.Paint();
        paint.setStrokeWidth(2.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        canvas.drawLine(3, 10, 30, 15, paint);
        const rrect = CanvasKit.RRectXY([5, 35, 45, 80], 15, 10);
        canvas.drawRRect(rrect, paint);

        canvas.drawOval(CanvasKit.LTRBRect(5, 35, 45, 80), paint);

        canvas.drawArc(CanvasKit.LTRBRect(55, 35, 95, 80), 15, 270, true, paint);

        const font = new CanvasKit.Font(null, 20);
        canvas.drawText('this is ascii text', 5, 100, paint, font);

        const blob = CanvasKit.TextBlob.MakeFromText('Unicode chars 💩 é É ص', font);
        canvas.drawTextBlob(blob, 5, 130, paint);

        font.delete();
        blob.delete();
        paint.delete();
        // See canvas2d for more API tests
    });

    gm('effect_and_text_example', (canvas) => {
        const path = starPath(CanvasKit);
        const paint = new CanvasKit.Paint();

        const textPaint = new CanvasKit.Paint();
        textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
        textPaint.setAntiAlias(true);

        const textFont = new CanvasKit.Font(null, 30);

        const dpe = CanvasKit.PathEffect.MakeDash([15, 5, 5, 10], 1);

        paint.setPathEffect(dpe);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(5.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

        canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

        canvas.drawPath(path, paint);
        canvas.drawText('This is text', 10, 280, textPaint, textFont);

        dpe.delete();
        path.delete();
        paint.delete();
        textFont.delete();
        textPaint.delete();
    });

    gm('patheffects_canvas', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        const path = starPath(CanvasKit, 100, 100, 100);
        const paint = new CanvasKit.Paint();

        const cornerEffect = CanvasKit.PathEffect.MakeCorner(10);
        const discreteEffect = CanvasKit.PathEffect.MakeDiscrete(5, 10, 0);

        paint.setPathEffect(cornerEffect);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(5.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));
        canvas.drawPath(path, paint);

        canvas.translate(200, 0);

        paint.setPathEffect(discreteEffect);
        canvas.drawPath(path, paint);

        cornerEffect.delete();
        path.delete();
        paint.delete();
    });

    it('returns the depth of the save state stack', () => {
        const canvas = new CanvasKit.Canvas();
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
    });

    gm('circle_canvas', (canvas) => {
        const path = starPath(CanvasKit);

        const paint = new CanvasKit.Paint();

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

        path.delete();
        paint.delete();
    });

    gm('rrect_canvas', (canvas) => {
        const path = starPath(CanvasKit);

        const paint = new CanvasKit.Paint();

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

        path.delete();
        paint.delete();
    });

    gm('rrect_8corners_canvas', (canvas) => {
        const path = starPath(CanvasKit);

        const paint = new CanvasKit.Paint();

        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(3.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.BLACK);

        canvas.clear(CanvasKit.WHITE);

        canvas.drawRRect([10, 10, 210, 210,
          // top left corner, going clockwise
          10, 30,
          30, 10,
          50, 75,
          120, 120,
        ], paint);

        path.delete();
        paint.delete();
    });

    // As above, except with the array passed in via malloc'd memory.
    gm('rrect_8corners_malloc_canvas', (canvas) => {
        const path = starPath(CanvasKit);

        const paint = new CanvasKit.Paint();

        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(3.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.BLACK);

        canvas.clear(CanvasKit.WHITE);

        const rrect = CanvasKit.Malloc(Float32Array, 12);
        rrect.toTypedArray().set([10, 10, 210, 210,
          // top left corner, going clockwise
          10, 30,
          30, 10,
          50, 75,
          120, 120,
        ]);

        canvas.drawRRect(rrect, paint);

        CanvasKit.Free(rrect);
        path.delete();
        paint.delete();
    });

    gm('drawDRRect_canvas', (canvas) => {
        const path = starPath(CanvasKit);

        const paint = new CanvasKit.Paint();

        paint.setStyle(CanvasKit.PaintStyle.Fill);
        paint.setStrokeWidth(3.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.BLACK);

        canvas.clear(CanvasKit.WHITE);

        const outer = CanvasKit.RRectXY(CanvasKit.LTRBRect(10, 60, 210, 260), 10, 5);
        const inner = CanvasKit.RRectXY(CanvasKit.LTRBRect(50, 90, 160, 210), 30, 30);

        canvas.drawDRRect(outer, inner, paint);

        path.delete();
        paint.delete();
    });

    gm('colorfilters_canvas', (canvas) => {
        const paint = new CanvasKit.Paint();

        const blue = CanvasKit.ColorFilter.MakeBlend(
            CanvasKit.BLUE, CanvasKit.BlendMode.SrcIn);
        const red =  CanvasKit.ColorFilter.MakeBlend(
            CanvasKit.Color(255, 0, 0, 0.8), CanvasKit.BlendMode.SrcOver);
        const lerp = CanvasKit.ColorFilter.MakeLerp(0.6, red, blue);

        paint.setStyle(CanvasKit.PaintStyle.Fill);
        paint.setAntiAlias(true);

        canvas.clear(CanvasKit.Color(230, 230, 230));

        paint.setColorFilter(blue)
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, 60, 60), paint);
        paint.setColorFilter(lerp)
        canvas.drawRect(CanvasKit.LTRBRect(50, 10, 100, 60), paint);
        paint.setColorFilter(red)
        canvas.drawRect4f(90, 10, 140, 60, paint);

        const r = CanvasKit.ColorMatrix.rotated(0, .707, -.707);
        const b = CanvasKit.ColorMatrix.rotated(2, .5, .866);
        const s = CanvasKit.ColorMatrix.scaled(0.9, 1.5, 0.8, 0.8);
        let cm = CanvasKit.ColorMatrix.concat(r, s);
        cm = CanvasKit.ColorMatrix.concat(cm, b);
        CanvasKit.ColorMatrix.postTranslate(cm, 20, 0, -10, 0);

        const mat = CanvasKit.ColorFilter.MakeMatrix(cm);
        const final = CanvasKit.ColorFilter.MakeCompose(mat, lerp);

        paint.setColorFilter(final)
        canvas.drawRect(CanvasKit.LTRBRect(10, 70, 140, 120), paint);

        paint.delete();
        blue.delete();
        red.delete();
        lerp.delete();
        final.delete();
    });

    gm('blendmodes_canvas', (canvas) => {
        canvas.clear(CanvasKit.WHITE);

        const blendModeNames = Object.keys(CanvasKit.BlendMode).filter((key) => key !== 'values');

        const PASTEL_MUSTARD_YELLOW = CanvasKit.Color(248, 213, 85, 1.0);
        const PASTEL_SKY_BLUE = CanvasKit.Color(74, 174, 245, 1.0);

        const shapePaint = new CanvasKit.Paint();
        shapePaint.setColor(PASTEL_MUSTARD_YELLOW);
        shapePaint.setAntiAlias(true);

        const textPaint = new CanvasKit.Paint();
        textPaint.setAntiAlias(true);

        const textFont = new CanvasKit.Font(null, 10);

        let x = 10;
        let y = 20;
        for (const blendModeName of blendModeNames) {
            // Draw a checkerboard for each blend mode.
            // Each checkerboard is labelled with a blendmode's name.
            canvas.drawText(blendModeName, x, y - 5, textPaint, textFont);
            drawCheckerboard(canvas, x, y, x + 80, y + 80);

            // A blue square is drawn on to each checkerboard with yellow circle.
            // In each checkerboard the blue square is drawn using a different blendmode.
            const blendMode = CanvasKit.BlendMode[blendModeName];
            canvas.drawOval(CanvasKit.LTRBRect(x + 5, y + 5, x + 55, y + 55), shapePaint);
            drawRectangle(x + 30, y + 30, x + 70, y + 70, PASTEL_SKY_BLUE, blendMode);

            x += 90;
            if (x > 500) {
                x = 10;
                y += 110;
            }
        }

        function drawCheckerboard(canvas, x1, y1, x2, y2) {
            const CHECKERBOARD_SQUARE_SIZE = 5;
            const GREY = CanvasKit.Color(220, 220, 220, 0.5);
            // Draw black border and white background for checkerboard
            drawRectangle(x1-1, y1-1, x2+1, y2+1, CanvasKit.BLACK);
            drawRectangle(x1, y1, x2, y2, CanvasKit.WHITE);

            // Draw checkerboard squares
            const numberOfColumns = (x2 - x1) / CHECKERBOARD_SQUARE_SIZE;
            const numberOfRows = (y2 - y1) / CHECKERBOARD_SQUARE_SIZE

            for (let row = 0; row < numberOfRows; row++) {
                for (let column = 0; column < numberOfColumns; column++) {
                    const rowIsEven = row % 2 === 0;
                    const columnIsEven = column % 2 === 0;

                    if ((rowIsEven && !columnIsEven) || (!rowIsEven && columnIsEven)) {
                        drawRectangle(
                            x1 + CHECKERBOARD_SQUARE_SIZE * row,
                            y1 + CHECKERBOARD_SQUARE_SIZE * column,
                            Math.min(x1 + CHECKERBOARD_SQUARE_SIZE * row + CHECKERBOARD_SQUARE_SIZE, x2),
                            Math.min(y1 + CHECKERBOARD_SQUARE_SIZE * column + CHECKERBOARD_SQUARE_SIZE, y2),
                            GREY
                        );
                    }
                }
            }
        }

        function drawRectangle(x1, y1, x2, y2, color, blendMode=CanvasKit.BlendMode.srcOver) {
            canvas.save();
            canvas.clipRect(CanvasKit.LTRBRect(x1, y1, x2, y2), CanvasKit.ClipOp.Intersect, true);
            canvas.drawColor(color, blendMode);
            canvas.restore();
        }
    });

    gm('colorfilters_malloc_canvas', (canvas) => {
        const paint = new CanvasKit.Paint();

        const src = [
             0.8,   0.45,      2,   0,  20,
            0.53, -0.918,  0.566,   0,   0,
            0.53, -0.918, -0.566,   0, -10,
               0,      0,      0, 0.8,   0,
        ]
        const colorObj = new CanvasKit.Malloc(Float32Array, 20);
        const cm = colorObj.toTypedArray();
        for (i in src) {
            cm[i] = src[i];
        }
        // MakeMatrix will free the malloc'd array when it is done with it.
        const final = CanvasKit.ColorFilter.MakeMatrix(cm);

        paint.setColorFilter(final)
        canvas.drawRect(CanvasKit.LTRBRect(10, 70, 140, 120), paint);

        CanvasKit.Free(colorObj);
        paint.delete();
        final.delete();
    });

    gm('clips_canvas', (canvas) => {
        const path = starPath(CanvasKit);
        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.BLUE);
        const rrect = CanvasKit.RRectXY(CanvasKit.LTRBRect(300, 300, 500, 500), 40, 40);

        canvas.save();
        // draw magenta around the outside edge of an rrect.
        canvas.clipRRect(rrect, CanvasKit.ClipOp.Difference, true);
        canvas.drawColorComponents(250/255, 30/255, 240/255, 0.9, CanvasKit.BlendMode.SrcOver);
        canvas.restore();

        // draw grey inside of a star pattern, then the blue star on top
        canvas.clipPath(path, CanvasKit.ClipOp.Intersect, false);
        canvas.drawColorInt(CanvasKit.ColorAsInt(200, 200, 200, 255), CanvasKit.BlendMode.SrcOver);
        canvas.drawPath(path, paint);

        path.delete();
        paint.delete();
    });

    // inspired by https://fiddle.skia.org/c/feb2a08bb09ede5309678d6a0ab3f981
    gm('savelayer_rect_paint_canvas', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        const redPaint = new CanvasKit.Paint();
        redPaint.setColor(CanvasKit.RED);
        const solidBluePaint = new CanvasKit.Paint();
        solidBluePaint.setColor(CanvasKit.BLUE);

        const thirtyBluePaint = new CanvasKit.Paint();
        thirtyBluePaint.setColor(CanvasKit.BLUE);
        thirtyBluePaint.setAlphaf(0.3);

        const alpha = new CanvasKit.Paint();
        alpha.setAlphaf(0.3);

        // Draw 4 solid red rectangles on the 0th layer.
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, 60, 60), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(150, 10, 200, 60), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(10, 70, 60, 120), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(150, 70, 200, 120), redPaint);

        // Draw 2 blue rectangles that overlap. One is solid, the other
        // is 30% transparent. We should see purple from the right one,
        // the left one overlaps the red because it is opaque.
        canvas.drawRect(CanvasKit.LTRBRect(30, 10, 80, 60), solidBluePaint);
        canvas.drawRect(CanvasKit.LTRBRect(170, 10, 220, 60), thirtyBluePaint);

        // Save a new layer. When the 1st layer gets merged onto the
        // 0th layer (i.e. when restore() is called), it will use the provided
        // paint to do so. The provided paint is set to have 30% opacity, but
        // it could also have things set like blend modes or image filters.
        // The rectangle is just a hint, so I've set it to be the area that
        // we actually draw in before restore is called. It could also be omitted,
        // see the test below.
        canvas.saveLayer(alpha, CanvasKit.LTRBRect(10, 10, 220, 180));

        // Draw the same blue overlapping rectangles as before. Notice in the
        // final output, we have two different shades of purple instead of the
        // solid blue overwriting the red. This proves the opacity was applied.
        canvas.drawRect(CanvasKit.LTRBRect(30, 70, 80, 120), solidBluePaint);
        canvas.drawRect(CanvasKit.LTRBRect(170, 70, 220, 120), thirtyBluePaint);

        // We draw two more sets of overlapping red and blue rectangles. Notice
        // the solid blue overwrites the red. This proves that the opacity from
        // the alpha paint isn't available when the drawing happens - it only
        // matters when restore() is called.
        canvas.drawRect(CanvasKit.LTRBRect(10, 130, 60, 180), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(30, 130, 80, 180), solidBluePaint);

        canvas.drawRect(CanvasKit.LTRBRect(150, 130, 200, 180), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(170, 130, 220, 180), thirtyBluePaint);

        canvas.restore();

        redPaint.delete();
        solidBluePaint.delete();
        thirtyBluePaint.delete();
        alpha.delete();
    });

    // identical to the test above, except the save layer only has the paint, not
    // the rectangle.
    gm('savelayer_paint_canvas', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        const redPaint = new CanvasKit.Paint();
        redPaint.setColor(CanvasKit.RED);
        const solidBluePaint = new CanvasKit.Paint();
        solidBluePaint.setColor(CanvasKit.BLUE);

        const thirtyBluePaint = new CanvasKit.Paint();
        thirtyBluePaint.setColor(CanvasKit.BLUE);
        thirtyBluePaint.setAlphaf(0.3);

        const alpha = new CanvasKit.Paint();
        alpha.setAlphaf(0.3);

        // Draw 4 solid red rectangles on the 0th layer.
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, 60, 60), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(150, 10, 200, 60), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(10, 70, 60, 120), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(150, 70, 200, 120), redPaint);

        // Draw 2 blue rectangles that overlap. One is solid, the other
        // is 30% transparent. We should see purple from the right one,
        // the left one overlaps the red because it is opaque.
        canvas.drawRect(CanvasKit.LTRBRect(30, 10, 80, 60), solidBluePaint);
        canvas.drawRect(CanvasKit.LTRBRect(170, 10, 220, 60), thirtyBluePaint);

        // Save a new layer. When the 1st layer gets merged onto the
        // 0th layer (i.e. when restore() is called), it will use the provided
        // paint to do so. The provided paint is set to have 30% opacity, but
        // it could also have things set like blend modes or image filters.
        canvas.saveLayerPaint(alpha);

        // Draw the same blue overlapping rectangles as before. Notice in the
        // final output, we have two different shades of purple instead of the
        // solid blue overwriting the red. This proves the opacity was applied.
        canvas.drawRect(CanvasKit.LTRBRect(30, 70, 80, 120), solidBluePaint);
        canvas.drawRect(CanvasKit.LTRBRect(170, 70, 220, 120), thirtyBluePaint);

        // We draw two more sets of overlapping red and blue rectangles. Notice
        // the solid blue overwrites the red. This proves that the opacity from
        // the alpha paint isn't available when the drawing happens - it only
        // matters when restore() is called.
        canvas.drawRect(CanvasKit.LTRBRect(10, 130, 60, 180), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(30, 130, 80, 180), solidBluePaint);

        canvas.drawRect(CanvasKit.LTRBRect(150, 130, 200, 180), redPaint);
        canvas.drawRect(CanvasKit.LTRBRect(170, 130, 220, 180), thirtyBluePaint);

        canvas.restore();

        redPaint.delete();
        solidBluePaint.delete();
        thirtyBluePaint.delete();
        alpha.delete();
    });

    gm('savelayerrec_canvas', (canvas) => {
        // Note: fiddle.skia.org quietly draws a white background before doing
        // other things, which is noticed in cases like this where we use saveLayer
        // with the rec struct.
        canvas.clear(CanvasKit.WHITE);
        canvas.scale(8, 8);
        const redPaint = new CanvasKit.Paint();
        redPaint.setColor(CanvasKit.RED);
        redPaint.setAntiAlias(true);
        canvas.drawCircle(21, 21, 8, redPaint);

        const bluePaint = new CanvasKit.Paint();
        bluePaint.setColor(CanvasKit.BLUE);
        canvas.drawCircle(31, 21, 8, bluePaint);

        const blurIF = CanvasKit.ImageFilter.MakeBlur(8, 0.2, CanvasKit.TileMode.Decal, null);

        const count = canvas.saveLayer(null, null, blurIF, 0);
        expect(count).toEqual(1);
        canvas.scale(1/4, 1/4);
        canvas.drawCircle(125, 85, 8, redPaint);
        canvas.restore();

        blurIF.delete();
        redPaint.delete();
        bluePaint.delete();
    });

    gm('drawpoints_canvas', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(10);
        paint.setColor(CanvasKit.Color(153, 204, 162, 0.82));

        const points = [32, 16, 48, 48, 16, 32];

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
            const mPointsObj = CanvasKit.Malloc(Float32Array, 3*2);
            const mPoints = mPointsObj.toTypedArray();
            mPoints.set([32, 16, 48, 48, 16, 32]);

            // The obj from Malloc can be passed in instead of the typed array.
            canvas.drawPoints(CanvasKit.PointMode.Polygon, mPointsObj, paint);
            canvas.translate(-192, 64);
            CanvasKit.Free(mPointsObj);
        }

        paint.delete();
    });

    gm('drawPoints in different modes', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        // From https://bugs.chromium.org/p/skia/issues/detail?id=11012
        const boxPaint = new CanvasKit.Paint();
        boxPaint.setStyle(CanvasKit.PaintStyle.Stroke);
        boxPaint.setStrokeWidth(1);

        const paint = new CanvasKit.Paint();
        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        paint.setStrokeWidth(5);
        paint.setStrokeCap(CanvasKit.StrokeCap.Round);
        paint.setColorInt(0xFF0000FF); // Blue
        paint.setAntiAlias(true);

        const points = Float32Array.of(40, 40, 80, 40, 120, 80, 160, 80);

        canvas.drawRect(CanvasKit.LTRBRect(35, 35, 165, 85), boxPaint);
        canvas.drawPoints(CanvasKit.PointMode.Points, points, paint);

        canvas.translate(0, 50);
        canvas.drawRect(CanvasKit.LTRBRect(35, 35, 165, 85), boxPaint);
        canvas.drawPoints(CanvasKit.PointMode.Lines, points, paint);

        canvas.translate(0, 50);
        canvas.drawRect(CanvasKit.LTRBRect(35, 35, 165, 85), boxPaint);
        canvas.drawPoints(CanvasKit.PointMode.Polygon, points, paint);

        // The control version using drawPath
        canvas.translate(0, 50);
        canvas.drawRect(CanvasKit.LTRBRect(35, 35, 165, 85), boxPaint);
        const path = new CanvasKit.Path();
        path.moveTo(40, 40);
        path.lineTo(80, 40);
        path.lineTo(120, 80);
        path.lineTo(160, 80);
        paint.setColorInt(0xFFFF0000); // RED
        canvas.drawPath(path, paint);

        paint.delete();
        path.delete();
        boxPaint.delete();
    });

    gm('drawImageNine_canvas', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();

        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();

        canvas.drawImageNine(img, CanvasKit.LTRBiRect(40, 40, 400, 300),
            CanvasKit.LTRBRect(5, 5, 300, 650), CanvasKit.FilterMode.Nearest, paint);
        paint.delete();
        img.delete();
    }, '/assets/mandrill_512.png');

        // This should be a nice, clear image.
    gm('makeImageShaderCubic_canvas', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();

        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        const shader = img.makeShaderCubic(CanvasKit.TileMode.Decal, CanvasKit.TileMode.Clamp,
                                           1/3 /*B*/, 1/3 /*C*/,
                                           CanvasKit.Matrix.rotated(0.1));
        paint.setShader(shader);

        canvas.drawPaint(paint);
        paint.delete();
        shader.delete();
        img.delete();
    }, '/assets/mandrill_512.png');

    // This will look more blocky than the version above.
    gm('makeImageShaderOptions_canvas', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();
        const imgWithMipMap = img.makeCopyWithDefaultMipmaps();

        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        const shader = imgWithMipMap.makeShaderOptions(CanvasKit.TileMode.Decal,
                                                       CanvasKit.TileMode.Clamp,
                                                       CanvasKit.FilterMode.Nearest,
                                                       CanvasKit.MipmapMode.Linear,
                                                       CanvasKit.Matrix.rotated(0.1));
        paint.setShader(shader);

        canvas.drawPaint(paint);
        paint.delete();
        shader.delete();
        img.delete();
        imgWithMipMap.delete();
    }, '/assets/mandrill_512.png');

    gm('drawvertices_canvas', (canvas) => {
        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);

        const points = [0, 0,  250, 0,  100, 100,  0, 250];
        // 2d float color array
        const colors = [CanvasKit.RED, CanvasKit.BLUE,
                        CanvasKit.YELLOW, CanvasKit.CYAN];
        const vertices = CanvasKit.MakeVertices(CanvasKit.VertexMode.TriangleFan,
            points, null /*textureCoordinates*/, colors, false /*isVolatile*/);

        const bounds = vertices.bounds();
        expect(bounds).toEqual(CanvasKit.LTRBRect(0, 0, 250, 250));

        canvas.drawVertices(vertices, CanvasKit.BlendMode.Src, paint);
        vertices.delete();
        paint.delete();
    });

    gm('drawvertices_canvas_flat_floats', (canvas) => {
        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);

        const points = [0, 0,  250, 0,  100, 100,  0, 250];
        // 1d float color array
        const colors = Float32Array.of(...CanvasKit.RED, ...CanvasKit.BLUE,
                                       ...CanvasKit.YELLOW, ...CanvasKit.CYAN);
        const vertices = CanvasKit.MakeVertices(CanvasKit.VertexMode.TriangleFan,
            points, null /*textureCoordinates*/, colors, false /*isVolatile*/);

        const bounds = vertices.bounds();
        expect(bounds).toEqual(CanvasKit.LTRBRect(0, 0, 250, 250));

        canvas.drawVertices(vertices, CanvasKit.BlendMode.Src, paint);
        vertices.delete();
        paint.delete();
    });

    gm('drawvertices_texture_canvas', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);

        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);

        const points = [
             70, 170,   40, 90,  130, 150,  100, 50,
            225, 150,  225, 60,  310, 180,  330, 100,
        ];
        const textureCoordinates = [
              0, 240,    0, 0,   80, 240,   80, 0,
            160, 240,  160, 0,  240, 240,  240, 0,
        ];
        const vertices = CanvasKit.MakeVertices(CanvasKit.VertexMode.TrianglesStrip,
            points, textureCoordinates, null /* colors */, false /*isVolatile*/);

        const shader = img.makeShaderCubic(CanvasKit.TileMode.Repeat, CanvasKit.TileMode.Mirror,
            1/3 /*B*/, 1/3 /*C*/,);
        paint.setShader(shader);
        canvas.drawVertices(vertices, CanvasKit.BlendMode.Src, paint);

        shader.delete();
        vertices.delete();
        paint.delete();
        img.delete();
    }, '/assets/brickwork-texture.jpg');

    it('can change the 3x3 matrix on the canvas and read it back', () => {
        const canvas = new CanvasKit.Canvas();

        let matr = canvas.getTotalMatrix();
        expect(matr).toEqual(CanvasKit.Matrix.identity());

        // This fills the internal _scratch4x4MatrixPtr with garbage (aka sentinel) values to
        // make sure the 3x3 matrix properly sets these to 0 when it uses the same buffer.
        canvas.save();
        const garbageMatrix = new Float32Array(16);
        garbageMatrix.fill(-3);
        canvas.concat(garbageMatrix);
        canvas.restore();

        canvas.concat(CanvasKit.Matrix.rotated(Math.PI/4));
        const d = new DOMMatrix().translate(20, 10);
        canvas.concat(d);

        matr = canvas.getTotalMatrix();
        const expected = CanvasKit.Matrix.multiply(
            CanvasKit.Matrix.rotated(Math.PI/4),
            CanvasKit.Matrix.translated(20, 10)
        );
        expect3x3MatricesToMatch(expected, matr);

        // The 3x3 should be expanded into a 4x4, with 0s in the 3rd row and column.
        matr = canvas.getLocalToDevice();
        expect4x4MatricesToMatch([
            0.707106, -0.707106, 0,  7.071067,
            0.707106,  0.707106, 0, 21.213203,
            0       ,  0       , 0,  0       ,
            0       ,  0       , 0,  1       ], matr);
    });

    it('can accept a 3x2 matrix', () => {
        const canvas = new CanvasKit.Canvas();

        let matr = canvas.getTotalMatrix();
        expect(matr).toEqual(CanvasKit.Matrix.identity());

        // This fills the internal _scratch4x4MatrixPtr with garbage (aka sentinel) values to
        // make sure the 3x2 matrix properly sets these to 0 when it uses the same buffer.
        canvas.save();
        const garbageMatrix = new Float32Array(16);
        garbageMatrix.fill(-3);
        canvas.concat(garbageMatrix);
        canvas.restore();

        canvas.concat([1.4, -0.2, 12,
                       0.2,  1.4, 24]);

        matr = canvas.getTotalMatrix();
        const expected = [1.4, -0.2, 12,
                          0.2,  1.4, 24,
                            0,    0,  1];
        expect3x3MatricesToMatch(expected, matr);

        // The 3x2 should be expanded into a 4x4, with 0s in the 3rd row and column
        // and the perspective filled in.
        matr = canvas.getLocalToDevice();
        expect4x4MatricesToMatch([
            1.4, -0.2, 0, 12,
            0.2,  1.4, 0, 24,
            0  ,  0  , 0,  0,
            0  ,  0  , 0,  1], matr);
    });

    it('can mark a CTM and retrieve it', () => {
        const canvas = new CanvasKit.Canvas();

        canvas.concat(CanvasKit.M44.rotated([0, 1, 0], Math.PI/4));
        canvas.concat(CanvasKit.M44.rotated([1, 0, 1], Math.PI/8));
        canvas.markCTM('krispykreme');

        const expected = CanvasKit.M44.multiply(
          CanvasKit.M44.rotated([0, 1, 0], Math.PI/4),
          CanvasKit.M44.rotated([1, 0, 1], Math.PI/8),
        );

        expect4x4MatricesToMatch(expected, canvas.findMarkedCTM('krispykreme'));
    });

    it('returns null for an invalid CTM marker', () => {
        const canvas = new CanvasKit.Canvas();
        expect(canvas.findMarkedCTM('dunkindonuts')).toBeNull();
    });

    it('can change the 4x4 matrix on the canvas and read it back', () => {
        const canvas = new CanvasKit.Canvas();

        let matr = canvas.getLocalToDevice();
        expect(matr).toEqual(CanvasKit.M44.identity());

        canvas.concat(CanvasKit.M44.rotated([0, 1, 0], Math.PI/4));
        canvas.concat(CanvasKit.M44.rotated([1, 0, 1], Math.PI/8));

        const expected = CanvasKit.M44.multiply(
          CanvasKit.M44.rotated([0, 1, 0], Math.PI/4),
          CanvasKit.M44.rotated([1, 0, 1], Math.PI/8),
        );

        expect4x4MatricesToMatch(expected, canvas.getLocalToDevice());
        // TODO(kjlubick) add test for DOMMatrix
        // TODO(nifong) add more involved test for camera-related math.
    });

    gm('concat_with4x4_canvas', (canvas) => {
        const path = starPath(CanvasKit, CANVAS_WIDTH/2, CANVAS_HEIGHT/2);
        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);
        canvas.clear(CanvasKit.WHITE);

        // Rotate it a bit on all 3 major axis, centered on the screen.
        // To play with rotations, see https://jsfiddle.skia.org/canvaskit/0525300405796aa87c3b84cc0d5748516fca0045d7d6d9c7840710ab771edcd4
        const turn = CanvasKit.M44.multiply(
          CanvasKit.M44.translated([CANVAS_WIDTH/2, 0, 0]),
          CanvasKit.M44.rotated([1, 0, 0], Math.PI/3),
          CanvasKit.M44.rotated([0, 1, 0], Math.PI/4),
          CanvasKit.M44.rotated([0, 0, 1], Math.PI/16),
          CanvasKit.M44.translated([-CANVAS_WIDTH/2, 0, 0]),
        );
        canvas.concat(turn);

        // Draw some stripes to help the eye detect the turn
        const stripeWidth = 10;
        paint.setColor(CanvasKit.BLACK);
        for (let i = 0; i < CANVAS_WIDTH; i += 2*stripeWidth) {
            canvas.drawRect(CanvasKit.LTRBRect(i, 0, i + stripeWidth, CANVAS_HEIGHT), paint);
        }

        paint.setColor(CanvasKit.YELLOW);
        canvas.drawPath(path, paint);
        paint.delete();
        path.delete();
    });

    gm('particles_canvas', (canvas) => {
        const curveParticles = {
            'MaxCount': 1000,
            'Drawable': {
               'Type': 'SkCircleDrawable',
               'Radius': 2
            },
            'Code': [
               `void effectSpawn(inout Effect effect) {
                  effect.rate = 200;
                  effect.color = float4(1, 0, 0, 1);
                }
                void spawn(inout Particle p) {
                  p.lifetime = 3 + rand(p.seed);
                  p.vel.y = -50;
                }

                void update(inout Particle p) {
                  float w = mix(15, 3, p.age);
                  p.pos.x = sin(radians(p.age * 320)) * mix(25, 10, p.age) + mix(-w, w, rand(p.seed));
                  if (rand(p.seed) < 0.5) { p.pos.x = -p.pos.x; }

                  p.color.g = (mix(75, 220, p.age) + mix(-30, 30, rand(p.seed))) / 255;
                }`
            ],
            'Bindings': []
        };

        const particles = CanvasKit.MakeParticles(JSON.stringify(curveParticles));
        particles.start(0, true);
        particles.setPosition([0, 0]);

        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.WHITE);
        const font = new CanvasKit.Font(null, 12);

        canvas.clear(CanvasKit.BLACK);

        // Draw a 5x5 set of different times in the particle system
        // like a filmstrip of motion of particles.
        const LEFT_MARGIN = 90;
        const TOP_MARGIN = 100;
        for (let row = 0; row < 5; row++) {
            for (let column = 0; column < 5; column++) {
                canvas.save();
                canvas.translate(LEFT_MARGIN + column*100, TOP_MARGIN + row*100);

                // Time moves in row-major order in increments of 0.02.
                const particleTime = row/10 + column/50;

                canvas.drawText('time ' + particleTime.toFixed(2), -30, 20, paint, font);
                particles.update(particleTime);

                particles.draw(canvas);
                canvas.restore();
            }
        }
    });
});

const expect3x3MatricesToMatch = (expected, actual) => {
    expect(expected.length).toEqual(9);
    expect(actual.length).toEqual(9);
    for (let i = 0; i < expected.length; i++) {
        expect(expected[i]).toBeCloseTo(actual[i], 5);
    }
};

const expect4x4MatricesToMatch = (expected, actual) => {
    expect(expected.length).toEqual(16);
    expect(actual.length).toEqual(16);
    for (let i = 0; i < expected.length; i++) {
        expect(expected[i]).toBeCloseTo(actual[i], 5);
    }
};
