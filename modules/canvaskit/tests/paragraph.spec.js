describe('Paragraph Behavior', function() {
    let container;

    let notoSerifFontBuffer = null;
    // This font is known to support kerning
    const notoSerifFontLoaded = fetch('/assets/NotoSerif-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            notoSerifFontBuffer = buffer;
        });

    let notoSerifBoldItalicFontBuffer = null;
    const notoSerifBoldItalicFontLoaded = fetch('/assets/NotoSerif-BoldItalic.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            notoSerifBoldItalicFontBuffer = buffer;
        });

    let emojiFontBuffer = null;
    const emojiFontLoaded = fetch('/assets/NotoColorEmoji.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            emojiFontBuffer = buffer;
        });

    let robotoFontBuffer = null;
    const robotoFontLoaded = fetch('/assets/Roboto-Regular.otf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            robotoFontBuffer = buffer;
        });

    beforeEach(async () => {
        await LoadCanvasKit;
        await notoSerifFontLoaded;
        await notoSerifBoldItalicFontLoaded;
        await emojiFontLoaded;
        container = document.createElement('div');
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    gm('paragraph_basic', (canvas) => {
        const paint = new CanvasKit.Paint();

        paint.setColor(CanvasKit.RED);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');

        const wrapTo = 200;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.BLACK,
                fontFamilies: ['Noto Serif'],
                fontSize: 20,
            },
            textAlign: CanvasKit.TextAlign.Center,
            maxLines: 8,
            ellipsis: '.._.',
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('VAVAVAVAVAVAVA\nVAVA\n');

        const blueText = new CanvasKit.TextStyle({
            backgroundColor: CanvasKit.Color(234, 208, 232), // light pink
            color: CanvasKit.Color(48, 37, 199),
            fontFamilies: ['Noto Serif'],
            decoration: CanvasKit.LineThroughDecoration,
            decorationThickness: 1.5, // multiplier based on font size
            fontSize: 24,
        });
        builder.pushStyle(blueText);
        builder.addText(`Gosh I hope this wraps at some point, it is such a long line.`)
        builder.pop();
        builder.addText(` I'm done with the blue now. `)
        builder.addText(`Now I hope we should stop before we get 8 lines tall. `);
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        expect(paragraph.didExceedMaxLines()).toBeTruthy();
        expect(paragraph.getAlphabeticBaseline()).toBeCloseTo(21.377, 3);
        expect(paragraph.getHeight()).toEqual(240);
        expect(paragraph.getIdeographicBaseline()).toBeCloseTo(27.236, 3);
        expect(paragraph.getLongestLine()).toBeCloseTo(193.820, 3);
        expect(paragraph.getMaxIntrinsicWidth()).toBeCloseTo(1444.250, 3);
        expect(paragraph.getMaxWidth()).toEqual(200);
        expect(paragraph.getMinIntrinsicWidth()).toBeCloseTo(172.360, 3);
        expect(paragraph.getWordBoundary(8)).toEqual({
            start: 0,
            end: 14,
        });
        expect(paragraph.getWordBoundary(25)).toEqual({
            start: 25,
            end: 26,
        });


        const lineMetrics = paragraph.getLineMetrics();
        expect(lineMetrics.length).toEqual(8); // 8 lines worth of metrics
        const flm = lineMetrics[0]; // First Line Metric
        expect(flm.startIndex).toEqual(0);
        expect(flm.endExcludingWhitespaces).toEqual(14)
        expect(flm.endIndex).toEqual(14); // Including whitespaces but excluding newlines
        expect(flm.endIncludingNewline).toEqual(15);
        expect(flm.lineNumber).toEqual(0);
        expect(flm.isHardBreak).toEqual(true);
        expect(flm.ascent).toBeCloseTo(21.377, 3);
        expect(flm.descent).toBeCloseTo(5.859, 3);
        expect(flm.height).toBeCloseTo(27.000, 3);
        expect(flm.width).toBeCloseTo(172.360, 3);
        expect(flm.left).toBeCloseTo(13.818, 3);
        expect(flm.baseline).toBeCloseTo(21.141, 3);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, 230), paint);
        canvas.drawParagraph(paragraph, 10, 10);

        paint.delete();
        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_foreground_and_background_color', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');

        const wrapTo = 200;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                foregroundColor: CanvasKit.Color4f(1.0, 0, 0, 0.8),
                backgroundColor: CanvasKit.Color4f(0, 0, 1.0, 0.8),
                // color should default to black
                fontFamilies: ['Noto Serif'],
                fontSize: 20,
            },

            textAlign: CanvasKit.TextAlign.Center,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText(
            'This text has a red foregroundColor and a blue backgroundColor.');
        const paragraph = builder.build();
        paragraph.layout(300);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_foreground_stroke_paint', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');

        const wrapTo = 200;

        const textStyle = {
            fontFamilies: ['Noto Serif'],
            fontSize: 40,
        };
        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: textStyle,
            textAlign: CanvasKit.TextAlign.Center,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);

        const fg = new CanvasKit.Paint();
        fg.setColor(CanvasKit.BLACK);
        fg.setStyle(CanvasKit.PaintStyle.Stroke);

        const bg = new CanvasKit.Paint();
        bg.setColor(CanvasKit.TRANSPARENT);

        builder.pushPaintStyle(textStyle, fg, bg);
        builder.addText(
            'This text is stroked in black and has no fill');
        const paragraph = builder.build();
        paragraph.layout(300);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);
        // Again 5px to the right so you can tell the fill is transparent
        canvas.drawParagraph(paragraph, 15, 10);

        fg.delete();
        bg.delete();
        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_letter_word_spacing', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');

        const wrapTo = 200;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                // color should default to black
                fontFamilies: ['Noto Serif'],
                fontSize: 20,
                letterSpacing: 5,
                wordSpacing: 10,
            },

            textAlign: CanvasKit.TextAlign.Center,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText(
            'This text should have a lot of space between the letters and words.');
        const paragraph = builder.build();
        paragraph.layout(300);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_shadows', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');

        const wrapTo = 200;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.WHITE,
                fontFamilies: ['Noto Serif'],
                fontSize: 20,
                shadows: [{color: CanvasKit.BLACK, blurRadius: 15},
                          {color: CanvasKit.RED, blurRadius: 5, offset: [10, 10]}],
            },

            textAlign: CanvasKit.TextAlign.Center,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('This text should have a shadow behind it.');
        const paragraph = builder.build();
        paragraph.layout(300);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_strut_style', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(robotoFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Roboto');

        // The lines in this paragraph should have the same height despite the third
        // line having a larger font size.
        const paraStrutStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                fontFamilies: ['Roboto'],
                color: CanvasKit.BLACK,
            },
            strutStyle: {
                strutEnabled: true,
                fontFamilies: ['Roboto'],
                fontSize: 28,
                heightMultiplier: 1.5,
                forceStrutHeight: true,
            },
        });
        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                fontFamilies: ['Roboto'],
                color: CanvasKit.BLACK,
            },
        });
        const roboto28Style = new CanvasKit.TextStyle({
            color: CanvasKit.BLACK,
            fontFamilies: ['Roboto'],
            fontSize: 28,
        });
        const roboto32Style = new CanvasKit.TextStyle({
            color: CanvasKit.BLACK,
            fontFamilies: ['Roboto'],
            fontSize: 32,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStrutStyle, fontMgr);
        builder.pushStyle(roboto28Style);
        builder.addText('This paragraph\n');
        builder.pushStyle(roboto32Style);
        builder.addText('is using\n');
        builder.pop();
        builder.pushStyle(roboto28Style);
        builder.addText('a strut style!\n');
        builder.pop();
        builder.pop();

        const builder2 = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder2.pushStyle(roboto28Style);
        builder2.addText('This paragraph\n');
        builder2.pushStyle(roboto32Style);
        builder2.addText('is not using\n');
        builder2.pop();
        builder2.pushStyle(roboto28Style);
        builder2.addText('a strut style!\n');
        builder2.pop();
        builder2.pop();

        const paragraph = builder.build();
        paragraph.layout(300);

        const paragraph2 = builder2.build();
        paragraph2.layout(300);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);
        canvas.drawParagraph(paragraph2, 220, 10);

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_font_features', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(robotoFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Roboto');


        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.BLACK,
                fontFamilies: ['Roboto'],
                fontSize: 30,
                fontFeatures: [{name: 'smcp', value: 1}]
            },
            textAlign: CanvasKit.TextAlign.Center,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('This Text Should Be In Small Caps');
        const paragraph = builder.build();
        paragraph.layout(300);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_placeholders', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(robotoFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Roboto');


        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.BLACK,
                fontFamilies: ['Roboto'],
                fontSize: 20,
            },
            textAlign: CanvasKit.TextAlign.Center,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('There should be ');
        builder.addPlaceholder(10, 10, CanvasKit.PlaceholderAlignment.AboveBaseline,
                               CanvasKit.TextBaseline.Ideographic);
        builder.addText('a space in this sentence.\n');

        builder.addText('There should be ');
        builder.addPlaceholder(10, 10, CanvasKit.PlaceholderAlignment.BelowBaseline,
                               CanvasKit.TextBaseline.Ideographic);
        builder.addText('a dropped space in this sentence.\n');

        builder.addText('There should be ');
        builder.addPlaceholder(10, 10, null, null, 20);
        builder.addText('an offset space in this sentence.\n');
        const paragraph = builder.build();
        paragraph.layout(300);

        let rects = paragraph.getRectsForPlaceholders();

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);

        for (const rect of rects) {
            const p = new CanvasKit.Paint();
            p.setColor(CanvasKit.Color(0, 0, 255));
            p.setStyle(CanvasKit.PaintStyle.Stroke);
            // Account for the (10, 10) offset when we painted the paragraph.
            const placeholder =
                CanvasKit.LTRBRect(rect[0]+10,rect[1]+10,rect[2]+10,rect[3]+10);
            canvas.drawRect(placeholder, p);
            p.delete();
        }

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    // loosely based on SkParagraph_GetRectsForRangeParagraph test in c++ code.
    gm('paragraph_rects', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);

        const wrapTo = 550;
        const hStyle = CanvasKit.RectHeightStyle.Max;
        const wStyle = CanvasKit.RectWidthStyle.Tight;

        const mallocedColor = CanvasKit.Malloc(Float32Array, 4);
        mallocedColor.toTypedArray().set([0.9, 0.1, 0.1, 1.0]);

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: mallocedColor,
                fontFamilies: ['Noto Serif'],
                fontSize: 50,
            },
            textAlign: CanvasKit.TextAlign.Left,
            maxLines: 10,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('12345,  \"67890\" 12345 67890 12345 67890 12345 67890 12345 67890 12345 67890 12345');
        const paragraph = builder.build();
        CanvasKit.Free(mallocedColor);

        paragraph.layout(wrapTo);

        const ranges = [
            {
                start: 0,
                end: 0,
                expectedNum: 0,
            },
            {
                start: 0,
                end: 1,
                expectedNum: 1,
                color: CanvasKit.Color(200, 0, 200),
            },
            {
                start: 2,
                end: 8,
                expectedNum: 1,
                color: CanvasKit.Color(255, 0, 0),
            },
            {
                start: 8,
                end: 21,
                expectedNum: 1,
                color: CanvasKit.Color(0, 255, 0),
            },
            {
                start: 30,
                end: 100,
                expectedNum: 4,
                color: CanvasKit.Color(0, 0, 255),
            },
            {
                start: 19,
                end: 22,
                expectedNum: 1,
                color: CanvasKit.Color(0, 200, 200),
            }
        ];
        canvas.clear(CanvasKit.WHITE);
        // Move it down a bit so we can see the rects that go above 0,0
        canvas.translate(10, 10);
        canvas.drawParagraph(paragraph, 0, 0);

        for (const test of ranges) {
            let rects = paragraph.getRectsForRange(test.start, test.end, hStyle, wStyle);
            expect(Array.isArray(rects)).toEqual(true);
            expect(rects.length).toEqual(test.expectedNum);

            for (const rect of rects) {
                expect(rect.direction.value).toEqual(CanvasKit.TextDirection.LTR.value);
                const p = new CanvasKit.Paint();
                p.setColor(test.color);
                p.setStyle(CanvasKit.PaintStyle.Stroke);
                canvas.drawRect(rect, p);
                p.delete();
            }
        }
        expect(CanvasKit.RectHeightStyle.Strut).toBeTruthy();

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_emoji', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData([notoSerifFontBuffer, emojiFontBuffer]);
        expect(fontMgr.countFamilies()).toEqual(2);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');
        expect(fontMgr.getFamilyName(1)).toEqual('Noto Color Emoji');

        const wrapTo = 450;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.BLACK,
                // Put text first, otherwise the "emoji space" is used and that looks bad.
                fontFamilies: ['Noto Serif', 'Noto Color Emoji'],
                fontSize: 30,
            },
            textAlign: CanvasKit.TextAlign.Left,
            maxLines: 10,
        });

        const textStyle = new CanvasKit.TextStyle({
            color: CanvasKit.BLACK,
            // The number 4 matches an emoji and looks strange w/o this additional style.
            fontFamilies: ['Noto Serif'],
            fontSize: 30,
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.pushStyle(textStyle);
        builder.addText('4 flags on following line:\n');
        builder.pop();
        builder.addText(`🏳️‍🌈 🇮🇹 🇱🇷 🇺🇸\n`);
        builder.addText('Rainbow Italy Liberia USA\n\n');
        builder.addText('Emoji below should wrap:\n');
        builder.addText(`🍕🍔🍟🥝🍱🕶🎩👩‍👩‍👦👩‍👩‍👧‍👧👩‍👩‍👦👩‍👩‍👧‍👧👩‍👩‍👦👩‍👩‍👧‍👧👩‍👩‍👦👩‍👩‍👧‍👧👩‍👩‍👦👩‍👩‍👧‍👧👩‍👩‍👦👩‍👩‍👧‍👧👩‍👩‍👦👩‍👩‍👧‍👧`);
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 10, 10);

        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.RED);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, wrapTo+10), paint);

        fontMgr.delete();
        paint.delete();
        builder.delete();
        paragraph.delete();
    });

    gm('paragraph_hits', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData([notoSerifFontBuffer]);

        const wrapTo = 300;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.BLACK,
                fontFamilies: ['Noto Serif'],
                fontSize: 50,
            },
            textAlign: CanvasKit.TextAlign.Left,
            maxLines: 10,
        });
        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('UNCOPYRIGHTABLE');
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        canvas.clear(CanvasKit.WHITE);
        canvas.translate(10, 10);
        canvas.drawParagraph(paragraph, 0, 0);

        const paint = new CanvasKit.Paint();

        paint.setColor(CanvasKit.Color(255, 0, 0));
        paint.setStyle(CanvasKit.PaintStyle.Fill);
        canvas.drawCircle(20, 30, 3, paint);

        paint.setColor(CanvasKit.Color(0, 0, 255));
        canvas.drawCircle(80, 90, 3, paint);

        paint.setColor(CanvasKit.Color(0, 255, 0));
        canvas.drawCircle(280, 2, 3, paint);

        let posU = paragraph.getGlyphPositionAtCoordinate(20, 30);
        expect(posU).toEqual({
            pos: 1,
            affinity: CanvasKit.Affinity.Upstream
        });
        let posA = paragraph.getGlyphPositionAtCoordinate(80, 90);
        expect(posA).toEqual({
            pos: 11,
            affinity: CanvasKit.Affinity.Downstream
        });
        let posG = paragraph.getGlyphPositionAtCoordinate(280, 2);
        expect(posG).toEqual({
            pos: 9,
            affinity: CanvasKit.Affinity.Upstream
        });

        builder.delete();
        paragraph.delete();
        paint.delete();
        fontMgr.delete();
    });

    gm('paragraph_styles', (canvas) => {
        const paint = new CanvasKit.Paint();

        paint.setColor(CanvasKit.RED);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer, notoSerifBoldItalicFontBuffer);

        const wrapTo = 250;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                fontFamilies: ['Noto Serif'],
                fontSize: 20,
                fontStyle: {
                    weight: CanvasKit.FontWeight.Light,
                }
            },
            textDirection: CanvasKit.TextDirection.RTL,
            disableHinting: true,
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('Default text\n');

        const boldItalic = new CanvasKit.TextStyle({
            color: CanvasKit.RED,
            fontFamilies: ['Noto Serif'],
            fontSize: 20,
            fontStyle: {
                weight: CanvasKit.FontWeight.Bold,
                width: CanvasKit.FontWidth.Expanded,
                slant: CanvasKit.FontSlant.Italic,
            }
        });
        builder.pushStyle(boldItalic);
        builder.addText(`Bold, Expanded, Italic\n`);
        builder.pop();
        builder.addText(`back to normal`);
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        canvas.clear(CanvasKit.WHITE);

        canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, wrapTo+10), paint);
        canvas.drawParagraph(paragraph, 10, 10);

        paint.delete();
        paragraph.delete();
        builder.delete();
        fontMgr.delete();
    });

    gm('paragraph_font_provider', (canvas) => {
        const paint = new CanvasKit.Paint();

        paint.setColor(CanvasKit.RED);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        // Register Noto Serif as 'sans-serif'.
        const fontSrc = CanvasKit.TypefaceFontProvider.Make();
        fontSrc.registerFont(notoSerifFontBuffer, 'sans-serif');
        fontSrc.registerFont(notoSerifBoldItalicFontBuffer, 'sans-serif');

        const wrapTo = 250;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                fontFamilies: ['sans-serif'],
                fontSize: 20,
                fontStyle: {
                    weight: CanvasKit.FontWeight.Light,
                }
            },
            textDirection: CanvasKit.TextDirection.RTL,
            disableHinting: true,
        });

        const builder = CanvasKit.ParagraphBuilder.MakeFromFontProvider(paraStyle, fontSrc);
        builder.addText('Default text\n');

        const boldItalic = new CanvasKit.TextStyle({
            color: CanvasKit.RED,
            fontFamilies: ['sans-serif'],
            fontSize: 20,
            fontStyle: {
                weight: CanvasKit.FontWeight.Bold,
                width: CanvasKit.FontWidth.Expanded,
                slant: CanvasKit.FontSlant.Italic,
            }
        });
        builder.pushStyle(boldItalic);
        builder.addText(`Bold, Expanded, Italic\n`);
        builder.pop();
        builder.addText(`back to normal`);
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        canvas.clear(CanvasKit.WHITE);

        canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, wrapTo+10), paint);
        canvas.drawParagraph(paragraph, 10, 10);

        paint.delete();
        paragraph.delete();
        builder.delete();
        fontSrc.delete();
    });

    gm('paragraph_text_styles', (canvas) => {
        const paint = new CanvasKit.Paint();

        paint.setColor(CanvasKit.GREEN);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');

        const wrapTo = 200;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.BLACK,
                fontFamilies: ['Noto Serif'],
                fontSize: 20,
                decoration: CanvasKit.UnderlineDecoration,
                decorationThickness: 1.5, // multiplier based on font size
                decorationStyle: CanvasKit.DecorationStyle.Wavy,
            },
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('VAVAVAVAVAVAVA\nVAVA\n');

        const blueText = new CanvasKit.TextStyle({
            backgroundColor: CanvasKit.Color(234, 208, 232), // light pink
            color: CanvasKit.Color(48, 37, 199),
            fontFamilies: ['Noto Serif'],
            textBaseline: CanvasKit.TextBaseline.Ideographic,
            decoration: CanvasKit.LineThroughDecoration,
            decorationThickness: 1.5, // multiplier based on font size
        });
        builder.pushStyle(blueText);
        builder.addText(`Gosh I hope this wraps at some point, it is such a long line.`);
        builder.pop();
        builder.addText(` I'm done with the blue now. `);
        builder.addText(`Now I hope we should stop before we get 8 lines tall. `);
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        expect(paragraph.getAlphabeticBaseline()).toBeCloseTo(21.377, 3);
        expect(paragraph.getHeight()).toEqual(227);
        expect(paragraph.getIdeographicBaseline()).toBeCloseTo(27.236, 3);
        expect(paragraph.getLongestLine()).toBeCloseTo(195.664, 3);
        expect(paragraph.getMaxIntrinsicWidth()).toBeCloseTo(1167.140, 3);
        expect(paragraph.getMaxWidth()).toEqual(200);
        expect(paragraph.getMinIntrinsicWidth()).toBeCloseTo(172.360, 3);
        // Check "VAVAVAVAVAVAVA"
        expect(paragraph.getWordBoundary(8)).toEqual({
            start: 0,
            end: 14,
        });
        // Check "I"
        expect(paragraph.getWordBoundary(25)).toEqual({
            start: 25,
            end: 26,
        });

        canvas.clear(CanvasKit.WHITE);
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, 230), paint);
        canvas.drawParagraph(paragraph, 10, 10);

        paint.delete();
        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_text_styles_mixed_leading_distribution', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');

        const wrapTo = 200;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                color: CanvasKit.BLACK,
                backgroundColor: CanvasKit.Color(234, 208, 232), // light pink
                fontFamilies: ['Noto Serif'],
                fontSize: 10,
                heightMultiplier: 10,
            },
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('Not half leading');

        const halfLeadingText = new CanvasKit.TextStyle({
            color: CanvasKit.Color(48, 37, 199),
            backgroundColor: CanvasKit.Color(234, 208, 232), // light pink
            fontFamilies: ['Noto Serif'],
            fontSize: 10,
            heightMultiplier: 10,
            halfLeading: true,
        });
        builder.pushStyle(halfLeadingText);
        builder.addText('Half Leading Text');
        const paragraph = builder.build();

        paragraph.layout(wrapTo);
        canvas.clear(CanvasKit.WHITE);
        canvas.drawParagraph(paragraph, 0, 0);

        fontMgr.delete();
        paragraph.delete();
        builder.delete();
    });

    gm('paragraph_mixed_text_height_behavior', (canvas) => {
        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer);
        expect(fontMgr.countFamilies()).toEqual(1);
        expect(fontMgr.getFamilyName(0)).toEqual('Noto Serif');
        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.RED);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const wrapTo = 220;
        const behaviors = ["All", "DisableFirstAscent", "DisableLastDescent", "DisableAll"];

        for (let i = 0; i < behaviors.length; i++) {
            const style = new CanvasKit.ParagraphStyle({
                textStyle: {
                    color: CanvasKit.BLACK,
                    fontFamilies: ['Noto Serif'],
                    fontSize: 20,
                    heightMultiplier: 3, // make the difference more obvious
                },
                textHeightBehavior: CanvasKit.TextHeightBehavior[behaviors[i]],
            });
            const builder = CanvasKit.ParagraphBuilder.Make(style, fontMgr);
            builder.addText('Text height behavior\nof '+behaviors[i]);
            const paragraph = builder.build();
            paragraph.layout(wrapTo);
            canvas.drawParagraph(paragraph, 0, 150 * i);
            canvas.drawRect(CanvasKit.LTRBRect(0, 150 * i, wrapTo, 150 * i + 120), paint);
            paragraph.delete();
            builder.delete();
        }
        paint.delete();
        fontMgr.delete();
    });

    it('should not crash if we omit font family on pushed textStyle', () => {
        const surface = CanvasKit.MakeCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface');

        const canvas = surface.getCanvas();
        const paint = new CanvasKit.Paint();

        paint.setColor(CanvasKit.RED);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer, notoSerifBoldItalicFontBuffer);

        const wrapTo = 250;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                fontFamilies: ['Noto Serif'],
                fontSize: 20,
            },
            textDirection: CanvasKit.TextDirection.RTL,
            disableHinting: true,
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('Default text\n');

        const boldItalic = new CanvasKit.TextStyle({
            fontStyle: {
                weight: CanvasKit.FontWeight.Bold,
                slant: CanvasKit.FontSlant.Italic,
            }
        });
        builder.pushStyle(boldItalic);
        builder.addText(`Bold, Italic\n`); // doesn't show up, but we don't crash
        builder.pop();
        builder.addText(`back to normal`);
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, wrapTo+10), paint);
        canvas.drawParagraph(paragraph, 10, 10);

        surface.flush();

        paragraph.delete();
        builder.delete();
        paint.delete();
        fontMgr.delete();
    });

    it('should not crash if we omit font family on paragraph style', () => {
        const surface = CanvasKit.MakeCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface');

        const canvas = surface.getCanvas();
        const paint = new CanvasKit.Paint();

        paint.setColor(CanvasKit.RED);
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        const fontMgr = CanvasKit.FontMgr.FromData(notoSerifFontBuffer, notoSerifBoldItalicFontBuffer);

        const wrapTo = 250;

        const paraStyle = new CanvasKit.ParagraphStyle({
            textStyle: {
                fontSize: 20,
            },
            textDirection: CanvasKit.TextDirection.RTL,
            disableHinting: true,
        });

        const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
        builder.addText('Default text\n');

        const boldItalic = new CanvasKit.TextStyle({
            fontStyle: {
                weight: CanvasKit.FontWeight.Bold,
                slant: CanvasKit.FontSlant.Italic,
            }
        });
        builder.pushStyle(boldItalic);
        builder.addText(`Bold, Italic\n`);
        builder.pop();
        builder.addText(`back to normal`);
        const paragraph = builder.build();

        paragraph.layout(wrapTo);

        canvas.clear(CanvasKit.WHITE);
        canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, wrapTo+10), paint);
        canvas.drawParagraph(paragraph, 10, 10);

        surface.flush();

        paragraph.delete();
        paint.delete();
        fontMgr.delete();
        builder.delete();
    });
});
