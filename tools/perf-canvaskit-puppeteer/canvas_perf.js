const tests = [];

tests.push({
    name: 'Draw 10K colored regions',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();
        ctx.frame = 0;
    },
    test: function(CanvasKit, ctx) {
        // Draw a lot of colored squares.
        for (let i=0; i<10000; i++) {
            // bespoke random number generator that depends on frame so you can visually see the framerate.
            const z = (i*i+ctx.frame)%131;
            const seed0 = (z*5+i)%255;
            const seed1 = ((131-z)*50+i)%255;
            ctx.canvas.save();
            ctx.canvas.clipRect(CanvasKit.LTRBRect(seed0*2, seed1*2, seed0*2+30, seed1*2+30),
                                CanvasKit.ClipOp.Intersect, false);
            ctx.canvas.drawColor(CanvasKit.Color4f((seed1+ctx.frame*2)%255/255, seed0/255, 0, 0.1),
                                 CanvasKit.BlendMode.SrcOver);
            ctx.canvas.restore();
        }
        ctx.frame++;
        ctx.surface.flush();
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'canvas_drawColor',
});

tests.push({
    name: 'Draw a gradient with an array of 10K colors',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();
        ctx.frame = 0;
    },
    test: function(CanvasKit, ctx) {
        ctx.canvas.clear(CanvasKit.WHITE);

        const num = 10000;
        colors = Array(num);
        positions = Array(num);
        // Create an array of colors spaced evenly along the 0..1 range of positions.
        for (let i=0; i<num; i++) {
            colors[i] = CanvasKit.Color((i*ctx.frame)%255, 255, (1-i/num)*255);
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
        ctx.frame++;
    },
    teardown: function(CanvasKit, ctx) {},
    perfKey: 'canvas_drawHugeGradient',
});