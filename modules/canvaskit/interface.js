// Adds JS functions to augment the CanvasKit interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.

// CanvasKit.onRuntimeInitialized is called after the WASM library has loaded.
// Anything that modifies an exposed class (e.g. SkPath) should be set
// after onRuntimeInitialized, otherwise, it can happen outside of that scope.
CanvasKit.onRuntimeInitialized = function() {
  // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

  // Add some helpers for matrices. This is ported from SkMatrix.cpp
  // to save complexity and overhead of going back and forth between
  // C++ and JS layers.
  // I would have liked to use something like DOMMatrix, except it
  // isn't widely supported (would need polyfills) and it doesn't
  // have a mapPoints() function (which could maybe be tacked on here).
  // If DOMMatrix catches on, it would be worth re-considering this usage.
  CanvasKit.SkMatrix = {};
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
  }

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
  }

  CanvasKit.SkMatrix.identity = function() {
    return identityN(3);
  };

  // Return the inverse (if it exists) of this matrix.
  // Otherwise, return the identity.
  CanvasKit.SkMatrix.invert = function(m) {
    // Find the determinant by the sarrus rule. https://en.wikipedia.org/wiki/Rule_of_Sarrus
    var det = m[0]*m[4]*m[8] + m[1]*m[5]*m[6] + m[2]*m[3]*m[7]
            - m[2]*m[4]*m[6] - m[1]*m[3]*m[8] - m[0]*m[5]*m[7];
    if (!det) {
      SkDebug('Warning, uninvertible matrix');
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
  CanvasKit.SkMatrix.mapPoints = function(matrix, ptArr) {
    if (skIsDebug && (ptArr.length % 2)) {
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

  function isnumber(val) { return val !== NaN; };

  // gereralized iterative algorithm for multiplying two matrices.
  function multiply(m1, m2, size) {

    if (skIsDebug && (!m1.every(isnumber) || !m2.every(isnumber))) {
      throw 'Some members of matrices are NaN m1='+m1+', m2='+m2+'';
    }
    if (skIsDebug && (m1.length !== m2.length)) {
      throw 'Undefined for matrices of different sizes. m1.length='+m1.length+', m2.length='+m2.length;
    }
    if (skIsDebug && (size*size !== m1.length)) {
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
  };

  // Accept an integer indicating the size of the matrices being multiplied (3 for 3x3), and any
  // number of matrices following it.
  function multiplyMany(size, listOfMatrices) {
    if (skIsDebug && (listOfMatrices.length < 2)) {
      throw 'multiplication expected two or more matrices';
    }
    var result = multiply(listOfMatrices[0], listOfMatrices[1], size);
    var next = 2;
    while (next < listOfMatrices.length) {
      result = multiply(result, listOfMatrices[next], size);
      next++;
    }
    return result;
  };

  // Accept any number 3x3 of matrices as arguments, multiply them together.
  // Matrix multiplication is associative but not commutative. the order of the arguments
  // matters, but it does not matter that this implementation multiplies them left to right.
  CanvasKit.SkMatrix.multiply = function() {
    return multiplyMany(3, arguments);
  };

  // Return a matrix representing a rotation by n radians.
  // px, py optionally say which point the rotation should be around
  // with the default being (0, 0);
  CanvasKit.SkMatrix.rotated = function(radians, px, py) {
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

  CanvasKit.SkMatrix.scaled = function(sx, sy, px, py) {
    px = px || 0;
    py = py || 0;
    var m = stride([sx, sy], identityN(3), 3, 0, 1);
    return stride([px-sx*px, py-sy*py], m, 3, 2, 0);
  };

  CanvasKit.SkMatrix.skewed = function(kx, ky, px, py) {
    px = px || 0;
    py = py || 0;
    var m = stride([kx, ky], identityN(3), 3, 1, -1);
    return stride([-kx*px, -ky*py], m, 3, 2, 0);
  };

  CanvasKit.SkMatrix.translated = function(dx, dy) {
    return stride(arguments, identityN(3), 3, 2, 0);
  };

  // Functions for manipulating vectors.
  // Loosely based off of SkV3 in SkM44.h but skia also has SkVec2 and Skv4. This combines them and
  // works on vectors of any length.
  CanvasKit.SkVector = {};
  CanvasKit.SkVector.dot = function(a, b) {
    if (skIsDebug && (a.length !== b.length)) {
      throw 'Cannot perform dot product on arrays of different length ('+a.length+' vs '+b.length+')';
    }
    return a.map(function(v, i) { return v*b[i] }).reduce(function(acc, cur) { return acc + cur; });
  }
  CanvasKit.SkVector.lengthSquared = function(v) {
    return CanvasKit.SkVector.dot(v, v);
  }
  CanvasKit.SkVector.length = function(v) {
    return Math.sqrt(CanvasKit.SkVector.lengthSquared(v));
  }
  CanvasKit.SkVector.mulScalar = function(v, s) {
    return v.map(function(i) { return i*s });
  }
  CanvasKit.SkVector.add = function(a, b) {
    return a.map(function(v, i) { return v+b[i] });
  }
  CanvasKit.SkVector.sub = function(a, b) {
    return a.map(function(v, i) { return v-b[i]; });
  }
  CanvasKit.SkVector.dist = function(a, b) {
    return CanvasKit.SkVector.length(CanvasKit.SkVector.sub(a, b));
  }
  CanvasKit.SkVector.normalize = function(v) {
    return CanvasKit.SkVector.mulScalar(v, 1/CanvasKit.SkVector.length(v));
  }
  CanvasKit.SkVector.cross = function(a, b) {
    if (skIsDebug && (a.length !== 3 || a.length !== 3)) {
      throw 'Cross product is only defined for 3-dimensional vectors (a.length='+a.length+', b.length='+b.length+')';
    }
    return [
      a[1]*b[2] - a[2]*b[1],
      a[2]*b[0] - a[0]*b[2],
      a[0]*b[1] - a[1]*b[0],
    ];
  }

  // Functions for creating and manipulating 4x4 matrices. Accepted in place of SkM44 in canvas
  // methods, for the same reasons as the 3x3 matrices above.
  // ported from C++ code in SkM44.cpp
  CanvasKit.SkM44 = {};
  // Create a 4x4 identity matrix
  CanvasKit.SkM44.identity = function() {
    return identityN(4);
  }

  // Anything named vec below is an array of length 3 representing a vector/point in 3D space.
  // Create a 4x4 matrix representing a translate by the provided 3-vec
  CanvasKit.SkM44.translated = function(vec) {
    return stride(vec, identityN(4), 4, 3, 0);
  }
  // Create a 4x4 matrix representing a scaling by the provided 3-vec
  CanvasKit.SkM44.scaled = function(vec) {
    return stride(vec, identityN(4), 4, 0, 1);
  }
  // Create a 4x4 matrix representing a rotation about the provided axis 3-vec.
  // axis does not need to be normalized.
  CanvasKit.SkM44.rotated = function(axisVec, radians) {
    return CanvasKit.SkM44.rotatedUnitSinCos(
      CanvasKit.SkVector.normalize(axisVec), Math.sin(radians), Math.cos(radians));
  }
  // Create a 4x4 matrix representing a rotation about the provided normalized axis 3-vec.
  // Rotation is provided redundantly as both sin and cos values.
  // This rotate can be used when you already have the cosAngle and sinAngle values
  // so you don't have to atan(cos/sin) to call roatated() which expects an angle in radians.
  // this does no checking! Behavior for invalid sin or cos values or non-normalized axis vectors
  // is incorrect. Prefer rotate().
  CanvasKit.SkM44.rotatedUnitSinCos = function(axisVec, sinAngle, cosAngle) {
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
  }
  // Create a 4x4 matrix representing a camera at eyeVec, pointed at centerVec.
  CanvasKit.SkM44.lookat = function(eyeVec, centerVec, upVec) {
    var f = CanvasKit.SkVector.normalize(CanvasKit.SkVector.sub(centerVec, eyeVec));
    var u = CanvasKit.SkVector.normalize(upVec);
    var s = CanvasKit.SkVector.normalize(CanvasKit.SkVector.cross(f, u));

    var m = CanvasKit.SkM44.identity();
    // set each column's top three numbers
    stride(s,                                 m, 4, 0, 0);
    stride(CanvasKit.SkVector.cross(s, f),      m, 4, 1, 0);
    stride(CanvasKit.SkVector.mulScalar(f, -1), m, 4, 2, 0);
    stride(eyeVec,                            m, 4, 3, 0);

    var m2 = CanvasKit.SkM44.invert(m);
    if (m2 === null) {
      return CanvasKit.SkM44.identity();
    }
    return m2;
  }
  // Create a 4x4 matrix representing a perspective. All arguments are scalars.
  // angle is in radians.
  CanvasKit.SkM44.perspective = function(near, far, angle) {
    if (skIsDebug && (far <= near)) {
      throw "far must be greater than near when constructing SkM44 using perspective.";
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
  }
  // Returns the number at the given row and column in matrix m.
  CanvasKit.SkM44.rc = function(m, r, c) {
    return m[r*4+c];
  }
  // Accepts any number of 4x4 matrix arguments, multiplies them left to right.
  CanvasKit.SkM44.multiply = function() {
    return multiplyMany(4, arguments);
  }

  // Invert the 4x4 matrix if it is invertible and return it. if not, return null.
  // taken from SkM44.cpp (altered to use row-major order)
  // m is not altered.
  CanvasKit.SkM44.invert = function(m) {
    if (skIsDebug && !m.every(isnumber)) {
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
      SkDebug('Warning, uninvertible matrix');
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


    if (!tmp.every(function(val) { return val !== NaN && val !== Infinity && val !== -Infinity; })) {
      SkDebug('inverted matrix contains infinities or NaN '+tmp);
      return null;
    }
    return tmp;
  }

  CanvasKit.SkM44.transpose = function(m) {
    return [
      m[0], m[4], m[8], m[12],
      m[1], m[5], m[9], m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15],
    ];
  }

  // An SkColorMatrix is a 4x4 color matrix that transforms the 4 color channels
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

  CanvasKit.SkColorMatrix = {};
  CanvasKit.SkColorMatrix.identity = function() {
    var m = new Float32Array(20);
    m[rScale] = 1;
    m[gScale] = 1;
    m[bScale] = 1;
    m[aScale] = 1;
    return m;
  }

  CanvasKit.SkColorMatrix.scaled = function(rs, gs, bs, as) {
    var m = new Float32Array(20);
    m[rScale] = rs;
    m[gScale] = gs;
    m[bScale] = bs;
    m[aScale] = as;
    return m;
  }

  var rotateIndices = [
    [6, 7, 11, 12],
    [0, 10, 2, 12],
    [0, 1,  5,  6],
  ];
  // axis should be 0, 1, 2 for r, g, b
  CanvasKit.SkColorMatrix.rotated = function(axis, sine, cosine) {
    var m = CanvasKit.SkColorMatrix.identity();
    var indices = rotateIndices[axis];
    m[indices[0]] = cosine;
    m[indices[1]] = sine;
    m[indices[2]] = -sine;
    m[indices[3]] = cosine;
    return m;
  }

  // m is a SkColorMatrix (i.e. a Float32Array), and this sets the 4 "special"
  // params that will translate the colors after they are multiplied by the 4x4 matrix.
  CanvasKit.SkColorMatrix.postTranslate = function(m, dr, dg, db, da) {
    m[rPostTrans] += dr;
    m[gPostTrans] += dg;
    m[bPostTrans] += db;
    m[aPostTrans] += da;
    return m;
  }

  // concat returns a new SkColorMatrix that is the result of multiplying outer*inner;
  CanvasKit.SkColorMatrix.concat = function(outer, inner) {
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
  }

  CanvasKit.SkPath.prototype.addArc = function(oval, startAngle, sweepAngle) {
    // see arc() for the HTMLCanvas version
    // note input angles are degrees.
    this._addArc(oval, startAngle, sweepAngle);
    return this;
  };

  CanvasKit.SkPath.prototype.addOval = function(oval, isCCW, startIndex) {
    if (startIndex === undefined) {
      startIndex = 1;
    }
    this._addOval(oval, !!isCCW, startIndex);
    return this;
  };

  CanvasKit.SkPath.prototype.addPath = function() {
    // Takes 1, 2, 7, or 10 required args, where the first arg is always the path.
    // The last arg is optional and chooses between add or extend mode.
    // The options for the remaining args are:
    //   - an array of 6 or 9 parameters (perspective is optional)
    //   - the 9 parameters of a full matrix or
    //     the 6 non-perspective params of a matrix.
    var args = Array.prototype.slice.call(arguments);
    var path = args[0];
    var extend = false;
    if (typeof args[args.length-1] === "boolean") {
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
      SkDebug('addPath expected to take 1, 2, 7, or 10 required args. Got ' + args.length);
      return null;
    }
    return this;
  };

  // points is either an array of [x, y] where x and y are numbers or
  // a typed array from Malloc where the even indices will be treated
  // as x coordinates and the odd indices will be treated as y coordinates.
  CanvasKit.SkPath.prototype.addPoly = function(points, close) {
    var ptr;
    var n;
    // This was created with CanvasKit.Malloc, so assume the user has
    // already been filled with data.
    if (points['_ck']) {
      ptr = points.byteOffset;
      n = points.length/2;
    } else {
      ptr = copy2dArray(points, CanvasKit.HEAPF32);
      n = points.length;
    }
    this._addPoly(ptr, n, close);
    CanvasKit._free(ptr);
    return this;
  };

  CanvasKit.SkPath.prototype.addRect = function() {
    // Takes 1, 2, 4 or 5 args
    //  - SkRect
    //  - SkRect, isCCW
    //  - left, top, right, bottom
    //  - left, top, right, bottom, isCCW
    if (arguments.length === 1 || arguments.length === 2) {
      var r = arguments[0];
      var ccw = arguments[1] || false;
      this._addRect(r.fLeft, r.fTop, r.fRight, r.fBottom, ccw);
    } else if (arguments.length === 4 || arguments.length === 5) {
      var a = arguments;
      this._addRect(a[0], a[1], a[2], a[3], a[4] || false);
    } else {
      SkDebug('addRect expected to take 1, 2, 4, or 5 args. Got ' + arguments.length);
      return null;
    }
    return this;
  };

  CanvasKit.SkPath.prototype.addRoundRect = function() {
    // Takes 3, 4, 6 or 7 args
    //  - SkRect, radii, ccw
    //  - SkRect, rx, ry, ccw
    //  - left, top, right, bottom, radii, ccw
    //  - left, top, right, bottom, rx, ry, ccw
    var args = arguments;
    if (args.length === 3 || args.length === 6) {
      var radii = args[args.length-2];
    } else if (args.length === 6 || args.length === 7){
      // duplicate the given (rx, ry) pairs for each corner.
      var rx = args[args.length-3];
      var ry = args[args.length-2];
      var radii = [rx, ry, rx, ry, rx, ry, rx, ry];
    } else {
      SkDebug('addRoundRect expected to take 3, 4, 6, or 7 args. Got ' + args.length);
      return null;
    }
    if (radii.length !== 8) {
      SkDebug('addRoundRect needs 8 radii provided. Got ' + radii.length);
      return null;
    }
    var rptr = copy1dArray(radii, CanvasKit.HEAPF32);
    if (args.length === 3 || args.length === 4) {
      var r = args[0];
      var ccw = args[args.length - 1];
      this._addRoundRect(r.fLeft, r.fTop, r.fRight, r.fBottom, rptr, ccw);
    } else if (args.length === 6 || args.length === 7) {
      var a = args;
      this._addRoundRect(a[0], a[1], a[2], a[3], rptr, ccw);
    }
    CanvasKit._free(rptr);
    return this;
  };

  CanvasKit.SkPath.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    // emulates the HTMLCanvas behavior.  See addArc() for the SkPath version.
    // Note input angles are radians.
    var bounds = CanvasKit.LTRBRect(x-radius, y-radius, x+radius, y+radius);
    var sweep = radiansToDegrees(endAngle - startAngle) - (360 * !!ccw);
    var temp = new CanvasKit.SkPath();
    temp.addArc(bounds, radiansToDegrees(startAngle), sweep);
    this.addPath(temp, true);
    temp.delete();
    return this;
  };

  CanvasKit.SkPath.prototype.arcTo = function() {
    // takes 4, 5 or 7 args
    // - 5 x1, y1, x2, y2, radius
    // - 4 oval (as Rect), startAngle, sweepAngle, forceMoveTo
    // - 7 rx, ry, xAxisRotate, useSmallArc, isCCW, x, y
    var args = arguments;
    if (args.length === 5) {
      this._arcTo(args[0], args[1], args[2], args[3], args[4]);
    } else if (args.length === 4) {
      this._arcTo(args[0], args[1], args[2], args[3]);
    } else if (args.length === 7) {
      this._arcTo(args[0], args[1], args[2], !!args[3], !!args[4], args[5], args[6]);
    } else {
      throw 'Invalid args for arcTo. Expected 4, 5, or 7, got '+ args.length;
    }

    return this;
  };

  CanvasKit.SkPath.prototype.close = function() {
    this._close();
    return this;
  };

  CanvasKit.SkPath.prototype.conicTo = function(x1, y1, x2, y2, w) {
    this._conicTo(x1, y1, x2, y2, w);
    return this;
  };

  CanvasKit.SkPath.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.SkPath.prototype.dash = function(on, off, phase) {
    if (this._dash(on, off, phase)) {
      return this;
    }
    return null;
  };

  CanvasKit.SkPath.prototype.lineTo = function(x, y) {
    this._lineTo(x, y);
    return this;
  };

  CanvasKit.SkPath.prototype.moveTo = function(x, y) {
    this._moveTo(x, y);
    return this;
  };

  CanvasKit.SkPath.prototype.offset = function(dx, dy) {
    this._transform(1, 0, dx,
                    0, 1, dy,
                    0, 0, 1);
    return this;
  };

  CanvasKit.SkPath.prototype.quadTo = function(cpx, cpy, x, y) {
    this._quadTo(cpx, cpy, x, y);
    return this;
  };

 CanvasKit.SkPath.prototype.rArcTo = function(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy) {
    this._rArcTo(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy);
    return this;
  };

  CanvasKit.SkPath.prototype.rConicTo = function(dx1, dy1, dx2, dy2, w) {
    this._rConicTo(dx1, dy1, dx2, dy2, w);
    return this;
  };

  // These params are all relative
  CanvasKit.SkPath.prototype.rCubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._rCubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.SkPath.prototype.rLineTo = function(dx, dy) {
    this._rLineTo(dx, dy);
    return this;
  };

  CanvasKit.SkPath.prototype.rMoveTo = function(dx, dy) {
    this._rMoveTo(dx, dy);
    return this;
  };

  // These params are all relative
  CanvasKit.SkPath.prototype.rQuadTo = function(cpx, cpy, x, y) {
    this._rQuadTo(cpx, cpy, x, y);
    return this;
  };

  CanvasKit.SkPath.prototype.stroke = function(opts) {
    // Fill out any missing values with the default values.
    /**
     * See externs.js for this definition
     * @type {StrokeOpts}
     */
    opts = opts || {};
    opts.width = opts.width || 1;
    opts.miter_limit = opts.miter_limit || 4;
    opts.cap = opts.cap || CanvasKit.StrokeCap.Butt;
    opts.join = opts.join || CanvasKit.StrokeJoin.Miter;
    opts.precision = opts.precision || 1;
    if (this._stroke(opts)) {
      return this;
    }
    return null;
  };

  CanvasKit.SkPath.prototype.transform = function() {
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
  CanvasKit.SkPath.prototype.trim = function(startT, stopT, isComplement) {
    if (this._trim(startT, stopT, !!isComplement)) {
      return this;
    }
    return null;
  };

  CanvasKit.SkImage.prototype.encodeToData = function() {
    if (!arguments.length) {
      return this._encodeToData();
    }

    if (arguments.length === 2) {
      var a = arguments;
      return this._encodeToDataWithFormat(a[0], a[1]);
    }

    throw 'encodeToData expected to take 0 or 2 arguments. Got ' + arguments.length;
  }

  CanvasKit.SkImage.prototype.makeShader = function(xTileMode, yTileMode, localMatrix) {
    if (localMatrix) {
      localMatrix = prepare3x3MatrixForGoingToWasm(localMatrix);
      return this._makeShader(xTileMode, yTileMode, localMatrix);
    } else {
      return this._makeShader(xTileMode, yTileMode);
    }
  }

  CanvasKit.SkImage.prototype.readPixels = function(imageInfo, srcX, srcY) {
    var rowBytes;
    // Important to use ["string"] notation here, otherwise the closure compiler will
    // minify away the colorType.
    switch (imageInfo["colorType"]) {
      case CanvasKit.ColorType.RGBA_8888:
        rowBytes = imageInfo.width * 4; // 1 byte per channel == 4 bytes per pixel in 8888
        break;
      case CanvasKit.ColorType.RGBA_F32:
        rowBytes = imageInfo.width * 16; // 4 bytes per channel == 16 bytes per pixel in F32
        break;
      default:
        SkDebug("Colortype not yet supported");
        return;
    }
    var pBytes = rowBytes * imageInfo.height;
    var pPtr = CanvasKit._malloc(pBytes);

    if (!this._readPixels(imageInfo, pPtr, rowBytes, srcX, srcY)) {
      SkDebug("Could not read pixels with the given inputs");
      return null;
    }

    // Put those pixels into a typed array of the right format and then
    // make a copy with slice() that we can return.
    var retVal = null;
    switch (imageInfo["colorType"]) {
      case CanvasKit.ColorType.RGBA_8888:
        retVal = new Uint8Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
      case CanvasKit.ColorType.RGBA_F32:
        retVal = new Float32Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
    }

    // Free the allocated pixels in the WASM memory
    CanvasKit._free(pPtr);
    return retVal;

  }

  // atlas is an SkImage, e.g. from CanvasKit.MakeImageFromEncoded
  // srcRects and dstXforms should be CanvasKit.SkRectBuilder and CanvasKit.RSXFormBuilder
  // or just arrays of floats in groups of 4.
  // colors, if provided, should be a CanvasKit.SkColorBuilder or array of Canvaskit.SimpleColor4f
  CanvasKit.SkCanvas.prototype.drawAtlas = function(atlas, srcRects, dstXforms, paint,
                                       /*optional*/ blendMode, colors) {
    if (!atlas || !paint || !srcRects || !dstXforms) {
      SkDebug('Doing nothing since missing a required input');
      return;
    }
    if (srcRects.length !== dstXforms.length || (colors && colors.length !== dstXforms.length)) {
      SkDebug('Doing nothing since input arrays length mismatches');
      return;
    }
    if (!blendMode) {
      blendMode = CanvasKit.BlendMode.SrcOver;
    }

    var srcRectPtr;
    if (srcRects.build) {
      srcRectPtr = srcRects.build();
    } else {
      srcRectPtr = copy1dArray(srcRects, CanvasKit.HEAPF32);
    }

    var dstXformPtr;
    if (dstXforms.build) {
      dstXformPtr = dstXforms.build();
    } else {
      dstXformPtr = copy1dArray(dstXforms, CanvasKit.HEAPF32);
    }

    var colorPtr = 0; // enscriptem doesn't like undefined for nullptr
    if (colors) {
      if (colors.build) {
        colorPtr = colors.build();
      } else {
        if (!isCanvasKitColor(colors[0])) {
          SkDebug('DrawAtlas color argument expected to be CanvasKit.SkRectBuilder or array of ' +
            'Canvaskit.SimpleColor4f, but got '+colors);
          return;
        }
        // convert here
        colors = colors.map(toUint32Color);
        colorPtr = copy1dArray(colors, CanvasKit.HEAPU32);
      }
    }

    this._drawAtlas(atlas, dstXformPtr, srcRectPtr, colorPtr, dstXforms.length,
                    blendMode, paint);

    if (srcRectPtr && !srcRects.build) {
      CanvasKit._free(srcRectPtr);
    }
    if (dstXformPtr && !dstXforms.build) {
      CanvasKit._free(dstXformPtr);
    }
    if (colorPtr && !colors.build) {
      CanvasKit._free(colorPtr);
    }

  }

  // points is either an array of [x, y] where x and y are numbers or
  // a typed array from Malloc where the even indices will be treated
  // as x coordinates and the odd indices will be treated as y coordinates.
  CanvasKit.SkCanvas.prototype.drawPoints = function(mode, points, paint) {
    var ptr;
    var n;
    // This was created with CanvasKit.Malloc, so assume the user has
    // already been filled with data.
    if (points['_ck']) {
      ptr = points.byteOffset;
      n = points.length/2;
    } else {
      ptr = copy2dArray(points, CanvasKit.HEAPF32);
      n = points.length;
    }
    this._drawPoints(mode, ptr, n, paint);
    CanvasKit._free(ptr);
  }

  // returns Uint8Array
  CanvasKit.SkCanvas.prototype.readPixels = function(x, y, w, h, alphaType,
                                                     colorType, dstRowBytes) {
    // supply defaults (which are compatible with HTMLCanvas's getImageData)
    alphaType = alphaType || CanvasKit.AlphaType.Unpremul;
    colorType = colorType || CanvasKit.ColorType.RGBA_8888;
    dstRowBytes = dstRowBytes || (4 * w);

    var len = h * dstRowBytes
    var pptr = CanvasKit._malloc(len);
    var ok = this._readPixels({
      'width': w,
      'height': h,
      'colorType': colorType,
      'alphaType': alphaType,
    }, pptr, dstRowBytes, x, y);
    if (!ok) {
      CanvasKit._free(pptr);
      return null;
    }

    // The first typed array is just a view into memory. Because we will
    // be free-ing that, we call slice to make a persistent copy.
    var pixels = new Uint8Array(CanvasKit.HEAPU8.buffer, pptr, len).slice();
    CanvasKit._free(pptr);
    return pixels;
  }

  // pixels is a TypedArray. No matter the input size, it will be treated as
  // a Uint8Array (essentially, a byte array).
  CanvasKit.SkCanvas.prototype.writePixels = function(pixels, srcWidth, srcHeight,
                                                      destX, destY, alphaType, colorType) {
    if (pixels.byteLength % (srcWidth * srcHeight)) {
      throw 'pixels length must be a multiple of the srcWidth * srcHeight';
    }
    var bytesPerPixel = pixels.byteLength / (srcWidth * srcHeight);
    // supply defaults (which are compatible with HTMLCanvas's putImageData)
    alphaType = alphaType || CanvasKit.AlphaType.Unpremul;
    colorType = colorType || CanvasKit.ColorType.RGBA_8888;
    var srcRowBytes = bytesPerPixel * srcWidth;

    var pptr = CanvasKit._malloc(pixels.byteLength);
    CanvasKit.HEAPU8.set(pixels, pptr);

    var ok = this._writePixels({
      'width': srcWidth,
      'height': srcHeight,
      'colorType': colorType,
      'alphaType': alphaType,
    }, pptr, srcRowBytes, destX, destY);

    CanvasKit._free(pptr);
    return ok;
  }

  // colorMatrix is an SkColorMatrix (e.g. Float32Array of length 20)
  CanvasKit.SkColorFilter.MakeMatrix = function(colorMatrix) {
    if (!colorMatrix || colorMatrix.length !== 20) {
      SkDebug('ignoring invalid color matrix');
      return;
    }
    var fptr = copy1dArray(colorMatrix, CanvasKit.HEAPF32);
    // We know skia memcopies the floats, so we can free our memory after the call returns.
    var m = CanvasKit.SkColorFilter._makeMatrix(fptr);
    CanvasKit._free(fptr);
    return m;
  }

  CanvasKit.SkSurface.prototype.captureFrameAsSkPicture = function(drawFrame) {
    // Set up SkPictureRecorder
    var spr = new CanvasKit.SkPictureRecorder();
    var canvas = spr.beginRecording(
                    CanvasKit.LTRBRect(0, 0, this.width(), this.height()));
    drawFrame(canvas);
    var pic = spr.finishRecordingAsPicture();
    spr.delete();
    // TODO: do we need to clean up the memory for canvas?
    // If we delete it here, saveAsFile doesn't work correctly.
    return pic;
  }

  CanvasKit.SkSurface.prototype.requestAnimationFrame = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    window.requestAnimationFrame(function() {
      if (this._context !== undefined) {
        CanvasKit.setCurrentContext(this._context);
      }

      callback(this._cached_canvas);

      // We do not dispose() of the SkSurface here, as the client will typically
      // call requestAnimationFrame again from within the supplied callback.
      // For drawing a single frame, prefer drawOnce().
      this.flush();
    }.bind(this));
  }

  // drawOnce will dispose of the surface after drawing the frame using the provided
  // callback.
  CanvasKit.SkSurface.prototype.drawOnce = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    window.requestAnimationFrame(function() {
      if (this._context !== undefined) {
        CanvasKit.setCurrentContext(this._context);
      }
      callback(this._cached_canvas);

      this.flush();
      this.dispose();
    }.bind(this));
  }

  CanvasKit.SkPathEffect.MakeDash = function(intervals, phase) {
    if (!phase) {
      phase = 0;
    }
    if (!intervals.length || intervals.length % 2 === 1) {
      throw 'Intervals array must have even length';
    }
    var ptr = copy1dArray(intervals, CanvasKit.HEAPF32);
    var dpe = CanvasKit.SkPathEffect._MakeDash(ptr, intervals.length, phase);
    CanvasKit._free(ptr);
    return dpe;
  }

  function prepare3x3MatrixForGoingToWasm(matr) {
    if (!matr) {
      return [
               1, 0, 0,
               0, 1, 0,
               0, 0, 1,
              ];
    }
    if (Array.isArray(matr)) {
      // Add perspective args if not provided.
      if (matr.length === 6) {
        matr.push(0, 0, 1);
      }
      return matr;
    }
    // DOMMatrix, which has their values in column major order.
    return [
             matr.a, matr.c, matr.e,
             matr.b, matr.d, matr.f,
             0, 0, 1,
            ];
  }

  CanvasKit.SkShader.MakeLinearGradient = function(start, end, colors, pos, mode, localMatrix, flags) {
    var colorPtr = copy2dArray(colors, CanvasKit.HEAPF32);
    var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);
    flags = flags || 0;

    if (localMatrix) {
      localMatrix = prepare3x3MatrixForGoingToWasm(localMatrix);
      var lgs = CanvasKit._MakeLinearGradientShader(start, end, colorPtr, posPtr,
                                                    colors.length, mode, flags, localMatrix);
    } else {
      var lgs = CanvasKit._MakeLinearGradientShader(start, end, colorPtr, posPtr,
                                                    colors.length, mode, flags);
    }

    CanvasKit._free(colorPtr);
    CanvasKit._free(posPtr);
    return lgs;
  }

  CanvasKit.SkShader.MakeRadialGradient = function(center, radius, colors, pos, mode, localMatrix, flags) {
    var colorPtr = copy2dArray(colors, CanvasKit.HEAPF32);
    var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);
    flags = flags || 0;

    if (localMatrix) {
      localMatrix = prepare3x3MatrixForGoingToWasm(localMatrix);
      var rgs = CanvasKit._MakeRadialGradientShader(center, radius, colorPtr, posPtr,
                                                    colors.length, mode, flags, localMatrix);
    } else {
      var rgs = CanvasKit._MakeRadialGradientShader(center, radius, colorPtr, posPtr,
                                                    colors.length, mode, flags);
    }

    CanvasKit._free(colorPtr);
    CanvasKit._free(posPtr);
    return rgs;
  }

  CanvasKit.SkShader.MakeSweepGradient = function(cx, cy, colors, pos, mode, localMatrix, flags, startAngle, endAngle) {
    var colorPtr = copy2dArray(colors, CanvasKit.HEAPF32);
    var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);
    flags = flags || 0;
    startAngle = startAngle || 0;
    endAngle = endAngle || 360;
    localMatrix = prepare3x3MatrixForGoingToWasm(localMatrix);

    var sgs = CanvasKit._MakeSweepGradientShader(cx, cy, colorPtr, posPtr,
                                                  colors.length, mode,
                                                  startAngle, endAngle, flags,
                                                  localMatrix);

    CanvasKit._free(colorPtr);
    CanvasKit._free(posPtr);
    return sgs;
  }

  CanvasKit.SkShader.MakeTwoPointConicalGradient = function(start, startRadius, end, endRadius,
                                                         colors, pos, mode, localMatrix, flags) {
    var colorPtr = copy2dArray(colors, CanvasKit.HEAPF32);
    var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);
    flags = flags || 0;

    if (localMatrix) {
      localMatrix = prepare3x3MatrixForGoingToWasm(localMatrix);
      var rgs = CanvasKit._MakeTwoPointConicalGradientShader(
                          start, startRadius, end, endRadius,
                          colorPtr, posPtr, colors.length, mode, flags, localMatrix);
    } else {
      var rgs = CanvasKit._MakeTwoPointConicalGradientShader(
                          start, startRadius, end, endRadius,
                          colorPtr, posPtr, colors.length, mode, flags);
    }

    CanvasKit._free(colorPtr);
    CanvasKit._free(posPtr);
    return rgs;
  }

  // temporary support for deprecated names.
  CanvasKit.MakeSkDashPathEffect = CanvasKit.SkPathEffect.MakeDash;
  CanvasKit.MakeLinearGradientShader = CanvasKit.SkShader.MakeLinearGradient;
  CanvasKit.MakeRadialGradientShader = CanvasKit.SkShader.MakeRadialGradient;
  CanvasKit.MakeTwoPointConicalGradientShader = CanvasKit.SkShader.MakeTwoPointConicalGradient;

  // Run through the JS files that are added at compile time.
  if (CanvasKit._extraInitializations) {
    CanvasKit._extraInitializations.forEach(function(init) {
      init();
    });
  }
}; // end CanvasKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.

