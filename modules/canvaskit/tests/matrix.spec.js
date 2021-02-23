describe('CanvasKit\'s Matrix Helpers', () => {

  beforeEach(async () => {
    await LoadCanvasKit;
  });

  const expectArrayCloseTo = (a, b, precision) => {
    precision = precision || 14; // digits of precision in base 10
    expect(a.length).toEqual(b.length);
    for (let i=0; i<a.length; i++) {
      expect(a[i]).toBeCloseTo(b[i], precision);
    }
  };

  describe('3x3 matrices', () => {

    it('can make a translated 3x3 matrix', () => {
      expectArrayCloseTo(
        CanvasKit.Matrix.translated(5, -1),
          [1, 0,  5,
           0, 1, -1,
           0, 0,  1]);
    });

    it('can make a scaled 3x3 matrix', () => {
      expectArrayCloseTo(
        CanvasKit.Matrix.scaled(2, 3),
          [2, 0, 0,
           0, 3, 0,
           0, 0, 1]);
    });

    it('can make a rotated 3x3 matrix', () => {
      expectArrayCloseTo(
        CanvasKit.Matrix.rotated(Math.PI, 9, 9),
          [-1,  0, 18,
            0, -1, 18,
            0,  0,  1]);
    });

    it('can make a skewed 3x3 matrix', () => {
      expectArrayCloseTo(
        CanvasKit.Matrix.skewed(4, 3, 2, 1),
          [1, 4, -8,
           3, 1, -3,
           0, 0,  1]);
    });

    it('can multiply 3x3 matrices', () => {
      const a = [
         0.1,  0.2,  0.3,
         0.0,  0.6,  0.7,
         0.9, -0.9, -0.8,
      ];
      const b = [
         2.0,  3.0,  4.0,
        -3.0, -4.0, -5.0,
         7.0,  8.0,  9.0,
      ];
      const expected = [
         1.7,  1.9,  2.1,
         3.1,  3.2,  3.3,
        -1.1, -0.1,  0.9,
      ];
      expectArrayCloseTo(
        CanvasKit.Matrix.multiply(a, b),
        expected);
    });

    it('satisfies the inverse rule for 3x3 matrics', () => {
      // a matrix times its inverse is the identity matrix.
      const a = [
         0.1,  0.2,  0.3,
         0.0,  0.6,  0.7,
         0.9, -0.9, -0.8,
      ];
      const b = CanvasKit.Matrix.invert(a);
      expectArrayCloseTo(
        CanvasKit.Matrix.multiply(a, b),
        CanvasKit.Matrix.identity());
    });

    it('maps 2D points correctly with a 3x3 matrix', () => {
        const a = [
           3,  0, -4,
           0,  2, 4,
           0,  0, 1,
        ];
        const points = [
          0, 0,
          1, 1,
        ];
        const expected = [
          -4, 4,
          -1, 6,
        ];
        expectArrayCloseTo(
          CanvasKit.Matrix.mapPoints(a, points),
          expected);
    });

  }); // describe 3x3
  describe('4x4 matrices', () => {

    it('can make a translated 4x4 matrix', () => {
      expectArrayCloseTo(
        CanvasKit.M44.translated([5, 6, 7]),
          [1, 0, 0, 5,
           0, 1, 0, 6,
           0, 0, 1, 7,
           0, 0, 0, 1]);
    });

    it('can make a scaled 4x4 matrix', () => {
      expectArrayCloseTo(
        CanvasKit.M44.scaled([5, 6, 7]),
          [5, 0, 0, 0,
           0, 6, 0, 0,
           0, 0, 7, 0,
           0, 0, 0, 1]);
    });

    it('can make a rotated 4x4 matrix', () => {
      expectArrayCloseTo(
        CanvasKit.M44.rotated([1,1,1], Math.PI),
          [-1/3,  2/3,  2/3, 0,
            2/3, -1/3,  2/3, 0,
            2/3,  2/3, -1/3, 0,
              0,    0,    0, 1]);
    });

    it('can make a 4x4 matrix looking from eye to center', () => {
      eye = [1, 0, 0];
      center = [1, 0, 1];
      up = [0, 1, 0]
      expectArrayCloseTo(
        CanvasKit.M44.lookat(eye, center, up),
          [-1,  0,  0,  1,
            0,  1,  0,  0,
            0,  0, -1,  0,
            0,  0,  0,  1]);
    });

    it('can make a 4x4 prespective matrix', () => {
      expectArrayCloseTo(
        CanvasKit.M44.perspective(2, 10, Math.PI/2),
          [1, 0,   0, 0,
           0, 1,   0, 0,
           0, 0, 1.5, 5,
           0, 0,  -1, 1]);
    });

    it('can multiply 4x4 matrices', () => {
      const a = [
         0.1,  0.2,  0.3,  0.4,
         0.0,  0.6,  0.7,  0.8,
         0.9, -0.9, -0.8, -0.7,
        -0.6, -0.5, -0.4, -0.3,
      ];
      const b = [
         2.0,  3.0,  4.0,  5.0,
        -3.0, -4.0, -5.0, -6.0,
         7.0,  8.0,  9.0, 10.0,
        -4.0, -3.0, -2.0, -1.0,
      ];
      const expected = [
         0.1,  0.7,  1.3,  1.9,
        -0.1,  0.8,  1.7,  2.6,
         1.7,  2.0,  2.3,  2.6,
        -1.3, -2.1, -2.9, -3.7,
      ];
      expectArrayCloseTo(
        CanvasKit.M44.multiply(a, b),
        expected);
    });

    it('satisfies the identity rule for 4x4 matrices', () => {
      const a = [
         0.1,  0.2,  0.3,  0.4,
         0.0,  0.6,  0.7,  0.8,
         0.9,  0.9, -0.8, -0.7,
        -0.6, -0.5, -0.4, -0.3,
      ];
      const b = CanvasKit.M44.invert(a)
      expectArrayCloseTo(
        CanvasKit.M44.multiply(a, b),
        CanvasKit.M44.identity());
    });

    it('can create a camera setup matrix', () => {
      const camAngle = Math.PI / 12;
      const cam = {
        'eye'  : [0, 0, 1 / Math.tan(camAngle/2) - 1],
        'coa'  : [0, 0, 0],
        'up'   : [0, 1, 0],
        'near' : 0.02,
        'far'  : 4,
        'angle': camAngle,
      };
      const mat = CanvasKit.M44.setupCamera(CanvasKit.LTRBRect(0, 0, 200, 200), 200, cam);
      // these values came from an invocation of setupCamera visually inspected.
      const expected = [
          7.595754, 0, -0.5, 0,
          0, 7.595754, -0.5, 0,
          0, 0, 1.010050, -1324.368418,
          0, 0, -0.005, 7.595754];
      expectArrayCloseTo(mat, expected, 5);
    });
  }); // describe 4x4
});
