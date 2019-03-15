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

    let notSerifFontBuffer = null;
    // This font is known to support kerning
    const notoSerifFontLoaded = fetch('/assets/NotoSerif-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            notSerifFontBuffer = buffer;
        });

    it('can draw shaped and unshaped text', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            notoSerifFontLoaded.then(() => {
                // This is taken from example.html
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface')
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                const paint = new CanvasKit.SkPaint();

                paint.setColor(CanvasKit.BLUE);
                paint.setStyle(CanvasKit.PaintStyle.Stroke);

                const fontMgr = CanvasKit.SkFontMgr.RefDefault();
                const notoSerif = fontMgr.MakeTypefaceFromData(notSerifFontBuffer);

                const textPaint = new CanvasKit.SkPaint();
                // use the built-in monospace typeface.
                const textFont = new CanvasKit.SkFont(notoSerif, 20);

                canvas.drawRect(CanvasKit.LTRBRect(30, 30, 200, 200), paint);
                canvas.drawText('This text is not shaped, and overflows the boundry',
                                35, 50, textPaint, textFont);

                const shapedText = new CanvasKit.ShapedText({
                  font: textFont,
                  leftToRight: true,
                  text: 'This text *is* shaped, and wraps to the right width.',
                  width: 160,
                });
                const textBoxX = 35;
                const textBoxY = 55;
                canvas.drawText(shapedText, textBoxX, textBoxY, textPaint);
                const bounds = shapedText.getBounds();

                bounds.fLeft += textBoxX;
                bounds.fRight += textBoxX;
                bounds.fTop += textBoxY;
                bounds.fBottom += textBoxY

                canvas.drawRect(bounds, paint);
                const SHAPE_TEST_TEXT = 'VAVAVAVAVAFIfi';
                const textFont2 = new CanvasKit.SkFont(notoSerif, 60);
                const shapedText2 = new CanvasKit.ShapedText({
                  font: textFont2,
                  leftToRight: true,
                  text: SHAPE_TEST_TEXT,
                  width: 600,
                });

                canvas.drawText('no kerning ↓', 10, 240, textPaint, textFont);
                canvas.drawText(SHAPE_TEST_TEXT, 10, 300, textPaint, textFont2);
                canvas.drawText(shapedText2, 10, 300, textPaint);
                canvas.drawText('kerning ↑', 10, 390, textPaint, textFont);

                surface.flush();

                paint.delete();
                notoSerif.delete();
                textPaint.delete();
                textFont.delete();
                shapedText.delete();
                textFont2.delete();
                shapedText2.delete();
                fontMgr.delete();
                reportSurface(surface, 'text_shaping', done);
            });
        }));
    });

    it('can draw text following a path', function(done) {
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

            const font = new CanvasKit.SkFont(null, 24);
            const fontPaint = new CanvasKit.SkPaint();
            fontPaint.setAntiAlias(true);
            fontPaint.setStyle(CanvasKit.PaintStyle.Fill);


            const arc = new CanvasKit.SkPath();
            arc.arcTo(CanvasKit.LTRBRect(20, 40, 280, 300), -160, 140, true);
            arc.lineTo(210, 140);
            arc.arcTo(CanvasKit.LTRBRect(20, 0, 280, 260), 160, -140, true);

            // Only 1 dot should show up in the image, because we run out of path.
            const str = 'This téxt should follow the curve across contours...';
            const textBlob = CanvasKit.SkTextBlob.MakeOnPath(str, arc, font);

            canvas.drawPath(arc, paint);
            canvas.drawTextBlob(textBlob, 0, 0, fontPaint);

            surface.flush();

            textBlob.delete();
            arc.delete();
            paint.delete();
            font.delete();
            fontPaint.delete();

            reportSurface(surface, 'monospace_text_on_path', done);
        }));
    });

    it('can draw text following a path with a non-serif font', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            notoSerifFontLoaded.then(() => {
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface')
                if (!surface) {
                    done();
                    return;
                }
                const fontMgr = CanvasKit.SkFontMgr.RefDefault();
                const notoSerif = fontMgr.MakeTypefaceFromData(notSerifFontBuffer);

                const canvas = surface.getCanvas();
                const paint = new CanvasKit.SkPaint();
                paint.setAntiAlias(true);
                paint.setStyle(CanvasKit.PaintStyle.Stroke);

                const font = new CanvasKit.SkFont(notoSerif, 24);
                const fontPaint = new CanvasKit.SkPaint();
                fontPaint.setAntiAlias(true);
                fontPaint.setStyle(CanvasKit.PaintStyle.Fill);


                const arc = new CanvasKit.SkPath();
                arc.arcTo(CanvasKit.LTRBRect(20, 40, 280, 300), -160, 140, true);
                arc.lineTo(210, 140);
                arc.arcTo(CanvasKit.LTRBRect(20, 0, 280, 260), 160, -140, true);

                const str = 'This téxt should follow the curve across contours...';
                const textBlob = CanvasKit.SkTextBlob.MakeOnPath(str, arc, font, 60.5);

                canvas.drawPath(arc, paint);
                canvas.drawTextBlob(textBlob, 0, 0, fontPaint);

                surface.flush();

                textBlob.delete();
                arc.delete();
                paint.delete();
                notoSerif.delete();
                font.delete();
                fontPaint.delete();
                fontMgr.delete();
                reportSurface(surface, 'serif_text_on_path', done);
            });
        }));
    });

    // TODO more tests
});
