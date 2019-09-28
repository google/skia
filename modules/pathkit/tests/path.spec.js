describe('PathKit\'s Path Behavior', function() {

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
            LoadPathKit.then(catchException(done, () => {
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
            }));
        });

        it('has a copy constructor', function(done) {
            LoadPathKit.then(catchException(done, () => {
                let orig = drawSimplePath();
                let copy = new PathKit.SkPath(orig);

                expect(orig.toSVGString()).toEqual(copy.toSVGString());
                expect(orig.equals(copy)).toBe(true);

                orig.delete();
                copy.delete();
                done();
            }));
        });

        it('has a copy method', function(done) {
            LoadPathKit.then(catchException(done, () => {
                let orig = drawSimplePath();
                let copy = orig.copy();

                expect(orig.toSVGString()).toEqual(copy.toSVGString());
                expect(orig.equals(copy)).toBe(true);

                orig.delete();
                copy.delete();
                done();
            }));
        });

        it('can create a copy with MakePath', function(done) {
            LoadPathKit.then(catchException(done, () => {
                let orig = drawSimplePath();
                let copy = PathKit.NewPath(orig);

                expect(orig.toSVGString()).toEqual(copy.toSVGString());
                expect(orig.equals(copy)).toBe(true);

                orig.delete();
                copy.delete();
                done();
            }));
        });
    });

    function ExpectRectsToBeEqual(actual, expected) {
        if (PathKit.usingWasm) {
            // exact match
            expect(actual).toEqual(expected);
        } else {
            // floats get rounded a little bit
            expect(actual.fLeft).toBeCloseTo(expected.fLeft, 4);
            expect(actual.fTop).toBeCloseTo(expected.fTop, 4);
            expect(actual.fRight).toBeCloseTo(expected.fRight, 4);
            expect(actual.fBottom).toBeCloseTo(expected.fBottom, 4);
        }
    }

    function bits2float(str) {
        return PathKit.SkBits2FloatUnsigned(parseInt(str))
    }

    describe('bounds and rect', function(){
        it('dynamically updates getBounds()', function(done){
            LoadPathKit.then(catchException(done, () => {
                // Based on test_bounds_crbug_513799
                let path = PathKit.NewPath();
                expect(path.getBounds()).toEqual(PathKit.LTRBRect(0, 0, 0, 0));
                path.moveTo(-5, -8);
                expect(path.getBounds()).toEqual(PathKit.LTRBRect(-5, -8, -5, -8));
                path.rect(1, 2, 2, 2);
                expect(path.getBounds()).toEqual(PathKit.LTRBRect(-5, -8, 3, 4));
                path.moveTo(1, 2);
                expect(path.getBounds()).toEqual(PathKit.LTRBRect(-5, -8, 3, 4));
                path.delete();
                done();
            }));
        });

        it('has getBounds() and computeTightBounds()', function(done){
            LoadPathKit.then(catchException(done, () => {
                // Based on PathOpsTightBoundsIllBehaved
                let path = PathKit.NewPath();
                path.moveTo(1, 1);
                path.quadraticCurveTo(4, 3, 2, 2);
                expect(path.getBounds()).toEqual(PathKit.LTRBRect(1, 1, 4, 3));
                ExpectRectsToBeEqual(path.computeTightBounds(),
                                     PathKit.LTRBRect(1, 1,
                                        bits2float("0x40333334"),  // 2.8
                                        bits2float("0x40155556"))); // 2.3333333
                path.delete();

                done();
            }));
        });
    });

    function ExpectCmdsToBeEqual(actual, expected) {
        if (PathKit.usingWasm) {
            // exact match
            expect(actual).toEqual(expected);
        } else {
            // lossy match
            actual.every((cmd, cmdIdx) => {
                cmd.every((arg, argIdx) => {
                    // The asm.js code is close to the wasm/c++ output, but not quite.
                    expect(arg).toBeCloseTo(expected[cmdIdx][argIdx], 4)
                });
            });
        }
    }

    describe('Command arrays', function(){
        it('does NOT approximates conics when dumping as toCmds', function(done) {
            LoadPathKit.then(catchException(done, () => {
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
                ExpectCmdsToBeEqual(path.toCmds(), expectedCmds);

                path.delete();
                done();
            }));
        });
    });

});
