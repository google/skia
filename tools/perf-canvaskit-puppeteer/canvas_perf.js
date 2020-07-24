const tests = [];

tests.push({
    name: 'Draw 10K colored regions',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();
    },
    test: function(CanvasKit, ctx) {
        for (let i=0; i<10000; i++) {
            const s  =(i*i).toString(2)
            const z = [...s.matchAll('0')].length;
            const seed0 = (z*50+i)%255
            const seed1 = ((s.length-z)*50+i)%255
            ctx.canvas.save();
            ctx.canvas.clipRect(CanvasKit.LTRBRect(seed0*2, seed1*2, seed0*2+30, seed1*2+30),
                                CanvasKit.ClipOp.Intersect, false);
            ctx.canvas.drawColor(CanvasKit.Color4f(seed1/255, seed0/255, 0, 1.0),
                                 CanvasKit.BlendMode.SrcOver);
            ctx.canvas.restore();
        }
        ctx.surface.flush();
    },
    teardown: function(CanvasKit, ctx) {
        //ctx.canvas.delete();
    },
    perfKey: 'canvas_drawColor',
});

tests.push({
    name: 'Draw a gradient with an array of 10K colors',
    setup: function(CanvasKit, ctx) {
        ctx.canvas = ctx.surface.getCanvas();
    },
    test: function(CanvasKit, ctx) {
        ctx.canvas.clear(CanvasKit.WHITE);

        const num = 10000;
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
    },
    teardown: function(CanvasKit, ctx) {
        ctx.canvas.delete();
    },
    perfKey: 'canvas_drawHugeGradient',
});