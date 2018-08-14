
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

    // see https://fiddle.skia.org/c/@discrete_path
    function drawStar() {
        let path = PathKit.NewPath();
        let R = 115.2, C = 128.0;
        path.moveTo(C + R + 22, C);
        for (let i = 1; i < 8; i++) {
            let a = 2.6927937 * i;
            path.lineTo(C + R * Math.cos(a) + 22, C + R * Math.sin(a));
        }
        path.closePath();
        return path;
    }

    describe('Dash Path Effect', function() {
        it('performs dash inplace with start, stop, phase', function(done) {
            LoadPathKit.then(() => {
                let dashed = drawStar().dash(10, 3, 0);
                let phased = drawStar().dash(10, 3, 2);

                expect(dashed.equals(phased)).toBe(false);
                // TODO(store to Gold), dashed_no_phase, dashed_with_phase

                dashed.delete();
                phased.delete();
                done();
            });
        });

        it('creates a dash copy with start, stop, phase', function(done) {
            LoadPathKit.then(() => {
                let path = drawStar();
                let dashed = path.makeDash(10, 3, 0);
                let phased = path.makeDash(10, 3, 2);

                expect(dashed.equals(phased)).toBe(false);
                expect(path.equals(phased)).toBe(false);
                expect(path.equals(dashed)).toBe(false);
                // TODO(store to Gold) dashed_no_phase_copy, dashed_with_phase_copy

                path.delete();
                phased.delete();
                dashed.delete();
                done();
            });
        });
    });

});
