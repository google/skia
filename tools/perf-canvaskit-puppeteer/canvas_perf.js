const tests = [];
// In all tests, the canvas is 600 by 600 px.

function randomColor(CanvasKit) {
    return CanvasKit.Color4f(Math.random(), Math.random(), Math.random(), Math.random());
}

function starPath(CanvasKit, X=128, Y=128, R=116) {
    const p = new CanvasKit.SkPath();
    p.moveTo(X + R, Y);
    for (let i = 1; i < 8; i++) {
      let a = 2.6927937 * i;
      p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
    }
    p.close();
    return p;
}

tests.push({
    description: 'Draw 10K colored regions',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();
    },
    test: function(CanvasKit, ctx) {
        // Draw a lot of colored squares.
        for (let i=0; i<10000; i++) {
            const x = Math.random()*550;
            const y = Math.random()*550;
            ctx.canvas.save();
            ctx.canvas.clipRect(CanvasKit.LTRBRect(x, y, x+50, y+50),
                                CanvasKit.ClipOp.Intersect, false);
            ctx.canvas.drawColor(randomColor(CanvasKit), CanvasKit.BlendMode.SrcOver);
            ctx.canvas.restore();
        }
        ctx.surface.flush();
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'canvas_drawColor',
});

tests.push({
    description: 'Compute tonal colors',
    setup: function(CanvasKit, ctx) {},
    test: function(CanvasKit, ctx) {
        for (let i = 0; i < 10; i++) {
            const input = {
                ambient: randomColor(CanvasKit),
                spot: randomColor(CanvasKit),
            };
            const out = CanvasKit.computeTonalColors(input);
            if (out.spot[2] > 10 || out.ambient[3] > 10) {
                // Something to make sure v8 can't optimize away the return value
                throw 'not possible';
            }
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'computeTonalColors',
});

tests.push({
    description: 'Get and set the color to a paint',
    setup: function(CanvasKit, ctx) {
        ctx.paint = new CanvasKit.SkPaint();
    },
    test: function(CanvasKit, ctx) {
        for (let i = 0; i < 10; i++) {
            ctx.paint.setColor(randomColor(CanvasKit));
            const color = ctx.paint.getColor();
            if (color[3] > 4) {
                // Something to make sure v8 can't optimize away the return value
                throw 'not possible';
            }
        }
    },
    teardown: function(CanvasKit, ctx) {
        ctx.paint.delete();
    },
    perfKey: 'paint_setColor_getColor',
});

tests.push({
    description: 'Set the color to a paint by components',
    setup: function(CanvasKit, ctx) {
        ctx.paint = new CanvasKit.SkPaint();
    },
    test: function(CanvasKit, ctx) {
        const r = Math.random();
        const g = Math.random();
        const b = Math.random();
        const a = Math.random();
        for (let i = 0; i < 10000; i++) {
            ctx.paint.setColorComponents(r, g, b, a);
        }
    },
    teardown: function(CanvasKit, ctx) {
        ctx.paint.delete();
    },
    perfKey: 'paint_setColorComponents',
});

tests.push({
    description: 'Draw a shadow with tonal colors',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();

        ctx.input = {
            ambient: CanvasKit.Color4f(0.2, 0.1, 0.3, 0.5),
            spot: CanvasKit.Color4f(0.8, 0.8, 0.9, 0.9),
        };
        ctx.lightRadius = 30;
        ctx.flags = 0;
        ctx.lightPos = [250,150,300];
        ctx.zPlaneParams = [0,0,1];
        ctx.path = starPath(CanvasKit);
    },
    test: function(CanvasKit, ctx) {
        const out = CanvasKit.computeTonalColors(ctx.input);
        ctx.canvas.drawShadow(ctx.path, ctx.zPlaneParams, ctx.lightPos, ctx.lightRadius,
                              out.ambient, out.spot, ctx.flags);
        ctx.surface.flush();
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'canvas_drawShadow',
});

tests.push({
    description: 'Draw a gradient with an array of 10K colors',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();
    },
    test: function(CanvasKit, ctx) {
        ctx.canvas.clear(CanvasKit.WHITE);

        const num = 10000;
        colors = Array(num);
        positions = Array(num);
        // Create an array of colors spaced evenly along the 0..1 range of positions.
        for (let i=0; i<num; i++) {
            colors[i] = randomColor(CanvasKit);
            positions[i] = i/num;
        }
        // make a gradient from those colors
        const shader = CanvasKit.SkShader.MakeRadialGradient(
            [300, 300], 50, // center, radius
            colors, positions,
            CanvasKit.TileMode.Mirror,
        );
        // Fill the canvas using the gradient shader.
        const paint = new CanvasKit.SkPaint();
        paint.setStyle(CanvasKit.PaintStyle.Fill);
        paint.setShader(shader);
        ctx.canvas.drawPaint(paint);
        ctx.surface.flush();

        shader.delete();
        paint.delete();
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'canvas_drawHugeGradient',
});

tests.push({
    description: 'Draw a png image',
    setup: async function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();
        ctx.paint = new CanvasKit.SkPaint();
        ctx.img = CanvasKit.MakeImageFromEncoded(ctx.files['test_512x512.png']);
        ctx.frame = 0;
    },
    test: function(CanvasKit, ctx) {
        ctx.canvas.clear(CanvasKit.WHITE);
        // Make the image to move so you can see visually that the test is running.
        ctx.canvas.drawImage(ctx.img, ctx.frame, ctx.frame, ctx.paint);
        ctx.surface.flush();
        ctx.frame++;
    },
    teardown: function(CanvasKit, ctx) {
        ctx.img.delete();
        ctx.paint.delete();
    },
    perfKey: 'canvas_drawPngImage',
});


function htmlImageElementToDataURL(htmlImageElement) {
    const canvas = document.createElement('canvas');
    canvas.height = htmlImageElement.height;
    canvas.width = htmlImageElement.width;
    const ctx = canvas.getContext('2d')
    ctx.drawImage(htmlImageElement, 0, 0);
    return canvas.toDataURL();
}

// This for loop generates two perf cases for each test image. One uses browser APIs
// to decode an image, and the other uses codecs included in the CanvasKit wasm to decode an
// image. wasm codec Image decoding is faster (50 microseconds vs 20000 microseconds), but
// including codecs in wasm increases the size of the CanvasKit wasm binary.
for (const testImageFilename of ['test_64x64.png', 'test_512x512.png', 'test_1500x959.jpg']) {
    const htmlImageElement = new Image();
    htmlImageElementLoadPromise = new Promise((resolve) =>
        htmlImageElement.addEventListener('load', resolve));
    // Create a data url of the image so that load and decode time can be measured
    // while ignoring the time of getting the image from disk / the network.
    imageDataURLPromise = htmlImageElementLoadPromise.then(() =>
        htmlImageElementToDataURL(htmlImageElement));
    htmlImageElement.src = `/static/assets/${testImageFilename}`;

    tests.push({
        description: 'Decode an image using HTMLImageElement and Canvas2D',
        setup: async function(CanvasKit, ctx) {
            ctx.imageDataURL = await imageDataURLPromise;
        },
        test: async function(CanvasKit, ctx) {
            const image = new Image();
            // Testing showed that waiting for the load event is faster than waiting for
            // image.decode().
            // Despite the name, both of them would decode the image, it was loaded in setup.
            // HTMLImageElement.decode() reference:
            // https://developer.mozilla.org/en-US/docs/Web/API/HTMLImageElement/decode
            const promise = new Promise((resolve) => image.addEventListener('load', resolve));
            image.src = ctx.imageDataURL;
            await promise;
            const img = await CanvasKit.MakeImageFromCanvasImageSource(image);
            img.delete();
        },
        teardown: function(CanvasKit, ctx) {},
        perfKey: `canvas_${testImageFilename}_HTMLImageElementDecoding`,
    });

    tests.push({
        description: 'Decode an image using codecs in wasm',
        setup: function(CanvasKit, ctx) {},
        test: function(CanvasKit, ctx) {
            const img = CanvasKit.MakeImageFromEncoded(ctx.files[testImageFilename]);
            img.delete();
        },
        teardown: function(CanvasKit, ctx) {},
        perfKey: '`canvas_${testImageFilename}_wasmImageDecoding`',
    });
}

// 3x3 matrix ops
tests.push({
    description: 'Multiply 3x3 matrices together',
    setup: function(CanvasKit, ctx) {
        ctx.first = CanvasKit.SkMatrix.rotated(Math.PI/2, 10, 20);
        ctx.second = CanvasKit.SkMatrix.scaled(1, 2, 3, 4);
    },
    test: function(CanvasKit, ctx) {
        ctx.result = CanvasKit.SkMatrix.multiply(ctx.first, ctx.second);
        if (ctx.result.length === 18) {
            throw 'this is here to keep the result from being optimized away';
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'skmatrix_multiply',
});

tests.push({
    description: 'Transform a point using a matrix (mapPoint)',
    setup: function(CanvasKit, ctx) {
        ctx.matr = CanvasKit.SkMatrix.multiply(
            CanvasKit.SkMatrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.SkMatrix.scaled(1, 2, 3, 4),
        ); // make an arbitrary, but interesting matrix
    },
    test: function(CanvasKit, ctx) {
        for (let i = 0; i < 30; i++) {
            const pt = CanvasKit.SkMatrix.mapPoints(ctx.matr, [i, i]);
            if (pt.length === 18) {
                throw 'this is here to keep pt from being optimized away';
            }
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'skmatrix_transformPoint',
});

tests.push({
    description: 'Invert a 3x3 matrix',
    setup: function(CanvasKit, ctx) {
        ctx.matr = CanvasKit.SkMatrix.multiply(
            CanvasKit.SkMatrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.SkMatrix.scaled(1, 2, 3, 4),
        );
    },
    test: function(CanvasKit, ctx) {
        ctx.result = CanvasKit.SkMatrix.invert(ctx.matr);
        if (ctx.result.length === 18) {
            throw 'this is here to keep the result from being optimized away';
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'skmatrix_invert',
});

tests.push({
    description: 'Create a shader from a 3x3 matrix',
    setup: function(CanvasKit, ctx) {
        ctx.matr = CanvasKit.SkMatrix.multiply(
            CanvasKit.SkMatrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.SkMatrix.scaled(1, 2, 3, 4),
        );
    },
    test: function(CanvasKit, ctx) {
        const shader = CanvasKit.SkShader.MakeSweepGradient(
            100, 100,
            [CanvasKit.GREEN, CanvasKit.BLUE],
            [0.0, 1.0],
            CanvasKit.TileMode.Clamp,
            ctx.matr);
        shader.delete();
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'skmatrix_makeShader',
});

// 4x4 matrix operations
tests.push({
    description: 'Multiply 4x4 matrices together',
    setup: function(CanvasKit, ctx) {
        ctx.first = CanvasKit.SkM44.rotated([10, 20], Math.PI/2);
        ctx.second = CanvasKit.SkM44.scaled([1, 2, 3]);
    },
    test: function(CanvasKit, ctx) {
        ctx.result = CanvasKit.SkM44.multiply(ctx.first, ctx.second);
        if (ctx.result.length === 18) {
            throw 'this is here to keep the result from being optimized away';
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'skm44_multiply',
});

tests.push({
    description: 'Invert a 4x4 matrix',
    setup: function(CanvasKit, ctx) {
        ctx.matr = CanvasKit.SkM44.multiply(
            CanvasKit.SkM44.rotated([10, 20], Math.PI/2),
            CanvasKit.SkM44.scaled([1, 2, 3]),
        );
    },
    test: function(CanvasKit, ctx) {
        ctx.result = CanvasKit.SkM44.invert(ctx.matr);
        if (ctx.result.length === 18) {
            throw 'this is here to keep the result from being optimized away';
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'skm44_invert',
});

tests.push({
    description: 'Concat 4x4 matrix on a canvas',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = new CanvasKit.SkCanvas();
        ctx.matr = CanvasKit.SkM44.multiply(
            CanvasKit.SkM44.rotated([10, 20], Math.PI/2),
            CanvasKit.SkM44.scaled([1, 2, 3]),
        );
    },
    test: function(CanvasKit, ctx) {
        ctx.canvas.concat(ctx.matr);
    },
    teardown: function(CanvasKit, ctx) {
        ctx.canvas.delete();
    },
    perfKey: 'skm44_concat',
});

// DOMMatrix operations
tests.push({
    description: 'Multiply DOM matrices together',
    setup: function(CanvasKit, ctx) {
        ctx.first = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20);
        ctx.second = new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4);
    },
    test: function(CanvasKit, ctx) {
        const result = ctx.first.multiply(ctx.second);
        if (result.length === 18) {
            throw 'this is here to keep the result from being optimized away';
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'dommatrix_multiply',
});

tests.push({
    description: 'Transform a point using a matrix (transformPoint)',
    setup: function(CanvasKit, ctx) {
        ctx.matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
            .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));

        ctx.reusablePt = new DOMPoint(0, 0)
    },
    test: function(CanvasKit, ctx) {
        for (let i = 0; i < 30; i++) {
            ctx.reusablePt.X = i; ctx.reusablePt.Y = i;
            const pt = ctx.matr.transformPoint(ctx.reusablePt);
            if (pt.length === 18) {
                throw 'this is here to keep pt from being optimized away';
            }
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'dommatrix_transformPoint',
});

tests.push({
    description: 'Invert a DOM matrix',
    setup: function(CanvasKit, ctx) {
        ctx.matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
            .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));
    },
    test: function(CanvasKit, ctx) {
        const inverted = ctx.matr.inverse();
        if (inverted.length === 18) {
            throw 'this is here to keep the result from being optimized away';
        }
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'dommatrix_invert',
});

tests.push({
    description: 'make a shader from a DOMMatrix',
    setup: function(CanvasKit, ctx) {
        ctx.matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
            .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));
    },
    test: function(CanvasKit, ctx) {
        const shader = CanvasKit.SkShader.MakeSweepGradient(
            100, 100,
            [CanvasKit.GREEN, CanvasKit.BLUE],
            [0.0, 1.0],
            CanvasKit.TileMode.Clamp,
            ctx.matr);
        shader.delete();
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'dommatrix_makeShader',
});
