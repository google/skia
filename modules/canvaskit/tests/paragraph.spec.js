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

    fit('draws shaped text in a paragraph', function(done) {
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
                height: wrapTo, // px ?
            });
            fontMgr.dumpFamilies();
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
            reportSurface(surface, 'paragraph_basic', null);
        }));
    });

});
