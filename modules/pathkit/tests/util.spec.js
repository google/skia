// Tests for util-related things
describe('PathKit\'s CubicMap Behavior', function() {
    it('computes YFromX correctly', function(done) {
        LoadPathKit.then(catchException(done, () => {
            // Spot check a few points
            const testcases = [
                // input x, expected y
                [0.025391,  0.117627],
                [0.333984,  0.276221],
                [0.662109,  0.366052],
                [0.939453,  0.643296],
            ];
            for (tc of testcases) {
                expect(PathKit.cubicYFromX(0, 0.5, 1.0, 0, tc[0])).toBeCloseTo(tc[1], 5);
            }
            done();
        }));
    });
    it('computes a point from T correctly', function(done) {
        LoadPathKit.then(catchException(done, () => {
            // Spot check a few points
            const testcases = [
                // input t, expected x, expected y
                [0.25, [0.128125, 0.240625]],
                [0.5,  [0.35, 0.35]],
                [0.75, [0.646875, 0.534375]],
                [1.0, [1.0, 1.0]],
            ];
            for (tc of testcases) {
                let ans = PathKit.cubicPtFromT(0.1, 0.5, 0.5, 0.1, tc[0]);
                expect(ans).toBeTruthy();
                expect(ans.length).toBe(2);
                expect(ans[0]).toBeCloseTo(tc[1][0]);
                expect(ans[1]).toBeCloseTo(tc[1][1]);
            }
            done();
        }));
    });

    it('does not leak, with or without cache', function(done) {
        LoadPathKit.then(catchException(done, () => {
            // Run it a lot to make sure we don't leak.
            for (let i = 0; i < 300000; i++) {
                PathKit.cubicYFromX(0.1, 0.5, 0.5, 0.1, 0.1);
                PathKit.cubicPtFromT(0.1, 0.5, 0.5, 0.1, 0.1);
            }
            done();
        }));
    });

});
