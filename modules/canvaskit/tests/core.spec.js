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
                expect(surface).toBeTruthy('Could not make surface');
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
                expect(surface).toBeTruthy('Could not make surface');
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
                expect(surface).toBeTruthy('Could not make surface');
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
                expect(aImg.width()).toEqual(320);
                expect(aImg.height()).toEqual(240);
                expect(aImg.getFrameCount()).toEqual(60);

                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface');
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

    it('can create an image "from scratch" by specifying pixels/colorInfo manually', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface');
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            canvas.clear(CanvasKit.WHITE);
            const paint = new CanvasKit.SkPaint();

            // This creates and draws an SkImage that is 1 pixel wide, 4 pixels tall with
            // the colors listed below.
            const pixels = Uint8Array.from([
                255,   0,   0, 255, // opaque red
                  0, 255,   0, 255, // opaque green
                  0,   0, 255, 255, // opaque blue
                255,   0, 255, 100, // transparent purple
            ]);
            const img = CanvasKit.MakeImage(pixels, 1, 4, CanvasKit.AlphaType.Unpremul, CanvasKit.ColorType.RGBA_8888);
            canvas.drawImage(img, 1, 1, paint);

            reportSurface(surface, '1x4_from_scratch', done);
        }));
    });

    it('can blur using ImageFilter or MaskFilter', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface');
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();
            const pathUL = starPath(CanvasKit, 100, 100, 80);
            const pathBR = starPath(CanvasKit, 400, 300, 80);
            const paint = new CanvasKit.SkPaint();
            const textFont = new CanvasKit.SkFont(null, 24);

            canvas.drawText('Above: MaskFilter', 20, 220, paint, textFont);
            canvas.drawText('Right: ImageFilter', 20, 260, paint, textFont);

            paint.setColor(CanvasKit.BLUE);

            const blurMask = CanvasKit.SkMaskFilter.MakeBlur(CanvasKit.BlurStyle.Normal, 5, true);
            paint.setMaskFilter(blurMask);
            canvas.drawPath(pathUL, paint);

            const blurIF = CanvasKit.SkImageFilter.MakeBlur(8, 1, CanvasKit.TileMode.Decal, null);
            paint.setImageFilter(blurIF);
            canvas.drawPath(pathBR, paint);

            surface.flush();

            pathUL.delete();
            pathBR.delete();
            paint.delete();
            blurMask.delete();
            blurIF.delete();
            textFont.delete();

            reportSurface(surface, 'blur_filters', done);
        }));
    });

    it('can use various image filters', function(done) {
        const imgPromise = fetch('/assets/mandrill_512.png')
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const pngData = values[0];
            expect(pngData).toBeTruthy();
            catchException(done, () => {
                let img = CanvasKit.MakeImageFromEncoded(pngData);
                expect(img).toBeTruthy();
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface');
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                canvas.clear(CanvasKit.WHITE);
                const paint = new CanvasKit.SkPaint();
                paint.setAntiAlias(true);
                paint.setColor(CanvasKit.Color(0, 255, 0, 1.0));
                const redCF =  CanvasKit.SkColorFilter.MakeBlend(
                        CanvasKit.Color(255, 0, 0, 0.1), CanvasKit.BlendMode.SrcOver);
                const redIF = CanvasKit.SkImageFilter.MakeColorFilter(redCF, null);
                const blurIF = CanvasKit.SkImageFilter.MakeBlur(8, 0.2, CanvasKit.TileMode.Decal, null);
                const combined = CanvasKit.SkImageFilter.MakeCompose(redIF, blurIF);

                // rotate 10 degrees centered on 200, 200
                const m = CanvasKit.SkMatrix.rotated(Math.PI/18, 200, 200);
                const rotated = CanvasKit.SkImageFilter.MakeMatrixTransform(m, CanvasKit.FilterQuality.Medium, combined);
                paint.setImageFilter(rotated);

                //canvas.rotate(10, 200, 200);
                canvas.drawImage(img, 0, 0, paint);
                canvas.drawRect(CanvasKit.LTRBRect(5, 35, 45, 80), paint);

                surface.flush();

                paint.delete();
                redIF.delete();
                redCF.delete();
                blurIF.delete();
                combined.delete();
                rotated.delete();
                img.delete();

                reportSurface(surface, 'combined_filters', done);
            })();
        });
    });

    it('can use DecodeCache APIs', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const initialLimit = CanvasKit.getDecodeCacheLimitBytes();
            expect(initialLimit).toBeGreaterThan(1024 * 1024);

            const newLimit = 42 * 1024 * 1024;
            CanvasKit.setDecodeCacheLimitBytes(newLimit);
            expect(CanvasKit.getDecodeCacheLimitBytes()).toEqual(newLimit);

            // We cannot make any assumptions about this value,
            // so we just make sure it doesn't crash.
            CanvasKit.getDecodeCacheUsedBytes();

            done();
        }));
    });

    it('can combine shaders', function(done) {
        LoadCanvasKit.then(catchException(done, () => {
            const surface = CanvasKit.MakeCanvasSurface('test');
            expect(surface).toBeTruthy('Could not make surface');
            if (!surface) {
                done();
                return;
            }
            const canvas = surface.getCanvas();

            const rShader = CanvasKit.SkShader.Color(CanvasKit.Color(255, 0, 0, 1.0));
            const gShader = CanvasKit.SkShader.Color(CanvasKit.Color(0, 255, 0, 0.6));
            const bShader = CanvasKit.SkShader.Color(CanvasKit.Color(0, 0, 255, 1.0));

            const rgShader = CanvasKit.SkShader.Blend(CanvasKit.BlendMode.SrcOver, rShader, gShader);

            const p = new CanvasKit.SkPaint();
            p.setShader(rgShader);
            canvas.drawPaint(p);

            const gbShader = CanvasKit.SkShader.Lerp(0.5, gShader, bShader);

            p.setShader(gbShader);
            canvas.drawRect(CanvasKit.LTRBRect(5, 100, 300, 400), p);


            reportSurface(surface, 'combined_shaders', done);
        }));
    });

});
