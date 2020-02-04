describe('CanvasKit\'s Font Behavior', function() {
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

    let bungeeFontBuffer = null;
    // This font has tofu for incorrect null terminators
    // see https://bugs.chromium.org/p/skia/issues/detail?id=9314
    const bungeeFontLoaded = fetch('/assets/Bungee-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            bungeeFontBuffer = buffer;
        });

    it('can draw shaped and unshaped text', function(done) {
        Promise.all([LoadCanvasKit, notoSerifFontLoaded]).then(catchException(done, () => {
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
            const textFont = new CanvasKit.SkFont(notoSerif, 20);

            canvas.drawRect(CanvasKit.LTRBRect(30, 30, 200, 200), paint);
            canvas.drawText('This text is not shaped, and overflows the boundary',
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
        Promise.all([LoadCanvasKit, notoSerifFontLoaded]).then(catchException(done, () => {
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
        }));
    });

    // https://bugs.chromium.org/p/skia/issues/detail?id=9314
    it('does not draw tofu for null terminators at end of text', function(done) {
        Promise.all([LoadCanvasKit, bungeeFontLoaded]).then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const fontMgr = CanvasKit.SkFontMgr.RefDefault();
            const bungee = fontMgr.MakeTypefaceFromData(bungeeFontBuffer);

            const canvas = surface.getCanvas();
            // yellow, to make sure tofu is plainly visible
            canvas.clear(CanvasKit.Color(255, 255, 0, 1));

            const font = new CanvasKit.SkFont(bungee, 24);
            const fontPaint = new CanvasKit.SkPaint();
            fontPaint.setAntiAlias(true);
            fontPaint.setStyle(CanvasKit.PaintStyle.Fill);


            const str = 'This is téxt';
            const textBlob = CanvasKit.SkTextBlob.MakeFromText(str + ' text blob', font);

            canvas.drawTextBlob(textBlob, 10, 50, fontPaint);

            canvas.drawText(str + ' normal', 10, 100, fontPaint, font);

            canvas.drawText('null terminator ->\u0000<- on purpose', 10, 150, fontPaint, font);

            surface.flush();

            textBlob.delete();
            bungee.delete();
            font.delete();
            fontPaint.delete();
            fontMgr.delete();
            reportSurface(surface, 'nullterminators_skbug_9314', done);
        }));
    });

    it('can make a font mgr with passed in fonts', function(done) {
        Promise.all([LoadCanvasKit, bungeeFontLoaded, notoSerifFontLoaded]).then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            // CanvasKit.SkFontMgr.FromData([bungeeFontBuffer, notSerifFontBuffer]) also works
            const fontMgr = CanvasKit.SkFontMgr.FromData(bungeeFontBuffer, notSerifFontBuffer);
            expect(fontMgr).toBeTruthy();
            expect(fontMgr.countFamilies()).toBe(2);
            // in debug mode, let's list them.
            if (fontMgr.dumpFamilies) {
                fontMgr.dumpFamilies();
            }
            fontMgr.delete();
            done();
        }));
    });

    it('can load fonts of a variety of types', function(done) {
        let otfFontBuffer = null;
        const otfFontLoaded = fetch('/assets/Roboto-Regular.otf').then(
            (response) => response.arrayBuffer()).then(
            (buffer) => {
                otfFontBuffer = buffer;
            });

        let woffFontBuffer = null;
        const woffFontLoaded = fetch('/assets/Roboto-Regular.woff').then(
            (response) => response.arrayBuffer()).then(
            (buffer) => {
                woffFontBuffer = buffer;
            });

        let woff2FontBuffer = null;
        const woff2FontLoaded = fetch('/assets/Roboto-Regular.woff2').then(
            (response) => response.arrayBuffer()).then(
            (buffer) => {
                woff2FontBuffer = buffer;
            });

        let ttcFontBuffer = null;
        const ttcFontLoaded = fetch('/assets/test.ttc').then(
            (response) => response.arrayBuffer()).then(
            (buffer) => {
                ttcFontBuffer = buffer;
            });

        Promise.all([LoadCanvasKit, otfFontLoaded, bungeeFontLoaded, woffFontLoaded,
                     woff2FontLoaded, ttcFontLoaded]).then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const fontMgr = CanvasKit.SkFontMgr.RefDefault();
            const canvas = surface.getCanvas();
            const fontPaint = new CanvasKit.SkPaint();
            fontPaint.setAntiAlias(true);
            fontPaint.setStyle(CanvasKit.PaintStyle.Fill);
            const inputs = [{
                type: '.ttf font',
                buffer: bungeeFontBuffer,
                y: 60,
            },{
                type: '.otf font',
                buffer: otfFontBuffer,
                y: 90,
            },{
                // Not currently supported by Skia
                type: '.woff font',
                buffer: woffFontBuffer,
                y: 120,
            },{
                // Not currently supported by Skia
                type: '.woff2 font',
                buffer: woff2FontBuffer,
                y: 150,
            }];

            const defaultFont = new CanvasKit.SkFont(null, 24);
            canvas.drawText(`The following should be ${inputs.length + 1} lines of text:`, 5, 30, fontPaint, defaultFont);

            for (const fontType of inputs) {
                // smoke test that the font bytes loaded.
                expect(fontType.buffer).toBeTruthy(fontType.type + ' did not load');

                const typeface = fontMgr.MakeTypefaceFromData(fontType.buffer);
                const font = new CanvasKit.SkFont(typeface, 24);

                if (font && typeface) {
                    canvas.drawText(fontType.type + ' loaded', 5, fontType.y, fontPaint, font);
                } else {
                    canvas.drawText(fontType.type + ' *not* loaded', 5, fontType.y, fontPaint, defaultFont);
                }
                font && font.delete();
                typeface && typeface.delete();
            }

            // The only ttc font I could find was 14 MB big, so I'm using the smaller test font,
            // which doesn't have very many glyphs in it, so we just check that we got a non-zero
            // typeface for it. I was able to load NotoSansCJK-Regular.ttc just fine in a
            // manual test.
            expect(ttcFontBuffer).toBeTruthy('.ttc font did not load');
            const typeface = fontMgr.MakeTypefaceFromData(ttcFontBuffer);
            expect(typeface).toBeTruthy('.ttc font');
            if (typeface) {
                canvas.drawText('.ttc loaded', 5, 180, fontPaint, defaultFont);
                typeface.delete();
            } else {
                canvas.drawText('.ttc *not* loaded', 5, 180, fontPaint, defaultFont);
            }

            defaultFont.delete();
            fontPaint.delete();
            fontMgr.delete();
            reportSurface(surface, 'various_font_formats', done);
        }));
    });

});
