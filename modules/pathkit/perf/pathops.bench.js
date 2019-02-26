

describe('PathKit\'s Pathops', function() {
    // Note, don't try to print the PathKit object - it can cause Karma/Jasmine to lock up.
    var PathKit = null;
    const LoadPathKit = new Promise(function(resolve, reject) {
        if (PathKit) {
            resolve();
        } else {
            PathKitInit({
                locateFile: (file) => '/pathkit/'+file,
            }).ready().then((_PathKit) => {
                PathKit = _PathKit;
                resolve();
            });
        }
    });

    // see https://fiddle.skia.org/c/@discrete_path
    function drawStar(X=128, Y=128, R=116) {
        let p = PathKit.NewPath();
        p.moveTo(X + R, Y);
        for (let i = 1; i < 8; i++) {
          let a = 2.6927937 * i;
          p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
        }
        p.closePath();
        return p;
    }

    it('pathops_simplify', function(done) {
        function setup(ctx) {
            ctx.path = drawStar();
        }

        function test(ctx) {
            let path = ctx.path.copy().simplify();
            path.delete();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('pathops_simplify', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('pathops_diff', function(done) {
        function setup(ctx) {
            // Values chosen abitrarily to have some overlap and some not.
            ctx.path1 = drawStar(X=120, Y=120);
            ctx.path2 = drawStar(X=140, Y=145);
        }

        function test(ctx) {
            let path = PathKit.MakeFromOp(ctx.path1, ctx.path2, PathKit.PathOp.DIFFERENCE);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path1.delete();
            ctx.path2.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('pathops_diff', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('pathops_intersect', function(done) {
        function setup(ctx) {
            // Values chosen abitrarily to have some overlap and some not.
            ctx.path1 = drawStar(X=120, Y=120);
            ctx.path2 = drawStar(X=140, Y=145);
        }

        function test(ctx) {
            let path = PathKit.MakeFromOp(ctx.path1, ctx.path2, PathKit.PathOp.INTERSECT);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path1.delete();
            ctx.path2.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('pathops_intersect', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('pathops_union', function(done) {
        function setup(ctx) {
            // Values chosen abitrarily to have some overlap and some not.
            ctx.path1 = drawStar(X=120, Y=120);
            ctx.path2 = drawStar(X=140, Y=145);
        }

        function test(ctx) {
            let path = PathKit.MakeFromOp(ctx.path1, ctx.path2, PathKit.PathOp.UNION);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path1.delete();
            ctx.path2.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('pathops_union', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('pathops_xor', function(done) {
        function setup(ctx) {
            // Values chosen abitrarily to have some overlap and some not.
            ctx.path1 = drawStar(X=120, Y=120);
            ctx.path2 = drawStar(X=140, Y=145);
        }

        function test(ctx) {
            let path = PathKit.MakeFromOp(ctx.path1, ctx.path2, PathKit.PathOp.XOR);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path1.delete();
            ctx.path2.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('pathops_xor', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('pathops_reverse_diff', function(done) {
        function setup(ctx) {
            // Values chosen abitrarily to have some overlap and some not.
            ctx.path1 = drawStar(X=120, Y=120);
            ctx.path2 = drawStar(X=140, Y=145);
        }

        function test(ctx) {
            let path = PathKit.MakeFromOp(ctx.path1, ctx.path2, PathKit.PathOp.REVERSE_DIFFERENCE);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path1.delete();
            ctx.path2.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('pathops_reverse_diff', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

});