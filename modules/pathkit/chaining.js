// Adds JS functions to allow for chaining w/o leaking by re-using 'this' path.
(function(PathKit){
  // PathKit.onRuntimeInitialized is called after the WASM library has loaded.
  // when onRuntimeInitialized is called, PathKit.SkPath is defined with many
  // functions on it (see pathkit_wasm_bindings.cpp@EMSCRIPTEN_BINDINGS)
  PathKit.onRuntimeInitialized = function() {
    // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

    PathKit.SkPath.prototype.addPath = function() {
      // Takes 1, 2, 7 or 10 args, where the first arg is always the path.
      // The options for the remaining args are:
      //   - an SVGMatrix
      //   - the 6 parameters of an SVG Matrix
      //   - the 9 parameters of a full Matrix
      var path = arguments[0];
      if (arguments.length === 1) {
        // Add path, unchanged.  Use identity matrix
        this._addPath(path, 1, 0, 0,
                            0, 1, 0,
                            0, 0, 1);
      } else if (arguments.length === 2) {
        // Takes SVGMatrix, which has its args in a counter-intuitive order
        // https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/transform#Transform_functions
        /**
         * @type {SVGMatrix}
         */
        var sm = arguments[1];
        this._addPath(path, sm.a, sm.c, sm.e,
                            sm.b, sm.d, sm.f,
                               0,    0,    1);
      } else if (arguments.length === 7) {
        // User provided the 6 params for an SVGMatrix directly.
        var a = arguments;
        this._addPath(path, a[1], a[3], a[5],
                            a[2], a[4], a[6],
                              0 ,   0 ,   1 );
      } else if (arguments.length === 10) {
        // User provided the 9 params of a (full) matrix directly.
        // These are in the same order as what Skia expects.
        var a = arguments;
        this._addPath(path, a[1], a[2], a[3],
                            a[4], a[5], a[6],
                            a[7], a[8], a[9]);
      } else {
        console.err('addPath expected to take 1, 2, 7, or 10 args. Got ' + arguments.length);
        return null;
      }
      return this;
    };

    // ccw (counter clock wise) is optional and defaults to false.
    PathKit.SkPath.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {
      this._arc(x, y, radius, startAngle, endAngle, !!ccw);
      return this;
    };

    PathKit.SkPath.prototype.arcTo = function(x1, y1, x2, y2, radius) {
      this._arcTo(x1, y1, x2, y2, radius);
      return this;
    };

    PathKit.SkPath.prototype.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
      this._cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
      return this;
    };

    PathKit.SkPath.prototype.close = function() {
      this._close();
      return this;
    };

    // Reminder, we have some duplicate definitions because we want to be a
    // superset of Path2D and also work like the original SkPath C++ object.
    PathKit.SkPath.prototype.closePath = function() {
      this._close();
      return this;
    };

    PathKit.SkPath.prototype.conicTo = function(x1, y1, x2, y2, w) {
      this._conicTo(x1, y1, x2, y2, w);
      return this;
    };

    PathKit.SkPath.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
      this._cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
      return this;
    };

    PathKit.SkPath.prototype.dash = function(on, off, phase) {
      if (this._dash(on, off, phase)) {
        return this;
      }
      return null;
    };

    // ccw (counter clock wise) is optional and defaults to false.
    PathKit.SkPath.prototype.ellipse = function(x, y, radiusX, radiusY, rotation, startAngle, endAngle, ccw) {
      this._ellipse(x, y, radiusX, radiusY, rotation, startAngle, endAngle, !!ccw);
      return this;
    };

    PathKit.SkPath.prototype.lineTo = function(x, y) {
      this._lineTo(x, y);
      return this;
    };

    PathKit.SkPath.prototype.moveTo = function(x, y) {
      this._moveTo(x, y);
      return this;
    };

    PathKit.SkPath.prototype.op = function(otherPath, op) {
      if (this._op(otherPath, op)) {
        return this;
      }
      return null;
    };

    PathKit.SkPath.prototype.quadraticCurveTo = function(cpx, cpy, x, y) {
      this._quadTo(cpx, cpy, x, y);
      return this;
    };

    PathKit.SkPath.prototype.quadTo = function(cpx, cpy, x, y) {
      this._quadTo(cpx, cpy, x, y);
      return this;
    };

    PathKit.SkPath.prototype.rect = function(x, y, w, h) {
      this._rect(x, y, w, h);
      return this;
    };

    PathKit.SkPath.prototype.simplify = function() {
      if (this._simplify()) {
        return this;
      }
      return null;
    };

    PathKit.SkPath.prototype.stroke = function(opts) {
      // Fill out any missing values with the default values.
      /**
       * See externs.js for this definition
       * @type {StrokeOpts}
       */
      opts = opts || {};
      opts.width = opts.width || 1;
      opts.miter_limit = opts.miter_limit || 4;
      opts.cap = opts.cap || PathKit.StrokeCap.BUTT;
      opts.join = opts.join || PathKit.StrokeJoin.MITER;
      if (this._stroke(opts)) {
        return this;
      }
      return null;
    };

    PathKit.SkPath.prototype.transform = function() {
      // Takes 1 or 9 args
      if (arguments.length === 1) {
        // argument 1 should be a 9 element array (which is transformed on the C++ side
        // to a SimpleMatrix)
        this._transform(arguments[0]);
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

    // isComplement is optional, defaults to false
    PathKit.SkPath.prototype.trim = function(startT, stopT, isComplement) {
      if (this._trim(startT, stopT, !!isComplement)) {
        return this;
      }
      return null;
    };
  };

}(Module)); // When this file is loaded in, the high level object is "Module";

