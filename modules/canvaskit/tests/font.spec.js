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

    it('can draw shaped and unshaped text', function(done) {
        let fontBuffer = null;

        // This font is known to support kerning
        const skFontLoaded = fetch('/assets/NotoSerif-Regular.ttf').then(
            (response) => response.arrayBuffer()).then(
            (buffer) => {
                fontBuffer = buffer;
            });

        LoadCanvasKit.then(catchException(done, () => {
            skFontLoaded.then(() => {
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
                const notoSerif = fontMgr.MakeTypefaceFromData(fontBuffer);

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
                reportSurface(surface, 'text_shaping', done);
            });
        }));
    });

    // TODO more tests
});
