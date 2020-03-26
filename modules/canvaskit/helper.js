// helper JS that could be used anywhere in the glue code

function clamp(c) {
  return Math.round(Math.max(0, Math.min(c || 0, 255)));
}

// Constructs a Color with the same API as CSS's rgba(), that is
// r,g,b are 0-255, and a is 0.0 to 1.0.
// if a is omitted, it will be assumed to be 1.0
// Internally, Colors are a TypedArray of four unpremultiplied 32-bit floats: a, r, g, b
// In order to construct one with more precision or in a wider gamut, use
// CanvasKit.Color4f
CanvasKit.Color = function(r, g, b, a) {
  if (a === undefined) {
      a = 1;
  }
  return CanvasKit.Color4f(clamp(r)/255, clamp(g)/255, clamp(b)/255, a);
}

// Construct a 4-float color.
// Opaque if opacity is omitted.
CanvasKit.Color4f = function(r, g, b, a) {
  if (a === undefined) {
    a = 1;
  }
  return Float32Array.of(r, g, b, a);
}

// Color constants use property getters to prevent other code from accidentally
// changing them.
Object.defineProperty(CanvasKit, "TRANSPARENT", {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 0); }
});
Object.defineProperty(CanvasKit, "BLACK", {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, "WHITE", {
    get: function() { return CanvasKit.Color4f(1, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, "RED", {
    get: function() { return CanvasKit.Color4f(1, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, "GREEN", {
    get: function() { return CanvasKit.Color4f(0, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, "BLUE", {
    get: function() { return CanvasKit.Color4f(0, 0, 1, 1); }
});
Object.defineProperty(CanvasKit, "YELLOW", {
    get: function() { return CanvasKit.Color4f(1, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, "CYAN", {
    get: function() { return CanvasKit.Color4f(0, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, "MAGENTA", {
    get: function() { return CanvasKit.Color4f(1, 0, 1, 1); }
});

// returns a css style [r, g, b, a] from a CanvasKit.Color
// where r, g, b are returned as ints in the range [0, 255]
// where a is scaled between 0 and 1.0
CanvasKit.getColorComponents = function(color) {
  return [
    Math.floor(color[0]*255),
    Math.floor(color[1]*255),
    Math.floor(color[2]*255),
    color[3]
  ];
}

// parseColorString takes in a CSS color value and returns a CanvasKit.Color
// (which is an array of 4 floats in RGBA order). An optional colorMap
// may be provided which maps custom strings to values.
// In the CanvasKit canvas2d shim layer, we provide this map for processing
// canvas2d calls, but not here for code size reasons.
CanvasKit.parseColorString = function(colorStr, colorMap) {
  colorStr = colorStr.toLowerCase();
  // See https://drafts.csswg.org/css-color/#typedef-hex-color
  if (colorStr.startsWith('#')) {
    var r, g, b, a = 255;
    switch (colorStr.length) {
      case 9: // 8 hex chars #RRGGBBAA
        a = parseInt(colorStr.slice(7, 9), 16);
      case 7: // 6 hex chars #RRGGBB
        r = parseInt(colorStr.slice(1, 3), 16);
        g = parseInt(colorStr.slice(3, 5), 16);
        b = parseInt(colorStr.slice(5, 7), 16);
        break;
      case 5: // 4 hex chars #RGBA
        // multiplying by 17 is the same effect as
        // appending another character of the same value
        // e.g. e => ee == 14 => 238
        a = parseInt(colorStr.slice(4, 5), 16) * 17;
      case 4: // 6 hex chars #RGB
        r = parseInt(colorStr.slice(1, 2), 16) * 17;
        g = parseInt(colorStr.slice(2, 3), 16) * 17;
        b = parseInt(colorStr.slice(3, 4), 16) * 17;
        break;
    }
    return CanvasKit.Color(r, g, b, a/255);

  } else if (colorStr.startsWith('rgba')) {
    // Trim off rgba( and the closing )
    colorStr = colorStr.slice(5, -1);
    var nums = colorStr.split(',');
    return CanvasKit.Color(+nums[0], +nums[1], +nums[2],
                           valueOrPercent(nums[3]));
  } else if (colorStr.startsWith('rgb')) {
    // Trim off rgba( and the closing )
    colorStr = colorStr.slice(4, -1);
    var nums = colorStr.split(',');
    // rgb can take 3 or 4 arguments
    return CanvasKit.Color(+nums[0], +nums[1], +nums[2],
                           valueOrPercent(nums[3]));
  } else if (colorStr.startsWith('gray(')) {
    // TODO
  } else if (colorStr.startsWith('hsl')) {
    // TODO
  } else if (colorMap) {
    // Try for named color
    var nc = colorMap[colorStr];
    if (nc !== undefined) {
      return nc;
    }
  }
  SkDebug('unrecognized color ' + colorStr);
  return CanvasKit.BLACK;
}

function isCanvasKitColor(ob) {
  if (!ob) {
    return false;
  }
  return (ob.constructor === Float32Array && ob.length === 4);
}

// Warning information is lost by this conversion
function toUint32Color(c) {
  return ((clamp(c[3]*255) << 24) | (clamp(c[0]*255) << 16) | (clamp(c[1]*255) << 8) | (clamp(c[2]*255) << 0)) >>> 0;
}
function uIntColorToCanvasKitColor(c) {
    return CanvasKit.Color(
     (c >> 16) & 0xFF,
     (c >>  8) & 0xFF,
     (c >>  0) & 0xFF,
    ((c >> 24) & 0xFF) / 255
  );
}

function valueOrPercent(aStr) {
  if (aStr === undefined) {
    return 1; // default to opaque.
  }
  var a = parseFloat(aStr);
  if (aStr && aStr.indexOf('%') !== -1) {
    return a / 100;
  }
  return a;
}

CanvasKit.multiplyByAlpha = function(color, alpha) {
  // make a copy of the color so the function remains pure.
  var result = color.slice();
  result[3] = Math.max(0, Math.min(result[3] * alpha, 1));
  return result;
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
  // This was created with CanvasKit.Malloc, so it's already been copied.
  if (arr['_ck']) {
    return arr.byteOffset;
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
//     inside themselves). A common use case is points.
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

/**
 * Malloc returns a TypedArray backed by the C++ memory of the
 * given length. It should only be used by advanced users who
 * can manage memory and initialize values properly. When used
 * correctly, it can save copying of data between JS and C++.
 * When used incorrectly, it can lead to memory leaks.
 *
 * const ta = CanvasKit.Malloc(Float32Array, 20);
 * // store data into ta
 * const cf = CanvasKit.SkColorFilter.MakeMatrix(ta);
 * // MakeMatrix cleans up the ptr automatically.
 *
 * @param {TypedArray} typedArray - constructor for the typedArray.
 * @param {number} len - number of elements to store.
 */
CanvasKit.Malloc = function(typedArray, len) {
  var byteLen = len * typedArray.BYTES_PER_ELEMENT;
  var ptr = CanvasKit._malloc(byteLen);
  var ta = new typedArray(CanvasKit.HEAPU8.buffer, ptr, len);
  // add a marker that this was allocated in C++ land
  ta['_ck'] = true;
  return ta;
}
