// Adds JS functions to augment the CanvasKit interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.
(function(CanvasKit){
  // CanvasKit.onRuntimeInitialized is called after the WASM library has loaded.
  // Anything that modifies an exposed class (e.g. SkPath) should be set
  // after onRuntimeInitialized, otherwise, it can happen outside of that scope.
  CanvasKit.onRuntimeInitialized = function() {
    // All calls to 'this' need to go in externs.js so closure doesn't minify them away.
    CanvasKit.SkPath.prototype.addPath = function() {
      // Takes 1, 2, or 10 args, where the first arg is always the path.
      // The options for the remaining args are:
      //   - an array of 9 parameters
      //   - the 9 parameters of a full Matrix
      if (arguments.length === 1) {
        // Add path, unchanged.  Use identify matrix
        this._addPath(arguments[0], 1, 0, 0,
                                    0, 1, 0,
                                    0, 0, 1);
      } else if (arguments.length === 2) {
        // User provided the 9 params of a full matrix as an array.
        var sm = arguments[1];
        this._addPath(arguments[0], a[1], a[2], a[3],
                                    a[4], a[5], a[6],
                                    a[7], a[8], a[9]);
      } else if (arguments.length === 10) {
        // User provided the 9 params of a (full) matrix directly.
        // These are in the same order as what Skia expects.
        var a = arguments;
        this._addPath(arguments[0], a[1], a[2], a[3],
                                    a[4], a[5], a[6],
                                    a[7], a[8], a[9]);
      } else {
        console.err('addPath expected to take 1, 2, or 10 args. Got ' + arguments.length);
        return null;
      }
      return this;
    };

    CanvasKit.SkPath.prototype.arcTo = function(x1, y1, x2, y2, radius) {
      this._arcTo(x1, y1, x2, y2, radius);
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

    CanvasKit.SkPath.prototype.lineTo = function(x, y) {
      this._lineTo(x, y);
      return this;
    };

    CanvasKit.SkPath.prototype.moveTo = function(x, y) {
      this._moveTo(x, y);
      return this;
    };

    CanvasKit.SkPath.prototype.op = function(otherPath, op) {
      if (this._op(otherPath, op)) {
        return this;
      }
      return null;
    };

    CanvasKit.SkPath.prototype.quadTo = function(cpx, cpy, x, y) {
      this._quadTo(cpx, cpy, x, y);
      return this;
    };

    CanvasKit.SkPath.prototype.simplify = function() {
      if (this._simplify()) {
        return this;
      }
      return null;
    };

    CanvasKit.SkPath.prototype.transform = function() {
      // Takes 1 or 9 args
      if (arguments.length === 1) {
        // argument 1 should be a 9 element array.
        var a = arguments[0];
        this._transform(a[0], a[1], a[2],
                        a[3], a[4], a[5],
                        a[6], a[7], a[8]);
      } else if (arguments.length === 9) {
        // these arguments are the 9 members of the matrix
        var a = arguments;
        this._transform(a[0], a[1], a[2],
                        a[3], a[4], a[5],
                        a[6], a[7], a[8]);
      } else {
        throw 'transform expected to take 1 or 9 arguments. Got ' + arguments.length;
      }
      return this;
    };

    // Run through the JS files that are added at compile time.
    if (CanvasKit._extraInitializations) {
      CanvasKit._extraInitializations.forEach(function(init) {
        init();
      });
    }
  } // end CanvasKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.

  // Likely only used for tests.
  CanvasKit.LTRBRect = function(l, t, r, b) {
    return {
      fLeft: l,
      fTop: t,
      fRight: r,
      fBottom: b,
    };
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

  // arr should be a non-jagged 2d JS array (TypeyArrays can't be nested
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

  CanvasKit.MakeSkDashPathEffect = function(intervals, phase) {
    if (!phase) {
      phase = 0;
    }
    if (!intervals.length || intervals.length % 2 === 1) {
      throw 'Intervals array must have even length';
    }
    var ptr = copy1dArray(intervals, CanvasKit.HEAPF32);
    var dpe = CanvasKit._MakeSkDashPathEffect(ptr, intervals.length, phase);
    CanvasKit._free(ptr);
    return dpe;
  }

  CanvasKit.MakeLinearGradientShader = function(start, end, colors, pos, mode) {
    var colorPtr = copy1dArray(colors, CanvasKit.HEAP32);
    var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);

    var dpe = CanvasKit._MakeLinearGradientShader(start, end, colorPtr, posPtr, colors.length, mode);
    CanvasKit._free(colorPtr);
    CanvasKit._free(posPtr);
    return dpe;
  }

  CanvasKit.MakeSkVertices = function(mode, positions, textureCoordinates, colors,
                                      indices, boneIndices, boneWeights) {
    // TODO(kjlubick) we ignore indices, boneIndices, boneWeights for now, pass those through.
    var positionPtr = copy2dArray(positions,          CanvasKit.HEAPF32);
    var texPtr =      copy2dArray(textureCoordinates, CanvasKit.HEAPF32);
    // Since we write the colors to memory as signed integers (JSColor), we can
    // read them out on the other side as unsigned ints (SkColor) just fine
    // - it's effectively casting.
    var colorPtr =    copy1dArray(colors,             CanvasKit.HEAP32);

    // _MakeVertices will copy all the values in, so we are free to release
    // the memory after.
    var vertices = CanvasKit._MakeSkVertices(mode, positions.length, positionPtr, texPtr, colorPtr);
    positionPtr && CanvasKit._free(positionPtr);
    texPtr && CanvasKit._free(texPtr);
    colorPtr && CanvasKit._free(colorPtr);
    return vertices;
  }

  CanvasKit.MakeNimaActor = function(nimaFile, nimaTexture) {
    var nptr = CanvasKit._malloc(nimaFile.byteLength);
    CanvasKit.HEAPU8.set(new Uint8Array(nimaFile), nptr);
    var tptr = CanvasKit._malloc(nimaTexture.byteLength);
    CanvasKit.HEAPU8.set(new Uint8Array(nimaTexture), tptr);

    return CanvasKit._MakeNimaActor(nptr, nimaFile.byteLength, tptr, nimaTexture.byteLength);
  }

}(Module)); // When this file is loaded in, the high level object is "Module";
