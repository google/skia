

describe('PathKit\'s Effects', function() {
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

    it('effects_dash', function(done) {
        function setup(ctx) {
            ctx.path = drawStar();
        }

        function test(ctx) {
            let path = ctx.path.copy().dash(10, 3, 1);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('effects_dash', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('effects_trim', function(done) {
        function setup(ctx) {
            ctx.path = drawStar();
        }

        function test(ctx) {
            let path = ctx.path.copy().trim(0.25, .8);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('effects_trim', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('effects_trim_complement', function(done) {
        function setup(ctx) {
            ctx.path = drawStar();
        }

        function test(ctx) {
            let path = ctx.path.copy().trim(0.25, .8, true);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('effects_trim_complement', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('effects_transform', function(done) {
        function setup(ctx) {
            ctx.path = drawStar();
        }

        function test(ctx) {
            let path = ctx.path.copy().transform(3, 0, 0,
                                             0, 3, 0,
                                             0, 0, 1);
            path.delete();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('effects_transform', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('effects_stroke', function(done) {
        function setup(ctx) {
            ctx.path = drawStar();
        }

        function test(ctx) {
            let path = ctx.path.copy().stroke({
                    width: 15,
                    join: PathKit.StrokeJoin.BEVEL,
                    cap: PathKit.StrokeCap.BUTT,
                    miter_limit: 2,
                });
            path.delete();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('effects_stroke', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });

});