

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
            path.moveTo(0, 0);
            path.lineTo(10, 0);
            path.lineTo(10, 10);
            path.close();
            ctx.path = path;
        }

        function test(ctx) {
            ctx.path.toCmds();
        }

        function teardown(ctx) {
            ctx.path.delete();
        }


        LoadPathKit.then(() => {
            let ctx = {};
            // warmup
            setup(ctx);
            test(ctx);
            teardown(ctx);

            ctx = {};
            setup(ctx);
            let start = new Date().getTime();
            let now = start;
            times = 0;
            while (now - start < 100) {
                test(ctx);
                now = new Date().getTime();
                times++;
            }

            teardown(ctx);

            // Try to make it go for 1 second
            let goalTimes = times * 50;
            setup(ctx);
            start = new Date();
            times = 0;
            while (times < goalTimes) {
                test(ctx);
                times++;
            }
            let end = new Date();
            teardown(ctx);

            let us = (end.getTime() - start.getTime()) * 1000 / times;

            console.log(`Each op was about about ${us} microseconds`);

            done();
        });
    });
});