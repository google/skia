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
  return (clamp(a*255) << 24) | (clamp(r) << 16) | (clamp(g) << 8) | (clamp(b) << 0);
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
  return clamp(a) << 24 | color;
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
function copy1dArray(arr, dest) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  var ptr = CanvasKit._malloc(arr.length * dest.BYTES_PER_ELEMENT);
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
function copy2dArray(arr, dest) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  var ptr = CanvasKit._malloc(arr.length * arr[0].length * dest.BYTES_PER_ELEMENT);
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
function copy3dArray(arr, dest) {
  if (!arr || !arr.length || !arr[0].length) {
    return nullptr;
  }
  var ptr = CanvasKit._malloc(arr.length * arr[0].length * arr[0][0].length * dest.BYTES_PER_ELEMENT);
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
