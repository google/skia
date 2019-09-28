// helper JS that could be used anywhere in the glue code

function clamp(c) {
  return Math.round(Math.max(0, Math.min(c || 0, 255)));
}

// Colors are just a 32 bit number with 8 bits each of a, r, g, b
// The API is the same as CSS's representation of color rgba(), that is
// r,g,b are 0-255, and a is 0.0 to 1.0.
// if a is omitted, it will be assumed to be 1.0
CanvasKit.Color = function(r, g, b, a) {
  if (a === undefined) {
      a = 1;
  }
  // The >>> 0 converts the signed int to an unsigned int. Skia's
  // SkColor object is an unsigned int.
  // https://stackoverflow.com/a/14891172
  return ((clamp(a*255) << 24) | (clamp(r) << 16) | (clamp(g) << 8) | (clamp(b) << 0)) >>> 0;
}

// returns [r, g, b, a] from a color
// where a is scaled between 0 and 1.0
CanvasKit.getColorComponents = function(color) {
  return [
     (color >> 16) & 0xFF,
     (color >>  8) & 0xFF,
     (color >>  0) & 0xFF,
    ((color >> 24) & 0xFF) / 255,
  ]
}

CanvasKit.multiplyByAlpha = function(color, alpha) {
  if (alpha === 1) {
    return color;
  }
  // extract as int from 0 to 255
  var a = (color >> 24) & 0xFF;
  a *= alpha;
  // mask off the old alpha
  color &= 0xFFFFFF;
  // back to unsigned int to match SkColor.
  return (clamp(a) << 24 | color) >>> 0;
}

function radiansToDegrees(rad) {
  return (rad / Math.PI) * 180;
}

function degreesToRadians(deg) {
  return (deg / 180) * Math.PI;
}

// See https://stackoverflow.com/a/31090240
// This contraption keeps closure from minifying away the check
// if btoa is defined *and* prevents runtime "btoa" or "window" is not defined.
// Defined outside any scopes to make it available in all files.
var isNode = !(new Function("try {return this===window;}catch(e){ return false;}")());

function almostEqual(floata, floatb) {
  return Math.abs(floata - floatb) < 0.00001;
}


var nullptr = 0; // emscripten doesn't like to take null as uintptr_t

// arr can be a normal JS array or a TypedArray
// dest is something like CanvasKit.HEAPF32
// ptr can be optionally provided if the memory was already allocated.
function copy1dArray(arr, dest, ptr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  if (!ptr) {
    ptr = CanvasKit._malloc(arr.length * dest.BYTES_PER_ELEMENT);
  }
  // In c++ terms, the WASM heap is a uint8_t*, a long buffer/array of single
  // byte elements. When we run _malloc, we always get an offset/pointer into
  // that block of memory.
  // CanvasKit exposes some different views to make it easier to work with
  // different types. HEAPF32 for example, exposes it as a float*
  // However, to make the ptr line up, we have to do some pointer arithmetic.
  // Concretely, we need to convert ptr to go from an index into a 1-byte-wide
  // buffer to an index into a 4-byte-wide buffer (in the case of HEAPF32)
  // and thus we divide ptr by 4.
  dest.set(arr, ptr / dest.BYTES_PER_ELEMENT);
  return ptr;
}

// arr should be a non-jagged 2d JS array (TypedArrays can't be nested
//     inside themselves.)
// dest is something like CanvasKit.HEAPF32
// ptr can be optionally provided if the memory was already allocated.
function copy2dArray(arr, dest, ptr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  if (!ptr) {
    ptr = CanvasKit._malloc(arr.length * arr[0].length * dest.BYTES_PER_ELEMENT);
  }
  var idx = 0;
  var adjustedPtr = ptr / dest.BYTES_PER_ELEMENT;
  for (var r = 0; r < arr.length; r++) {
    for (var c = 0; c < arr[0].length; c++) {
      dest[adjustedPtr + idx] = arr[r][c];
      idx++;
    }
  }
  return ptr;
}

