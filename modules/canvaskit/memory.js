/*
 * This file houses utilities for copying blocks of memory to and from
 * the WASM heap.
 */

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
      // If it's falsy, then we haven't created an array yet.
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
  if (!wasMalloced(arr)) {
    CanvasKit._free(ptr);
  }
}

// wasMalloced returns true if the object was created by a call to Malloc. This is determined
// by looking at a property that was added to our Malloc obj and typed arrays.
function wasMalloced(obj) {
  return obj && obj['_ck'];
}

// We define some "scratch" variables which will house both the pointer to
// memory we allocate at startup as well as a Malloc object, which we can
// use to get a TypedArray view of that memory.

var _scratch3x3MatrixPtr = nullptr;
var _scratch3x3Matrix;  // the result from CanvasKit.Malloc

var _scratch4x4MatrixPtr = nullptr;
var _scratch4x4Matrix;

var _scratchColorPtr = nullptr;
var _scratchColor;

var _scratchFourFloatsA;
var _scratchFourFloatsAPtr = nullptr;

var _scratchFourFloatsB;
var _scratchFourFloatsBPtr = nullptr;

var _scratchThreeFloatsA;
var _scratchThreeFloatsAPtr = nullptr;

var _scratchThreeFloatsB;
var _scratchThreeFloatsBPtr = nullptr;

var _scratchIRect;
var _scratchIRectPtr = nullptr;

var _scratchRRect;
var _scratchRRectPtr = nullptr;

var _scratchRRect2;
var _scratchRRect2Ptr = nullptr;

// arr can be a normal JS array or a TypedArray
// dest is a string like 'HEAPU32' that specifies the type the src array
// should be copied into.
// ptr can be optionally provided if the memory was already allocated.
// Callers should eventually free the data unless the C++ object owns the memory,
// or the provided pointer is a scratch pointer or a user-malloced value.
// see also freeArraysThatAreNotMallocedByUsers().
function copy1dArray(arr, dest, ptr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  // This was created with CanvasKit.Malloc, so it's already been copied.
  if (wasMalloced(arr)) {
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

// Copies the given DOMMatrix/Array/TypedArray to the CanvasKit heap and
// returns a pointer to the memory. This memory is a float* of length 9.
// If the passed in matrix is null/undefined, we return 0 (nullptr). The
// returned pointer should NOT be freed, as it is either null or a scratch
// pointer.
function copy3x3MatrixToWasm(matr) {
  if (!matr) {
    return nullptr;
  }

  if (matr.length) {
    if (matr.length === 6 || matr.length === 9) {
      // matr should be an array or typed array.
      copy1dArray(matr, 'HEAPF32', _scratch3x3MatrixPtr);
      if (matr.length === 6) {
        // Overwrite the last 3 floats with the default perspective. The divide
        // by 4 casts the pointer into a float pointer.
        CanvasKit.HEAPF32.set(defaultPerspective, 6 + _scratch3x3MatrixPtr / 4);
      }
      return _scratch3x3MatrixPtr;
    } else if (matr.length === 16) {
      // Downsample the 4x4 matrix into a 3x3
      var wasm3x3Matrix = _scratch3x3Matrix['toTypedArray']();
      wasm3x3Matrix[0] = matr[0];
      wasm3x3Matrix[1] = matr[1];
      wasm3x3Matrix[2] = matr[3];

      wasm3x3Matrix[3] = matr[4];
      wasm3x3Matrix[4] = matr[5];
      wasm3x3Matrix[5] = matr[7];

      wasm3x3Matrix[6] = matr[12];
      wasm3x3Matrix[7] = matr[13];
      wasm3x3Matrix[8] = matr[15];
      return _scratch3x3MatrixPtr;
    }
    throw 'invalid matrix size';
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


// Copies the given DOMMatrix/Array/TypedArray to the CanvasKit heap and
// returns a pointer to the memory. This memory is a float* of length 16.
// If the passed in matrix is null/undefined, we return 0 (nullptr). The
// returned pointer should NOT be freed, as it is either null or a scratch
// pointer.
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

// copies a 4x4 matrix at the given pointer into a JS array.
function copy4x4MatrixFromWasm(matrPtr) {
  // read them out into an array. TODO(kjlubick): If we change Matrix to be
  // typedArrays, then we should return a typed array here too.
  var rv = new Array(16);
  for (var i = 0; i < 16; i++) {
    rv[i] = CanvasKit.HEAPF32[matrPtr/4 + i]; // divide by 4 to cast to float.
  }
  return rv;
}

// copies the given floats into the wasm heap as an SkColor4f. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyColorToWasm(color4f, ptr) {
  return copy1dArray(color4f, 'HEAPF32', ptr || _scratchColorPtr);
}

// copies the given color into the wasm heap. Callers do not need to free the returned pointer.
function copyColorComponentsToWasm(r, g, b, a) {
  var colors = _scratchColor['toTypedArray']();
  colors[0] = r;
  colors[1] = g;
  colors[2] = b;
  colors[3] = a;
  return _scratchColorPtr;
}

// copies the given color into the wasm heap. Callers must free the returned pointer.
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

// copies the given floats into the wasm heap as an SkRect. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyRectToWasm(fourFloats, ptr) {
  return copy1dArray(fourFloats, 'HEAPF32', ptr || _scratchFourFloatsAPtr);
}

// copies the given ints into the wasm heap as an SkIRect. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyIRectToWasm(fourInts, ptr) {
  return copy1dArray(fourInts, 'HEAP32', ptr || _scratchIRectPtr);
}

// copies the given floats into the wasm heap as an SkRRect. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyRRectToWasm(twelveFloats, ptr) {
  return copy1dArray(twelveFloats, 'HEAPF32', ptr || _scratchRRectPtr);
}