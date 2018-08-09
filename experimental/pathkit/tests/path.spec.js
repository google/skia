
describe('PathKit\'s Path Behavior', function() {
    // Note, don't try to print the PathKit object - it can cause Karma/Jasmine to lock up.
    var PathKit = null;
    const LoadPathKit = new Promise(function(resolve, reject){
        if (PathKit) {
            resolve();
        } else {
            PathKitInit({
                locateFile: (file) => '/base/npm-wasm/bin/test/'+file,
            }).then((_PathKit) => {
                PathKit = _PathKit;
                resolve();
            });
        }
    });

    it('dynamically updates getBounds()', function(done){
        LoadPathKit.then(() => {
            // Based on test_bounds_crbug_513799
            let path = PathKit.NewPath();
            expect(path.getBounds()).toEqual(PathKit.MakeLTRBRect(0, 0, 0, 0));
            path.moveTo(-5, -8);
            expect(path.getBounds()).toEqual(PathKit.MakeLTRBRect(-5, -8, -5, -8));
            path.rect(1, 2, 2, 2);
            expect(path.getBounds()).toEqual(PathKit.MakeLTRBRect(-5, -8, 3, 4));
            path.moveTo(1, 2);
            expect(path.getBounds()).toEqual(PathKit.MakeLTRBRect(-5, -8, 3, 4));
            path.delete();
            done();
        });
    });

    it('has getBounds() and computeTightBounds()', function(done){
        LoadPathKit.then(() => {
            // Based on PathOpsTightBoundsIllBehaved
            let path = PathKit.NewPath();
            path.moveTo(1, 1);
            path.quadraticCurveTo(4, 3, 2, 2);
            expect(path.getBounds()).toEqual(PathKit.MakeLTRBRect(1, 1, 4, 3));
            expect(path.computeTightBounds()).toEqual(PathKit.MakeLTRBRect(1, 1,
                        PathKit.SkBits2FloatUnsigned(parseInt("0x40333334")),  // 2.8
                        PathKit.SkBits2FloatUnsigned(parseInt("0x40155556")))); // 2.3333333
            path.delete();

            done();
        });
    });

});
