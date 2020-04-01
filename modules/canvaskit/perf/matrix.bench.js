describe('SkMatrix', () => {

    beforeEach(async () => {
        await LoadCanvasKit;
    });

    it('can multiply matrices together', () => {
        const first = CanvasKit.SkMatrix.rotated(Math.PI/2, 10, 20);
        const second = CanvasKit.SkMatrix.scaled(1, 2, 3, 4);
        function setup(ctx) {}

        function test(ctx) {
            ctx.result = CanvasKit.SkMatrix.multiply(first, second);
            if (ctx.result.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('skmatrix_multiply', setup, test, teardown);
    });

    it('can transform a point using a matrix (mapPoint)', () => {
        const matr = CanvasKit.SkMatrix.multiply(
            CanvasKit.SkMatrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.SkMatrix.scaled(1, 2, 3, 4),
        ); // make an arbitrary, but interesting matrix
        function setup(ctx) {}

        function test(ctx) {
            for (let i = 0; i < 30; i++) {
                const pt = CanvasKit.SkMatrix.mapPoints(matr, [i, i]);
                if (pt.length === 18) {
                    throw 'this is here to keep pt from being optimized away';
                }
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('skmatrix_transformPoint', setup, test, teardown);
    });

    it('can be used to create a shader', () => {
        const matr = CanvasKit.SkMatrix.multiply(
            CanvasKit.SkMatrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.SkMatrix.scaled(1, 2, 3, 4),
        );
        function setup(ctx) {}

        function test(ctx) {
            const shader = CanvasKit.SkShader.MakeSweepGradient(
                100, 100,
                [CanvasKit.GREEN, CanvasKit.BLUE],
                [0.0, 1.0],
                CanvasKit.TileMode.Clamp,
                matr,
            );
            shader.delete();
        }

        function teardown(ctx) {}

        benchmarkAndReport('skmatrix_makeShader', setup, test, teardown);
    });
});

describe('DOMMatrix', () => {

    beforeEach(async () => {
        await LoadCanvasKit;
    });

    it('can multiply matrices together', () => {
        const first = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20);
        const second = new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4);
        function setup(ctx) {}

        function test(ctx) {
            ctx.result = first.multiply(second)
            if (ctx.result.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('dommatrix_multiply', setup, test, teardown);
    });

    it('can transform a point using a matrix (transformPoint)', () => {
        const matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
                .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));
        function setup(ctx) {}

        function test(ctx) {
            for (let i = 0; i < 30; i++) {
                const pt = matr.transformPoint(new DOMPoint(i, i))
                if (pt.length === 18) {
                    throw 'this is here to keep pt from being optimized away';
                }
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('dommatrix_transformPoint', setup, test, teardown);
    });

    it('can be used to create a shader', () => {
        const matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
                .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));
        function setup(ctx) {}

        function test(ctx) {
            const shader = CanvasKit.SkShader.MakeSweepGradient(
                100, 100,
                [CanvasKit.GREEN, CanvasKit.BLUE],
                [0.0, 1.0],
                CanvasKit.TileMode.Clamp,
                matr,
            );
            shader.delete();
        }

        function teardown(ctx) {}

        benchmarkAndReport('dommatrix_makeShader', setup, test, teardown);
    });
});