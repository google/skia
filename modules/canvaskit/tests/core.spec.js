describe('Core canvas behavior', function() {
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

    it('can draw an SkPicture', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            // This is taken from example.html
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const spr = new CanvasKit.SkPictureRecorder();
            const rcanvas = spr.beginRecording(
                            CanvasKit.LTRBRect(0, 0, surface.width(), surface.height()));
            const paint = new CanvasKit.SkPaint();
            paint.setStrokeWidth(2.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
            paint.setStyle(CanvasKit.PaintStyle.Stroke);

            rcanvas.drawRoundRect(CanvasKit.LTRBRect(5, 35, 45, 80), 15, 10, paint);

            const font = new CanvasKit.SkFont(null, 20);
            rcanvas.drawText('this picture has a round rect', 5, 100, paint, font);
            const pic = spr.finishRecordingAsPicture();
            spr.delete();


            const canvas = surface.getCanvas();
            canvas.drawPicture(pic);

            reportSurface(surface, 'picture_test', done);
        }));
    });

    it('can compute tonal colors', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const input = {
                ambient: CanvasKit.BLUE,
                spot: CanvasKit.RED,
            };
            const out = CanvasKit.computeTonalColors(input);

            expect(out.ambient).toEqual(CanvasKit.Color(0,0,0,1));

            const [r,g,b,a] = CanvasKit.getColorComponents(out.spot);
            expect(r).toEqual(44);
            expect(g).toEqual(0);
            expect(b).toEqual(0);
            expect(a).toBeCloseTo(0.969, 2);
            done();
        }));
    })

});
