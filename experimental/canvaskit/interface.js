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
        console.err('transform expected to take 1 or 9 arguments. Got ' + arguments.length);
        return null;
      }
      return this;
    };
  }

  CanvasKit.getWebGLSurface = function(htmlID) {
    var canvas = document.getElementById(htmlID);
    if (!canvas) {
      throw 'Canvas with id ' + htmlID + ' was not found';
    }
    // Maybe better to use clientWidth/height.  See:
    // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
    return this._getWebGLSurface(htmlID, canvas.width, canvas.height);
  }

  // Likely only used for tests.
  CanvasKit.LTRBRect = function(l, t, r, b) {
    return {
      fLeft: l,
      fTop: t,
      fRight: r,
      fBottom: b,
    };
  }

  CanvasKit.MakeSkDashPathEffect = function(intervals, phase) {
    if (!phase) {
      phase = 0;
    }
    if (!intervals.length || intervals.length % 2 === 1) {
      throw 'Intervals array must have even length';
    }
    if (!(intervals instanceof Float32Array)) {
      intervals = Float32Array.from(intervals);
    }
    var BYTES_PER_ELEMENT = 4; // Float32Array always has 4 bytes per element
    var ptr = CanvasKit._malloc(intervals.length * BYTES_PER_ELEMENT);
    CanvasKit.HEAPF32.set(intervals, ptr / BYTES_PER_ELEMENT);
    return CanvasKit._MakeSkDashPathEffect(ptr, intervals.length, phase);
  }

}(Module)); // When this file is loaded in, the high level object is "Module";
