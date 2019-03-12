
describe('PathKit\'s Path Behavior', function() {
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
        it('performs dash in-place with start, stop, phase', function(done) {
            LoadPathKit.then(catchException(done, () => {
                let orig = drawStar();
                let dashed = drawStar();
                let notACopy = dashed.dash(10, 3, 0);
                let phased = drawStar().dash(10, 3, 2);

                expect(dashed === notACopy).toBe(true);
                expect(dashed.equals(phased)).toBe(false);
                expect(dashed.equals(orig)).toBe(false);

                reportPath(dashed, 'dashed_no_phase', () => {
                    reportPath(phased, 'dashed_with_phase', done);
                    orig.delete();
                    dashed.delete();
                    phased.delete();
                });
            }));
        });
    });

    describe('Trim Path Effect', function() {
        it('performs trim in-place with start, stop, phase', function(done) {
            LoadPathKit.then(catchException(done, () => {
                let orig = drawStar();
                let trimmed = drawStar();
                let notACopy = trimmed.trim(0.25, .8);
                let complement = drawStar().trim(.1, .9, true);

                expect(trimmed === notACopy).toBe(true);
                expect(trimmed.equals(complement)).toBe(false);
                expect(trimmed.equals(orig)).toBe(false);
                expect(complement.equals(orig)).toBe(false);

                reportPath(trimmed, 'trimmed_non_complement', () => {
                    reportPath(complement, 'trimmed_complement', done);
                    orig.delete();
                    trimmed.delete();
                    complement.delete();
                });
            }));
        });
    });

    describe('Transform Path Effect', function() {
        it('performs matrix transform in-place', function(done) {
            LoadPathKit.then(catchException(done, () => {
                let orig = drawStar();
                let scaled = drawStar();
                let notACopy = scaled.transform(3, 0, 0,
                                                0, 3, 0,
                                                0, 0, 1);

                let scaled2 = drawStar().transform([3, 0, 0,
                                                    0, 3, 0,
                                                    0, 0, 1]);

                expect(scaled === notACopy).toBe(true);
                expect(scaled.equals(scaled2)).toBe(true);
                expect(scaled.equals(orig)).toBe(false);

                reportPath(scaled, 'transformed_scale', () => {
                    reportPath(scaled2, 'transformed_scale2', done);
                    orig.delete();
                    scaled.delete();
                    scaled2.delete();
                });
            }));
        });
    });

    describe('Stroke Path Effect', function() {
        it('creates a stroked path in-place', function(done) {
            LoadPathKit.then(catchException(done, () => {
                let orig = drawStar();
                let stroked = drawStar();
                let notACopy = stroked.stroke({
                    width: 15,
                    join: PathKit.StrokeJoin.BEVEL,
                    cap: PathKit.StrokeCap.BUTT,
                    miter_limit: 2,
                });

                // Don't have to specify all of the fields, defaults will
                // be used instead.
                let rounded = drawStar().stroke({
                    width: 10,
                    join: PathKit.StrokeJoin.ROUND,
                    cap:PathKit.StrokeCap.SQUARE,
                });

                expect(stroked === notACopy).toBe(true);
                expect(stroked.equals(rounded)).toBe(false);
                expect(stroked.equals(orig)).toBe(false);

                reportPath(stroked, 'stroke_bevel_butt', () => {
                    reportPath(rounded, 'stroke_round_square', done);
                    orig.delete();
                    stroked.delete();
                    rounded.delete();
                });
            }));
        });
    });

});
