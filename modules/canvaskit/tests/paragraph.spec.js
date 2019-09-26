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

    let bungeeFontBuffer = null;
    const bungeeFontLoaded = fetch('/assets/Bungee-Regular.ttf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            bungeeFontBuffer = buffer;
        });

    xit('draws shaped text in a paragraph', function(done) {
        Promise.all([LoadCanvasKit, notoSerifFontLoaded]).then(catchException(done, () => {
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

            const fontMgr = CanvasKit.SkFontMgr.FromData(bungeeFontBuffer);

            const textPaint = new CanvasKit.SkPaint();
            const builder = new CanvasKit.ParagraphBuilder(fontMgr, style);

            surface.flush();

            paint.delete();
            notoSerif.delete();
            textPaint.delete();
            textFont.delete();
            shapedText.delete();
            textFont2.delete();
            shapedText2.delete();
            fontMgr.delete();
            reportSurface(surface, 'text_shaping', null);
        }));
    });

});
