describe('CanvasKit\'s Canvas Behavior', function() {
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

    it('can draw directly to a canvas', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            // This is taken from example.html
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const paint = new CanvasKit.SkPaint();
            paint.setStrokeWidth(2.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
            paint.setStyle(CanvasKit.PaintStyle.Stroke);

            canvas.drawLine(3, 10, 30, 15, paint);
            canvas.drawRoundRect(CanvasKit.LTRBRect(5, 35, 45, 80), 15, 10, paint);

            canvas.drawOval(CanvasKit.LTRBRect(5, 35, 45, 80), paint);

            canvas.drawArc(CanvasKit.LTRBRect(55, 35, 95, 80), 15, 270, true, paint);

            const font = new CanvasKit.SkFont(null, 20);
            canvas.drawText('this is ascii text', 5, 100, paint, font);

            const blob = CanvasKit.SkTextBlob.MakeFromText('Unicode chars 💩 é É ص', font);
            canvas.drawTextBlob(blob, 5, 130, paint);

            surface.flush();
            font.delete();
            blob.delete();
            paint.delete();

            reportSurface(surface, 'canvas_api_example', done);
        }));
        // See canvas2d for more API tests
    });

    it('can apply an effect and draw text', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const path = starPath(CanvasKit);

            const paint = new CanvasKit.SkPaint();

            const textPaint = new CanvasKit.SkPaint();
            textPaint.setColor(CanvasKit.Color(40, 0, 0, 1.0));
            textPaint.setAntiAlias(true);

            const textFont = new CanvasKit.SkFont(null, 30);

            const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], 1);

            paint.setPathEffect(dpe);
            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(5.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

            canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

            canvas.drawPath(path, paint);
            canvas.drawText('This is text', 10, 280, textPaint, textFont);
            surface.flush();

            dpe.delete();
            path.delete();
            paint.delete();
            textFont.delete();
            textPaint.delete();

            reportSurface(surface, 'effect_and_text_example', done);
        }));
    });

    it('returns the depth of the save state stack', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            expect(canvas.getSaveCount()).toEqual(1);
            canvas.save();
            canvas.save();
            canvas.restore();
            canvas.save();
            canvas.save();
            expect(canvas.getSaveCount()).toEqual(4);
            // does nothing, by the SkCanvas API
            canvas.restoreToCount(500);
            expect(canvas.getSaveCount()).toEqual(4);
            canvas.restore();
            expect(canvas.getSaveCount()).toEqual(3);
            canvas.save();
            canvas.restoreToCount(2);
            expect(canvas.getSaveCount()).toEqual(2);

            surface.delete();

            done();
        }));
    });

    it('draws circles', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface')
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const path = starPath(CanvasKit);

            const paint = new CanvasKit.SkPaint();

            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            paint.setStrokeWidth(5.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.CYAN);

            canvas.clear(CanvasKit.WHITE);

            canvas.drawCircle(30, 50, 15, paint);

            paint.setStyle(CanvasKit.PaintStyle.Fill);
            paint.setColor(CanvasKit.RED);
            canvas.drawCircle(130, 80, 60, paint);
            canvas.drawCircle(20, 150, 60, paint);

            surface.flush();
            path.delete();
            paint.delete();

            reportSurface(surface, 'circle_canvas', done);
        }));
    });

});