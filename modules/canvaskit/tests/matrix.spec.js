describe('CanvasKit\'s Matrix Helpers', function() {

  let expectArrayCloseTo = function(a, b) {
    expect(a.length).toEqual(b.length);
    for (let i=0; i<a.length; i++) {
      expect(a[i]).toBeCloseTo(b[i], 9); // 9 digits of precision in base 10
    }
  };

  it('can make a translated 3x3 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      expectArrayCloseTo(
        CanvasKit.SkMatrix.translated(5, -1),
          [1, 0,  5,
           0, 1, -1,
           0, 0,  1]);
      done();
    }));
  });

  it('can make a scaled 3x3 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      expectArrayCloseTo(
        CanvasKit.SkMatrix.scaled(2, 3),
          [2, 0, 0,
           0, 3, 0,
           0, 0, 1]);
      done();
    }));
  });

  it('can make a rotated 3x3 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      expectArrayCloseTo(
        CanvasKit.SkMatrix.rotated(Math.PI, 9, 9),
          [-1,  0, 18,
            0, -1, 18,
            0,  0,  1]);
      done();
    }));
  });

  it('can make a skewed 3x3 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      expectArrayCloseTo(
        CanvasKit.SkMatrix.skewed(4, 3, 2, 1),
          [1, 4, -8,
           3, 1, -3,
           0, 0,  1]);
      done();
    }));
  });

  it('can multiply 3x3 matrices', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      let a = [
         0.1,  0.2,  0.3,
         0.0,  0.6,  0.7,
         0.9, -0.9, -0.8,
      ];
      let b = [
         2.0,  3.0,  4.0,
        -3.0, -4.0, -5.0,
         7.0,  8.0,  9.0,
      ];
      let expected = [
         1.7,  1.9,  2.1,
         3.1,  3.2,  3.3,
        -1.1, -0.1,  0.9,
      ];
      expectArrayCloseTo(
        CanvasKit.SkMatrix.multiply(a, b),
        expected);
      done();
    }));
  });

  it('satisfies the inverse rule for 3x3 matrics', function(done) {
    // a matrix times it's inverse is the identity matrix.
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      let a = [
         0.1,  0.2,  0.3,
         0.0,  0.6,  0.7,
         0.9, -0.9, -0.8,
      ];
      expectArrayCloseTo(
        CanvasKit.SkMatrix.multiply(a, CanvasKit.SkMatrix.invert(a)),
        CanvasKit.SkMatrix.identity());
      done();
    }));
  });

  it('maps 2D points correctly with a 3x3 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      let a = [
         3,  0, -4,
         0,  2, 4,
         0,  0, 1,
      ];
      let points = [
        0, 0,
        1, 1,
      ];
      let expected = [
        -4, 4,
        -1, 6,
      ];
      expectArrayCloseTo(
        CanvasKit.SkMatrix.mapPoints(a, points),
        expected);
      done();
    }));
  });

  it('can make a translated 4x4 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      expectArrayCloseTo(
        CanvasKit.SkM44.translated([5, 6, 7]),
          [1, 0, 0, 5,
           0, 1, 0, 6,
           0, 0, 1, 7,
           0, 0, 0, 1]);
      done();
    }));
  });

  it('can make a scaled 4x4 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      expectArrayCloseTo(
        CanvasKit.SkM44.scaled([5, 6, 7]),
          [5, 0, 0, 0,
           0, 6, 0, 0,
           0, 0, 7, 0,
           0, 0, 0, 1]);
      done();
    }));
  });

  it('can make a rotated 4x4 matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      a = 1/3;
      b = 1/Math.sqrt(3);
      expectArrayCloseTo(
        CanvasKit.SkM44.rotated([1,1,1], Math.PI/2),
          [a,   a-b, a+b, 0,
           a+b, a,   a-b, 0,
           a-b, a+b, a,   0,
           0,   0,   0,   1]);
      done();
    }));
  });

  it('can make a 4x4 matrix looking from eye to center', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      eye = [1, 0, 0];
      center = [1, 0, 1];
      up = [0, 1, 0]
      expectArrayCloseTo(
        CanvasKit.SkM44.lookat(eye, center, up),
          [-1,  0,  0,  1,
            0,  1,  0,  0,
            0,  0, -1,  0,
            0,  0,  0,  1]);
      done();
    }));
  });

  it('can make a 4x4 prespectoive matrix', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      expectArrayCloseTo(
        CanvasKit.SkM44.perspective(2, 10, Math.PI/2),
          [1, 0,   0, 0,
           0, 1,   0, 0,
           0, 0, 1.5, 5,
           0, 0,  -1, 1]);
      done();
    }));
  });

  it('can multiply 4x4 matrices', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      let a = [
         0.1,  0.2,  0.3,  0.4,
         0.0,  0.6,  0.7,  0.8,
         0.9, -0.9, -0.8, -0.7,
        -0.6, -0.5, -0.4, -0.3,
      ];
      let b = [
         2.0,  3.0,  4.0,  5.0,
        -3.0, -4.0, -5.0, -6.0,
         7.0,  8.0,  9.0, 10.0,
        -4.0, -3.0, -2.0, -1.0,
      ];
      let expected = [
         0.1,  0.7,  1.3,  1.9,
        -0.1,  0.8,  1.7,  2.6,
         1.7,  2.0,  2.3,  2.6,
        -1.3, -2.1, -2.9, -3.7,
      ];
      expectArrayCloseTo(
        CanvasKit.SkM44.multiply(a, b),
        expected);
      done();
    }));
  });

  it('satisfies the identity rule for 4x4 matrices', function(done) {
    Promise.all([LoadCanvasKit]).then(catchException(done, () => {
      let a = [
         0.1,  0.2,  0.3,  0.4,
         0.0,  0.6,  0.7,  0.8,
         0.9,  0.9, -0.8, -0.7,
        -0.6, -0.5, -0.4, -0.3,
      ];

      let b = [...a];
      expect(CanvasKit.SkM44.invert(b)).toBeTruthy();
      expectArrayCloseTo(
        CanvasKit.SkM44.multiply(a, b),
        CanvasKit.SkM44.identity());
      done();
    }));
  });

});