// arr should be a non-jagged 3d JS array (TypedArrays can't be nested
//     inside themselves.)
// dest is something like CanvasKit.HEAPF32
// ptr can be optionally provided if the memory was already allocated.
function copy3dArray(arr, dest, ptr) {
  if (!arr || !arr.length || !arr[0].length) {
    return nullptr;
  }
  if (!ptr) {
    ptr = CanvasKit._malloc(arr.length * arr[0].length * arr[0][0].length * dest.BYTES_PER_ELEMENT);
  }
  var idx = 0;
  var adjustedPtr = ptr / dest.BYTES_PER_ELEMENT;
  for (var x = 0; x < arr.length; x++) {
    for (var y = 0; y < arr[0].length; y++) {
      for (var z = 0; z < arr[0][0].length; z++) {
        dest[adjustedPtr + idx] = arr[x][y][z];
        idx++;
      }
    }
  }
  return ptr;
}

// Caching the Float32Arrays can save having to reallocate them
// over and over again.
var Float32ArrayCache = {};

// Takes a 2D array of commands and puts them into the WASM heap
// as a 1D array. This allows them to referenced from the C++ code.
// Returns a 2 element array, with the first item being essentially a
// pointer to the array and the second item being the length of
// the new 1D array.
//
// Example usage:
// let cmds = [
//   [CanvasKit.MOVE_VERB, 0, 10],
//   [CanvasKit.LINE_VERB, 30, 40],
//   [CanvasKit.QUAD_VERB, 20, 50, 45, 60],
// ];
function loadCmdsTypedArray(arr) {
  var len = 0;
  for (var r = 0; r < arr.length; r++) {
    len += arr[r].length;
  }

  var ta;
  if (Float32ArrayCache[len]) {
    ta = Float32ArrayCache[len];
  } else {
    ta = new Float32Array(len);
    Float32ArrayCache[len] = ta;
  }
  // Flatten into a 1d array
  var i = 0;
  for (var r = 0; r < arr.length; r++) {
    for (var c = 0; c < arr[r].length; c++) {
      var item = arr[r][c];
      ta[i] = item;
      i++;
    }
  }

  var ptr = copy1dArray(ta, CanvasKit.HEAPF32);
  return [ptr, len];
}

function saveBytesToFile(bytes, fileName) {
  if (!isNode) {
    // https://stackoverflow.com/a/32094834
    var blob = new Blob([bytes], {type: 'application/octet-stream'});
    url = window.URL.createObjectURL(blob);
    var a = document.createElement('a');
    document.body.appendChild(a);
    a.href = url;
    a.download = fileName;
    a.click();
    // clean up after because FF might not download it synchronously
    setTimeout(function() {
      URL.revokeObjectURL(url);
      a.remove();
    }, 50);
  } else {
    var fs = require('fs');
    // https://stackoverflow.com/a/42006750
    // https://stackoverflow.com/a/47018122
    fs.writeFile(fileName, new Buffer(bytes), function(err) {
      if (err) throw err;
    });
  }
}
/**
 * Generic helper for dealing with an array of four floats.
 */
CanvasKit.FourFloatArrayHelper = function() {
  this._floats = [];
  this._ptr = null;

  Object.defineProperty(this, 'length', {
    enumerable: true,
    get: function() {
      return this._floats.length / 4;
    },
  });
}

/**
 * push the four floats onto the end of the array - if build() has already
 * been called, the call will return without modifying anything.
 */
CanvasKit.FourFloatArrayHelper.prototype.push = function(f1, f2, f3, f4) {
  if (this._ptr) {
    SkDebug('Cannot push more points - already built');
    return;
  }
  this._floats.push(f1, f2, f3, f4);
}

/**
 * Set the four floats at a given index - if build() has already
 * been called, the WASM memory will be written to directly.
 */
CanvasKit.FourFloatArrayHelper.prototype.set = function(idx, f1, f2, f3, f4) {
  if (idx < 0 || idx >= this._floats.length/4) {
    SkDebug('Cannot set index ' + idx + ', it is out of range', this._floats.length/4);
    return;
  }
  idx *= 4;
  var BYTES_PER_ELEMENT = 4;
  if (this._ptr) {
    // convert this._ptr from uint8_t* to SkScalar* by dividing by 4
    var floatPtr = (this._ptr / BYTES_PER_ELEMENT) + idx;
    CanvasKit.HEAPF32[floatPtr]     = f1;
    CanvasKit.HEAPF32[floatPtr + 1] = f2;
    CanvasKit.HEAPF32[floatPtr + 2] = f3;
    CanvasKit.HEAPF32[floatPtr + 3] = f4;
    return;
  }
  this._floats[idx]     = f1;
  this._floats[idx + 1] = f2;
  this._floats[idx + 2] = f3;
  this._floats[idx + 3] = f4;
}

