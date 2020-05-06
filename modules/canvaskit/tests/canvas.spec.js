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

        font.delete();
        blob.delete();
        paint.delete();
        // See canvas2d for more API tests
    });

    gm('effect_and_text_example', (canvas) => {
        const path = starPath(CanvasKit);
        const paint = new CanvasKit.SkPaint();

        const textPaint = new CanvasKit.SkPaint();
        textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
        textPaint.setAntiAlias(true);

        const textFont = new CanvasKit.SkFont(null, 30);

        const dpe = CanvasKit.SkPathEffect.MakeDash([15, 5, 5, 10], 1);

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
        const paint = new CanvasKit.SkPaint();

        const cornerEffect = CanvasKit.SkPathEffect.MakeCorner(10);
        const discreteEffect = CanvasKit.SkPathEffect.MakeDiscrete(5, 10, 0);

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
        const canvas = new CanvasKit.SkCanvas();
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

        path.delete();
        paint.delete();
    });

    gm('rrect_canvas', (canvas) => {
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

        path.delete();
        paint.delete();
    });

    gm('rrect_8corners_canvas', (canvas) => {
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

        path.delete();
        paint.delete();
    });

    gm('drawDRRect_canvas', (canvas) => {
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

        path.delete();
        paint.delete();
    });

    gm('colorfilters_canvas', (canvas) => {
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

        paint.delete();
        blue.delete();
        red.delete();
        lerp.delete();
        final.delete();
    });

    gm('colorfilters_malloc_canvas', (canvas) => {
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
        // MakeMatrix will free the malloc'd array when it is done with it.
        const final = CanvasKit.SkColorFilter.MakeMatrix(cm);

        paint.setColorFilter(final)
        canvas.drawRect(CanvasKit.LTRBRect(10, 70, 140, 120), paint);

        paint.delete();
        final.delete();
    });

    gm('clips_canvas', (canvas) => {
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

        path.delete();
        paint.delete();
    });

    // inspired by https://fiddle.skia.org/c/feb2a08bb09ede5309678d6a0ab3f981
    gm('savelayer_rect_paint_canvas', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        const redPaint = new CanvasKit.SkPaint();
        redPaint.setColor(CanvasKit.RED);
        const solidBluePaint = new CanvasKit.SkPaint();
        solidBluePaint.setColor(CanvasKit.BLUE);

        const thirtyBluePaint = new CanvasKit.SkPaint();
        thirtyBluePaint.setColor(CanvasKit.BLUE);
        thirtyBluePaint.setAlphaf(0.3);

        const alpha = new CanvasKit.SkPaint();
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
        canvas.saveLayer(CanvasKit.LTRBRect(10, 10, 220, 180), alpha);

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
        const redPaint = new CanvasKit.SkPaint();
        redPaint.setColor(CanvasKit.RED);
        const solidBluePaint = new CanvasKit.SkPaint();
        solidBluePaint.setColor(CanvasKit.BLUE);

        const thirtyBluePaint = new CanvasKit.SkPaint();
        thirtyBluePaint.setColor(CanvasKit.BLUE);
        thirtyBluePaint.setAlphaf(0.3);

        const alpha = new CanvasKit.SkPaint();
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
        canvas.saveLayer(alpha);

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

        blurIF.delete();
        redPaint.delete();
        bluePaint.delete();
    });

    gm('drawpoints_canvas', (canvas) => {
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

        paint.delete();
    });

    gm('drawImageNine_canvas', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();

        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.SkPaint();

        canvas.drawImageNine(img, {
            fLeft: 40,
            fTop: 40,
            fRight: 400,
            fBottom: 300,
        }, CanvasKit.LTRBRect(5, 5, 300, 650), paint);
        paint.delete();
        img.delete();
    }, '/assets/mandrill_512.png');

    gm('drawvertices_canvas', (canvas) => {
        const paint = new CanvasKit.SkPaint();
        paint.setAntiAlias(true);

        const points = [[ 0, 0 ], [ 250, 0 ], [ 100, 100 ], [ 0, 250 ]];
        const colors = [CanvasKit.RED, CanvasKit.BLUE,
                      CanvasKit.YELLOW, CanvasKit.CYAN];
        const vertices = CanvasKit.MakeSkVertices(CanvasKit.VertexMode.TriangleFan,
            points, null /*textureCoordinates*/, colors, false /*isVolatile*/);

        const bounds = vertices.bounds();
        expect(bounds.fLeft).toEqual(0);
        expect(bounds.fTop).toEqual(0);
        expect(bounds.fRight).toEqual(250);
        expect(bounds.fBottom).toEqual(250);

        canvas.drawVertices(vertices, CanvasKit.BlendMode.Src, paint);
        vertices.delete();
        paint.delete();
    });

    gm('drawvertices_texture_canvas', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);

        const paint = new CanvasKit.SkPaint();
        paint.setAntiAlias(true);

        const points = [
            [ 70, 170 ], [ 40, 90 ], [ 130, 150 ], [ 100, 50 ],
            [ 225, 150 ], [ 225, 60 ], [ 310, 180 ], [ 330, 100 ]
        ];
        const textureCoordinates = [
            [ 0, 240 ], [ 0, 0 ], [ 80, 240 ], [ 80, 0 ],
            [ 160, 240 ], [ 160, 0 ], [ 240, 240 ], [ 240, 0 ]
        ];
        const vertices = CanvasKit.MakeSkVertices(CanvasKit.VertexMode.TrianglesStrip,
            points, textureCoordinates, null /* colors */, false /*isVolatile*/);

        const shader = img.makeShader(CanvasKit.TileMode.Repeat, CanvasKit.TileMode.Mirror);
        paint.setShader(shader);
        canvas.drawVertices(vertices, CanvasKit.BlendMode.Src, paint);

        shader.delete();
        vertices.delete();
        paint.delete();
        img.delete();
    }, '/assets/brickwork-texture.jpg');

    it('can change the 3x3 matrix on the canvas and read it back', () => {
        const canvas = new CanvasKit.SkCanvas();

        let matr = canvas.getTotalMatrix();
        expect(matr).toEqual(CanvasKit.SkMatrix.identity());

        canvas.concat(CanvasKit.SkMatrix.rotated(Math.PI/4));
        const d = new DOMMatrix().translate(20, 10);
        canvas.concat(d);

        matr = canvas.getTotalMatrix();
        const expected = CanvasKit.SkMatrix.multiply(
            CanvasKit.SkMatrix.rotated(Math.PI/4),
            CanvasKit.SkMatrix.translated(20, 10)
        );
        expect3x3MatricesToMatch(expected, matr);
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

    it('can mark a CTM and retrieve it', () => {
        const canvas = new CanvasKit.SkCanvas();

        canvas.concat(CanvasKit.SkM44.rotated([0, 1, 0], Math.PI/4));
        canvas.concat(CanvasKit.SkM44.rotated([1, 0, 1], Math.PI/8));
        canvas.markCTM('krispykreme');

        const expected = CanvasKit.SkM44.multiply(
          CanvasKit.SkM44.rotated([0, 1, 0], Math.PI/4),
          CanvasKit.SkM44.rotated([1, 0, 1], Math.PI/8),
        );

        expect4x4MatricesToMatch(expected, canvas.findMarkedCTM('krispykreme'));
    });

    it('returns null for an invalid CTM marker', () => {
        const canvas = new CanvasKit.SkCanvas();
        expect(canvas.findMarkedCTM('dunkindonuts')).toBeNull();
    });

    it('can change the 4x4 matrix on the canvas and read it back', () => {
        const canvas = new CanvasKit.SkCanvas();

        let matr = canvas.getLocalToDevice();
        expect(matr).toEqual(CanvasKit.SkM44.identity());

        canvas.concat(CanvasKit.SkM44.rotated([0, 1, 0], Math.PI/4));
        canvas.concat(CanvasKit.SkM44.rotated([1, 0, 1], Math.PI/8));

        const expected = CanvasKit.SkM44.multiply(
          CanvasKit.SkM44.rotated([0, 1, 0], Math.PI/4),
          CanvasKit.SkM44.rotated([1, 0, 1], Math.PI/8),
        );

        expect4x4MatricesToMatch(expected, canvas.getLocalToDevice());
        // TODO(kjlubick) add test for DOMMatrix
        // TODO(nifong) add more involved test for camera-related math.
    });

    gm('concat_with4x4_canvas', (canvas) => {
        const path = starPath(CanvasKit, CANVAS_WIDTH/2, CANVAS_HEIGHT/2);
        const paint = new CanvasKit.SkPaint();
        paint.setAntiAlias(true);
        canvas.clear(CanvasKit.WHITE);

        // Rotate it a bit on all 3 major axis, centered on the screen.
        // To play with rotations, see https://jsfiddle.skia.org/canvaskit/0525300405796aa87c3b84cc0d5748516fca0045d7d6d9c7840710ab771edcd4
        const turn = CanvasKit.SkM44.multiply(
          CanvasKit.SkM44.translated([CANVAS_WIDTH/2, 0, 0]),
          CanvasKit.SkM44.rotated([1, 0, 0], Math.PI/3),
          CanvasKit.SkM44.rotated([0, 1, 0], Math.PI/4),
          CanvasKit.SkM44.rotated([0, 0, 1], Math.PI/16),
          CanvasKit.SkM44.translated([-CANVAS_WIDTH/2, 0, 0]),
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

});
