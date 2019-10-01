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

    let notoSerifFontBuffer = null;
    // This font is known to support kerning
    const notoSerifFontLoaded = fetch('/assets/NotoSerif-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            notoSerifFontBuffer = buffer;
        });

    let bungeeFontBuffer = null;
    const bungeeFontLoaded = fetch('/assets/Bungee-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            bungeeFontBuffer = buffer;
        });

    it('draws shaped text in a paragraph', function(done) {
        Promise.all([LoadCanvasKit, notoSerifFontLoaded]).then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();

            paint.setColor(CanvasKit.RED);
            paint.setStyle(CanvasKit.PaintStyle.Stroke);

            const fontMgr = CanvasKit.SkFontMgr.FromData(notoSerifFontBuffer);

            const wrapTo = 200;

            const paraStyle = new CanvasKit.ParagraphStyle({
                textStyle: {
                    color: CanvasKit.BLACK,
                    fontFamilies: ['Noto Serif'],
                    fontSize: 20,
                },
                textAlign: CanvasKit.TextAlign.Center,
                maxLines: 8, // FIXME
            });

            const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
            builder.addText('VAVAVAVAVAVAVA\nVAVA\n');

            const blueText = new CanvasKit.TextStyle({
                backgroundColor: CanvasKit.Color(234, 208, 232), // light pink
                color: CanvasKit.Color(48, 37, 199),
                decoration: CanvasKit.LineThroughDecoration,
                decorationThickness: 1.5, // multiplier based on font size
                fontSize: 24,
            });
            builder.pushStyle(blueText)
            builder.addText(`Gosh I hope this wraps at some point, it is such a long line.`)
            builder.pop();
            builder.addText(` I'm done with the blue now. `)
            builder.addText(`Now I hope we should stop before we get to 100 pixels tall. `);
            const paragraph = builder.build();

            paragraph.layout(wrapTo);

            canvas.drawRect(CanvasKit.LTRBRect(10, 10, wrapTo+10, wrapTo+10), paint);
            canvas.drawParagraph(paragraph, 10, 10);

            surface.flush();

            paint.delete();
            fontMgr.delete();
            reportSurface(surface, 'paragraph_basic', done);
        }));
    });

    fit('provides rectangles around', function(done) {
        // loosely based on SkParagraph_GetRectsForRangeParagraph test in c++ code.
        Promise.all([LoadCanvasKit, notoSerifFontLoaded]).then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();

            const fontMgr = CanvasKit.SkFontMgr.FromData(notoSerifFontBuffer);

            const wrapTo = 550;
            const hStyle = CanvasKit.RectHeightStyle.Max;
            const wStyle = CanvasKit.RectWidthStyle.Tight;

            const paraStyle = new CanvasKit.ParagraphStyle({
                textStyle: {
                    color: CanvasKit.BLACK,
                    fontFamilies: ['Noto Serif'],
                    fontSize: 50,
                    // TODO(kjlubick): font style
                },
                textAlign: CanvasKit.TextAlign.Left,
                maxLines: 10,
            });
            const builder = CanvasKit.ParagraphBuilder.Make(paraStyle, fontMgr);
            builder.addText('12345,  \"67890\" 12345 67890 12345 67890 12345 67890 12345 67890 12345 67890 12345');
            const paragraph = builder.build();

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
            // Move it down a bit so we can see the rects that go above 0,0
            canvas.translate(10, 10);
            canvas.drawParagraph(paragraph, 0, 0);

            for (const test of ranges) {
                let rects = paragraph.getRectsForRange(test.start, test.end, hStyle, wStyle);
                expect(Array.isArray(rects)).toEqual(true);
                expect(rects.length).toEqual(test.expectedNum);

                for (const rect of rects) {
                    const p = new CanvasKit.SkPaint();
                    p.setColor(test.color);
                    p.setStyle(CanvasKit.PaintStyle.Stroke);
                    canvas.drawRect(rect, p);
                    p.delete();
                }
            }

            surface.flush();
            fontMgr.delete();
            reportSurface(surface, 'paragraph_rects', null);
        }));
    });

});
