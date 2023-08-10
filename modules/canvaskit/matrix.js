//
// Add some helpers for matrices. This is ported from SkMatrix.cpp and others
// to save complexity and overhead of going back and forth between C++ and JS layers.
// I would have liked to use something like DOMMatrix, except it
// isn't widely supported (would need polyfills) and it doesn't
// have a mapPoints() function (which could maybe be tacked on here).
// If DOMMatrix catches on, it would be worth re-considering this usage.
//

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
    m[size] = size%(n+1) === 0 ? 1.0 : 0.0;
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
//   'up'   : [0, 1, 0], // a unit vector pointing in the camera's up direction, because eye and
//                       // coa alone leave roll unspecified.
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