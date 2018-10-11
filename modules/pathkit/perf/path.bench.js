

describe('PathKit\'s Path Behavior', function() {
    // Note, don't try to print the PathKit object - it can cause Karma/Jasmine to lock up.
    var PathKit = null;
    const LoadPathKit = new Promise(function(resolve, reject) {
        if (PathKit) {
            resolve();
        } else {
            PathKitInit({
                locateFile: (file) => '/pathkit/'+file,
            }).then((_PathKit) => {
                PathKit = _PathKit;
                resolve();
            });
        }
    });

    it('toCmds', function(done) {
        function setup(ctx) {
            let path = PathKit.NewPath();
            path.moveTo(20, 5);
            path.lineTo(30, 20);
            path.lineTo(40, 10);
            path.lineTo(50, 20);
            path.lineTo(60, 0);
            path.lineTo(20, 5);

            path.moveTo(20, 80);
            path.bezierCurveTo(90, 10, 160, 150, 190, 10);

            path.moveTo(36, 148);
            path.quadraticCurveTo(66, 188, 120, 136);
            path.lineTo(36, 148);

            path.rect(5, 170, 20, 20);

            path.moveTo(150, 180);
            path.arcTo(150, 100, 50, 200, 20);
            path.lineTo(160, 160);

            path.moveTo(20, 120);
            path.arc(20, 120, 18, 0, 1.75 * Math.PI);
            path.lineTo(20, 120);

            let secondPath = PathKit.NewPath();
            secondPath.ellipse(130, 25, 30, 10, -1*Math.PI/8, Math.PI/6, 1.5*Math.PI, false);

            path.addPath(secondPath);

            let m = document.createElementNS('http://www.w3.org/2000/svg', 'svg').createSVGMatrix();
            m.a = 1; m.b = 0;
            m.c = 0; m.d = 1;
            m.e = 0; m.f = 20.5;

            path.addPath(secondPath, m);
            secondPath.delete();
            ctx.path = path;
        }

        function test(ctx) {
            ctx.path.toCmds();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }

        LoadPathKit.then(() => {
            benchmarkAndReport('toCmds', setup, test, teardown).then(() => {
                done();
            }).catch(reportError(done));
        });
    });
});