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
};

// Constructs a Color as a 32 bit unsigned integer, with 8 bits assigned to each channel.
// Channels are expected to be between 0 and 255 and will be clamped as such.
CanvasKit.ColorAsInt = function(r, g, b, a) {
  // default to opaque
  if (a === undefined) {
      a = 255;
  }
  // This is consistent with how Skia represents colors in C++, as an unsigned int.
  // This is also consistent with how Flutter represents colors:
  // https://github.com/flutter/engine/blob/243bb59c7179a7e701ce478080d6ce990710ae73/lib/web_ui/lib/src/ui/painting.dart#L50
  return (((clamp(a) << 24) | (clamp(r) << 16) | (clamp(g) << 8) | (clamp(b) << 0)
   & 0xFFFFFFF) // This truncates the unsigned to 32 bits and signals to JS engines they can
                // represent the number with an int instead of a double.
    >>> 0);     // This makes the value an unsigned int.
};
// Construct a 4-float color.
// Opaque if opacity is omitted.
CanvasKit.Color4f = function(r, g, b, a) {
  if (a === undefined) {
    a = 1;
  }
  return Float32Array.of(r, g, b, a);
};

// Color constants use property getters to prevent other code from accidentally
// changing them.
Object.defineProperty(CanvasKit, 'TRANSPARENT', {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 0); }
});
Object.defineProperty(CanvasKit, 'BLACK', {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, 'WHITE', {
    get: function() { return CanvasKit.Color4f(1, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, 'RED', {
    get: function() { return CanvasKit.Color4f(1, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, 'GREEN', {
    get: function() { return CanvasKit.Color4f(0, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, 'BLUE', {
    get: function() { return CanvasKit.Color4f(0, 0, 1, 1); }
});
Object.defineProperty(CanvasKit, 'YELLOW', {
    get: function() { return CanvasKit.Color4f(1, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, 'CYAN', {
    get: function() { return CanvasKit.Color4f(0, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, 'MAGENTA', {
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
};

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
  Debug('unrecognized color ' + colorStr);
  return CanvasKit.BLACK;
};

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
// Accepts various colors representations and converts them to an array of int colors.
// Does not handle builders.
function assureIntColors(arr) {
  if (arr instanceof Float32Array) {
    var count = Math.floor(arr.length / 4);
    var result = new Uint32Array(count);
    for (var i = 0; i < count; i ++) {
      result[i] = toUint32Color(arr.slice(i*4, (i+1)*4));
    }
    return result;
  } else if (arr instanceof Uint32Array) {
    return arr;
  } else if (arr instanceof Array && arr[0] instanceof Float32Array) {
    return arr.map(toUint32Color);
  }
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
};

function radiansToDegrees(rad) {
  return (rad / Math.PI) * 180;
}

function degreesToRadians(deg) {
  return (deg / 180) * Math.PI;
}

// See https://stackoverflow.com/a/31090240
// This contraption keeps closure from minifying away the check
// if btoa is defined *and* prevents runtime 'btoa' or 'window' is not defined.
// Defined outside any scopes to make it available in all files.
var isNode = !(new Function('try {return this===window;}catch(e){ return false;}')());

function almostEqual(floata, floatb) {
  return Math.abs(floata - floatb) < 0.00001;
}

var nullptr = 0; // emscripten doesn't like to take null as uintptr_t

// arr can be a normal JS array or a TypedArray
// dest is a string like 'HEAPU32' that specifies the type the src array
// should be copied into.
// ptr can be optionally provided if the memory was already allocated.
function copy1dArray(arr, dest, ptr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  // This was created with CanvasKit.Malloc, so it's already been copied.
  if (arr['_ck']) {
    return arr.byteOffset;
  }
  var bytesPerElement = CanvasKit[dest].BYTES_PER_ELEMENT;
  if (!ptr) {
    ptr = CanvasKit._malloc(arr.length * bytesPerElement);
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
  // It is important to make sure we are grabbing the freshest view of the
  // memory possible because if we call _malloc and the heap needs to grow,
  // the TypedArrayView will no longer be valid.
  CanvasKit[dest].set(arr, ptr / bytesPerElement);
  return ptr;
}

// Copies an array of colors to wasm, returning an object with the pointer
// and info necessary to use the copied colors.
// Accepts either a flat Float32Array, flat Uint32Array or Array of Float32Arrays.
// If color is an object that was allocated with CanvasKit.Malloc, its pointer is
// returned and no extra copy is performed.
// TODO(nifong): have this accept color builders.
function copyFlexibleColorArray(colors) {
  var result = {
    colorPtr: nullptr,
    count: colors.length,
    colorType: CanvasKit.ColorType.RGBA_F32,
  };
  if (colors instanceof Float32Array) {
    result.colorPtr = copy1dArray(colors, 'HEAPF32');
    result.count = colors.length / 4;

  } else if (colors instanceof Uint32Array) {
    result.colorPtr = copy1dArray(colors, 'HEAPU32');
    result.colorType = CanvasKit.ColorType.RGBA_8888;

  } else if (colors instanceof Array) {
    result.colorPtr = copyColorArray(colors);
  } else {
    throw('Invalid argument to copyFlexibleColorArray, Not a color array '+typeof(colors));
  }
  return result;
}

function copyColorArray(arr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  // 4 floats per color, 4 bytes per float.
  var ptr = CanvasKit._malloc(arr.length * 4 * 4);

  var idx = 0;
  var adjustedPtr = ptr / 4; // cast the byte pointer into a float pointer.
  for (var r = 0; r < arr.length; r++) {
    for (var c = 0; c < 4; c++) {
      CanvasKit.HEAPF32[adjustedPtr + idx] = arr[r][c];
      idx++;
    }
  }
  return ptr;
}

var defaultPerspective = Float32Array.of(0, 0, 1);

var _scratch3x3MatrixPtr = nullptr;
var _scratch3x3Matrix;  // the result from CanvasKit.Malloc

// Copies the given DOMMatrix/Array/TypedArray to the CanvasKit heap and
// returns a pointer to the memory. This memory is a float* of length 9.
// If the passed in matrix is null/undefined, we return 0 (nullptr). All calls
// on the C++ side should check for nullptr where appropriate. It is generally
// the responsibility of the JS side code to call CanvasKit._free on the
// allocated memory before returning to the user code.
function copy3x3MatrixToWasm(matr) {
  if (!matr) {
    return nullptr;
  }

  if (matr.length) {
    // TODO(kjlubick): Downsample a 16 length (4x4 matrix)
    if (matr.length !== 6 && matr.length !== 9) {
      throw 'invalid matrix size';
    }
    // matr should be an array or typed array.
    var mPtr = copy1dArray(matr, 'HEAPF32', _scratch3x3MatrixPtr);
    if (matr.length === 6) {
      // Overwrite the last 3 floats with the default perspective. The divide
      // by 4 casts the pointer into a float pointer.
      CanvasKit.HEAPF32.set(defaultPerspective, 6 + mPtr / 4);
    }
    return mPtr;
  }
  var wasm3x3Matrix = _scratch3x3Matrix['toTypedArray']();
  // Try as if it's a DOMMatrix. Reminder that DOMMatrix is column-major.
  wasm3x3Matrix[0] = matr.m11;
  wasm3x3Matrix[1] = matr.m21;
  wasm3x3Matrix[2] = matr.m41;

  wasm3x3Matrix[3] = matr.m12;
  wasm3x3Matrix[4] = matr.m22;
  wasm3x3Matrix[5] = matr.m42;

  wasm3x3Matrix[6] = matr.m14;
  wasm3x3Matrix[7] = matr.m24;
  wasm3x3Matrix[8] = matr.m44;
  return _scratch3x3MatrixPtr;
}

var _scratch4x4MatrixPtr = nullptr;
var _scratch4x4Matrix; // the result from CanvasKit.Malloc

function copy4x4MatrixToWasm(matr) {
  if (!matr) {
    return nullptr;
  }
  var wasm4x4Matrix = _scratch4x4Matrix['toTypedArray']();
  if (matr.length) {
    if (matr.length !== 16 && matr.length !== 6 && matr.length !== 9) {
      throw 'invalid matrix size';
    }
    if (matr.length === 16) {
      // matr should be an array or typed array.
      return copy1dArray(matr, 'HEAPF32', _scratch4x4MatrixPtr);
    }
    // Upscale the row-major 3x3 or 3x2 matrix into a 4x4 row-major matrix
    // TODO(skbug.com/10108) This will need to change when we convert our
    //   JS 4x4 to be column-major.
    // When upscaling, we need to overwrite the 3rd column and the 3rd row with
    // 0s. It's easiest to just do that with a fill command.
    wasm4x4Matrix.fill(0);
    wasm4x4Matrix[0] = matr[0];
    wasm4x4Matrix[1] = matr[1];
    // skip col 2
    wasm4x4Matrix[3] = matr[2];

    wasm4x4Matrix[4] = matr[3];
    wasm4x4Matrix[5] = matr[4];
    // skip col 2
    wasm4x4Matrix[7] = matr[5];

    // skip row 2

    wasm4x4Matrix[12] = matr[6];
    wasm4x4Matrix[13] = matr[7];
    // skip col 2
    wasm4x4Matrix[15] = matr[8];

    if (matr.length === 6) {
      // fix perspective for the 3x2 case (from above, they will be undefined).
      wasm4x4Matrix[12]=0;
      wasm4x4Matrix[13]=0;
      wasm4x4Matrix[15]=1;
    }
    return _scratch4x4MatrixPtr;
  }
  // Try as if it's a DOMMatrix. Reminder that DOMMatrix is column-major.
  wasm4x4Matrix[0] = matr.m11;
  wasm4x4Matrix[1] = matr.m21;
  wasm4x4Matrix[2] = matr.m31;
  wasm4x4Matrix[3] = matr.m41;

  wasm4x4Matrix[4] = matr.m12;
  wasm4x4Matrix[5] = matr.m22;
  wasm4x4Matrix[6] = matr.m32;
  wasm4x4Matrix[7] = matr.m42;

  wasm4x4Matrix[8] = matr.m13;
  wasm4x4Matrix[9] = matr.m23;
  wasm4x4Matrix[10] = matr.m33;
  wasm4x4Matrix[11] = matr.m43;

  wasm4x4Matrix[12] = matr.m14;
  wasm4x4Matrix[13] = matr.m24;
  wasm4x4Matrix[14] = matr.m34;
  wasm4x4Matrix[15] = matr.m44;
  return _scratch4x4MatrixPtr;
}

// copies a 4x4 matrix at the given pointer into a JS array. It is the caller's
// responsibility to free the matrPtr if needed.
function copy4x4MatrixFromWasm(matrPtr) {
  // read them out into an array. TODO(kjlubick): If we change Matrix to be
  // typedArrays, then we should return a typed array here too.
  var rv = new Array(16);
  for (var i = 0; i < 16; i++) {
    rv[i] = CanvasKit.HEAPF32[matrPtr/4 + i]; // divide by 4 to cast to float.
  }
  return rv;
}

var _scratchColorPtr = nullptr;
var _scratchColor; // the result from CanvasKit.Malloc

function copyColorToWasm(color4f, ptr) {
  return copy1dArray(color4f, 'HEAPF32', ptr || _scratchColorPtr);
}

function copyColorComponentsToWasm(r, g, b, a) {
  var colors = _scratchColor['toTypedArray']();
  colors[0] = r;
  colors[1] = g;
  colors[2] = b;
  colors[3] = a;
  return _scratchColorPtr;
}

function copyColorToWasmNoScratch(color4f) {
  // TODO(kjlubick): accept 4 floats or int color
  return copy1dArray(color4f, 'HEAPF32');
}

// copies the four floats at the given pointer in a js Float32Array
function copyColorFromWasm(colorPtr) {
  var rv = new Float32Array(4);
  for (var i = 0; i < 4; i++) {
    rv[i] = CanvasKit.HEAPF32[colorPtr/4 + i]; // divide by 4 to cast to float.
  }
  return rv;
}

// These will be initialized after loading.
var _scratchRect;
var _scratchRectPtr = nullptr;

var _scratchRect2;
var _scratchRect2Ptr = nullptr;

function copyRectToWasm(fourFloats, ptr) {
  return copy1dArray(fourFloats, 'HEAPF32', ptr || _scratchRectPtr);
}

var _scratchIRect;
var _scratchIRectPtr = nullptr;
function copyIRectToWasm(fourInts, ptr) {
  return copy1dArray(fourInts, 'HEAP32', ptr || _scratchIRectPtr);
}

// These will be initialized after loading.
var _scratchRRect;
var _scratchRRectPtr = nullptr;

var _scratchRRect2;
var _scratchRRect2Ptr = nullptr;


function copyRRectToWasm(twelveFloats, ptr) {
  return copy1dArray(twelveFloats, 'HEAPF32', ptr || _scratchRRectPtr);
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

/**
 * Malloc returns a TypedArray backed by the C++ memory of the
 * given length. It should only be used by advanced users who
 * can manage memory and initialize values properly. When used
 * correctly, it can save copying of data between JS and C++.
 * When used incorrectly, it can lead to memory leaks.
 * Any memory allocated by CanvasKit.Malloc needs to be released with CanvasKit.Free.
 *
 * const mObj = CanvasKit.Malloc(Float32Array, 20);
 * Get a TypedArray view around the malloc'd memory (this does not copy anything).
 * const ta = mObj.toTypedArray();
 * // store data into ta
 * const cf = CanvasKit.ColorFilter.MakeMatrix(ta); // mObj could also be used.
 *
 * // eventually...
 * CanvasKit.Free(mObj);
 *
 * @param {TypedArray} typedArray - constructor for the typedArray.
 * @param {number} len - number of *elements* to store.
 */
CanvasKit.Malloc = function(typedArray, len) {
  var byteLen = len * typedArray.BYTES_PER_ELEMENT;
  var ptr = CanvasKit._malloc(byteLen);
  return {
    '_ck': true,
    'length': len,
    'byteOffset': ptr,
    typedArray: null,
    'subarray': function(start, end) {
      var sa = this['toTypedArray']().subarray(start, end);
      sa['_ck'] = true;
      return sa;
    },
    'toTypedArray': function() {
      // Check if the previously allocated array is still usable.
      // If it's falsey, then we haven't created an array yet.
      // If it's empty, then WASM resized memory and emptied the array.
      if (this.typedArray && this.typedArray.length) {
        return this.typedArray;
      }
      this.typedArray = new typedArray(CanvasKit.HEAPU8.buffer, ptr, len);
      // add a marker that this was allocated in C++ land
      this.typedArray['_ck'] = true;
      return this.typedArray;
    },
  };
};

/**
 * Free frees the memory returned by Malloc.
 * Any memory allocated by CanvasKit.Malloc needs to be released with CanvasKit.Free.
 */
CanvasKit.Free = function(mallocObj) {
  CanvasKit._free(mallocObj['byteOffset']);
  mallocObj['byteOffset'] = nullptr;
  // Set these to null to make sure the TypedArrays can be garbage collected.
  mallocObj['toTypedArray'] = null;
  mallocObj.typedArray = null;
};

// This helper will free the given pointer unless the provided array is one
// that was returned by CanvasKit.Malloc.
function freeArraysThatAreNotMallocedByUsers(ptr, arr) {
  if (arr && !arr['_ck']) {
    CanvasKit._free(ptr);
  }
}
