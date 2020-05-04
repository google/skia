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
});
