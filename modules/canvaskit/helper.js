// TODO(kjlubick)
// The remaining functions here are deprecated and should be removed eventually.

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
// TODO(kjlubick) remove this and Float32ArrayCache (superceded by Malloc).
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

  var ptr = copy1dArray(ta, 'HEAPF32');
  return [ptr, len];
}

// TODO(kjlubick) remove Builders - no longer needed now that Malloc is a thing.
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
};

/**
 * push the four floats onto the end of the array - if build() has already
 * been called, the call will return without modifying anything.
 */
CanvasKit.FourFloatArrayHelper.prototype.push = function(f1, f2, f3, f4) {
  if (this._ptr) {
    Debug('Cannot push more points - already built');
    return;
  }
  this._floats.push(f1, f2, f3, f4);
};

/**
 * Set the four floats at a given index - if build() has already
 * been called, the WASM memory will be written to directly.
 */
CanvasKit.FourFloatArrayHelper.prototype.set = function(idx, f1, f2, f3, f4) {
  if (idx < 0 || idx >= this._floats.length/4) {
    Debug('Cannot set index ' + idx + ', it is out of range', this._floats.length/4);
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
};

/**
 * Copies the float data to the WASM memory and returns a pointer
 * to that allocated memory. Once build has been called, this
 * float array cannot be made bigger.
 */
CanvasKit.FourFloatArrayHelper.prototype.build = function() {
  if (this._ptr) {
    return this._ptr;
  }
  this._ptr = copy1dArray(this._floats, 'HEAPF32');
  return this._ptr;
};

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
};

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
};

/**
 * push the unsigned int onto the end of the array - if build() has already
 * been called, the call will return without modifying anything.
 */
CanvasKit.OneUIntArrayHelper.prototype.push = function(u) {
  if (this._ptr) {
    Debug('Cannot push more points - already built');
    return;
  }
  this._uints.push(u);
};

/**
 * Set the uint at a given index - if build() has already
 * been called, the WASM memory will be written to directly.
 */
CanvasKit.OneUIntArrayHelper.prototype.set = function(idx, u) {
  if (idx < 0 || idx >= this._uints.length) {
    Debug('Cannot set index ' + idx + ', it is out of range', this._uints.length);
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
};

/**
 * Copies the uint data to the WASM memory and returns a pointer
 * to that allocated memory. Once build has been called, this
 * unit array cannot be made bigger.
 */
CanvasKit.OneUIntArrayHelper.prototype.build = function() {
  if (this._ptr) {
    return this._ptr;
  }
  this._ptr = copy1dArray(this._uints, 'HEAPU32');
  return this._ptr;
};

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
};

/**
 * Helper for building an array of Rects (which are just structs
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
CanvasKit.RectBuilder = CanvasKit.FourFloatArrayHelper;
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
 * Helper for building an array of Color
 *
 * It can be more performant to use this helper, as
 * the C++-side array is only allocated once (on the first call)
 * to build. Subsequent set() operations operate directly on
 * the C++-side array, avoiding having to re-allocate (and free)
 * the array every time.
 */
CanvasKit.ColorBuilder = CanvasKit.OneUIntArrayHelper;
