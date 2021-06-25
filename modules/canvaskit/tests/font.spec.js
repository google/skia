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

    gm('monospace_text_on_path', (canvas) => {
        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const font = new CanvasKit.Font(null, 24);
        const fontPaint = new CanvasKit.Paint();
        fontPaint.setAntiAlias(true);
        fontPaint.setStyle(CanvasKit.PaintStyle.Fill);


        const arc = new CanvasKit.Path();
        arc.arcToOval(CanvasKit.LTRBRect(20, 40, 280, 300), -160, 140, true);
        arc.lineTo(210, 140);
        arc.arcToOval(CanvasKit.LTRBRect(20, 0, 280, 260), 160, -140, true);

        // Only 1 dot should show up in the image, because we run out of path.
        const str = 'This téxt should follow the curve across contours...';
        const textBlob = CanvasKit.TextBlob.MakeOnPath(str, arc, font);

        canvas.drawPath(arc, paint);
        canvas.drawTextBlob(textBlob, 0, 0, fontPaint);

        textBlob.delete();
        arc.delete();
        paint.delete();
        font.delete();
        fontPaint.delete();
    });

    gm('serif_text_on_path', (canvas) => {
        const notoSerif = CanvasKit.Typeface.MakeFreeTypeFaceFromData(notoSerifFontBuffer);

        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const font = new CanvasKit.Font(notoSerif, 24);
        const fontPaint = new CanvasKit.Paint();
        fontPaint.setAntiAlias(true);
        fontPaint.setStyle(CanvasKit.PaintStyle.Fill);

        const arc = new CanvasKit.Path();
        arc.arcToOval(CanvasKit.LTRBRect(20, 40, 280, 300), -160, 140, true);
        arc.lineTo(210, 140);
        arc.arcToOval(CanvasKit.LTRBRect(20, 0, 280, 260), 160, -140, true);

        const str = 'This téxt should follow the curve across contours...';
        const textBlob = CanvasKit.TextBlob.MakeOnPath(str, arc, font, 60.5);

        canvas.drawPath(arc, paint);
        canvas.drawTextBlob(textBlob, 0, 0, fontPaint);

        textBlob.delete();
        arc.delete();
        paint.delete();
        notoSerif.delete();
        font.delete();
        fontPaint.delete();
    });

    // https://bugs.chromium.org/p/skia/issues/detail?id=9314
    gm('nullterminators_skbug_9314', (canvas) => {
        const bungee = CanvasKit.Typeface.MakeFreeTypeFaceFromData(bungeeFontBuffer);

        // yellow, to make sure tofu is plainly visible
        canvas.clear(CanvasKit.Color(255, 255, 0, 1));

        const font = new CanvasKit.Font(bungee, 24);
        const fontPaint = new CanvasKit.Paint();
        fontPaint.setAntiAlias(true);
        fontPaint.setStyle(CanvasKit.PaintStyle.Fill);


        const str = 'This is téxt';
        const textBlob = CanvasKit.TextBlob.MakeFromText(str + ' text blob', font);

        canvas.drawTextBlob(textBlob, 10, 50, fontPaint);

        canvas.drawText(str + ' normal', 10, 100, fontPaint, font);

        canvas.drawText('null terminator ->\u0000<- on purpose', 10, 150, fontPaint, font);

        textBlob.delete();
        bungee.delete();
        font.delete();
        fontPaint.delete();
    });

    gm('textblobs_with_glyphs', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        const notoSerif = CanvasKit.Typeface.MakeFreeTypeFaceFromData(notoSerifFontBuffer);

        const font = new CanvasKit.Font(notoSerif, 24);
        const bluePaint = new CanvasKit.Paint();
        bluePaint.setColor(CanvasKit.parseColorString('#04083f')); // arbitrary deep blue
        bluePaint.setAntiAlias(true);
        bluePaint.setStyle(CanvasKit.PaintStyle.Fill);

        const redPaint = new CanvasKit.Paint();
        redPaint.setColor(CanvasKit.parseColorString('#770b1e')); // arbitrary deep red

        const ids = notoSerif.getGlyphIDs('AEGIS ægis');
        expect(ids.length).toEqual(10); // one glyph id per glyph
        expect(ids[0]).toEqual(36); // spot check this, should be consistent as long as the font is.

        const bounds = font.getGlyphBounds(ids, bluePaint);
        expect(bounds.length).toEqual(40); // 4 measurements per glyph
        expect(bounds[0]).toEqual(0); // again, spot check the measurements for the first glyph.
        expect(bounds[1]).toEqual(-17);
        expect(bounds[2]).toEqual(17);
        expect(bounds[3]).toEqual(0);

        const widths = font.getGlyphWidths(ids, bluePaint);
        expect(widths.length).toEqual(10); // 1 width per glyph
        expect(widths[0]).toEqual(17);

        const topBlob = CanvasKit.TextBlob.MakeFromGlyphs(ids, font);
        canvas.drawTextBlob(topBlob, 5, 30, bluePaint);
        canvas.drawTextBlob(topBlob, 5, 60, redPaint);
        topBlob.delete();

        const mIDs = CanvasKit.MallocGlyphIDs(ids.length);
        const mArr = mIDs.toTypedArray();
        mArr.set(ids);

        const mXforms = CanvasKit.Malloc(Float32Array, ids.length * 4);
        const mXformsArr = mXforms.toTypedArray();
        // Draw each glyph rotated slightly and slightly lower than the glyph before it.
        let currX = 0;
        for (let i = 0; i < ids.length; i++) {
            mXformsArr[i * 4] = Math.cos(-Math.PI / 16); // scos
            mXformsArr[i * 4 + 1] = Math.sin(-Math.PI / 16); // ssin
            mXformsArr[i * 4 + 2] = currX; // tx
            mXformsArr[i * 4 + 3] = i*2; // ty
            currX += widths[i];
        }

        const bottomBlob = CanvasKit.TextBlob.MakeFromRSXformGlyphs(mIDs, mXforms, font);
        canvas.drawTextBlob(bottomBlob, 5, 110, bluePaint);
        canvas.drawTextBlob(bottomBlob, 5, 140, redPaint);
        bottomBlob.delete();

        CanvasKit.Free(mIDs);
        CanvasKit.Free(mXforms);
        bluePaint.delete();
        redPaint.delete();
        notoSerif.delete();
        font.delete();
    });

    it('can make a font mgr with passed in fonts', () => {
        // CanvasKit.FontMgr.FromData([bungeeFontBuffer, notoSerifFontBuffer]) also works
        const fontMgr = CanvasKit.FontMgr.FromData(bungeeFontBuffer, notoSerifFontBuffer);
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
        const fontPaint = new CanvasKit.Paint();
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
            type: '.woff font',
            buffer: fetchedByteBuffers[1],
            y: 120,
        },{
            type: '.woff2 font',
            buffer: fetchedByteBuffers[2],
            y: 150,
        }];

        const defaultFont = new CanvasKit.Font(null, 24);
        canvas.drawText(`The following should be ${inputs.length + 1} lines of text:`, 5, 30, fontPaint, defaultFont);

        for (const fontType of inputs) {
            // smoke test that the font bytes loaded.
            expect(fontType.buffer).toBeTruthy(fontType.type + ' did not load');

            const typeface = CanvasKit.Typeface.MakeFreeTypeFaceFromData(fontType.buffer);
            const font = new CanvasKit.Font(typeface, 24);

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
        const typeface = CanvasKit.Typeface.MakeFreeTypeFaceFromData(fetchedByteBuffers[3]);
        expect(typeface).toBeTruthy('.ttc font');
        if (typeface) {
            canvas.drawText('.ttc loaded', 5, 180, fontPaint, defaultFont);
            typeface.delete();
        } else {
            canvas.drawText('.ttc *not* loaded', 5, 180, fontPaint, defaultFont);
        }

        defaultFont.delete();
        fontPaint.delete();
    }, '/assets/Roboto-Regular.otf', '/assets/Roboto-Regular.woff', '/assets/Roboto-Regular.woff2', '/assets/test.ttc');

    it('can measure text very precisely with proper settings', () => {
        const typeface = CanvasKit.Typeface.MakeFreeTypeFaceFromData(notoSerifFontBuffer);
        const fontSizes = [257, 100, 11];
        // The point of these values is to let us know 1) we can measure to sub-pixel levels
        // and 2) that measurements don't drastically change. If these change a little bit,
        // just update them with the new values. For super-accurate readings, one could
        // run a C++ snippet of code and compare the values, but that is likely unnecessary
        // unless we suspect a bug with the bindings.
        const expectedSizes = [241.06299, 93.79883, 10.31787];
        for (const idx in fontSizes) {
            const font = new CanvasKit.Font(typeface, fontSizes[idx]);
            font.setHinting(CanvasKit.FontHinting.None);
            font.setLinearMetrics(true);
            font.setSubpixel(true);

            const ids = font.getGlyphIDs('M');
            const widths = font.getGlyphWidths(ids);
            expect(widths[0]).toBeCloseTo(expectedSizes[idx], 5);
            font.delete();
        }

        typeface.delete();
    });

    gm('font_edging', (canvas) => {
        // Draw a small font scaled up to see the aliasing artifacts.
        canvas.scale(8, 8);
        canvas.clear(CanvasKit.WHITE);
        const notoSerif = CanvasKit.Typeface.MakeFreeTypeFaceFromData(notoSerifFontBuffer);

        const textPaint = new CanvasKit.Paint();
        const annotationFont = new CanvasKit.Font(notoSerif, 6);

        canvas.drawText('Default', 5, 5, textPaint, annotationFont);
        canvas.drawText('Alias', 5, 25, textPaint, annotationFont);
        canvas.drawText('AntiAlias', 5, 45, textPaint, annotationFont);
        canvas.drawText('Subpixel', 5, 65, textPaint, annotationFont);

        const testFont = new CanvasKit.Font(notoSerif, 20);

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
    });

    it('can get the intercepts of glyphs', () => {
        const font = new CanvasKit.Font(null, 100);
        const ids = font.getGlyphIDs('I');
        expect(ids.length).toEqual(1);

        // aim for the middle of the I at 100 point, expecting a hit
        let sects = font.getGlyphIntercepts(ids, [0, 0], -60, -40);
        expect(sects.length).toEqual(2, "expected one pair of intercepts");
        expect(sects[0]).toBeCloseTo(25.39063, 5);
        expect(sects[1]).toBeCloseTo(34.52148, 5);

        // aim below the baseline where we expect no intercepts
        sects = font.getGlyphIntercepts(ids, [0, 0], 20, 30);
        expect(sects.length).toEqual(0, "expected no intercepts");
        font.delete();
    });

    it('can use mallocd and normal arrays', () => {
        const font = new CanvasKit.Font(null, 100);
        const ids = font.getGlyphIDs('I');
        expect(ids.length).toEqual(1);
        const glyphID = ids[0];

        // aim for the middle of the I at 100 point, expecting a hit
        const sects = font.getGlyphIntercepts(Array.of(glyphID), Float32Array.of(0, 0), -60, -40);
        expect(sects.length).toEqual(2);
        expect(sects[0]).toBeLessThan(sects[1]);
        // these values were recorded from the first time it was run
        expect(sects[0]).toBeCloseTo(25.39063, 5);
        expect(sects[1]).toBeCloseTo(34.52148, 5);

        const free_list = [];   // will free CanvasKit.Malloc objects at the end

        // Want to exercise 4 different ways we can receive an array:
        //  1. normal array
        //  2. typed-array
        //  3. CanvasKit.Malloc typeed-array
        //  4. CavnasKit.Malloc (raw)

        const id_makers = [
            (id) => [ id ],
            (id) => new Uint16Array([ id ]),
            (id) => {
                const a = CanvasKit.Malloc(Uint16Array, 1);
                free_list.push(a);
                const ta = a.toTypedArray();
                ta[0] = id;
                return ta;  // return typed-array
            },
            (id) => {
                const a = CanvasKit.Malloc(Uint16Array, 1);
                free_list.push(a);
                a.toTypedArray()[0] = id;
                return a;   // return raw obj
            },
        ];
        const pos_makers = [
            (x, y) => [ x, y ],
            (x, y) => new Float32Array([ x, y ]),
            (x, y) => {
                const a = CanvasKit.Malloc(Float32Array, 2);
                free_list.push(a);
                const ta = a.toTypedArray();
                ta[0] = x;
                ta[1] = y;
                return ta;  // return typed-array
            },
            (x, y) => {
                const a = CanvasKit.Malloc(Float32Array, 2);
                free_list.push(a);
                const ta = a.toTypedArray();
                ta[0] = x;
                ta[1] = y;
                return a;   // return raw obj
            },
        ];

        for (const idm of id_makers) {
            for (const posm of pos_makers) {
                const s = font.getGlyphIntercepts(idm(glyphID), posm(0, 0), -60, -40);
                expect(s.length).toEqual(sects.length);
                for (let i = 0; i < s.length; ++i) {
                    expect(s[i]).toEqual(sects[i]);
                }
            }

        }

        free_list.forEach(obj => CanvasKit.Free(obj));
        font.delete();
    });

});
