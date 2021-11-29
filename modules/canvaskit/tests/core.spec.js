describe('Core canvas behavior', () => {
    let container;

    beforeEach(async () => {
        await LoadCanvasKit;
        container = document.createElement('div');
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    gm('picture_test', (canvas) => {
        const spr = new CanvasKit.PictureRecorder();
        const rcanvas = spr.beginRecording(
                        CanvasKit.LTRBRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT));
        const paint = new CanvasKit.Paint();
        paint.setStrokeWidth(2.0);
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
        paint.setStyle(CanvasKit.PaintStyle.Stroke);

        rcanvas.drawRRect(CanvasKit.RRectXY([5, 35, 45, 80], 15, 10), paint);

        const font = new CanvasKit.Font(null, 20);
        rcanvas.drawText('this picture has a round rect', 5, 100, paint, font);
        const pic = spr.finishRecordingAsPicture();
        spr.delete();
        paint.delete();

        canvas.drawPicture(pic);

        const bytes = pic.serialize();
        expect(bytes).toBeTruthy();

        pic.delete();
    });

    const uIntColorToCanvasKitColor = (c) => {
        return CanvasKit.Color(
         (c >> 16) & 0xFF,
         (c >>  8) & 0xFF,
         (c >>  0) & 0xFF,
        ((c >> 24) & 0xFF) / 255
      );
    };

    it('can compute tonal colors', () => {
        const input = {
            ambient: CanvasKit.BLUE,
            spot: CanvasKit.RED,
        };
        const out = CanvasKit.computeTonalColors(input);
        expect(new Float32Array(out.ambient)).toEqual(CanvasKit.BLACK);
        const expectedSpot = [0.173, 0, 0, 0.969];
        expect(out.spot.length).toEqual(4);
        expect(out.spot[0]).toBeCloseTo(expectedSpot[0], 3);
        expect(out.spot[1]).toBeCloseTo(expectedSpot[1], 3);
        expect(out.spot[2]).toBeCloseTo(expectedSpot[2], 3);
        expect(out.spot[3]).toBeCloseTo(expectedSpot[3], 3);
    });

    it('can compute tonal colors with malloced values', () => {
        const ambientColor = CanvasKit.Malloc(Float32Array, 4);
        ambientColor.toTypedArray().set(CanvasKit.BLUE);
        const spotColor = CanvasKit.Malloc(Float32Array, 4);
        spotColor.toTypedArray().set(CanvasKit.RED);
        const input = {
            ambient: ambientColor,
            spot: spotColor,
        };
        const out = CanvasKit.computeTonalColors(input);
        expect(new Float32Array(out.ambient)).toEqual(CanvasKit.BLACK);
        const expectedSpot = [0.173, 0, 0, 0.969];
        expect(out.spot.length).toEqual(4);
        expect(out.spot[0]).toBeCloseTo(expectedSpot[0], 3);
        expect(out.spot[1]).toBeCloseTo(expectedSpot[1], 3);
        expect(out.spot[2]).toBeCloseTo(expectedSpot[2], 3);
        expect(out.spot[3]).toBeCloseTo(expectedSpot[3], 3);
    });

    // This helper is used for all the MakeImageFromEncoded tests.
    // TODO(kjlubick): rewrite this and callers to use gm
    function decodeAndDrawSingleFrameImage(imgName, goldName, done) {
        const imgPromise = fetch(imgName)
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const imgData = values[0];
            expect(imgData).toBeTruthy();
            catchException(done, () => {
                let img = CanvasKit.MakeImageFromEncoded(imgData);
                expect(img).toBeTruthy();
                const surface = CanvasKit.MakeCanvasSurface('test');
                expect(surface).toBeTruthy('Could not make surface');
                if (!surface) {
                    done();
                    return;
                }
                const canvas = surface.getCanvas();
                let paint = new CanvasKit.Paint();
                canvas.drawImage(img, 0, 0, paint);

                paint.delete();
                img.delete();

                reportSurface(surface, goldName, done);
            })();
        });
    }

    it('can decode and draw a png', (done) => {
        decodeAndDrawSingleFrameImage('/assets/mandrill_512.png', 'drawImage_png', done);
    });

    it('can decode and draw a jpg', (done) => {
        decodeAndDrawSingleFrameImage('/assets/mandrill_h1v1.jpg', 'drawImage_jpg', done);
    });

    it('can decode and draw a (still) gif', (done) => {
        decodeAndDrawSingleFrameImage('/assets/flightAnim.gif', 'drawImage_gif', done);
    });

    it('can decode and draw a still webp', (done) => {
        decodeAndDrawSingleFrameImage('/assets/color_wheel.webp', 'drawImage_webp', done);
    });

   it('can readPixels from an Image', (done) => {
        const imgPromise = fetch('/assets/mandrill_512.png')
            .then((response) => response.arrayBuffer());
        Promise.all([imgPromise, LoadCanvasKit]).then((values) => {
            const imgData = values[0];
            expect(imgData).toBeTruthy();
            catchException(done, () => {
                let img = CanvasKit.MakeImageFromEncoded(imgData);
                expect(img).toBeTruthy();
                const imageInfo = {
                    alphaType: CanvasKit.AlphaType.Unpremul,
                    colorType: CanvasKit.ColorType.RGBA_8888,
                    colorSpace: CanvasKit.ColorSpace.SRGB,
                    width: img.width(),
                    height: img.height(),
                };
                const rowBytes = 4 * img.width();

                const pixels = img.readPixels(0, 0, imageInfo);
                // We know the image is 512 by 512 pixels in size, each pixel
                // requires 4 bytes (R, G, B, A).
                expect(pixels.length).toEqual(512 * 512 * 4);

                // Make enough space for a 5x5 8888 surface (4 bytes for R, G, B, A)
                const rdsData = CanvasKit.Malloc(Uint8Array, 512 * 5*512 * 4);
                const pixels2 = rdsData.toTypedArray();
                pixels2[0] = 127;  // sentinel value, should be overwritten by readPixels.
                img.readPixels(0, 0, imageInfo, rdsData, rowBytes);
                expect(rdsData.toTypedArray()[0]).toEqual(pixels[0]);

                img.delete();
                CanvasKit.Free(rdsData);
                done();
            })();
        });
    });

    gm('drawDrawable_animated_gif', (canvas, fetchedByteBuffers) => {
        let aImg = CanvasKit.MakeAnimatedImageFromEncoded(fetchedByteBuffers[0]);
        expect(aImg).toBeTruthy();
        expect(aImg.getRepetitionCount()).toEqual(-1); // infinite loop
        expect(aImg.width()).toEqual(320);
        expect(aImg.height()).toEqual(240);
        expect(aImg.getFrameCount()).toEqual(60);

        const drawCurrentFrame = function(x, y) {
            let img = aImg.makeImageAtCurrentFrame();
            canvas.drawImage(img, x, y, null);
            img.delete();
        }

        drawCurrentFrame(0, 0);

        let c = aImg.decodeNextFrame();
        expect(c).not.toEqual(-1);
        drawCurrentFrame(300, 0);
        for(let i = 0; i < 10; i++) {
            c = aImg.decodeNextFrame();
            expect(c).not.toEqual(-1);
        }
        drawCurrentFrame(0, 300);
        for(let i = 0; i < 10; i++) {
            c = aImg.decodeNextFrame();
            expect(c).not.toEqual(-1);
        }
        drawCurrentFrame(300, 300);

        aImg.delete();
    }, '/assets/flightAnim.gif');

    gm('exif_orientation', (canvas, fetchedByteBuffers) => {
        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        const font = new CanvasKit.Font(null, 14);
        canvas.drawText('The following heart should be rotated 90 CCW due to exif.',
            5, 25, paint, font);

        // TODO(kjlubick) it would be nice to also to test MakeAnimatedImageFromEncoded but
        //   I could not create a sample animated image that worked.
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();
        canvas.drawImage(img, 5, 35, null);

        img.delete();
        paint.delete();
        font.delete();
    }, '/assets/exif_rotated_heart.jpg');

    gm('1x4_from_scratch', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();

        // This creates and draws an Image that is 1 pixel wide, 4 pixels tall with
        // the colors listed below.
        const pixels = Uint8Array.from([
            255,   0,   0, 255, // opaque red
              0, 255,   0, 255, // opaque green
              0,   0, 255, 255, // opaque blue
            255,   0, 255, 100, // transparent purple
        ]);
        const img = CanvasKit.MakeImage({
          'width': 1,
          'height': 4,
          'alphaType': CanvasKit.AlphaType.Unpremul,
          'colorType': CanvasKit.ColorType.RGBA_8888,
          'colorSpace': CanvasKit.ColorSpace.SRGB
        }, pixels, 4);
        canvas.drawImage(img, 1, 1, paint);

        const info = img.getImageInfo();
        expect(info).toEqual({
          'width': 1,
          'height': 4,
          'alphaType': CanvasKit.AlphaType.Unpremul,
          'colorType': CanvasKit.ColorType.RGBA_8888,
        });
        const cs = img.getColorSpace();
        expect(CanvasKit.ColorSpace.Equals(cs, CanvasKit.ColorSpace.SRGB)).toBeTruthy();

        cs.delete();
        img.delete();
        paint.delete();
    });

    gm('draw_atlas_with_builders', (canvas, fetchedByteBuffers) => {
        const atlas = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(atlas).toBeTruthy();
        canvas.clear(CanvasKit.WHITE);

        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.Color(0, 0, 0, 0.8));

        // Allocate space for 4 rectangles.
        const srcs = CanvasKit.Malloc(Float32Array, 16);
        srcs.toTypedArray().set([
            0,   0, 256, 256, // LTRB
          256,   0, 512, 256,
            0, 256, 256, 512,
          256, 256, 512, 512
        ]);

        // Allocate space for 4 RSXForms.
        const dsts = CanvasKit.Malloc(Float32Array, 16);
        dsts.toTypedArray().set([
            0.5, 0,  20,  20, // scos, ssin, tx, ty
            0.5, 0, 300,  20,
            0.5, 0,  20, 300,
            0.5, 0, 300, 300
        ]);

        // Allocate space for 4 colors.
        const colors = new CanvasKit.Malloc(Uint32Array, 4);
        colors.toTypedArray().set([
          CanvasKit.ColorAsInt( 85, 170,  10, 128), // light green
          CanvasKit.ColorAsInt( 51,  51, 191, 128), // light blue
          CanvasKit.ColorAsInt(  0,   0,   0, 128),
          CanvasKit.ColorAsInt(256, 256, 256, 128),
        ]);

        canvas.drawAtlas(atlas, srcs, dsts, paint, CanvasKit.BlendMode.Modulate, colors);

        atlas.delete();
        CanvasKit.Free(srcs);
        CanvasKit.Free(dsts);
        CanvasKit.Free(colors);
        paint.delete();
    }, '/assets/mandrill_512.png');

    gm('draw_atlas_with_arrays', (canvas, fetchedByteBuffers) => {
        const atlas = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(atlas).toBeTruthy();
        canvas.clear(CanvasKit.WHITE);

        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.Color(0, 0, 0, 0.8));

        const srcs = [
            0, 0,  8,  8,
            8, 0, 16,  8,
            0, 8,  8, 16,
            8, 8, 16, 16,
        ];

        const dsts = [
            10, 0,   0,   0,
            10, 0, 100,   0,
            10, 0,   0, 100,
            10, 0, 100, 100,
        ];

        const colors = Uint32Array.of(
            CanvasKit.ColorAsInt( 85, 170,  10, 128), // light green
            CanvasKit.ColorAsInt( 51,  51, 191, 128), // light blue
            CanvasKit.ColorAsInt(  0,   0,   0, 128),
            CanvasKit.ColorAsInt(255, 255, 255, 128),
        );

        // sampling for each of the 4 instances
        const sampling = [
            null,
            {B: 0, C: 0.5},
            {filter: CanvasKit.FilterMode.Nearest, mipmap: CanvasKit.MipmapMode.None},
            {filter: CanvasKit.FilterMode.Linear,  mipmap: CanvasKit.MipmapMode.Nearest},
        ];

        // positioning for each of the 4 instances
        const offset = [
            [0, 0], [256, 0], [0, 256], [256, 256]
        ];

        canvas.translate(20, 20);
        for (let i = 0; i < 4; ++i) {
            canvas.save();
            canvas.translate(offset[i][0], offset[i][1]);
            canvas.drawAtlas(atlas, srcs, dsts, paint, CanvasKit.BlendMode.SrcOver, colors,
                             sampling[i]);
            canvas.restore();
        }

        atlas.delete();
        paint.delete();
    }, '/assets/mandrill_16.png');

    gm('draw_patch', (canvas, fetchedByteBuffers) => {
        const image = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(image).toBeTruthy();
        canvas.clear(CanvasKit.WHITE);

        const paint = new CanvasKit.Paint();
        const shader = image.makeShaderOptions(CanvasKit.TileMode.Clamp,
                                               CanvasKit.TileMode.Clamp,
                                               CanvasKit.FilterMode.Linear,
                                               CanvasKit.MipmapMode.None);
        const cubics = [0,0, 80,50, 160,50,
                        240,0, 200,80, 200,160,
                        240,240, 160,160, 80,240,
                        0,240, 50,160, 0,80];
         const colors = [CanvasKit.RED, CanvasKit.BLUE, CanvasKit.YELLOW, CanvasKit.CYAN];
         const texs = [0,0, 16,0, 16,16, 0,16];

         const params = [
             [  0,   0, colors, null, null,   CanvasKit.BlendMode.Dst],
             [256,   0, null,   texs, shader, null],
             [  0, 256, colors, texs, shader, null],
             [256, 256, colors, texs, shader, CanvasKit.BlendMode.Screen],
         ];
         for (const p of params) {
             paint.setShader(p[4]);
             canvas.save();
             canvas.translate(p[0], p[1]);
             canvas.drawPatch(cubics, p[2], p[3], p[5], paint);
             canvas.restore();
         }
        paint.delete();
    }, '/assets/mandrill_16.png');

    gm('draw_glyphs', (canvas, fetchedByteBuffers) => {
        canvas.clear(CanvasKit.WHITE);

        const paint = new CanvasKit.Paint();
        const font = new CanvasKit.Font(null, 24);
        paint.setAntiAlias(true);

        const DIM = 16; // row/col count for the grid
        const GAP = 32; // spacing between each glyph
        const glyphs = new Uint16Array(256);
        const positions = new Float32Array(256*2);
        for (let i = 0; i < 256; ++i) {
            glyphs[i] = i;
            positions[2*i+0] = (i%DIM) * GAP;
            positions[2*i+1] = Math.round(i/DIM) * GAP;
        }
        canvas.drawGlyphs(glyphs, positions, 16, 20, font, paint);

        font.delete();
        paint.delete();
    });

    gm('image_decoding_methods', async (canvas) => {
        canvas.clear(CanvasKit.WHITE);

        const IMAGE_FILE_PATHS = [
            '/assets/brickwork-texture.jpg',
            '/assets/mandrill_512.png',
            '/assets/color_wheel.gif'
        ];

        let row = 1;
        // Test 4 different methods of decoding an image for each of the three images in
        // IMAGE_FILE_PATHS.
        // Resulting Images are drawn to visually show that all methods decode correctly.
        for (const imageFilePath of IMAGE_FILE_PATHS) {
            const response = await fetch(imageFilePath);
            const arrayBuffer = await response.arrayBuffer();
            // response.blob() is preferable when you don't need both a Blob *and* an ArrayBuffer.
            const blob = new Blob([ arrayBuffer ]);

            // Method 1 - decode TypedArray using wasm codecs:
            const skImage1 = CanvasKit.MakeImageFromEncoded(arrayBuffer);

            // Method 2 (slower and does not work in Safari) decode using ImageBitmap:
            const imageBitmap = await createImageBitmap(blob);
            // Testing showed that transferring an ImageBitmap to a canvas using the 'bitmaprenderer'
            // context and passing that canvas to CanvasKit.MakeImageFromCanvasImageSource() is
            // marginally faster than passing ImageBitmap to
            // CanvasKit.MakeImageFromCanvasImageSource() directly.
            const canvasBitmapElement = document.createElement('canvas');
            canvasBitmapElement.width = imageBitmap.width;
            canvasBitmapElement.height = imageBitmap.height;
            const ctxBitmap = canvasBitmapElement.getContext('bitmaprenderer');
            ctxBitmap.transferFromImageBitmap(imageBitmap);
            const skImage2 = CanvasKit.MakeImageFromCanvasImageSource(canvasBitmapElement);

            // Method 3 (slowest) decode using HTMLImageElement directly:
            const image = new Image();
            // Testing showed that waiting for a load event is faster than waiting on image.decode()
            // HTMLImageElement.decode() reference: https://developer.mozilla.org/en-US/docs/Web/API/HTMLImageElement/decode
            const promise1 = new Promise((resolve) => image.addEventListener('load', resolve));
            image.src = imageFilePath;
            await promise1;
            const skImage3 = CanvasKit.MakeImageFromCanvasImageSource(image);

            // Method 4 (roundabout, but works if all you have is a Blob) decode from Blob using
            // HTMLImageElement:
            const imageObjectUrl = URL.createObjectURL( blob );
            const image2 = new Image();
            const promise2 = new Promise((resolve) => image2.addEventListener('load', resolve));
            image2.src = imageObjectUrl;
            await promise2;
            const skImage4 = CanvasKit.MakeImageFromCanvasImageSource(image2);

            // Draw decoded images
            const sourceRect = CanvasKit.XYWHRect(0, 0, 150, 150);
            canvas.drawImageRect(skImage1, sourceRect, CanvasKit.XYWHRect(0, row * 100, 90, 90), null, false);
            canvas.drawImageRect(skImage2, sourceRect, CanvasKit.XYWHRect(100, row * 100, 90, 90), null, false);
            canvas.drawImageRect(skImage3, sourceRect, CanvasKit.XYWHRect(200, row * 100, 90, 90), null, false);
            canvas.drawImageRect(skImage4, sourceRect, CanvasKit.XYWHRect(300, row * 100, 90, 90), null, false);

            row++;
        }
        // Label images with the method used to decode them
        const paint = new CanvasKit.Paint();
        const textFont = new CanvasKit.Font(null, 7);
        canvas.drawText('WASM Decoding', 0, 90, paint, textFont);
        canvas.drawText('ImageBitmap Decoding', 100, 90, paint, textFont);
        canvas.drawText('HTMLImageEl Decoding', 200, 90, paint, textFont);
        canvas.drawText('Blob Decoding', 300, 90, paint, textFont);
    });

    gm('sweep_gradient', (canvas) => {
        const paint = new CanvasKit.Paint();
        const shader = CanvasKit.Shader.MakeSweepGradient(
            100, 100, // X, Y coordinates
            [CanvasKit.GREEN, CanvasKit.BLUE],
            [0.0, 1.0],
            CanvasKit.TileMode.Clamp,
        );
        expect(shader).toBeTruthy('Could not make shader');

        paint.setShader(shader);
        canvas.drawPaint(paint);

        paint.delete();
        shader.delete();
    });

    // TODO(kjlubick): There's a lot of shared code between the gradient gms
    // It would be best to deduplicate that in a nice DAMP way.
    // Inspired by https://fiddle.skia.org/c/b29ce50a341510784ac7d5281586d076
    gm('linear_gradients', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        canvas.scale(2, 2);
        const strokePaint = new CanvasKit.Paint();
        strokePaint.setStyle(CanvasKit.PaintStyle.Stroke);
        strokePaint.setColor(CanvasKit.BLACK);

        const paint = new CanvasKit.Paint();
        paint.setStyle(CanvasKit.PaintStyle.Fill);
        const transparentGreen = CanvasKit.Color(0, 255, 255, 0);

        const lgs = CanvasKit.Shader.MakeLinearGradient(
            [0, 0], [50, 100], // start and stop points
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror
        );
        paint.setShader(lgs);
        let r = CanvasKit.LTRBRect(0, 0, 100, 100);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const lgsPremul = CanvasKit.Shader.MakeLinearGradient(
            [100, 0], [150, 100], // start and stop points
            Uint32Array.of(
                CanvasKit.ColorAsInt(0, 255, 255, 0),
                CanvasKit.ColorAsInt(0, 0, 255, 255),
                CanvasKit.ColorAsInt(255, 0, 0, 255)),
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            null, // no local matrix
            1 // interpolate colors in premul
        );
        paint.setShader(lgsPremul);
        r = CanvasKit.LTRBRect(100, 0, 200, 100);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const lgs45 = CanvasKit.Shader.MakeLinearGradient(
            [0, 100], [50, 200], // start and stop points
            Float32Array.of(...transparentGreen, ...CanvasKit.BLUE, ...CanvasKit.RED),
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            CanvasKit.Matrix.rotated(Math.PI/4, 0, 100),
        );
        paint.setShader(lgs45);
        r = CanvasKit.LTRBRect(0, 100, 100, 200);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        // malloc'd color array
        const colors = CanvasKit.Malloc(Float32Array, 12);
        const typedColorsArray = colors.toTypedArray();
        typedColorsArray.set(transparentGreen, 0);
        typedColorsArray.set(CanvasKit.BLUE, 4);
        typedColorsArray.set(CanvasKit.RED, 8);
        const lgs45Premul = CanvasKit.Shader.MakeLinearGradient(
            [100, 100], [150, 200], // start and stop points
            typedColorsArray,
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            CanvasKit.Matrix.rotated(Math.PI/4, 100, 100),
            1 // interpolate colors in premul
        );
        CanvasKit.Free(colors);
        paint.setShader(lgs45Premul);
        r = CanvasKit.LTRBRect(100, 100, 200, 200);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        lgs.delete();
        lgs45.delete();
        lgsPremul.delete();
        lgs45Premul.delete();
        strokePaint.delete();
        paint.delete();
    });

    gm('radial_gradients', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        canvas.scale(2, 2);
        const strokePaint = new CanvasKit.Paint();
        strokePaint.setStyle(CanvasKit.PaintStyle.Stroke);
        strokePaint.setColor(CanvasKit.BLACK);

        const paint = new CanvasKit.Paint();
        paint.setStyle(CanvasKit.PaintStyle.Fill);
        const transparentGreen = CanvasKit.Color(0, 255, 255, 0);

        const rgs = CanvasKit.Shader.MakeRadialGradient(
            [50, 50], 50, // center, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror
        );
        paint.setShader(rgs);
        let r = CanvasKit.LTRBRect(0, 0, 100, 100);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const rgsPremul = CanvasKit.Shader.MakeRadialGradient(
            [150, 50], 50, // center, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            null, // no local matrix
            1 // interpolate colors in premul
        );
        paint.setShader(rgsPremul);
        r = CanvasKit.LTRBRect(100, 0, 200, 100);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const rgsSkew = CanvasKit.Shader.MakeRadialGradient(
            [50, 150], 50, // center, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            CanvasKit.Matrix.skewed(0.5, 0, 100, 100),
            null, // color space
        );
        paint.setShader(rgsSkew);
        r = CanvasKit.LTRBRect(0, 100, 100, 200);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const rgsSkewPremul = CanvasKit.Shader.MakeRadialGradient(
            [150, 150], 50, // center, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            CanvasKit.Matrix.skewed(0.5, 0, 100, 100),
            1, // interpolate colors in premul
            null, // color space
        );
        paint.setShader(rgsSkewPremul);
        r = CanvasKit.LTRBRect(100, 100, 200, 200);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        rgs.delete();
        rgsPremul.delete();
        rgsSkew.delete();
        rgsSkewPremul.delete();
        strokePaint.delete();
        paint.delete();
    });

    gm('conical_gradients', (canvas) => {
        canvas.clear(CanvasKit.WHITE);
        canvas.scale(2, 2);
        const strokePaint = new CanvasKit.Paint();
        strokePaint.setStyle(CanvasKit.PaintStyle.Stroke);
        strokePaint.setColor(CanvasKit.BLACK);

        const paint = new CanvasKit.Paint();
        paint.setStyle(CanvasKit.PaintStyle.Fill);
        paint.setAntiAlias(true);
        const transparentGreen = CanvasKit.Color(0, 255, 255, 0);

        const cgs = CanvasKit.Shader.MakeTwoPointConicalGradient(
            [80, 10], 15, // start, radius
            [10, 110], 60, // end, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            null, // no local matrix
        );
        paint.setShader(cgs);
        let r = CanvasKit.LTRBRect(0, 0, 100, 100);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const cgsPremul = CanvasKit.Shader.MakeTwoPointConicalGradient(
            [180, 10], 15, // start, radius
            [110, 110], 60, // end, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            null, // no local matrix
            1, // interpolate colors in premul
            null, // color space
        );
        paint.setShader(cgsPremul);
        r = CanvasKit.LTRBRect(100, 0, 200, 100);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const cgs45 = CanvasKit.Shader.MakeTwoPointConicalGradient(
            [80, 110], 15, // start, radius
            [10, 210], 60, // end, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            CanvasKit.Matrix.rotated(Math.PI/4, 0, 100),
            null, // color space
        );
        paint.setShader(cgs45);
        r = CanvasKit.LTRBRect(0, 100, 100, 200);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        const cgs45Premul = CanvasKit.Shader.MakeTwoPointConicalGradient(
            [180, 110], 15, // start, radius
            [110, 210], 60, // end, radius
            [transparentGreen, CanvasKit.BLUE, CanvasKit.RED],
            [0, 0.65, 1.0],
            CanvasKit.TileMode.Mirror,
            CanvasKit.Matrix.rotated(Math.PI/4, 100, 100),
            1, // interpolate colors in premul
            null, // color space
        );
        paint.setShader(cgs45Premul);
        r = CanvasKit.LTRBRect(100, 100, 200, 200);
        canvas.drawRect(r, paint);
        canvas.drawRect(r, strokePaint);

        cgs.delete();
        cgsPremul.delete();
        cgs45.delete();
        strokePaint.delete();
        paint.delete();
    });

    gm('blur_filters', (canvas) => {
        const pathUL = starPath(CanvasKit, 100, 100, 80);
        const pathBR = starPath(CanvasKit, 400, 300, 80);
        const paint = new CanvasKit.Paint();
        const textFont = new CanvasKit.Font(null, 24);

        canvas.drawText('Above: MaskFilter', 20, 220, paint, textFont);
        canvas.drawText('Right: ImageFilter', 20, 260, paint, textFont);

        paint.setColor(CanvasKit.BLUE);

        const blurMask = CanvasKit.MaskFilter.MakeBlur(CanvasKit.BlurStyle.Normal, 5, true);
        paint.setMaskFilter(blurMask);
        canvas.drawPath(pathUL, paint);

        const blurIF = CanvasKit.ImageFilter.MakeBlur(8, 1, CanvasKit.TileMode.Decal, null);
        paint.setImageFilter(blurIF);
        canvas.drawPath(pathBR, paint);

        pathUL.delete();
        pathBR.delete();
        paint.delete();
        blurMask.delete();
        blurIF.delete();
        textFont.delete();
    });

    gm('combined_filters', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();

        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 255, 0, 1.0));
        const redCF =  CanvasKit.ColorFilter.MakeBlend(
                CanvasKit.Color(255, 0, 0, 0.1), CanvasKit.BlendMode.SrcOver);
        const redIF = CanvasKit.ImageFilter.MakeColorFilter(redCF, null);
        const blurIF = CanvasKit.ImageFilter.MakeBlur(8, 0.2, CanvasKit.TileMode.Decal, null);
        const combined = CanvasKit.ImageFilter.MakeCompose(redIF, blurIF);

        // rotate 10 degrees centered on 200, 200
        const m = CanvasKit.Matrix.rotated(Math.PI/18, 200, 200);
        const filtering = { filter: CanvasKit.FilterMode.Linear };
        const rotated = CanvasKit.ImageFilter.MakeMatrixTransform(m, filtering, combined);
        paint.setImageFilter(rotated);

        //canvas.rotate(10, 200, 200);
        canvas.drawImage(img, 0, 0, paint);
        canvas.drawRect(CanvasKit.LTRBRect(5, 35, 45, 80), paint);

        paint.delete();
        redIF.delete();
        redCF.delete();
        blurIF.delete();
        combined.delete();
        rotated.delete();
        img.delete();
    }, '/assets/mandrill_512.png');

    gm('animated_filters', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeAnimatedImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();
        img.decodeNextFrame();
        img.decodeNextFrame();
        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        paint.setAntiAlias(true);
        paint.setColor(CanvasKit.Color(0, 255, 0, 1.0));
        const redCF =  CanvasKit.ColorFilter.MakeBlend(
                CanvasKit.Color(255, 0, 0, 0.1), CanvasKit.BlendMode.SrcOver);
        const redIF = CanvasKit.ImageFilter.MakeColorFilter(redCF, null);
        const blurIF = CanvasKit.ImageFilter.MakeBlur(8, 0.2, CanvasKit.TileMode.Decal, null);
        const combined = CanvasKit.ImageFilter.MakeCompose(redIF, blurIF);
        paint.setImageFilter(combined);

        const frame = img.makeImageAtCurrentFrame();
        canvas.drawImage(frame, 100, 50, paint);

        paint.delete();
        redIF.delete();
        redCF.delete();
        blurIF.delete();
        combined.delete();
        frame.delete();
        img.delete();
    }, '/assets/flightAnim.gif');

    gm('drawImageVariants', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();

        canvas.clear(CanvasKit.WHITE);
        canvas.scale(2, 2);
        const paint = new CanvasKit.Paint();
        const clipTo = (x, y) => {
            canvas.save();
            canvas.clipRect(CanvasKit.XYWHRect(x, y, 128, 128), CanvasKit.ClipOp.Intersect);
        };

        clipTo(0, 0);
        canvas.drawImage(img, 0, 0, paint);
        canvas.restore();

        clipTo(128, 0);
        canvas.drawImageCubic(img, 128, 0, 1/3, 1/3, null);
        canvas.restore();

        clipTo(0, 128);
        canvas.drawImageOptions(img, 0, 128, CanvasKit.FilterMode.Linear, CanvasKit.MipmapMode.None, null);
        canvas.restore();

        const mipImg = img.makeCopyWithDefaultMipmaps();
        clipTo(128, 128);
        canvas.drawImageOptions(mipImg, 128, 128,
                                CanvasKit.FilterMode.Nearest, CanvasKit.MipmapMode.Nearest, null);
        canvas.restore();

        paint.delete();
        mipImg.delete();
        img.delete();
    }, '/assets/mandrill_512.png');

    gm('drawImageRectVariants', (canvas, fetchedByteBuffers) => {
        const img = CanvasKit.MakeImageFromEncoded(fetchedByteBuffers[0]);
        expect(img).toBeTruthy();

        canvas.clear(CanvasKit.WHITE);
        const paint = new CanvasKit.Paint();
        const src = CanvasKit.XYWHRect(100, 100, 128, 128);
        canvas.drawImageRect(img, src, CanvasKit.XYWHRect(0, 0, 256, 256), paint);
        canvas.drawImageRectCubic(img, src, CanvasKit.XYWHRect(256, 0, 256, 256), 1/3, 1/3);
        canvas.drawImageRectOptions(img, src, CanvasKit.XYWHRect(0, 256, 256, 256),
                                    CanvasKit.FilterMode.Linear, CanvasKit.MipmapMode.None);
        const mipImg = img.makeCopyWithDefaultMipmaps();
        canvas.drawImageRectOptions(mipImg, src, CanvasKit.XYWHRect(256, 256, 256, 256),
                                CanvasKit.FilterMode.Nearest, CanvasKit.MipmapMode.Nearest);

        paint.delete();
        mipImg.delete();
        img.delete();
    }, '/assets/mandrill_512.png');

    gm('drawImage_skp', (canvas, fetchedByteBuffers) => {
        const pic = CanvasKit.MakePicture(fetchedByteBuffers[0]);
        canvas.clear(CanvasKit.TRANSPARENT);
        canvas.drawPicture(pic);
        // The asset below can be re-downloaded from
        // https://fiddle.skia.org/c/cbb8dee39e9f1576cd97c2d504db8eee
    }, '/assets/red_line.skp');

    it('can draw once using drawOnce utility method', (done) => {
        const surface = CanvasKit.MakeCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface');
        if (!surface) {
            done();
            return;
        }

        const drawFrame = (canvas) => {
            const paint = new CanvasKit.Paint();
            paint.setStrokeWidth(1.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            const path = new CanvasKit.Path();
            path.moveTo(20, 5);
            path.lineTo(30, 20);
            path.lineTo(40, 10);
            canvas.drawPath(path, paint);
            path.delete();
            paint.delete();
            // surface hasn't been flushed yet (nor should we call flush
            // ourselves), so reportSurface would likely be blank if we
            // were to call it.
            done();
        };
        surface.drawOnce(drawFrame);
        // Reminder: drawOnce is async. In this test, we are just making
        // sure the drawOnce function is there and doesn't crash, so we can
        // just call done() when the frame is rendered.
    });

    it('can draw client-supplied dirty rects', (done) => {
        // dirty rects are only honored by software (CPU) canvases today.
        const surface = CanvasKit.MakeSWCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface');
        if (!surface) {
            done();
            return;
        }

        const drawFrame = (canvas) => {
            const paint = new CanvasKit.Paint();
            paint.setStrokeWidth(1.0);
            paint.setAntiAlias(true);
            paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
            paint.setStyle(CanvasKit.PaintStyle.Stroke);
            const path = new CanvasKit.Path();
            path.moveTo(20, 5);
            path.lineTo(30, 20);
            path.lineTo(40, 10);
            canvas.drawPath(path, paint);
            path.delete();
            paint.delete();
            done();
        };
        const dirtyRect = CanvasKit.XYWHRect(10, 10, 15, 15);
        surface.drawOnce(drawFrame, dirtyRect);
        // We simply ensure that passing a dirty rect doesn't crash.
    });

    it('can use DecodeCache APIs', () => {
        const initialLimit = CanvasKit.getDecodeCacheLimitBytes();
        expect(initialLimit).toBeGreaterThan(1024 * 1024);

        const newLimit = 42 * 1024 * 1024;
        CanvasKit.setDecodeCacheLimitBytes(newLimit);
        expect(CanvasKit.getDecodeCacheLimitBytes()).toEqual(newLimit);

        // We cannot make any assumptions about this value,
        // so we just make sure it doesn't crash.
        CanvasKit.getDecodeCacheUsedBytes();
    });

    gm('combined_shaders', (canvas) => {
        const rShader = CanvasKit.Shader.Color(CanvasKit.Color(255, 0, 0, 1.0)); // deprecated
        const gShader = CanvasKit.Shader.MakeColor(CanvasKit.Color(0, 255, 0, 0.6));

        const rgShader = CanvasKit.Shader.MakeBlend(CanvasKit.BlendMode.SrcOver, rShader, gShader);

        const p = new CanvasKit.Paint();
        p.setShader(rgShader);
        canvas.drawPaint(p);

        rShader.delete();
        gShader.delete();
        rgShader.delete();
        p.delete();
    });

    it('exports consts correctly', () => {
        expect(CanvasKit.TRANSPARENT).toEqual(Float32Array.of(0, 0, 0, 0));
        expect(CanvasKit.RED).toEqual(Float32Array.of(1, 0, 0, 1));

        expect(CanvasKit.QUAD_VERB).toEqual(2);
        expect(CanvasKit.CONIC_VERB).toEqual(3);

        expect(CanvasKit.SaveLayerInitWithPrevious).toEqual(4);
        expect(CanvasKit.SaveLayerF16ColorType).toEqual(16);
    });

    it('can set color on a paint and get it as four floats', () => {
        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.Color4f(3.3, 2.2, 1.1, 0.5));
        expect(paint.getColor()).toEqual(Float32Array.of(3.3, 2.2, 1.1, 0.5));

        paint.setColorComponents(0.5, 0.6, 0.7, 0.8);
        expect(paint.getColor()).toEqual(Float32Array.of(0.5, 0.6, 0.7, 0.8));

        paint.setColorInt(CanvasKit.ColorAsInt(50, 100, 150, 200));
        let color = paint.getColor();
        expect(color.length).toEqual(4);
        expect(color[0]).toBeCloseTo(50/255, 5);  // Red
        expect(color[1]).toBeCloseTo(100/255, 5); // Green
        expect(color[2]).toBeCloseTo(150/255, 5); // Blue
        expect(color[3]).toBeCloseTo(200/255, 5); // Alpha

        paint.setColorInt(0xFF000000);
        expect(paint.getColor()).toEqual(Float32Array.of(0, 0, 0, 1.0));
    });

    gm('draw shadow', (canvas) => {
        const lightRadius = 20;
        const lightPos = [500,500,20];
        const zPlaneParams = [0,0,1];
        const path = starPath(CanvasKit);
        const textFont = new CanvasKit.Font(null, 24);
        const textPaint = new CanvasKit.Paint();

        canvas.drawShadow(path, zPlaneParams, lightPos, lightRadius,
                          CanvasKit.BLACK, CanvasKit.MAGENTA, 0);
        canvas.drawText('Default Flags', 5, 250, textPaint, textFont);

        let bounds = CanvasKit.getShadowLocalBounds(CanvasKit.Matrix.identity(),
            path, zPlaneParams, lightPos, lightRadius, 0);
        expectTypedArraysToEqual(bounds, Float32Array.of(-3.64462, -12.67541, 245.50, 242.59164));

        bounds = CanvasKit.getShadowLocalBounds(CanvasKit.M44.identity(),
            path, zPlaneParams, lightPos, lightRadius, 0);
        expectTypedArraysToEqual(bounds, Float32Array.of(-3.64462, -12.67541, 245.50, 242.59164));

        // Test that the APIs accept Malloc objs and the Malloced typearray
        const mZPlane = CanvasKit.Malloc(Float32Array, 3);
        mZPlane.toTypedArray().set(zPlaneParams);
        const mLight = CanvasKit.Malloc(Float32Array, 3);
        const lightTA = mLight.toTypedArray();
        lightTA.set(lightPos);

        canvas.translate(250, 250);
        canvas.drawShadow(path, mZPlane, lightTA, lightRadius,
                          CanvasKit.BLACK, CanvasKit.MAGENTA,
                          CanvasKit.ShadowTransparentOccluder | CanvasKit.ShadowGeometricOnly | CanvasKit.ShadowDirectionalLight);
        canvas.drawText('All Flags', 5, 250, textPaint, textFont);

        const outBounds = new Float32Array(4);
        CanvasKit.getShadowLocalBounds(CanvasKit.Matrix.rotated(Math.PI / 6),
            path, mZPlane, mLight, lightRadius,
            CanvasKit.ShadowTransparentOccluder | CanvasKit.ShadowGeometricOnly | CanvasKit.ShadowDirectionalLight,
            outBounds);
        expectTypedArraysToEqual(outBounds, Float32Array.of(-31.6630249, -15.24227, 245.5, 252.94101));

        CanvasKit.Free(mZPlane);
        CanvasKit.Free(mLight);

        path.delete();
        textFont.delete();
        textPaint.delete();
    });

    gm('fractal_noise_shader', (canvas) => {
        const shader = CanvasKit.Shader.MakeFractalNoise(0.1, 0.05, 2, 0, 0, 0);
        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.BLACK);
        paint.setShader(shader);
        canvas.drawPaint(paint);
        paint.delete();
        shader.delete();
    });

    gm('turbulance_shader', (canvas) => {
        const shader = CanvasKit.Shader.MakeTurbulence(0.1, 0.05, 2, 117, 0, 0);
        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.BLACK);
        paint.setShader(shader);
        canvas.drawPaint(paint);
        paint.delete();
        shader.delete();
    });

    gm('fractal_noise_tiled_shader', (canvas) => {
        const shader = CanvasKit.Shader.MakeFractalNoise(0.1, 0.05, 2, 0, 80, 80);
        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.BLACK);
        paint.setShader(shader);
        canvas.drawPaint(paint);
        paint.delete();
        shader.delete();
    });

    gm('turbulance_tiled_shader', (canvas) => {
        const shader = CanvasKit.Shader.MakeTurbulence(0.1, 0.05, 2, 117, 80, 80);
        const paint = new CanvasKit.Paint();
        paint.setColor(CanvasKit.BLACK);
        paint.setShader(shader);
        canvas.drawPaint(paint);
        paint.delete();
        shader.delete();
    });

    describe('ColorSpace Support', () => {
        it('Can create an SRGB 8888 surface', () => {
            const colorSpace = CanvasKit.ColorSpace.SRGB;
            const surface = CanvasKit.MakeCanvasSurface('test', CanvasKit.ColorSpace.SRGB);
            expect(surface).toBeTruthy('Could not make surface');
            let info = surface.imageInfo();
            expect(info.alphaType).toEqual(CanvasKit.AlphaType.Unpremul);
            expect(info.colorType).toEqual(CanvasKit.ColorType.RGBA_8888);
            expect(CanvasKit.ColorSpace.Equals(info.colorSpace, colorSpace))
                .toBeTruthy("Surface not created with correct color space.");

            const mObj = CanvasKit.Malloc(Uint8Array, CANVAS_WIDTH * CANVAS_HEIGHT * 4);
            mObj.toTypedArray()[0] = 127; // sentinel value. Should be overwritten by readPixels.
            const canvas = surface.getCanvas();
            canvas.clear(CanvasKit.TRANSPARENT);
            const pixels = canvas.readPixels(0, 0, {
                width: CANVAS_WIDTH,
                height: CANVAS_HEIGHT,
                colorType: CanvasKit.ColorType.RGBA_8888,
                alphaType: CanvasKit.AlphaType.Unpremul,
                colorSpace: colorSpace
            }, mObj, 4 * CANVAS_WIDTH);
            expect(pixels).toBeTruthy('Could not read pixels from surface');
            expect(pixels[0] !== 127).toBeTruthy();
            expect(pixels[0]).toEqual(mObj.toTypedArray()[0]);
            CanvasKit.Free(mObj);
            surface.delete();
        });
        it('Can create a Display P3 surface', () => {
            const colorSpace = CanvasKit.ColorSpace.DISPLAY_P3;
            const surface = CanvasKit.MakeCanvasSurface('test', CanvasKit.ColorSpace.DISPLAY_P3);
            expect(surface).toBeTruthy('Could not make surface');
            if (!surface.reportBackendTypeIsGPU()) {
                console.log('Not expecting color space support in cpu backed suface.');
                return;
            }
            let info = surface.imageInfo();
            expect(info.alphaType).toEqual(CanvasKit.AlphaType.Unpremul);
            expect(info.colorType).toEqual(CanvasKit.ColorType.RGBA_F16);
            expect(CanvasKit.ColorSpace.Equals(info.colorSpace, colorSpace))
                .toBeTruthy("Surface not created with correct color space.");

            const pixels = surface.getCanvas().readPixels(0, 0, {
                width: CANVAS_WIDTH,
                height: CANVAS_HEIGHT,
                colorType: CanvasKit.ColorType.RGBA_F16,
                alphaType: CanvasKit.AlphaType.Unpremul,
                colorSpace: colorSpace
            });
            expect(pixels).toBeTruthy('Could not read pixels from surface');
        });
        it('Can create an Adobe RGB surface', () => {
            const colorSpace = CanvasKit.ColorSpace.ADOBE_RGB;
            const surface = CanvasKit.MakeCanvasSurface('test', CanvasKit.ColorSpace.ADOBE_RGB);
            expect(surface).toBeTruthy('Could not make surface');
            if (!surface.reportBackendTypeIsGPU()) {
                console.log('Not expecting color space support in cpu backed surface.');
                return;
            }
            let info = surface.imageInfo();
            expect(info.alphaType).toEqual(CanvasKit.AlphaType.Unpremul);
            expect(info.colorType).toEqual(CanvasKit.ColorType.RGBA_F16);
            expect(CanvasKit.ColorSpace.Equals(info.colorSpace, colorSpace))
                .toBeTruthy("Surface not created with correct color space.");

            const pixels = surface.getCanvas().readPixels(0, 0, {
                width: CANVAS_WIDTH,
                height: CANVAS_HEIGHT,
                colorType: CanvasKit.ColorType.RGBA_F16,
                alphaType: CanvasKit.AlphaType.Unpremul,
                colorSpace: colorSpace
            });
            expect(pixels).toBeTruthy('Could not read pixels from surface');
        });

        it('combine draws from several color spaces', () => {
            const surface = CanvasKit.MakeCanvasSurface('test', CanvasKit.ColorSpace.ADOBE_RGB);
            expect(surface).toBeTruthy('Could not make surface');
            if (!surface.reportBackendTypeIsGPU()) {
                console.log('Not expecting color space support in cpu backed suface.');
                return;
            }
            const canvas = surface.getCanvas();

            let paint = new CanvasKit.Paint();
            paint.setColor(CanvasKit.RED, CanvasKit.ColorSpace.ADOBE_RGB);
            canvas.drawPaint(paint);
            paint.setColor(CanvasKit.RED, CanvasKit.ColorSpace.DISPLAY_P3); // 93.7 in adobeRGB
            canvas.drawRect(CanvasKit.LTRBRect(200, 0, 400, 600), paint);
            paint.setColor(CanvasKit.RED, CanvasKit.ColorSpace.SRGB); // 85.9 in adobeRGB
            canvas.drawRect(CanvasKit.LTRBRect(400, 0, 600, 600), paint);

            // this test paints three bands of red, each the maximum red that a color space supports.
            // They are each represented by skia by some color in the Adobe RGB space of the surface,
            // as floats between 0 and 1.

            // TODO(nifong) readpixels and verify correctness after f32 readpixels bug is fixed
        });
    }); // end describe('ColorSpace Support')

    describe('DOMMatrix support', () => {
        gm('sweep_gradient_dommatrix', (canvas) => {
            const paint = new CanvasKit.Paint();
            const shader = CanvasKit.Shader.MakeSweepGradient(
                100, 100, // x y coordinates
                [CanvasKit.GREEN, CanvasKit.BLUE],
                [0.0, 1.0],
                CanvasKit.TileMode.Clamp,
                new DOMMatrix().translate(-10, 100),
            );
            expect(shader).toBeTruthy('Could not make shader');
            if (!shader) {
                return;
            }

            paint.setShader(shader);
            canvas.drawPaint(paint);

            paint.delete();
            shader.delete();
        });

        const radiansToDegrees = (rad) => {
           return (rad / Math.PI) * 180;
        };

        // this should draw the same as concat_with4x4_canvas
        gm('concat_dommatrix', (canvas) => {
            const path = starPath(CanvasKit, CANVAS_WIDTH/2, CANVAS_HEIGHT/2);
            const paint = new CanvasKit.Paint();
            paint.setAntiAlias(true);
            canvas.clear(CanvasKit.WHITE);
            canvas.concat(new DOMMatrix().translate(CANVAS_WIDTH/2, 0, 0));
            canvas.concat(new DOMMatrix().rotateAxisAngle(1, 0, 0, radiansToDegrees(Math.PI/3)));
            canvas.concat(new DOMMatrix().rotateAxisAngle(0, 1, 0, radiansToDegrees(Math.PI/4)));
            canvas.concat(new DOMMatrix().rotateAxisAngle(0, 0, 1, radiansToDegrees(Math.PI/16)));
            canvas.concat(new DOMMatrix().translate(-CANVAS_WIDTH/2, 0, 0));

            const localMatrix = canvas.getLocalToDevice();
            expect4x4MatricesToMatch([
             0.693519, -0.137949,  0.707106,   91.944030,
             0.698150,  0.370924, -0.612372, -209.445297,
            -0.177806,  0.918359,  0.353553,   53.342029,
             0       ,  0       ,  0       ,    1       ], localMatrix);

            // Draw some stripes to help the eye detect the turn
            const stripeWidth = 10;
            paint.setColor(CanvasKit.BLACK);
            for (let i = 0; i < CANVAS_WIDTH; i += 2*stripeWidth) {
                canvas.drawRect(CanvasKit.LTRBRect(i, 0, i + stripeWidth, CANVAS_HEIGHT), paint);
            }

            paint.setColor(CanvasKit.YELLOW);
            canvas.drawPath(path, paint);
            paint.delete();
            path.delete();
        });
    }); // end describe('DOMMatrix support')

    it('can call subarray on a Malloced object', () => {
        const mThings = CanvasKit.Malloc(Float32Array, 6);
        mThings.toTypedArray().set([4, 5, 6, 7, 8, 9]);
        expectTypedArraysToEqual(Float32Array.of(4, 5, 6, 7, 8, 9), mThings.toTypedArray());
        expectTypedArraysToEqual(Float32Array.of(4, 5, 6, 7, 8, 9), mThings.subarray(0));
        expectTypedArraysToEqual(Float32Array.of(7, 8, 9), mThings.subarray(3));
        expectTypedArraysToEqual(Float32Array.of(7), mThings.subarray(3, 4));
        expectTypedArraysToEqual(Float32Array.of(7, 8), mThings.subarray(3, 5));

        // mutations on the subarray affect the entire array (because they are backed by the
        // same memory)
        mThings.subarray(3)[0] = 100.5;
        expectTypedArraysToEqual(Float32Array.of(4, 5, 6, 100.5, 8, 9), mThings.toTypedArray());
        CanvasKit.Free(mThings);
    });

    function expectTypedArraysToEqual(expected, actual) {
        expect(expected.constructor.name).toEqual(actual.constructor.name);
        expect(expected.length).toEqual(actual.length);
        for (let i = 0; i < expected.length; i++) {
            expect(expected[i]).toBeCloseTo(actual[i], 5, `element ${i}`);
        }
    }

    it('can create a RasterDirectSurface', () => {
        // Make enough space for a 5x5 8888 surface (4 bytes for R, G, B, A)
        const rdsData = CanvasKit.Malloc(Uint8Array, 5 * 5 * 4);
        const surface = CanvasKit.MakeRasterDirectSurface({
            'width': 5,
            'height': 5,
            'colorType': CanvasKit.ColorType.RGBA_8888,
            'alphaType': CanvasKit.AlphaType.Premul,
            'colorSpace': CanvasKit.ColorSpace.SRGB,
        }, rdsData, 5 * 4);

        surface.getCanvas().clear(CanvasKit.Color(200, 100, 0, 0.8));
        const pixels = rdsData.toTypedArray();
        // Check that the first pixels colors are right.
        expect(pixels[0]).toEqual(160); // red (premul, 0.8 * 200)
        expect(pixels[1]).toEqual(80); // green (premul, 0.8 * 100)
        expect(pixels[2]).toEqual(0); // blue (premul, not that it matters)
        expect(pixels[3]).toEqual(204); // alpha (0.8 * 255)
        surface.delete();
        CanvasKit.Free(rdsData);
    });

    gm('makeImageFromTextureSource_TypedArray', (canvas, _, surface) => {
        if (!CanvasKit.gpu) {
            return;
        }
        // This creates and draws an Unpremul Image that is 1 pixel wide, 4 pixels tall with
        // the colors listed below.
        const pixels = Uint8Array.from([
            255,   0,   0, 255, // opaque red
              0, 255,   0, 255, // opaque green
              0,   0, 255, 255, // opaque blue
            255,   0, 255, 100, // transparent purple
        ]);
        const img = surface.makeImageFromTextureSource(pixels, {
              'width': 1,
              'height': 4,
              'alphaType': CanvasKit.AlphaType.Unpremul,
              'colorType': CanvasKit.ColorType.RGBA_8888,
            });
        canvas.drawImage(img, 1, 1, null);

        const info = img.getImageInfo();
        expect(info).toEqual({
          'width': 1,
          'height': 4,
          'alphaType': CanvasKit.AlphaType.Unpremul,
          'colorType': CanvasKit.ColorType.RGBA_8888,
        });
        const cs = img.getColorSpace();
        expect(CanvasKit.ColorSpace.Equals(cs, CanvasKit.ColorSpace.SRGB)).toBeTruthy();

        cs.delete();
        img.delete();
    });

    gm('makeImageFromTextureSource_PremulTypedArray', (canvas, _, surface) => {
        if (!CanvasKit.gpu) {
            return;
        }
        // This creates and draws an Unpremul Image that is 1 pixel wide, 4 pixels tall with
        // the colors listed below.
        const pixels = Uint8Array.from([
            255,   0,   0, 255, // opaque red
              0, 255,   0, 255, // opaque green
              0,   0, 255, 255, // opaque blue
            100,   0, 100, 100, // transparent purple
        ]);
        const img = surface.makeImageFromTextureSource(pixels, {
              'width': 1,
              'height': 4,
              'alphaType': CanvasKit.AlphaType.Premul,
              'colorType': CanvasKit.ColorType.RGBA_8888,
            });
        canvas.drawImage(img, 1, 1, null);

        const info = img.getImageInfo();
        expect(info).toEqual({
          'width': 1,
          'height': 4,
          'alphaType': CanvasKit.AlphaType.Premul,
          'colorType': CanvasKit.ColorType.RGBA_8888,
        });
        img.delete();
    });

    gm('makeImageFromTextureSource_imgElement', (canvas, _, surface) => {
        if (!CanvasKit.gpu) {
            return;
        }
        // This makes an offscreen <img> with the provided source.
        const imageEle = new Image();
        imageEle.src = '/assets/mandrill_512.png';

        // We need to wait until the image is loaded before the texture can use it. For good
        // measure, we also wait for it to be decoded.
        return imageEle.decode().then(() => {
            const img = surface.makeImageFromTextureSource(imageEle);
            canvas.drawImage(img, 0, 0, null);

            const info = img.getImageInfo();
            expect(info).toEqual({
              'width': 512, // width and height should be derived from the image.
              'height': 512,
              'alphaType': CanvasKit.AlphaType.Unpremul,
              'colorType': CanvasKit.ColorType.RGBA_8888,
            });
            img.delete();
        });
    });

    gm('MakeLazyImageFromTextureSource_imgElement', (canvas) => {
        if (!CanvasKit.gpu) {
            return;
        }
        // This makes an offscreen <img> with the provided source.
        const imageEle = new Image();
        imageEle.src = '/assets/mandrill_512.png';

        // We need to wait until the image is loaded before the texture can use it. For good
        // measure, we also wait for it to be decoded.
        return imageEle.decode().then(() => {
            const img = CanvasKit.MakeLazyImageFromTextureSource(imageEle);
            canvas.drawImage(img, 5, 5, null);

            const info = img.getImageInfo();
            expect(info).toEqual({
              'width': 512, // width and height should be derived from the image.
              'height': 512,
              'alphaType': CanvasKit.AlphaType.Unpremul,
              'colorType': CanvasKit.ColorType.RGBA_8888,
            });
            img.delete();
        });
    });

    gm('MakeLazyImageFromTextureSource_imageInfo', (canvas) => {
        if (!CanvasKit.gpu) {
            return;
        }
        // This makes an offscreen <img> with the provided source.
        const imageEle = new Image();
        imageEle.src = '/assets/mandrill_512.png';

        // We need to wait until the image is loaded before the texture can use it. For good
        // measure, we also wait for it to be decoded.
        return imageEle.decode().then(() => {
            const img = CanvasKit.MakeLazyImageFromTextureSource(imageEle, {
              'width': 400,
              'height': 400,
              'alphaType': CanvasKit.AlphaType.Premul,
              'colorType': CanvasKit.ColorType.RGBA_8888,
            });
            canvas.drawImage(img, 20, 20, null);

            img.delete();
        });
    });
});