CanvasKit.LTRBRect = function(l, t, r, b) {
  return {
    fLeft: l,
    fTop: t,
    fRight: r,
    fBottom: b,
  };
}

CanvasKit.XYWHRect = function(x, y, w, h) {
  return {
    fLeft: x,
    fTop: y,
    fRight: x+w,
    fBottom: y+h,
  };
}

// RRectXY returns an RRect with the given rect and a radiusX and radiusY for
// all 4 corners.
CanvasKit.RRectXY = function(rect, rx, ry) {
  return {
    rect: rect,
    rx1: rx,
    ry1: ry,
    rx2: rx,
    ry2: ry,
    rx3: rx,
    ry3: ry,
    rx4: rx,
    ry4: ry,
  };
}

CanvasKit.MakePathFromCmds = function(cmds) {
  var ptrLen = loadCmdsTypedArray(cmds);
  var path = CanvasKit._MakePathFromCmds(ptrLen[0], ptrLen[1]);
  CanvasKit._free(ptrLen[0]);
  return path;
}

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeAnimatedImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeAnimatedImage(iptr, data.byteLength);
  if (!img) {
    SkDebug('Could not decode animated image');
    return null;
  }
  return img;
}

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeImage(iptr, data.byteLength);
  if (!img) {
    SkDebug('Could not decode image');
    return null;
  }
  return img;
}

