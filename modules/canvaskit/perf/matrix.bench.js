describe('Matrix (3x3)', () => {

    beforeEach(async () => {
        await LoadCanvasKit;
    });

    it('can multiply matrices together', () => {
        const first = CanvasKit.Matrix.rotated(Math.PI/2, 10, 20);
        const second = CanvasKit.Matrix.scaled(1, 2, 3, 4);
        function setup(ctx) {}

        function test(ctx) {
            ctx.result = CanvasKit.Matrix.multiply(first, second);
            if (ctx.result.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('skmatrix_multiply', setup, test, teardown);
    });

    it('can transform a point using a matrix (mapPoint)', () => {
        const matr = CanvasKit.Matrix.multiply(
            CanvasKit.Matrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.Matrix.scaled(1, 2, 3, 4),
        ); // make an arbitrary, but interesting matrix
        function setup(ctx) {}

        function test(ctx) {
            for (let i = 0; i < 30; i++) {
                const pt = CanvasKit.Matrix.mapPoints(matr, [i, i]);
                if (pt.length === 18) {
                    throw 'this is here to keep pt from being optimized away';
                }
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('skmatrix_transformPoint', setup, test, teardown);
    });

    it('can invert a matrix', () => {
        const matr = CanvasKit.Matrix.multiply(
            CanvasKit.Matrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.Matrix.scaled(1, 2, 3, 4),
        );
        function setup(ctx) {}

        function test(ctx) {
            ctx.result = CanvasKit.Matrix.invert(matr);
            if (ctx.result.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('skmatrix_invert', setup, test, teardown);
    });

    it('can be used to create a shader', () => {
        const matr = CanvasKit.Matrix.multiply(
            CanvasKit.Matrix.rotated(Math.PI/2, 10, 20),
            CanvasKit.Matrix.scaled(1, 2, 3, 4),
        );
        function setup(ctx) {}

        function test(ctx) {
            const shader = CanvasKit.Shader.MakeSweepGradient(
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

describe('M44 (4x4 matrix)', () => {

    beforeEach(async () => {
        await LoadCanvasKit;
    });

    it('can multiply matrices together', () => {
        const first = CanvasKit.M44.rotated([10, 20, 30], Math.PI/2);
        const second = CanvasKit.M44.scaled([1, 2, 3]);
        function setup(ctx) {}

        function test(ctx) {
            ctx.result = CanvasKit.M44.multiply(first, second);
            if (ctx.result.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('skm44_multiply', setup, test, teardown);
    });

    it('can invert a matrix', () => {
        const matr = CanvasKit.M44.multiply(
            CanvasKit.M44.rotated([10, 20, 30], Math.PI/2),
            CanvasKit.M44.scaled([1, 2, 3]),
        );
        function setup(ctx) {}

        function test(ctx) {
            ctx.result = CanvasKit.M44.invert(matr);
            if (ctx.result.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('skm44_invert', setup, test, teardown);
    });

    it('can be used on a canvas', () => {
        const matr = CanvasKit.M44.multiply(
            CanvasKit.M44.rotated([10, 20, 30], Math.PI/2),
            CanvasKit.M44.scaled([1, 2, 3]),
        );
        function setup(ctx) {
            ctx.canvas = new CanvasKit.Canvas();
        }

        function test(ctx) {
            ctx.canvas.concat(matr);
        }

        function teardown(ctx) {
            ctx.canvas.delete();
        }

        benchmarkAndReport('skm44_concat', setup, test, teardown);
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
            const result = first.multiply(second)
            if (result.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('dommatrix_multiply', setup, test, teardown);
    });

    it('can transform a point using a matrix (transformPoint)', () => {
        const matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
                .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));

        const reusablePt = new DOMPoint(0, 0)
        function setup(ctx) {}

        function test(ctx) {
            for (let i = 0; i < 30; i++) {
                reusablePt.X = i; reusablePt.Y = i;
                const pt = matr.transformPoint(reusablePt);
                if (pt.length === 18) {
                    throw 'this is here to keep pt from being optimized away';
                }
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('dommatrix_transformPoint', setup, test, teardown);
    });

    it('can invert a matrix', () => {
        const matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
                .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));
        function setup(ctx) {}

        function test(ctx) {
            const inverted = matr.inverse();
            if (inverted.length === 18) {
                throw 'this is here to keep the result from being optimized away';
            }
        }

        function teardown(ctx) {}

        benchmarkAndReport('dommatrix_invert', setup, test, teardown);
    });

    it('can be used to create a shader', () => {
        const matr = new DOMMatrix().translate(10, 20).rotate(90).translate(-10, -20)
                .multiply(new DOMMatrix().translate(3, 4).scale(1, 2).translate(-3, -4));
        function setup(ctx) {}

        function test(ctx) {
            const shader = CanvasKit.Shader.MakeSweepGradient(
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
