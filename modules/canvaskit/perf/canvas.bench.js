describe('Basic Canvas ops', () => {

    let container = document.createElement('div');
    document.body.appendChild(container);

    beforeEach(async () => {
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>`;
        await LoadCanvasKit;
    });

    afterEach(function() {
        container.innerHTML = '';
    });

    it('can draw 100 colored regions', () => {
        function setup(ctx) {
            ctx.surface = CanvasKit.MakeCanvasSurface('test');
            ctx.canvas = ctx.surface.getCanvas();
        };

        function test(ctx) {
            for (let i=0; i<100; i++) {
                ctx.canvas.save();
                ctx.canvas.clipRect(CanvasKit.LTRBRect(i, i, i+100, i+100),
                                    CanvasKit.ClipOp.Intersect, false);
                ctx.canvas.drawColor(CanvasKit.Color4f(1-i/100, 1.0, i/100, 1.0),
                                     CanvasKit.BlendMode.SrcOver);
                ctx.canvas.restore();
            }
            ctx.surface.flush();
        };

        function teardown(ctx) {
            ctx.surface.delete();
        };

        benchmarkAndReport('canvas_drawColor', setup, test, teardown);
    });

    it('can compute tonal colors', () => {
        function setup(ctx) {};

        function test(ctx) {
            for (let i = 0; i < 10; i++) {
                const input = {
                    ambient: randomColor(),
                    spot: randomColor(),
                };
                const out = CanvasKit.computeTonalColors(input);
                if (out.spot[2] > 10 || out.ambient[3] > 10) {
                    // Something to make sure v8 can't optimize away the return value
                    throw 'not possible';
                }
            }
        };

        function teardown(ctx) {};

        benchmarkAndReport('computeTonalColors', setup, test, teardown);
    });

    function randomColor() {
        return CanvasKit.Color4f(Math.random(), Math.random(), Math.random(), Math.random());
    }

    it('can get and set the color to a paint', () => {
        function setup(ctx) {
            ctx.paint = new CanvasKit.SkPaint();
        };

        function test(ctx) {
            for (let i = 0; i < 10; i++) {
                ctx.paint.setColor(randomColor());
                const color = ctx.paint.getColor();
                if (color[3] > 4) {
                    // Something to make sure v8 can't optimize away the return value
                    throw 'not possible';
                }
            }
        };

        function teardown(ctx) {
            ctx.paint.delete();
        };

        benchmarkAndReport('paint_setColor_getColor', setup, test, teardown);
    });

    it('can set the color to a paint by components', () => {
        function setup(ctx) {
            ctx.paint = new CanvasKit.SkPaint();
        };

        function test(ctx) {
            const r = Math.random();
            const g = Math.random();
            const b = Math.random();
            const a = Math.random();
            for (let i = 0; i < 10000; i++) {
                ctx.paint.setColorComponents(r, g, b, a);
            }
        };

        function teardown(ctx) {
            ctx.paint.delete();
        };

        benchmarkAndReport('paint_setColorComponents', setup, test, teardown);
    });

    it('can draw a shadow with tonal colors', () => {
        function setup(ctx) {
            ctx.surface = CanvasKit.MakeCanvasSurface('test');
            ctx.canvas = ctx.surface.getCanvas();
        };

        const input = {
            ambient: CanvasKit.Color4f(0.2, 0.1, 0.3, 0.5),
            spot: CanvasKit.Color4f(0.8, 0.8, 0.9, 0.9),
        };
        const lightRadius = 30;
        const flags = 0;
        const lightPos = [250,150,300];
        const zPlaneParams = [0,0,1];
        const path = starPath(CanvasKit);

        function test(ctx) {
            const out = CanvasKit.computeTonalColors(input);
            ctx.canvas.drawShadow(path, zPlaneParams, lightPos, lightRadius,
                                  out.ambient, out.spot, flags);
            ctx.surface.flush();
        };

        function teardown(ctx) {
            ctx.surface.delete();
        };

        benchmarkAndReport('canvas_drawShadow', setup, test, teardown);
    });

    it('can draw a gradient with an array of 1M colors', () => {
        function setup(ctx) {
            ctx.surface = CanvasKit.MakeCanvasSurface('test');
            ctx.canvas = ctx.surface.getCanvas();
        };

        function test(ctx) {
            ctx.canvas.clear(CanvasKit.WHITE);

            const num = 1000000;
            colors = Array(num);
            positions = Array(num);
            for (let i=0; i<num; i++) {
                colors[i] = CanvasKit.Color((i*37)%255, 255, (1-i/num)*255);
                positions[i] = i/num;
            }

            const shader = CanvasKit.SkShader.MakeRadialGradient(
                [300, 300], 50, // center, radius
                colors, positions,
                CanvasKit.TileMode.Mirror,
            );
            const paint = new CanvasKit.SkPaint();
            paint.setStyle(CanvasKit.PaintStyle.Fill);
            paint.setShader(shader);
            ctx.canvas.drawPaint(paint);
            ctx.surface.flush();
            paint.delete();
        };

        function teardown(ctx) {
            ctx.surface.delete();
        };

        benchmarkAndReport('canvas_drawHugeGradient', setup, test, teardown);
    });
});