/**
 * Copies the float data to the WASM memory and returns a pointer
 * to that allocated memory. Once build has been called, this
 * float array cannot be made bigger.
 */
CanvasKit.FourFloatArrayHelper.prototype.build = function() {
  if (this._ptr) {
    return this._ptr;
  }
  this._ptr = copy1dArray(this._floats, CanvasKit.HEAPF32);
  return this._ptr;
}

/**
 * Frees the wasm memory associated with this array. Of note,
 * the points are not removed, so push/set/build can all
 * be called to make a newly allocated (possibly bigger)
 * float array.
 */
CanvasKit.FourFloatArrayHelper.prototype.delete = function() {
  if (this._ptr) {
    CanvasKit._free(this._ptr);
    this._ptr = null;
  }
}

/**
 * Generic helper for dealing with an array of unsigned ints.
 */
CanvasKit.OneUIntArrayHelper = function() {
  this._uints = [];
  this._ptr = null;

  Object.defineProperty(this, 'length', {
    enumerable: true,
    get: function() {
      return this._uints.length;
    },
  });
}

/**
 * push the unsigned int onto the end of the array - if build() has already
 * been called, the call will return without modifying anything.
 */
CanvasKit.OneUIntArrayHelper.prototype.push = function(u) {
  if (this._ptr) {
    SkDebug('Cannot push more points - already built');
    return;
  }
  this._uints.push(u);
}

/**
 * Set the uint at a given index - if build() has already
 * been called, the WASM memory will be written to directly.
 */
CanvasKit.OneUIntArrayHelper.prototype.set = function(idx, u) {
  if (idx < 0 || idx >= this._uints.length) {
    SkDebug('Cannot set index ' + idx + ', it is out of range', this._uints.length);
    return;
  }
  idx *= 4;
  var BYTES_PER_ELEMENT = 4;
  if (this._ptr) {
    // convert this._ptr from uint8_t* to SkScalar* by dividing by 4
    var uintPtr = (this._ptr / BYTES_PER_ELEMENT) + idx;
    CanvasKit.HEAPU32[uintPtr] = u;
    return;
  }
  this._uints[idx] = u;
}

/**
 * Copies the uint data to the WASM memory and returns a pointer
 * to that allocated memory. Once build has been called, this
 * unit array cannot be made bigger.
 */
CanvasKit.OneUIntArrayHelper.prototype.build = function() {
  if (this._ptr) {
    return this._ptr;
  }
  this._ptr = copy1dArray(this._uints, CanvasKit.HEAPU32);
  return this._ptr;
}

/**
 * Frees the wasm memory associated with this array. Of note,
 * the points are not removed, so push/set/build can all
 * be called to make a newly allocated (possibly bigger)
 * uint array.
 */
CanvasKit.OneUIntArrayHelper.prototype.delete = function() {
  if (this._ptr) {
    CanvasKit._free(this._ptr);
    this._ptr = null;
  }
}

/**
 * Helper for building an array of SkRects (which are just structs
 * of 4 floats).
 *
 * It can be more performant to use this helper, as
 * the C++-side array is only allocated once (on the first call)
 * to build. Subsequent set() operations operate directly on
 * the C++-side array, avoiding having to re-allocate (and free)
 * the array every time.
 *
 * Input points are taken as left, top, right, bottom
 */
CanvasKit.SkRectBuilder = CanvasKit.FourFloatArrayHelper;
/**
 * Helper for building an array of RSXForms (which are just structs
 * of 4 floats).
 *
 * It can be more performant to use this helper, as
 * the C++-side array is only allocated once (on the first call)
 * to build. Subsequent set() operations operate directly on
 * the C++-side array, avoiding having to re-allocate (and free)
 * the array every time.
 *
 *  An RSXForm is a compressed form of a rotation+scale matrix.
 *
 *  [ scos    -ssin    tx ]
 *  [ ssin     scos    ty ]
 *  [    0        0     1 ]
 *
 * Input points are taken as scos, ssin, tx, ty
 */
CanvasKit.RSXFormBuilder = CanvasKit.FourFloatArrayHelper;

/**
 * Helper for building an array of SkColor
 *
 * It can be more performant to use this helper, as
 * the C++-side array is only allocated once (on the first call)
 * to build. Subsequent set() operations operate directly on
 * the C++-side array, avoiding having to re-allocate (and free)
 * the array every time.
 */
CanvasKit.SkColorBuilder = CanvasKit.OneUIntArrayHelper;