// pixels must be a Uint8Array with bytes representing the pixel values
// (e.g. each set of 4 bytes could represent RGBA values for a single pixel).
CanvasKit.MakeImage = function(pixels, width, height, alphaType, colorType) {
  var bytesPerPixel = pixels.length / (width * height);
  var info = {
    'width': width,
    'height': height,
    'alphaType': alphaType,
    'colorType': colorType,
  };
  var pptr = copy1dArray(pixels, CanvasKit.HEAPU8);
  // No need to _free pptr, Image takes it with SkData::MakeFromMalloc

  return CanvasKit._MakeImage(info, pptr, pixels.length, width * bytesPerPixel);
}

// colors is an array of SimpleColor4f
CanvasKit.MakeSkVertices = function(mode, positions, textureCoordinates, colors,
                                    indices, isVolatile) {
  // Default isVolitile to true if not set
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

  var builder = new CanvasKit._SkVerticesBuilder(mode,  positions.length, idxCount, flags);

  copy2dArray(positions,            CanvasKit.HEAPF32, builder.positions());
  if (builder.texCoords()) {
    copy2dArray(textureCoordinates, CanvasKit.HEAPF32, builder.texCoords());
  }
  if (builder.colors()) {
    // Convert from canvaskit 4f colors to 32 bit uint colors which builder supports.
    copy1dArray(colors.map(toUint32Color), CanvasKit.HEAPU32, builder.colors());
  }
  if (builder.indices()) {
    copy1dArray(indices,            CanvasKit.HEAPU16, builder.indices());
  }

  var idxCount = (indices && indices.length) || 0;
  // Create the vertices, which owns the memory that the builder had allocated.
  return builder.detach();
};
