// Adds JS functions to augment the CanvasKit interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.

// CanvasKit.onRuntimeInitialized is called after the WASM library has loaded.
// Anything that modifies an exposed class (e.g. Path) should be set
// after onRuntimeInitialized, otherwise, it can happen outside of that scope.
CanvasKit.onRuntimeInitialized = function() {
  // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

  _scratchColor = CanvasKit.Malloc(Float32Array, 4); // 4 color scalars.
  _scratchColorPtr = _scratchColor['byteOffset'];

  _scratch4x4Matrix = CanvasKit.Malloc(Float32Array, 16); // 16 matrix scalars.
  _scratch4x4MatrixPtr = _scratch4x4Matrix['byteOffset'];

  _scratch3x3Matrix = CanvasKit.Malloc(Float32Array, 9); // 9 matrix scalars.
  _scratch3x3MatrixPtr = _scratch3x3Matrix['byteOffset'];

  _scratchRRect = CanvasKit.Malloc(Float32Array, 12); // 4 scalars for rrect, 8 for radii.
  _scratchRRectPtr = _scratchRRect['byteOffset'];

  _scratchRRect2 = CanvasKit.Malloc(Float32Array, 12); // 4 scalars for rrect, 8 for radii.
  _scratchRRect2Ptr = _scratchRRect2['byteOffset'];

  _scratchRect = CanvasKit.Malloc(Float32Array, 4);
  _scratchRectPtr = _scratchRect['byteOffset'];

  _scratchRect2 = CanvasKit.Malloc(Float32Array, 4);
  _scratchRect2Ptr = _scratchRect2['byteOffset'];

  _scratchIRect = CanvasKit.Malloc(Int32Array, 4);
  _scratchIRectPtr = _scratchIRect['byteOffset'];

  // Create single copies of all three supported color spaces
  // These are sk_sp<ColorSpace>
  CanvasKit.ColorSpace.SRGB = CanvasKit.ColorSpace._MakeSRGB();
  CanvasKit.ColorSpace.DISPLAY_P3 = CanvasKit.ColorSpace._MakeDisplayP3();
  CanvasKit.ColorSpace.ADOBE_RGB = CanvasKit.ColorSpace._MakeAdobeRGB();

  // Add some helpers for matrices. This is ported from SkMatrix.cpp
  // to save complexity and overhead of going back and forth between
  // C++ and JS layers.
  // I would have liked to use something like DOMMatrix, except it
  // isn't widely supported (would need polyfills) and it doesn't
  // have a mapPoints() function (which could maybe be tacked on here).
  // If DOMMatrix catches on, it would be worth re-considering this usage.
  CanvasKit.Matrix = {};
  function sdot() { // to be called with an even number of scalar args
    var acc = 0;
    for (var i=0; i < arguments.length-1; i+=2) {
      acc += arguments[i] * arguments[i+1];
    }
    return acc;
  }


  // Private general matrix functions used in both 3x3s and 4x4s.
  // Return a square identity matrix of size n.
  var identityN = function(n) {
    var size = n*n;
    var m = new Array(size);
    while(size--) {
      m[size] = size%(n+1) == 0 ? 1.0 : 0.0;
    }
    return m;
  };

  // Stride, a function for compactly representing several ways of copying an array into another.
  // Write vector `v` into matrix `m`. `m` is a matrix encoded as an array in row-major
  // order. Its width is passed as `width`. `v` is an array with length < (m.length/width).
  // An element of `v` is copied into `m` starting at `offset` and moving `colStride` cols right
  // each row.
  //
  // For example, a width of 4, offset of 3, and stride of -1 would put the vector here.
  // _ _ 0 _
  // _ 1 _ _
  // 2 _ _ _
  // _ _ _ 3
  //
  var stride = function(v, m, width, offset, colStride) {
    for (var i=0; i<v.length; i++) {
      m[i * width + // column
        (i * colStride + offset + width) % width // row
      ] = v[i];
    }
    return m;
  };

  CanvasKit.Matrix.identity = function() {
    return identityN(3);
  };

  // Return the inverse (if it exists) of this matrix.
  // Otherwise, return null.
  CanvasKit.Matrix.invert = function(m) {
    // Find the determinant by the sarrus rule. https://en.wikipedia.org/wiki/Rule_of_Sarrus
    var det = m[0]*m[4]*m[8] + m[1]*m[5]*m[6] + m[2]*m[3]*m[7]
            - m[2]*m[4]*m[6] - m[1]*m[3]*m[8] - m[0]*m[5]*m[7];
    if (!det) {
      Debug('Warning, uninvertible matrix');
      return null;
    }
    // Return the inverse by the formula adj(m)/det.
    // adj (adjugate) of a 3x3 is the transpose of it's cofactor matrix.
    // a cofactor matrix is a matrix where each term is +-det(N) where matrix N is the 2x2 formed
    // by removing the row and column we're currently setting from the source.
    // the sign alternates in a checkerboard pattern with a `+` at the top left.
    // that's all been combined here into one expression.
    return [
      (m[4]*m[8] - m[5]*m[7])/det, (m[2]*m[7] - m[1]*m[8])/det, (m[1]*m[5] - m[2]*m[4])/det,
      (m[5]*m[6] - m[3]*m[8])/det, (m[0]*m[8] - m[2]*m[6])/det, (m[2]*m[3] - m[0]*m[5])/det,
      (m[3]*m[7] - m[4]*m[6])/det, (m[1]*m[6] - m[0]*m[7])/det, (m[0]*m[4] - m[1]*m[3])/det,
    ];
  };

  // Maps the given points according to the passed in matrix.
  // Results are done in place.
  // See SkMatrix.h::mapPoints for the docs on the math.
  CanvasKit.Matrix.mapPoints = function(matrix, ptArr) {
    if (IsDebug && (ptArr.length % 2)) {
      throw 'mapPoints requires an even length arr';
    }
    for (var i = 0; i < ptArr.length; i+=2) {
      var x = ptArr[i], y = ptArr[i+1];
      // Gx+Hy+I
      var denom  = matrix[6]*x + matrix[7]*y + matrix[8];
      // Ax+By+C
      var xTrans = matrix[0]*x + matrix[1]*y + matrix[2];
      // Dx+Ey+F
      var yTrans = matrix[3]*x + matrix[4]*y + matrix[5];
      ptArr[i]   = xTrans/denom;
      ptArr[i+1] = yTrans/denom;
    }
    return ptArr;
  };

  function isnumber(val) { return !isNaN(val); }

  // generalized iterative algorithm for multiplying two matrices.
  function multiply(m1, m2, size) {

    if (IsDebug && (!m1.every(isnumber) || !m2.every(isnumber))) {
      throw 'Some members of matrices are NaN m1='+m1+', m2='+m2+'';
    }
    if (IsDebug && (m1.length !== m2.length)) {
      throw 'Undefined for matrices of different sizes. m1.length='+m1.length+', m2.length='+m2.length;
    }
    if (IsDebug && (size*size !== m1.length)) {
      throw 'Undefined for non-square matrices. array size was '+size;
    }

    var result = Array(m1.length);
    for (var r = 0; r < size; r++) {
      for (var c = 0; c < size; c++) {
        // accumulate a sum of m1[r,k]*m2[k, c]
        var acc = 0;
        for (var k = 0; k < size; k++) {
          acc += m1[size * r + k] * m2[size * k + c];
        }
        result[r * size + c] = acc;
      }
    }
    return result;
  }

  // Accept an integer indicating the size of the matrices being multiplied (3 for 3x3), and any
  // number of matrices following it.
  function multiplyMany(size, listOfMatrices) {
    if (IsDebug && (listOfMatrices.length < 2)) {
      throw 'multiplication expected two or more matrices';
    }
    var result = multiply(listOfMatrices[0], listOfMatrices[1], size);
    var next = 2;
    while (next < listOfMatrices.length) {
      result = multiply(result, listOfMatrices[next], size);
      next++;
    }
    return result;
  }

  // Accept any number 3x3 of matrices as arguments, multiply them together.
  // Matrix multiplication is associative but not commutative. the order of the arguments
  // matters, but it does not matter that this implementation multiplies them left to right.
  CanvasKit.Matrix.multiply = function() {
    return multiplyMany(3, arguments);
  };

  // Return a matrix representing a rotation by n radians.
  // px, py optionally say which point the rotation should be around
  // with the default being (0, 0);
  CanvasKit.Matrix.rotated = function(radians, px, py) {
    px = px || 0;
    py = py || 0;
    var sinV = Math.sin(radians);
    var cosV = Math.cos(radians);
    return [
      cosV, -sinV, sdot( sinV, py, 1 - cosV, px),
      sinV,  cosV, sdot(-sinV, px, 1 - cosV, py),
      0,        0,                             1,
    ];
  };

  CanvasKit.Matrix.scaled = function(sx, sy, px, py) {
    px = px || 0;
    py = py || 0;
    var m = stride([sx, sy], identityN(3), 3, 0, 1);
    return stride([px-sx*px, py-sy*py], m, 3, 2, 0);
  };

  CanvasKit.Matrix.skewed = function(kx, ky, px, py) {
    px = px || 0;
    py = py || 0;
    var m = stride([kx, ky], identityN(3), 3, 1, -1);
    return stride([-kx*px, -ky*py], m, 3, 2, 0);
  };

  CanvasKit.Matrix.translated = function(dx, dy) {
    return stride(arguments, identityN(3), 3, 2, 0);
  };

  // Functions for manipulating vectors.
  // Loosely based off of SkV3 in SkM44.h but skia also has SkVec2 and Skv4. This combines them and
  // works on vectors of any length.
  CanvasKit.Vector = {};
  CanvasKit.Vector.dot = function(a, b) {
    if (IsDebug && (a.length !== b.length)) {
      throw 'Cannot perform dot product on arrays of different length ('+a.length+' vs '+b.length+')';
    }
    return a.map(function(v, i) { return v*b[i] }).reduce(function(acc, cur) { return acc + cur; });
  };
  CanvasKit.Vector.lengthSquared = function(v) {
    return CanvasKit.Vector.dot(v, v);
  };
  CanvasKit.Vector.length = function(v) {
    return Math.sqrt(CanvasKit.Vector.lengthSquared(v));
  };
  CanvasKit.Vector.mulScalar = function(v, s) {
    return v.map(function(i) { return i*s });
  };
  CanvasKit.Vector.add = function(a, b) {
    return a.map(function(v, i) { return v+b[i] });
  };
  CanvasKit.Vector.sub = function(a, b) {
    return a.map(function(v, i) { return v-b[i]; });
  };
  CanvasKit.Vector.dist = function(a, b) {
    return CanvasKit.Vector.length(CanvasKit.Vector.sub(a, b));
  };
  CanvasKit.Vector.normalize = function(v) {
    return CanvasKit.Vector.mulScalar(v, 1/CanvasKit.Vector.length(v));
  };
  CanvasKit.Vector.cross = function(a, b) {
    if (IsDebug && (a.length !== 3 || a.length !== 3)) {
      throw 'Cross product is only defined for 3-dimensional vectors (a.length='+a.length+', b.length='+b.length+')';
    }
    return [
      a[1]*b[2] - a[2]*b[1],
      a[2]*b[0] - a[0]*b[2],
      a[0]*b[1] - a[1]*b[0],
    ];
  };

  // Functions for creating and manipulating (row-major) 4x4 matrices. Accepted in place of
  // SkM44 in canvas methods, for the same reasons as the 3x3 matrices above.
  // ported from C++ code in SkM44.cpp
  CanvasKit.M44 = {};
  // Create a 4x4 identity matrix
  CanvasKit.M44.identity = function() {
    return identityN(4);
  };

  // Anything named vec below is an array of length 3 representing a vector/point in 3D space.
  // Create a 4x4 matrix representing a translate by the provided 3-vec
  CanvasKit.M44.translated = function(vec) {
    return stride(vec, identityN(4), 4, 3, 0);
  };
  // Create a 4x4 matrix representing a scaling by the provided 3-vec
  CanvasKit.M44.scaled = function(vec) {
    return stride(vec, identityN(4), 4, 0, 1);
  };
  // Create a 4x4 matrix representing a rotation about the provided axis 3-vec.
  // axis does not need to be normalized.
  CanvasKit.M44.rotated = function(axisVec, radians) {
    return CanvasKit.M44.rotatedUnitSinCos(
      CanvasKit.Vector.normalize(axisVec), Math.sin(radians), Math.cos(radians));
  };
  // Create a 4x4 matrix representing a rotation about the provided normalized axis 3-vec.
  // Rotation is provided redundantly as both sin and cos values.
  // This rotate can be used when you already have the cosAngle and sinAngle values
  // so you don't have to atan(cos/sin) to call roatated() which expects an angle in radians.
  // this does no checking! Behavior for invalid sin or cos values or non-normalized axis vectors
  // is incorrect. Prefer rotated().
  CanvasKit.M44.rotatedUnitSinCos = function(axisVec, sinAngle, cosAngle) {
    var x = axisVec[0];
    var y = axisVec[1];
    var z = axisVec[2];
    var c = cosAngle;
    var s = sinAngle;
    var t = 1 - c;
    return [
      t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
      t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
      t*x*z - s*y, t*y*z + s*x, t*z*z + c,   0,
      0,           0,           0,           1
    ];
  };
  // Create a 4x4 matrix representing a camera at eyeVec, pointed at centerVec.
  CanvasKit.M44.lookat = function(eyeVec, centerVec, upVec) {
    var f = CanvasKit.Vector.normalize(CanvasKit.Vector.sub(centerVec, eyeVec));
    var u = CanvasKit.Vector.normalize(upVec);
    var s = CanvasKit.Vector.normalize(CanvasKit.Vector.cross(f, u));

    var m = CanvasKit.M44.identity();
    // set each column's top three numbers
    stride(s,                                   m, 4, 0, 0);
    stride(CanvasKit.Vector.cross(s, f),      m, 4, 1, 0);
    stride(CanvasKit.Vector.mulScalar(f, -1), m, 4, 2, 0);
    stride(eyeVec,                              m, 4, 3, 0);

    var m2 = CanvasKit.M44.invert(m);
    if (m2 === null) {
      return CanvasKit.M44.identity();
    }
    return m2;
  };
  // Create a 4x4 matrix representing a perspective. All arguments are scalars.
  // angle is in radians.
  CanvasKit.M44.perspective = function(near, far, angle) {
    if (IsDebug && (far <= near)) {
      throw 'far must be greater than near when constructing M44 using perspective.';
    }
    var dInv = 1 / (far - near);
    var halfAngle = angle / 2;
    var cot = Math.cos(halfAngle) / Math.sin(halfAngle);
    return [
      cot, 0,   0,               0,
      0,   cot, 0,               0,
      0,   0,   (far+near)*dInv, 2*far*near*dInv,
      0,   0,   -1,              1,
    ];
  };
  // Returns the number at the given row and column in matrix m.
  CanvasKit.M44.rc = function(m, r, c) {
    return m[r*4+c];
  };
  // Accepts any number of 4x4 matrix arguments, multiplies them left to right.
  CanvasKit.M44.multiply = function() {
    return multiplyMany(4, arguments);
  };

  // Invert the 4x4 matrix if it is invertible and return it. if not, return null.
  // taken from SkM44.cpp (altered to use row-major order)
  // m is not altered.
  CanvasKit.M44.invert = function(m) {
    if (IsDebug && !m.every(isnumber)) {
      throw 'some members of matrix are NaN m='+m;
    }

    var a00 = m[0];
    var a01 = m[4];
    var a02 = m[8];
    var a03 = m[12];
    var a10 = m[1];
    var a11 = m[5];
    var a12 = m[9];
    var a13 = m[13];
    var a20 = m[2];
    var a21 = m[6];
    var a22 = m[10];
    var a23 = m[14];
    var a30 = m[3];
    var a31 = m[7];
    var a32 = m[11];
    var a33 = m[15];

    var b00 = a00 * a11 - a01 * a10;
    var b01 = a00 * a12 - a02 * a10;
    var b02 = a00 * a13 - a03 * a10;
    var b03 = a01 * a12 - a02 * a11;
    var b04 = a01 * a13 - a03 * a11;
    var b05 = a02 * a13 - a03 * a12;
    var b06 = a20 * a31 - a21 * a30;
    var b07 = a20 * a32 - a22 * a30;
    var b08 = a20 * a33 - a23 * a30;
    var b09 = a21 * a32 - a22 * a31;
    var b10 = a21 * a33 - a23 * a31;
    var b11 = a22 * a33 - a23 * a32;

    // calculate determinate
    var det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    var invdet = 1.0 / det;

    // bail out if the matrix is not invertible
    if (det === 0 || invdet === Infinity) {
      Debug('Warning, uninvertible matrix');
      return null;
    }

    b00 *= invdet;
    b01 *= invdet;
    b02 *= invdet;
    b03 *= invdet;
    b04 *= invdet;
    b05 *= invdet;
    b06 *= invdet;
    b07 *= invdet;
    b08 *= invdet;
    b09 *= invdet;
    b10 *= invdet;
    b11 *= invdet;

    // store result in row major order
    var tmp = [
      a11 * b11 - a12 * b10 + a13 * b09,
      a12 * b08 - a10 * b11 - a13 * b07,
      a10 * b10 - a11 * b08 + a13 * b06,
      a11 * b07 - a10 * b09 - a12 * b06,

      a02 * b10 - a01 * b11 - a03 * b09,
      a00 * b11 - a02 * b08 + a03 * b07,
      a01 * b08 - a00 * b10 - a03 * b06,
      a00 * b09 - a01 * b07 + a02 * b06,

      a31 * b05 - a32 * b04 + a33 * b03,
      a32 * b02 - a30 * b05 - a33 * b01,
      a30 * b04 - a31 * b02 + a33 * b00,
      a31 * b01 - a30 * b03 - a32 * b00,

      a22 * b04 - a21 * b05 - a23 * b03,
      a20 * b05 - a22 * b02 + a23 * b01,
      a21 * b02 - a20 * b04 - a23 * b00,
      a20 * b03 - a21 * b01 + a22 * b00,
    ];


    if (!tmp.every(function(val) { return !isNaN(val) && val !== Infinity && val !== -Infinity; })) {
      Debug('inverted matrix contains infinities or NaN '+tmp);
      return null;
    }
    return tmp;
  };

  CanvasKit.M44.transpose = function(m) {
    return [
      m[0], m[4], m[8], m[12],
      m[1], m[5], m[9], m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15],
    ];
  };

  // Return the inverse of an SkM44. throw an error if it's not invertible
  CanvasKit.M44.mustInvert = function(m) {
    var m2 = CanvasKit.M44.invert(m);
    if (m2 === null) {
      throw 'Matrix not invertible';
    }
    return m2;
  };

  // returns a matrix that sets up a 3D perspective view from a given camera.
  //
  // area - a rect describing the viewport. (0, 0, canvas_width, canvas_height) suggested
  // zscale - a scalar describing the scale of the z axis. min(width, height)/2 suggested
  // cam - an object with the following attributes
  // const cam = {
  //   'eye'  : [0, 0, 1 / Math.tan(Math.PI / 24) - 1], // a 3D point locating the camera
  //   'coa'  : [0, 0, 0], // center of attention - the 3D point the camera is looking at.
  //   'up'   : [0, 1, 0], // a unit vector pointing in the camera's up direction, because eye and coa alone leave roll unspecified.
  //   'near' : 0.02,      // near clipping plane
  //   'far'  : 4,         // far clipping plane
  //   'angle': Math.PI / 12, // field of view in radians
  // };
  CanvasKit.M44.setupCamera = function(area, zscale, cam) {
    var camera = CanvasKit.M44.lookat(cam['eye'], cam['coa'], cam['up']);
    var perspective = CanvasKit.M44.perspective(cam['near'], cam['far'], cam['angle']);
    var center = [(area[0] + area[2])/2, (area[1] + area[3])/2, 0];
    var viewScale = [(area[2] - area[0])/2, (area[3] - area[1])/2, zscale];
    var viewport = CanvasKit.M44.multiply(
      CanvasKit.M44.translated(center),
      CanvasKit.M44.scaled(viewScale));
    return CanvasKit.M44.multiply(
      viewport, perspective, camera, CanvasKit.M44.mustInvert(viewport));
  };

  // An ColorMatrix is a 4x4 color matrix that transforms the 4 color channels
  //  with a 1x4 matrix that post-translates those 4 channels.
  // For example, the following is the layout with the scale (S) and post-transform
  // (PT) items indicated.
  // RS,  0,  0,  0 | RPT
  //  0, GS,  0,  0 | GPT
  //  0,  0, BS,  0 | BPT
  //  0,  0,  0, AS | APT
  //
  // Much of this was hand-transcribed from SkColorMatrix.cpp, because it's easier to
  // deal with a Float32Array of length 20 than to try to expose the SkColorMatrix object.

  var rScale = 0;
  var gScale = 6;
  var bScale = 12;
  var aScale = 18;

  var rPostTrans = 4;
  var gPostTrans = 9;
  var bPostTrans = 14;
  var aPostTrans = 19;

  CanvasKit.ColorMatrix = {};
  CanvasKit.ColorMatrix.identity = function() {
    var m = new Float32Array(20);
    m[rScale] = 1;
    m[gScale] = 1;
    m[bScale] = 1;
    m[aScale] = 1;
    return m;
  };

  CanvasKit.ColorMatrix.scaled = function(rs, gs, bs, as) {
    var m = new Float32Array(20);
    m[rScale] = rs;
    m[gScale] = gs;
    m[bScale] = bs;
    m[aScale] = as;
    return m;
  };

  var rotateIndices = [
    [6, 7, 11, 12],
    [0, 10, 2, 12],
    [0, 1,  5,  6],
  ];
  // axis should be 0, 1, 2 for r, g, b
  CanvasKit.ColorMatrix.rotated = function(axis, sine, cosine) {
    var m = CanvasKit.ColorMatrix.identity();
    var indices = rotateIndices[axis];
    m[indices[0]] = cosine;
    m[indices[1]] = sine;
    m[indices[2]] = -sine;
    m[indices[3]] = cosine;
    return m;
  };

  // m is a ColorMatrix (i.e. a Float32Array), and this sets the 4 "special"
  // params that will translate the colors after they are multiplied by the 4x4 matrix.
  CanvasKit.ColorMatrix.postTranslate = function(m, dr, dg, db, da) {
    m[rPostTrans] += dr;
    m[gPostTrans] += dg;
    m[bPostTrans] += db;
    m[aPostTrans] += da;
    return m;
  };

  // concat returns a new ColorMatrix that is the result of multiplying outer*inner
  CanvasKit.ColorMatrix.concat = function(outer, inner) {
    var m = new Float32Array(20);
    var index = 0;
    for (var j = 0; j < 20; j += 5) {
        for (var i = 0; i < 4; i++) {
            m[index++] =  outer[j + 0] * inner[i + 0] +
                          outer[j + 1] * inner[i + 5] +
                          outer[j + 2] * inner[i + 10] +
                          outer[j + 3] * inner[i + 15];
        }
        m[index++] =  outer[j + 0] * inner[4] +
                      outer[j + 1] * inner[9] +
                      outer[j + 2] * inner[14] +
                      outer[j + 3] * inner[19] +
                      outer[j + 4];
    }

    return m;
  };

  CanvasKit.Path.MakeFromCmds = function(cmds) {
    var ptrLen = loadCmdsTypedArray(cmds);
    var path = CanvasKit.Path._MakeFromCmds(ptrLen[0], ptrLen[1]);
    CanvasKit._free(ptrLen[0]);
    return path;
  };

  // The weights array is optional (only used for conics).
  CanvasKit.Path.MakeFromVerbsPointsWeights = function(verbs, pts, weights) {
    var verbsPtr = copy1dArray(verbs, 'HEAPU8');
    var pointsPtr = copy1dArray(pts, 'HEAPF32');
    var weightsPtr = copy1dArray(weights, 'HEAPF32');
    var numWeights = (weights && weights.length) || 0;
    var path = CanvasKit.Path._MakeFromVerbsPointsWeights(
        verbsPtr, verbs.length, pointsPtr, pts.length, weightsPtr, numWeights);
    freeArraysThatAreNotMallocedByUsers(verbsPtr, verbs);
    freeArraysThatAreNotMallocedByUsers(pointsPtr, pts);
    freeArraysThatAreNotMallocedByUsers(weightsPtr, weights);
    return path;
  };

  CanvasKit.Path.prototype.addArc = function(oval, startAngle, sweepAngle) {
    // see arc() for the HTMLCanvas version
    // note input angles are degrees.
    var oPtr = copyRectToWasm(oval);
    this._addArc(oPtr, startAngle, sweepAngle);
    return this;
  };

  CanvasKit.Path.prototype.addOval = function(oval, isCCW, startIndex) {
    if (startIndex === undefined) {
      startIndex = 1;
    }
    var oPtr = copyRectToWasm(oval);
    this._addOval(oPtr, !!isCCW, startIndex);
    return this;
  };

  // TODO(kjlubick) clean up this API - split it apart if necessary
  CanvasKit.Path.prototype.addPath = function() {
    // Takes 1, 2, 7, or 10 required args, where the first arg is always the path.
    // The last arg is optional and chooses between add or extend mode.
    // The options for the remaining args are:
    //   - an array of 6 or 9 parameters (perspective is optional)
    //   - the 9 parameters of a full matrix or
    //     the 6 non-perspective params of a matrix.
    var args = Array.prototype.slice.call(arguments);
    var path = args[0];
    var extend = false;
    if (typeof args[args.length-1] === 'boolean') {
      extend = args.pop();
    }
    if (args.length === 1) {
      // Add path, unchanged.  Use identity matrix
      this._addPath(path, 1, 0, 0,
                          0, 1, 0,
                          0, 0, 1,
                          extend);
    } else if (args.length === 2) {
      // User provided the 9 params of a full matrix as an array.
      var a = args[1];
      this._addPath(path, a[0],      a[1],      a[2],
                          a[3],      a[4],      a[5],
                          a[6] || 0, a[7] || 0, a[8] || 1,
                          extend);
    } else if (args.length === 7 || args.length === 10) {
      // User provided the 9 params of a (full) matrix directly.
      // (or just the 6 non perspective ones)
      // These are in the same order as what Skia expects.
      var a = args;
      this._addPath(path, a[1],      a[2],      a[3],
                          a[4],      a[5],      a[6],
                          a[7] || 0, a[8] || 0, a[9] || 1,
                          extend);
    } else {
      Debug('addPath expected to take 1, 2, 7, or 10 required args. Got ' + args.length);
      return null;
    }
    return this;
  };

  // points is a 1d array of length 2n representing n points where the even indices
  // will be treated as x coordinates and the odd indices will be treated as y coordinates.
  // Like other APIs, this accepts a malloced type array or malloc obj.
  CanvasKit.Path.prototype.addPoly = function(points, close) {
    var ptr = copy1dArray(points, 'HEAPF32');
    this._addPoly(ptr, points.length / 2, close);
    freeArraysThatAreNotMallocedByUsers(ptr, points);
    return this;
  };

  CanvasKit.Path.prototype.addRect = function(rect, isCCW) {
    var rPtr = copyRectToWasm(rect);
    this._addRect(rPtr, !!isCCW);
    return this;
  };

  CanvasKit.Path.prototype.addRRect = function(rrect, isCCW) {
    var rPtr = copyRRectToWasm(rrect);
    this._addRRect(rPtr, !!isCCW);
    return this;
  };

  // The weights array is optional (only used for conics).
  CanvasKit.Path.prototype.addVerbsPointsWeights = function(verbs, points, weights) {
    var verbsPtr = copy1dArray(verbs, 'HEAPU8');
    var pointsPtr = copy1dArray(points, 'HEAPF32');
    var weightsPtr = copy1dArray(weights, 'HEAPF32');
    var numWeights = (weights && weights.length) || 0;
    this._addVerbsPointsWeights(verbsPtr, verbs.length, pointsPtr, points.length,
                                weightsPtr, numWeights);
    freeArraysThatAreNotMallocedByUsers(verbsPtr, verbs);
    freeArraysThatAreNotMallocedByUsers(pointsPtr, points);
    freeArraysThatAreNotMallocedByUsers(weightsPtr, weights);
  };

  CanvasKit.Path.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    // emulates the HTMLCanvas behavior.  See addArc() for the Path version.
    // Note input angles are radians.
    var bounds = CanvasKit.LTRBRect(x-radius, y-radius, x+radius, y+radius);
    var sweep = radiansToDegrees(endAngle - startAngle) - (360 * !!ccw);
    var temp = new CanvasKit.Path();
    temp.addArc(bounds, radiansToDegrees(startAngle), sweep);
    this.addPath(temp, true);
    temp.delete();
    return this;
  };

  // Appends arc to Path. Arc added is part of ellipse
  // bounded by oval, from startAngle through sweepAngle. Both startAngle and
  // sweepAngle are measured in degrees, where zero degrees is aligned with the
  // positive x-axis, and positive sweeps extends arc clockwise.
  CanvasKit.Path.prototype.arcToOval = function(oval, startAngle, sweepAngle, forceMoveTo) {
    var oPtr = copyRectToWasm(oval);
    this._arcToOval(oPtr, startAngle, sweepAngle, forceMoveTo);
    return this;
  };

  // Appends arc to Path. Arc is implemented by one or more conics weighted to
  // describe part of oval with radii (rx, ry) rotated by xAxisRotate degrees. Arc
  // curves from last point to (x, y), choosing one of four possible routes:
  // clockwise or counterclockwise, and smaller or larger.

  // Arc sweep is always less than 360 degrees. arcTo() appends line to (x, y) if
  // either radii are zero, or if last point equals (x, y). arcTo() scales radii
  // (rx, ry) to fit last point and (x, y) if both are greater than zero but
  // too small.

  // arcToRotated() appends up to four conic curves.
  // arcToRotated() implements the functionality of SVG arc, although SVG sweep-flag value
  // is opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise,
  // while kCW_Direction cast to int is zero.
  CanvasKit.Path.prototype.arcToRotated = function(rx, ry, xAxisRotate, useSmallArc, isCCW, x, y) {
    this._arcToRotated(rx, ry, xAxisRotate, !!useSmallArc, !!isCCW, x, y);
    return this;
  };

  // Appends arc to Path, after appending line if needed. Arc is implemented by conic
  // weighted to describe part of circle. Arc is contained by tangent from
  // last Path point to (x1, y1), and tangent from (x1, y1) to (x2, y2). Arc
  // is part of circle sized to radius, positioned so it touches both tangent lines.

  // If last Path Point does not start Arc, arcTo appends connecting Line to Path.
  // The length of Vector from (x1, y1) to (x2, y2) does not affect Arc.

  // Arc sweep is always less than 180 degrees. If radius is zero, or if
  // tangents are nearly parallel, arcTo appends Line from last Path Point to (x1, y1).

  // arcToTangent appends at most one Line and one conic.
  // arcToTangent implements the functionality of PostScript arct and HTML Canvas arcTo.
  CanvasKit.Path.prototype.arcToTangent = function(x1, y1, x2, y2, radius) {
    this._arcToTangent(x1, y1, x2, y2, radius);
    return this;
  };

  CanvasKit.Path.prototype.close = function() {
    this._close();
    return this;
  };

  CanvasKit.Path.prototype.conicTo = function(x1, y1, x2, y2, w) {
    this._conicTo(x1, y1, x2, y2, w);
    return this;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Path.prototype.computeTightBounds = function(optionalOutputArray) {
    this._computeTightBounds(_scratchRectPtr);
    var ta = _scratchRect['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  CanvasKit.Path.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.Path.prototype.dash = function(on, off, phase) {
    if (this._dash(on, off, phase)) {
      return this;
    }
    return null;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Path.prototype.getBounds = function(optionalOutputArray) {
    this._getBounds(_scratchRectPtr);
    var ta = _scratchRect['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  CanvasKit.Path.prototype.lineTo = function(x, y) {
    this._lineTo(x, y);
    return this;
  };

  CanvasKit.Path.prototype.moveTo = function(x, y) {
    this._moveTo(x, y);
    return this;
  };

  CanvasKit.Path.prototype.offset = function(dx, dy) {
    this._transform(1, 0, dx,
                    0, 1, dy,
                    0, 0, 1);
    return this;
  };

  CanvasKit.Path.prototype.quadTo = function(cpx, cpy, x, y) {
    this._quadTo(cpx, cpy, x, y);
    return this;
  };

 CanvasKit.Path.prototype.rArcTo = function(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy) {
    this._rArcTo(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy);
    return this;
  };

  CanvasKit.Path.prototype.rConicTo = function(dx1, dy1, dx2, dy2, w) {
    this._rConicTo(dx1, dy1, dx2, dy2, w);
    return this;
  };

  // These params are all relative
  CanvasKit.Path.prototype.rCubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._rCubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.Path.prototype.rLineTo = function(dx, dy) {
    this._rLineTo(dx, dy);
    return this;
  };

  CanvasKit.Path.prototype.rMoveTo = function(dx, dy) {
    this._rMoveTo(dx, dy);
    return this;
  };

  // These params are all relative
  CanvasKit.Path.prototype.rQuadTo = function(cpx, cpy, x, y) {
    this._rQuadTo(cpx, cpy, x, y);
    return this;
  };

  CanvasKit.Path.prototype.stroke = function(opts) {
    // Fill out any missing values with the default values.
    opts = opts || {};
    opts['width'] = opts['width'] || 1;
    opts['miter_limit'] = opts['miter_limit'] || 4;
    opts['cap'] = opts['cap'] || CanvasKit.StrokeCap.Butt;
    opts['join'] = opts['join'] || CanvasKit.StrokeJoin.Miter;
    opts['precision'] = opts['precision'] || 1;
    if (this._stroke(opts)) {
      return this;
    }
    return null;
  };

  // TODO(kjlubick) Change this to take a 3x3 or 4x4 matrix (optionally malloc'd)
  CanvasKit.Path.prototype.transform = function() {
    // Takes 1 or 9 args
    if (arguments.length === 1) {
      // argument 1 should be a 6 or 9 element array.
      var a = arguments[0];
      this._transform(a[0], a[1], a[2],
                      a[3], a[4], a[5],
                      a[6] || 0, a[7] || 0, a[8] || 1);
    } else if (arguments.length === 6 || arguments.length === 9) {
      // these arguments are the 6 or 9 members of the matrix
      var a = arguments;
      this._transform(a[0], a[1], a[2],
                      a[3], a[4], a[5],
                      a[6] || 0, a[7] || 0, a[8] || 1);
    } else {
      throw 'transform expected to take 1 or 9 arguments. Got ' + arguments.length;
    }
    return this;
  };
  // isComplement is optional, defaults to false
  CanvasKit.Path.prototype.trim = function(startT, stopT, isComplement) {
    if (this._trim(startT, stopT, !!isComplement)) {
      return this;
    }
    return null;
  };

  CanvasKit.Image.prototype.encodeToData = function() {
    if (!arguments.length) {
      return this._encodeToData();
    }

    if (arguments.length === 2) {
      var a = arguments;
      return this._encodeToDataWithFormat(a[0], a[1]);
    }

    throw 'encodeToData expected to take 0 or 2 arguments. Got ' + arguments.length;
  };

  // makeShaderCubic returns a shader for a given image, allowing it to be used on
  // a paint as well as other purposes. This shader will be higher quality than
  // other shader functions. See CubicResampler in SkSamplingOptions.h for more information
  // on the cubicResampler params.
  CanvasKit.Image.prototype.makeShaderCubic = function(xTileMode, yTileMode,
                                                       cubicResamplerB, cubicResamplerC,
                                                       localMatrix) {
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    return this._makeShaderCubic(xTileMode, yTileMode, cubicResamplerB,
                                 cubicResamplerC, localMatrixPtr);
  };

  // makeShaderCubic returns a shader for a given image, allowing it to be used on
  // a paint as well as other purposes. This shader will draw more quickly than
  // other shader functions, but at a lower quality.
  CanvasKit.Image.prototype.makeShaderOptions = function(xTileMode, yTileMode,
                                                         filterMode, mipmapMode,
                                                         localMatrix) {
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    return this._makeShaderOptions(xTileMode, yTileMode, filterMode, mipmapMode, localMatrixPtr);
  };

  function readPixels(source, srcX, srcY, imageInfo, destMallocObj, bytesPerRow) {
    if (!bytesPerRow) {
      bytesPerRow = 4 * imageInfo['width'];
      if (imageInfo['colorType'] === CanvasKit.ColorType.RGBA_F16) {
        bytesPerRow *= 2;
      }
      else if (imageInfo['colorType'] === CanvasKit.ColorType.RGBA_F32) {
        bytesPerRow *= 4;
      }
    }
    var pBytes = bytesPerRow * imageInfo.height;
    var pPtr;
    if (destMallocObj) {
      pPtr = destMallocObj['byteOffset'];
    } else {
      pPtr = CanvasKit._malloc(pBytes);
    }

    if (!source._readPixels(imageInfo, pPtr, bytesPerRow, srcX, srcY)) {
      Debug('Could not read pixels with the given inputs');
      if (!destMallocObj) {
        CanvasKit._free(pPtr);
      }
      return null;
    }

    // If the user provided us a buffer to copy into, we don't need to allocate a new TypedArray.
    if (destMallocObj) {
      return destMallocObj['toTypedArray'](); // Return the typed array wrapper w/o allocating.
    }

    // Put those pixels into a typed array of the right format and then
    // make a copy with slice() that we can return.
    var retVal = null;
    switch (imageInfo['colorType']) {
      case CanvasKit.ColorType.RGBA_8888:
      case CanvasKit.ColorType.RGBA_F16: // there is no half-float JS type, so we return raw bytes.
        retVal = new Uint8Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
      case CanvasKit.ColorType.RGBA_F32:
        retVal = new Float32Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
      default:
        Debug('ColorType not yet supported');
        return null;
    }

    // Free the allocated pixels in the WASM memory
    CanvasKit._free(pPtr);
    return retVal;
  }

  CanvasKit.Image.prototype.readPixels = function(srcX, srcY, imageInfo, destMallocObj,
                                                  bytesPerRow) {
    return readPixels(this, srcX, srcY, imageInfo, destMallocObj, bytesPerRow);
  };

  // Accepts an array of four numbers in the range of 0-1 representing a 4f color
  CanvasKit.Canvas.prototype.clear = function(color4f) {
    var cPtr = copyColorToWasm(color4f);
    this._clear(cPtr);
  };

  CanvasKit.Canvas.prototype.clipRRect = function(rrect, op, antialias) {
    var rPtr = copyRRectToWasm(rrect);
    this._clipRRect(rPtr, op, antialias);
  };

  CanvasKit.Canvas.prototype.clipRect = function(rect, op, antialias) {
    var rPtr = copyRectToWasm(rect);
    this._clipRect(rPtr, op, antialias);
  };

  // concat takes a 3x2, a 3x3, or a 4x4 matrix and upscales it (if needed) to 4x4. This is because
  // under the hood, SkCanvas uses a 4x4 matrix.
  CanvasKit.Canvas.prototype.concat = function(matr) {
    var matrPtr = copy4x4MatrixToWasm(matr);
    this._concat(matrPtr);
  };

  CanvasKit.Canvas.prototype.drawArc = function(oval, startAngle, sweepAngle, useCenter, paint) {
    var oPtr = copyRectToWasm(oval);
    this._drawArc(oPtr, startAngle, sweepAngle, useCenter, paint);
  };

  // atlas is an Image, e.g. from CanvasKit.MakeImageFromEncoded
  // srcRects, dstXforms, and colors should be CanvasKit.RectBuilder, CanvasKit.RSXFormBuilder,
  // and CanvasKit.ColorBuilder (fastest)
  // Or they can be an array of floats of length 4*number of destinations.
  // colors are optional and used to tint the drawn images using the optional blend mode
  // Colors may be an ColorBuilder, a Uint32Array of int colors,
  // a Flat Float32Array of float colors or a 2d Array of Float32Array(4) (deprecated)
  // TODO(kjlubick) remove Builders - no longer needed now that Malloc is a thing.
  CanvasKit.Canvas.prototype.drawAtlas = function(atlas, srcRects, dstXforms, paint,
                                       /*optional*/ blendMode, colors) {
    if (!atlas || !paint || !srcRects || !dstXforms) {
      Debug('Doing nothing since missing a required input');
      return;
    }

    // builder arguments report the length as the number of rects, but when passed as arrays
    // their.length attribute is 4x higher because it's the number of total components of all rects.
    // colors is always going to report the same length, at least until floats colors are supported
    // by this function.
    if (srcRects.length !== dstXforms.length) {
      Debug('Doing nothing since input arrays length mismatches');
      return;
    }
    if (!blendMode) {
      blendMode = CanvasKit.BlendMode.SrcOver;
    }

    var srcRectPtr;
    if (srcRects.build) {
      srcRectPtr = srcRects.build();
    } else {
      srcRectPtr = copy1dArray(srcRects, 'HEAPF32');
    }

    var count = 1;
    var dstXformPtr;
    if (dstXforms.build) {
      dstXformPtr = dstXforms.build();
      count = dstXforms.length;
    } else {
      dstXformPtr = copy1dArray(dstXforms, 'HEAPF32');
      count = dstXforms.length / 4;
    }

    var colorPtr = nullptr;
    if (colors) {
      if (colors.build) {
        colorPtr = colors.build();
      } else {
        colorPtr = copy1dArray(assureIntColors(colors), 'HEAPU32');
      }
    }

    this._drawAtlas(atlas, dstXformPtr, srcRectPtr, colorPtr, count, blendMode, paint);

    if (srcRectPtr && !srcRects.build) {
      freeArraysThatAreNotMallocedByUsers(srcRectPtr, srcRects);
    }
    if (dstXformPtr && !dstXforms.build) {
      freeArraysThatAreNotMallocedByUsers(dstXformPtr, dstXforms);
    }
    if (colorPtr && !colors.build) {
      freeArraysThatAreNotMallocedByUsers(colorPtr, colors);
    }
  };

  CanvasKit.Canvas.prototype.drawColor = function (color4f, mode) {
    var cPtr = copyColorToWasm(color4f);
    if (mode !== undefined) {
      this._drawColor(cPtr, mode);
    } else {
      this._drawColor(cPtr);
    }
  };

  CanvasKit.Canvas.prototype.drawColorComponents = function (r, g, b, a, mode) {
    var cPtr = copyColorComponentsToWasm(r, g, b, a);
    if (mode !== undefined) {
      this._drawColor(cPtr, mode);
    } else {
      this._drawColor(cPtr);
    }
  };

  CanvasKit.Canvas.prototype.drawDRRect = function(outer, inner, paint) {
    var oPtr = copyRRectToWasm(outer, _scratchRRectPtr);
    var iPtr = copyRRectToWasm(inner, _scratchRRect2Ptr);
    this._drawDRRect(oPtr, iPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawImageNine = function(img, center, dest, filter, paint) {
    var cPtr = copyIRectToWasm(center);
    var dPtr = copyRectToWasm(dest);
    this._drawImageNine(img, cPtr, dPtr, filter, paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageRect = function(img, src, dest, paint, fastSample) {
    var sPtr = copyRectToWasm(src,  _scratchRectPtr);
    var dPtr = copyRectToWasm(dest, _scratchRect2Ptr);
    this._drawImageRect(img, sPtr, dPtr, paint, !!fastSample);
  };

  CanvasKit.Canvas.prototype.drawImageRectCubic = function(img, src, dest, B, C, paint) {
    var sPtr = copyRectToWasm(src,  _scratchRectPtr);
    var dPtr = copyRectToWasm(dest, _scratchRect2Ptr);
    this._drawImageRectCubic(img, sPtr, dPtr, B, C, paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageRectOptions = function(img, src, dest, filter, mipmap, paint) {
    var sPtr = copyRectToWasm(src,  _scratchRectPtr);
    var dPtr = copyRectToWasm(dest, _scratchRect2Ptr);
    this._drawImageRectOptions(img, sPtr, dPtr, filter, mipmap, paint || null);
  };

  CanvasKit.Canvas.prototype.drawOval = function(oval, paint) {
    var oPtr = copyRectToWasm(oval);
    this._drawOval(oPtr, paint);
  };

  // points is a 1d array of length 2n representing n points where the even indices
  // will be treated as x coordinates and the odd indices will be treated as y coordinates.
  // Like other APIs, this accepts a malloced type array or malloc obj.
  CanvasKit.Canvas.prototype.drawPoints = function(mode, points, paint) {
    var ptr = copy1dArray(points, 'HEAPF32');
    this._drawPoints(mode, ptr, points.length / 2, paint);
    freeArraysThatAreNotMallocedByUsers(ptr, points);
  };

  CanvasKit.Canvas.prototype.drawRRect = function(rrect, paint) {
    var rPtr = copyRRectToWasm(rrect);
    this._drawRRect(rPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawRect = function(rect, paint) {
    var rPtr = copyRectToWasm(rect);
    this._drawRect(rPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawShadow = function(path, zPlaneParams, lightPos, lightRadius, ambientColor, spotColor, flags) {
    var ambiPtr = copyColorToWasmNoScratch(ambientColor);
    var spotPtr = copyColorToWasmNoScratch(spotColor);
    this._drawShadow(path, zPlaneParams, lightPos, lightRadius, ambiPtr, spotPtr, flags);
    freeArraysThatAreNotMallocedByUsers(ambiPtr, ambientColor);
    freeArraysThatAreNotMallocedByUsers(spotPtr, spotColor);
  };

  // getLocalToDevice returns a 4x4 matrix.
  CanvasKit.Canvas.prototype.getLocalToDevice = function() {
    // _getLocalToDevice will copy the values into the pointer.
    this._getLocalToDevice(_scratch4x4MatrixPtr);
    return copy4x4MatrixFromWasm(_scratch4x4MatrixPtr);
  };

  // findMarkedCTM returns a 4x4 matrix, or null if a matrix was not found at
  // the provided marker.
  CanvasKit.Canvas.prototype.findMarkedCTM = function(marker) {
    // _getLocalToDevice will copy the values into the pointer.
    var found = this._findMarkedCTM(marker, _scratch4x4MatrixPtr);
    if (!found) {
      return null;
    }
    return copy4x4MatrixFromWasm(_scratch4x4MatrixPtr);
  };

  // getTotalMatrix returns the current matrix as a 3x3 matrix.
  CanvasKit.Canvas.prototype.getTotalMatrix = function() {
    // _getTotalMatrix will copy the values into the pointer.
    this._getTotalMatrix(_scratch3x3MatrixPtr);
    // read them out into an array. TODO(kjlubick): If we change Matrix to be
    // typedArrays, then we should return a typed array here too.
    var rv = new Array(9);
    for (var i = 0; i < 9; i++) {
      rv[i] = CanvasKit.HEAPF32[_scratch3x3MatrixPtr/4 + i]; // divide by 4 to "cast" to float.
    }
    return rv;
  };

  CanvasKit.Canvas.prototype.readPixels = function(srcX, srcY, imageInfo, destMallocObj,
                                                   bytesPerRow) {
    return readPixels(this, srcX, srcY, imageInfo, destMallocObj, bytesPerRow);
  };

  CanvasKit.Canvas.prototype.saveLayer = function(paint, boundsRect, backdrop, flags) {
    // bPtr will be 0 (nullptr) if boundsRect is undefined/null.
    var bPtr = copyRectToWasm(boundsRect);
    // These or clauses help emscripten, which does not deal with undefined well.
    return this._saveLayer(paint || null, bPtr, backdrop || null, flags || 0);
  };

  // pixels should be a Uint8Array or a plain JS array.
  CanvasKit.Canvas.prototype.writePixels = function(pixels, srcWidth, srcHeight,
                                                      destX, destY, alphaType, colorType, colorSpace) {
    if (pixels.byteLength % (srcWidth * srcHeight)) {
      throw 'pixels length must be a multiple of the srcWidth * srcHeight';
    }
    var bytesPerPixel = pixels.byteLength / (srcWidth * srcHeight);
    // supply defaults (which are compatible with HTMLCanvas's putImageData)
    alphaType = alphaType || CanvasKit.AlphaType.Unpremul;
    colorType = colorType || CanvasKit.ColorType.RGBA_8888;
    colorSpace = colorSpace || CanvasKit.ColorSpace.SRGB;
    var srcRowBytes = bytesPerPixel * srcWidth;

    var pptr = copy1dArray(pixels, 'HEAPU8');
    var ok = this._writePixels({
      'width': srcWidth,
      'height': srcHeight,
      'colorType': colorType,
      'alphaType': alphaType,
      'colorSpace': colorSpace,
    }, pptr, srcRowBytes, destX, destY);

    freeArraysThatAreNotMallocedByUsers(pptr, pixels);
    return ok;
  };

  CanvasKit.ColorFilter.MakeBlend = function(color4f, mode) {
    var cPtr = copyColorToWasm(color4f);
    var result = CanvasKit.ColorFilter._MakeBlend(cPtr, mode);
    return result;
  };

  // colorMatrix is an ColorMatrix (e.g. Float32Array of length 20)
  CanvasKit.ColorFilter.MakeMatrix = function(colorMatrix) {
    if (!colorMatrix || colorMatrix.length !== 20) {
      throw 'invalid color matrix';
    }
    var fptr = copy1dArray(colorMatrix, 'HEAPF32');
    // We know skia memcopies the floats, so we can free our memory after the call returns.
    var m = CanvasKit.ColorFilter._makeMatrix(fptr);
    freeArraysThatAreNotMallocedByUsers(fptr, colorMatrix);
    return m;
  };

  CanvasKit.ImageFilter.MakeMatrixTransform = function(matr, filterQuality, input) {
    var matrPtr = copy3x3MatrixToWasm(matr);
    return CanvasKit.ImageFilter._MakeMatrixTransform(matrPtr, filterQuality, input);
  };

  CanvasKit.Paint.prototype.getColor = function() {
    this._getColor(_scratchColorPtr);
    return copyColorFromWasm(_scratchColorPtr);
  };

  CanvasKit.Paint.prototype.setColor = function(color4f, colorSpace) {
    colorSpace = colorSpace || null; // null will be replaced with sRGB in the C++ method.
    // emscripten wouldn't bind undefined to the sk_sp<ColorSpace> expected here.
    var cPtr = copyColorToWasm(color4f);
    this._setColor(cPtr, colorSpace);
  };

  // The color components here are expected to be floating point values (nominally between
  // 0.0 and 1.0, but with wider color gamuts, the values could exceed this range). To convert
  // between standard 8 bit colors and floats, just divide by 255 before passing them in.
  CanvasKit.Paint.prototype.setColorComponents = function(r, g, b, a, colorSpace) {
    colorSpace = colorSpace || null; // null will be replaced with sRGB in the C++ method.
    // emscripten wouldn't bind undefined to the sk_sp<ColorSpace> expected here.
    var cPtr = copyColorComponentsToWasm(r, g, b, a);
    this._setColor(cPtr, colorSpace);
  };

  CanvasKit.PictureRecorder.prototype.beginRecording = function(bounds) {
    var bPtr = copyRectToWasm(bounds);
    return this._beginRecording(bPtr);
  };

  CanvasKit.Surface.prototype.makeImageSnapshot = function(optionalBoundsRect) {
    var bPtr = copyIRectToWasm(optionalBoundsRect);
    return this._makeImageSnapshot(bPtr);
  };

  CanvasKit.Surface.prototype.requestAnimationFrame = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    requestAnimationFrame(function() {
      if (this._context !== undefined) {
        CanvasKit.setCurrentContext(this._context);
      }

      callback(this._cached_canvas);

      // We do not dispose() of the Surface here, as the client will typically
      // call requestAnimationFrame again from within the supplied callback.
      // For drawing a single frame, prefer drawOnce().
      this.flush(dirtyRect);
    }.bind(this));
  };

  // drawOnce will dispose of the surface after drawing the frame using the provided
  // callback.
  CanvasKit.Surface.prototype.drawOnce = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    requestAnimationFrame(function() {
      if (this._context !== undefined) {
        CanvasKit.setCurrentContext(this._context);
      }
      callback(this._cached_canvas);

      this.flush(dirtyRect);
      this.dispose();
    }.bind(this));
  };

  CanvasKit.PathEffect.MakeDash = function(intervals, phase) {
    if (!phase) {
      phase = 0;
    }
    if (!intervals.length || intervals.length % 2 === 1) {
      throw 'Intervals array must have even length';
    }
    var ptr = copy1dArray(intervals, 'HEAPF32');
    var dpe = CanvasKit.PathEffect._MakeDash(ptr, intervals.length, phase);
    freeArraysThatAreNotMallocedByUsers(ptr, intervals);
    return dpe;
  };

  CanvasKit.Shader.MakeColor = function(color4f, colorSpace) {
    colorSpace = colorSpace || null
    var cPtr = copyColorToWasm(color4f);
    return CanvasKit.Shader._MakeColor(cPtr, colorSpace);
  };

  // TODO(kjlubick) remove deprecated names.
  CanvasKit.Shader.Blend = CanvasKit.Shader.MakeBlend;
  CanvasKit.Shader.Color = CanvasKit.Shader.MakeColor;
  CanvasKit.Shader.Lerp = CanvasKit.Shader.MakeLerp;

  CanvasKit.Shader.MakeLinearGradient = function(start, end, colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var lgs = CanvasKit.Shader._MakeLinearGradient(start, end, cPtrInfo.colorPtr, cPtrInfo.colorType, posPtr,
                                                   cPtrInfo.count, mode, flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return lgs;
  };

  CanvasKit.Shader.MakeRadialGradient = function(center, radius, colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var rgs = CanvasKit.Shader._MakeRadialGradient(center, radius, cPtrInfo.colorPtr, cPtrInfo.colorType, posPtr,
                                                   cPtrInfo.count, mode, flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return rgs;
  };

  CanvasKit.Shader.MakeSweepGradient = function(cx, cy, colors, pos, mode, localMatrix, flags, startAngle, endAngle, colorSpace) {
    colorSpace = colorSpace || null
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    startAngle = startAngle || 0;
    endAngle = endAngle || 360;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var sgs = CanvasKit.Shader._MakeSweepGradient(cx, cy, cPtrInfo.colorPtr, cPtrInfo.colorType, posPtr,
                                                  cPtrInfo.count, mode,
                                                  startAngle, endAngle, flags,
                                                  localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return sgs;
  };

  CanvasKit.Shader.MakeTwoPointConicalGradient = function(start, startRadius, end, endRadius,
                                                          colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr =   copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var rgs = CanvasKit.Shader._MakeTwoPointConicalGradient(
                          start, startRadius, end, endRadius, cPtrInfo.colorPtr, cPtrInfo.colorType,
                          posPtr, cPtrInfo.count, mode, flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return rgs;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Vertices.prototype.bounds = function(optionalOutputArray) {
    this._bounds(_scratchRectPtr);
    var ta = _scratchRect['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  // Run through the JS files that are added at compile time.
  if (CanvasKit._extraInitializations) {
    CanvasKit._extraInitializations.forEach(function(init) {
      init();
    });
  }
}; // end CanvasKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.

// Accepts an object holding two canvaskit colors.
// {
//    ambient: [r, g, b, a],
//    spot: [r, g, b, a],
// }
// Returns the same format. Note, if malloced colors are passed in, the memory
// housing the passed in colors passed in will be overwritten with the computed
// tonal colors.
CanvasKit.computeTonalColors = function(tonalColors) {
    // copy the colors into WASM
    var cPtrAmbi = copyColorToWasmNoScratch(tonalColors['ambient']);
    var cPtrSpot = copyColorToWasmNoScratch(tonalColors['spot']);
    // The output of this function will be the same pointers we passed in.
    this._computeTonalColors(cPtrAmbi, cPtrSpot);
    // Read the results out.
    var result =  {
      'ambient': copyColorFromWasm(cPtrAmbi),
      'spot': copyColorFromWasm(cPtrSpot),
    };
    // If the user passed us malloced colors in here, we don't want to clean them up.
    freeArraysThatAreNotMallocedByUsers(cPtrAmbi, tonalColors['ambient']);
    freeArraysThatAreNotMallocedByUsers(cPtrSpot, tonalColors['spot']);
    return result;
};

CanvasKit.LTRBRect = function(l, t, r, b) {
  return Float32Array.of(l, t, r, b);
};

CanvasKit.XYWHRect = function(x, y, w, h) {
  return Float32Array.of(x, y, x+w, y+h);
};

CanvasKit.LTRBiRect = function(l, t, r, b) {
  return Int32Array.of(l, t, r, b);
};

CanvasKit.XYWHiRect = function(x, y, w, h) {
  return Int32Array.of(x, y, x+w, y+h);
};

// RRectXY returns a TypedArray representing an RRect with the given rect and a radiusX and
// radiusY for all 4 corners.
CanvasKit.RRectXY = function(rect, rx, ry) {
  return Float32Array.of(
    rect[0], rect[1], rect[2], rect[3],
    rx, ry,
    rx, ry,
    rx, ry,
    rx, ry,
  );
};

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeAnimatedImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeAnimatedImage(iptr, data.byteLength);
  if (!img) {
    Debug('Could not decode animated image');
    return null;
  }
  return img;
};

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeImage(iptr, data.byteLength);
  if (!img) {
    Debug('Could not decode image');
    return null;
  }
  return img;
};

// A variable to hold a canvasElement which can be reused once created the first time.
var memoizedCanvas2dElement = null;

// Alternative to CanvasKit.MakeImageFromEncoded. Allows for CanvasKit users to take advantage of
// browser APIs to decode images instead of using codecs included in the CanvasKit wasm binary.
// Expects that the canvasImageSource has already loaded/decoded.
// CanvasImageSource reference: https://developer.mozilla.org/en-US/docs/Web/API/CanvasImageSource
CanvasKit.MakeImageFromCanvasImageSource = function(canvasImageSource) {
  var width = canvasImageSource.width;
  var height = canvasImageSource.height;

  if (!memoizedCanvas2dElement) {
    memoizedCanvas2dElement = document.createElement('canvas');
  }
  memoizedCanvas2dElement.width = width;
  memoizedCanvas2dElement.height = height;

  var ctx2d = memoizedCanvas2dElement.getContext('2d');
  ctx2d.drawImage(canvasImageSource, 0, 0);

  var imageData = ctx2d.getImageData(0, 0, width, height);

  return CanvasKit.MakeImage({
      'width': width,
      'height': height,
      'alphaType': CanvasKit.AlphaType.Unpremul,
      'colorType': CanvasKit.ColorType.RGBA_8888,
      'colorSpace': CanvasKit.ColorSpace.SRGB
    }, imageData.data, 4 * width);
};

// pixels may be an array but Uint8Array or Uint8ClampedArray is recommended,
// with the bytes representing the pixel values.
// (e.g. each set of 4 bytes could represent RGBA values for a single pixel).
CanvasKit.MakeImage = function(info, pixels, bytesPerRow) {
  var pptr = CanvasKit._malloc(pixels.length);
  CanvasKit.HEAPU8.set(pixels, pptr); // We always want to copy the bytes into the WASM heap.
  // No need to _free pptr, Image takes it with SkData::MakeFromMalloc
  return CanvasKit._MakeImage(info, pptr, pixels.length, bytesPerRow);
};

// Colors may be a Uint32Array of int colors, a Flat Float32Array of float colors
// or a 2d Array of Float32Array(4) (deprecated)
// the underlying Skia function accepts only int colors so it is recommended
// to pass an array of int colors to avoid an extra conversion.
// ColorBuilder is not accepted.
CanvasKit.MakeVertices = function(mode, positions, textureCoordinates, colors,
                                  indices, isVolatile) {
  // Default isVolatile to true if not set
  isVolatile = isVolatile === undefined ? true : isVolatile;
  var idxCount = (indices && indices.length) || 0;

  var flags = 0;
  // These flags are from SkVertices.h and should be kept in sync with those.
  if (textureCoordinates && textureCoordinates.length) {
    flags |= (1 << 0);
  }
  if (colors && colors.length) {
    flags |= (1 << 1);
  }
  if (!isVolatile) {
    flags |= (1 << 2);
  }

  var builder = new CanvasKit._VerticesBuilder(mode, positions.length / 2, idxCount, flags);

  copy1dArray(positions, 'HEAPF32', builder.positions());
  if (builder.texCoords()) {
    copy1dArray(textureCoordinates, 'HEAPF32', builder.texCoords());
  }
  if (builder.colors()) {
    if (colors.build) {
      throw('Color builder not accepted by MakeVertices, use array of ints');
    } else {
      copy1dArray(assureIntColors(colors), 'HEAPU32', builder.colors());
    }
  }
  if (builder.indices()) {
    copy1dArray(indices, 'HEAPU16', builder.indices());
  }

  // Create the vertices, which owns the memory that the builder had allocated.
  return builder.detach();
};
