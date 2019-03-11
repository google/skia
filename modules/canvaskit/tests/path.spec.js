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

    it('can draw a path', function(done) {
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
            paint.setStrokeWidth(1.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
            paint.setStyle(CanvasKit.PaintStyle.Stroke);

            const path = new CanvasKit.SkPath();
            path.moveTo(20, 5);
            path.lineTo(30, 20);
            path.lineTo(40, 10);
            path.lineTo(50, 20);
            path.lineTo(60, 0);
            path.lineTo(20, 5);

            path.moveTo(20, 80);
            path.cubicTo(90, 10, 160, 150, 190, 10);

            path.moveTo(36, 148);
            path.quadTo(66, 188, 120, 136);
            path.lineTo(36, 148);

            path.moveTo(150, 180);
            path.arcTo(150, 100, 50, 200, 20);
            path.lineTo(160, 160);

            path.moveTo(20, 120);
            path.lineTo(20, 120);

            path.transform([2, 0, 0,
                            0, 2, 0,
                            0, 0, 1 ])

            canvas.drawPath(path, paint);

            let rrect = new CanvasKit.SkPath()
                               .addRoundRect(100, 10, 140, 62,
                                             10, 4, true);

            canvas.drawPath(rrect, paint);
            rrect.delete();

            surface.flush();

            path.delete();
            paint.delete();

            reportSurface(surface, 'path_api_example', done);
        }));
        // See PathKit for more tests, since they share implementation
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

    function starPath(CanvasKit, X=128, Y=128, R=116) {
        let p = new CanvasKit.SkPath();
        p.moveTo(X + R, Y);
        for (let i = 1; i < 8; i++) {
          let a = 2.6927937 * i;
          p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
        }
        return p;
      }

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

            reportSurface(surface, 'effect_and_text_example', done);
        }));
    });

    it('can create a path from an SVG string', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            //.This is a parallelagram from
            // https://upload.wikimedia.org/wikipedia/commons/e/e7/Simple_parallelogram.svg
            let path = CanvasKit.MakePathFromSVGString('M 205,5 L 795,5 L 595,295 L 5,295 L 205,5 z');

            let cmds = path.toCmds();
            expect(cmds).toBeTruthy();
            // 1 move, 4 lines, 1 close
            // each element in cmds is an array, with index 0 being the verb, and the rest being args
            expect(cmds.length).toBe(6);
            expect(cmds).toEqual([[CanvasKit.MOVE_VERB, 205, 5],
                                  [CanvasKit.LINE_VERB, 795, 5],
                                  [CanvasKit.LINE_VERB, 595, 295],
                                  [CanvasKit.LINE_VERB, 5, 295],
                                  [CanvasKit.LINE_VERB, 205, 5],
                                  [CanvasKit.CLOSE_VERB]]);
            path.delete();
            done();
        }));
    });

     it('can create an SVG string from a path', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            let cmds = [[CanvasKit.MOVE_VERB, 205, 5],
                       [CanvasKit.LINE_VERB, 795, 5],
                       [CanvasKit.LINE_VERB, 595, 295],
                       [CanvasKit.LINE_VERB, 5, 295],
                       [CanvasKit.LINE_VERB, 205, 5],
                       [CanvasKit.CLOSE_VERB]];
            let path = CanvasKit.MakePathFromCmds(cmds);

            let svgStr = path.toSVGString();
            // We output it in terse form, which is different than Wikipedia's version
            expect(svgStr).toEqual('M205 5L795 5L595 295L5 295L205 5Z');
            path.delete();
            done();
        }));
    });
});
