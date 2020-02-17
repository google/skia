describe('CanvasKit\'s Matrix Helpers', function() {

  let expectArrayCloseTo = function(a, b, precision) {
    expect(a.length).toEqual(b.length);
    for (let i=0; i<a.length; i++) {
      expect(a[i]).toBeCloseTo(b[i], precision);
    }
  };

  it('3x3 matrix times its inverse is identity', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      let a = [
        99, 88, -77,
        -66, 55, 44,
        33, 22, 11,
      ];
      expectArrayCloseTo([0,0], [0.0000001, -0.0000001], 3);
      expectArrayCloseTo(
        CanvasKit.SkMatrix.multiply(a, CanvasKit.SkMatrix.invert(a)),
        CanvasKit.SkMatrix.identity(), 9);
      done();
    }));
  });
});