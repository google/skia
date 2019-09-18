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
            expect(surface).toBeTruthy('Could not make surface');
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
    });

    it('can decode and draw a png', function(done) {
        const imgPromise = fetch('/assets/mandrill_512.png')
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const pngData = values[0];
            expect(pngData).toBeTruthy();
            catchException(done, () => {
                let img = CanvasKit.MakeImageFromEncoded(pngData);
                expect(img).toBeTruthy();
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface')
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                let paint = new CanvasKit.SkPaint();
                canvas.drawImage(img, 0, 0, paint);

                paint.delete();
                img.delete();

                reportSurface(surface, 'drawImage_png', done);
            })();
        });
    });

    it('can decode and draw a jpg', function(done) {
        const imgPromise = fetch('/assets/mandrill_h1v1.jpg')
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const jpgData = values[0];
            expect(jpgData).toBeTruthy();
            catchException(done, () => {
                let img = CanvasKit.MakeImageFromEncoded(jpgData);
                expect(img).toBeTruthy();
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface')
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                let paint = new CanvasKit.SkPaint();
                canvas.drawImage(img, 0, 0, paint);

                paint.delete();
                img.delete();

                reportSurface(surface, 'drawImage_jpg', done);
            })();
        });
    });

    it('can decode and draw a (still) gif', function(done) {
        const imgPromise = fetch('/assets/flightAnim.gif')
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const gifData = values[0];
            expect(gifData).toBeTruthy();
            catchException(done, () => {
                let img = CanvasKit.MakeImageFromEncoded(gifData);
                expect(img).toBeTruthy();
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface')
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                let paint = new CanvasKit.SkPaint();
                canvas.drawImage(img, 0, 0, paint);

                paint.delete();
                img.delete();

                reportSurface(surface, 'drawImage_gif', done);
            })();
        });
    });

    it('can decode and draw an animated gif', function(done) {
        const imgPromise = fetch('/assets/flightAnim.gif')
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const gifData = values[0];
            expect(gifData).toBeTruthy();
            catchException(done, () => {
                let aImg = CanvasKit.MakeAnimatedImageFromEncoded(gifData);
                expect(aImg).toBeTruthy();
                expect(aImg.getRepetitionCount()).toEqual(-1); // infinite loop

                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface')
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                canvas.drawAnimatedImage(aImg, 0, 0);

                let c = aImg.decodeNextFrame();
                expect(c).not.toEqual(-1);
                canvas.drawAnimatedImage(aImg, 300, 0);
                for(let i = 0; i < 10; i++) {
                    c = aImg.decodeNextFrame();
                    expect(c).not.toEqual(-1);
                }
                canvas.drawAnimatedImage(aImg, 0, 300);
                for(let i = 0; i < 10; i++) {
                    c = aImg.decodeNextFrame();
                    expect(c).not.toEqual(-1);
                }
                 canvas.drawAnimatedImage(aImg, 300, 300);

                aImg.delete();

                reportSurface(surface, 'drawDrawable_animated_gif', done);
            })();
        });
    });

});
