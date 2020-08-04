describe('Font Behavior', () => {
    let container;

    let notoSerifFontBuffer = null;
    // This font is known to support kerning
    const notoSerifFontLoaded = fetch('/assets/NotoSerif-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            notoSerifFontBuffer = buffer;
        });

    let bungeeFontBuffer = null;
    // This font has tofu for incorrect null terminators
    // see https://bugs.chromium.org/p/skia/issues/detail?id=9314
    const bungeeFontLoaded = fetch('/assets/Bungee-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            bungeeFontBuffer = buffer;
        });

    beforeEach(async () => {
        await LoadCanvasKit;
        await notoSerifFontLoaded;
        await bungeeFontLoaded;
        container = document.createElement('div');
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    gm('text_shaping', (canvas) => {
        const paint = new CanvasKit.SkPaint();

        paint.setColor(CanvasKit.BLUE);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const fontMgr = CanvasKit.SkFontMgr.RefDefault();
        const notoSerif = fontMgr.MakeTypefaceFromData(notoSerifFontBuffer);

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

        paint.delete();
        notoSerif.delete();
        textPaint.delete();
        textFont.delete();
        shapedText.delete();
        textFont2.delete();
        shapedText2.delete();
        fontMgr.delete();
    });

    gm('monospace_text_on_path', (canvas) => {
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

        textBlob.delete();
        arc.delete();
        paint.delete();
        font.delete();
        fontPaint.delete();
    });

    gm('serif_text_on_path', (canvas) => {
        const fontMgr = CanvasKit.SkFontMgr.RefDefault();
        const notoSerif = fontMgr.MakeTypefaceFromData(notoSerifFontBuffer);

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

        textBlob.delete();
        arc.delete();
        paint.delete();
        notoSerif.delete();
        font.delete();
        fontPaint.delete();
        fontMgr.delete();
    });

    // https://bugs.chromium.org/p/skia/issues/detail?id=9314
    gm('nullterminators_skbug_9314', (canvas) => {
        const fontMgr = CanvasKit.SkFontMgr.RefDefault();
        const bungee = fontMgr.MakeTypefaceFromData(bungeeFontBuffer);

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

        textBlob.delete();
        bungee.delete();
        font.delete();
        fontPaint.delete();
        fontMgr.delete();
    });

    it('can make a font mgr with passed in fonts', () => {
        // CanvasKit.SkFontMgr.FromData([bungeeFontBuffer, notoSerifFontBuffer]) also works
        const fontMgr = CanvasKit.SkFontMgr.FromData(bungeeFontBuffer, notoSerifFontBuffer);
        expect(fontMgr).toBeTruthy();
        expect(fontMgr.countFamilies()).toBe(2);
        // in debug mode, let's list them.
        if (fontMgr.dumpFamilies) {
            fontMgr.dumpFamilies();
        }
        fontMgr.delete();
    });

    it('can make a font provider with passed in fonts and aliases', () => {
        const fontProvider = CanvasKit.TypefaceFontProvider.Make();
        fontProvider.registerFont(bungeeFontBuffer, "My Bungee Alias");
        fontProvider.registerFont(notoSerifFontBuffer, "My Noto Serif Alias");
        expect(fontProvider).toBeTruthy();
        expect(fontProvider.countFamilies()).toBe(2);
        // in debug mode, let's list them.
        if (fontProvider.dumpFamilies) {
            fontProvider.dumpFamilies();
        }
        fontProvider.delete();
    });

    gm('various_font_formats', (canvas, fetchedByteBuffers) => {
        const fontMgr = CanvasKit.SkFontMgr.RefDefault();
        const fontPaint = new CanvasKit.SkPaint();
        fontPaint.setAntiAlias(true);
        fontPaint.setStyle(CanvasKit.PaintStyle.Fill);
        const inputs = [{
            type: '.ttf font',
            buffer: bungeeFontBuffer,
            y: 60,
        },{
            type: '.otf font',
            buffer: fetchedByteBuffers[0],
            y: 90,
        },{
            // Not currently supported by Skia
            type: '.woff font',
            buffer: fetchedByteBuffers[1],
            y: 120,
        },{
            // Not currently supported by Skia
            type: '.woff2 font',
            buffer: fetchedByteBuffers[2],
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
        const typeface = fontMgr.MakeTypefaceFromData(fetchedByteBuffers[3]);
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
    }, '/assets/Roboto-Regular.otf', '/assets/Roboto-Regular.woff', '/assets/Roboto-Regular.woff2', '/assets/test.ttc');

    it('can measure text very precisely with proper settings', () => {
        const fontMgr = CanvasKit.SkFontMgr.RefDefault();
        const typeface = fontMgr.MakeTypefaceFromData(notoSerifFontBuffer);
        const fontSizes = [257, 100, 11];
        // The point of these values is to let us know 1) we can measure to sub-pixel levels
        // and 2) that measurements don't drastically change. If these change a little bit,
        // just update them with the new values. For super-accurate readings, one could
        // run a C++ snippet of code and compare the values, but that is likely unnecessary
        // unless we suspect a bug with the bindings.
        const expectedSizes = [1178.71143, 458.64258, 50.450683];
        for (const idx in fontSizes) {
            const font = new CanvasKit.SkFont(typeface, fontSizes[idx]);
            font.setHinting(CanvasKit.FontHinting.None);
            font.setLinearMetrics(true);
            font.setSubpixel(true);

            const res = font.measureText('someText');
            expect(res).toBeCloseTo(expectedSizes[idx], 5);
            font.delete();
        }

        typeface.delete();
        fontMgr.delete();
    });

    gm('font_edging', (canvas) => {
        // Draw a small font scaled up to see the aliasing artifacts.
        canvas.scale(8, 8);
        canvas.clear(CanvasKit.WHITE);
        const fontMgr = CanvasKit.SkFontMgr.RefDefault();
        const notoSerif = fontMgr.MakeTypefaceFromData(notoSerifFontBuffer);

        const textPaint = new CanvasKit.SkPaint();
        const annotationFont = new CanvasKit.SkFont(notoSerif, 6);

        canvas.drawText('Default', 5, 5, textPaint, annotationFont);
        canvas.drawText('Alias', 5, 25, textPaint, annotationFont);
        canvas.drawText('AntiAlias', 5, 45, textPaint, annotationFont);
        canvas.drawText('Subpixel', 5, 65, textPaint, annotationFont);

        const testFont = new CanvasKit.SkFont(notoSerif, 20);

        canvas.drawText('SEA', 35, 15, textPaint, testFont);
        testFont.setEdging(CanvasKit.FontEdging.Alias);
        canvas.drawText('SEA', 35, 35, textPaint, testFont);
        testFont.setEdging(CanvasKit.FontEdging.AntiAlias);
        canvas.drawText('SEA', 35, 55, textPaint, testFont);
        testFont.setEdging(CanvasKit.FontEdging.SubpixelAntiAlias);
        canvas.drawText('SEA', 35, 75, textPaint, testFont);

        textPaint.delete();
        annotationFont.delete();
        testFont.delete();
        notoSerif.delete();
        fontMgr.delete();
    });

});
