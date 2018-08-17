
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

    describe('Basic Path Features', function() {
        function drawSimplePath() {
            let path = PathKit.NewPath();
            path.moveTo(0, 0);
            path.lineTo(10, 0);
            path.lineTo(10, 10);
            path.close();
            return path;
        }

        it('supports.equals()', function(done) {
            LoadPathKit.then(() => {
                let path = drawSimplePath();
                let otherPath = drawSimplePath();
                let blank = PathKit.NewPath();

                expect(path.equals(path)).toBe(true);
                expect(otherPath.equals(path)).toBe(true);
                expect(path.equals(otherPath)).toBe(true);

                expect(path.equals(blank)).toBe(false);
                expect(otherPath.equals(blank)).toBe(false);
                expect(blank.equals(path)).toBe(false);
                expect(blank.equals(otherPath)).toBe(false);

                path.delete();
                otherPath.delete();
                blank.delete();
                done();
            });
        });

        it('has a copy constructor', function(done) {
            LoadPathKit.then(() => {
                let orig = drawSimplePath();
                let copy = new PathKit.SkPath(orig);

                expect(orig.toSVGString()).toEqual(copy.toSVGString());
                expect(orig.equals(copy)).toBe(true);

                orig.delete();
                copy.delete();
                done();
            });
        });

        it('has a copy method', function(done) {
            LoadPathKit.then(() => {
                let orig = drawSimplePath();
                let copy = orig.copy();

                expect(orig.toSVGString()).toEqual(copy.toSVGString());
                expect(orig.equals(copy)).toBe(true);

                orig.delete();
                copy.delete();
                done();
            });
        });

        it('can create a copy with MakePath', function(done) {
            LoadPathKit.then(() => {
                let orig = drawSimplePath();
                let copy = PathKit.NewPath(orig);

                expect(orig.toSVGString()).toEqual(copy.toSVGString());
                expect(orig.equals(copy)).toBe(true);

                orig.delete();
                copy.delete();
                done();
            });
        });
    });


    function bits2float(str) {
        return PathKit.SkBits2FloatUnsigned(parseInt(str))
    }

    describe('bounds and rect', function(){
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
                            bits2float("0x40333334"),  // 2.8
                            bits2float("0x40155556"))); // 2.3333333
                path.delete();

                done();
            });
        });
    });

    describe('Command arrays', function(){
        it('does NOT approximates conics when dumping as toCmds', function(done){
            LoadPathKit.then(() => {
                let path = PathKit.NewPath();
                path.moveTo(20, 120);
                path.arc(20, 120, 18, 0, 1.75 * Math.PI);
                path.lineTo(20, 120);

                let expectedCmds = [
                    [PathKit.MOVE_VERB, 20, 120],
                    [PathKit.LINE_VERB, 38, 120],
                    [PathKit.CONIC_VERB, 38, 138, 20, 138, bits2float("0x3f3504f3)")], // 0.707107f
                    [PathKit.CONIC_VERB, 2, 138, 2, 120, bits2float("0x3f3504f3)")],   // 0.707107f
                    [PathKit.CONIC_VERB, 2, 102, 20, 102, bits2float("0x3f3504f3)")],  // 0.707107f
                    [PathKit.CONIC_VERB, bits2float("0x41dba58e"), 102, bits2float("0x4202e962"), bits2float("0x42d68b4d"), bits2float("0x3f6c8361")],  // 27.4558, 102, 32.7279, 107.272, 0.92388
                    [PathKit.LINE_VERB, 20, 120],
                ];
                let actual = path.toCmds();
                expect(actual).toEqual(expectedCmds);

                path.delete();
                done();
            });
        });
    });

});
