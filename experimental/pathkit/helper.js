// Adds any extra JS functions/helpers we want to the PathKit Library.
// Wrapped in a function to avoid leaking global variables.
(function(PathKit){

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
  //   [PathKit.MOVE_VERB, 0, 10],
  //   [PathKit.LINE_VERB, 30, 40],
  //   [PathKit.QUAD_VERB, 20, 50, 45, 60],
  // ];
  //
  // // The following uses ES6 syntactic sugar "Array Destructuring".
  // // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Destructuring_assignment#Array_destructuring
  // let [ptr, len] = PathKit.loadCmdsTypedArray(cmds);
  // let path = PathKit.FromCmds(ptr, len);
  //
  // If arguments at index 1... in each cmd row are strings, they will be
  // parsed as hex, and then converted to floats using SkBits2FloatUnsigned
  PathKit.loadCmdsTypedArray = function(arr) {
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
        if (typeof item === 'string') {
          // Converts hex to an int, which can be passed to SkBits2FloatUnsigned
          item = PathKit.SkBits2FloatUnsigned(parseInt(item));
        }
        ta[i] = item;
        i++;
      }
    }

    var ptr = PathKit._malloc(ta.length * ta.BYTES_PER_ELEMENT);
    PathKit.HEAPF32.set(ta, ptr / ta.BYTES_PER_ELEMENT);
    return [ptr, len];
  }

  // Experimentation has shown that using TypedArrays to pass arrays from
  // JS to C++ is faster than passing the JS Arrays across.
  // See above for example of cmds.
  PathKit.FromCmds = function(cmds) {
    var ptrLen = PathKit.loadCmdsTypedArray(cmds);
    return PathKit._FromCmds(ptrLen[0], ptrLen[1]);
  }
}(Module)); // When this file is loaded in, the high level object is "Module";

