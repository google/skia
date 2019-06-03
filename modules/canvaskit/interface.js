// Adds JS functions to augment the CanvasKit interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.

// CanvasKit.onRuntimeInitialized is called after the WASM library has loaded.
// Anything that modifies an exposed class (e.g. SkPath) should be set
// after onRuntimeInitialized, otherwise, it can happen outside of that scope.
CanvasKit.onRuntimeInitialized = function() {
  // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

  // Add some helpers for matrices. This is ported from SkMatrix.cpp
  // to save complexity and overhead of going back and forth between
  // C++ and JS layers.
  // I would have liked to use something like DOMMatrix, except it
  // isn't widely supported (would need polyfills) and it doesn't
  // have a mapPoints() function (which could maybe be tacked on here).
  // If DOMMatrix catches on, it would be worth re-considering this usage.
  CanvasKit.SkMatrix = {};
  function sdot(a, b, c, d, e, f) {
    e = e || 0;
    f = f || 0;
    return a * b + c * d + e * f;
  }

  CanvasKit.SkMatrix.identity = function() {
    return [
      1, 0, 0,
      0, 1, 0,
      0, 0, 1,
    ];
  };

  // Return the inverse (if it exists) of this matrix.
  // Otherwise, return the identity.
  CanvasKit.SkMatrix.invert = function(m) {
    var det = m[0]*m[4]*m[8] + m[1]*m[5]*m[6] + m[2]*m[3]*m[7]
            - m[2]*m[4]*m[6] - m[1]*m[3]*m[8] - m[0]*m[5]*m[7];
    if (!det) {
      SkDebug('Warning, uninvertible matrix');
      return CanvasKit.SkMatrix.identity();
    }
    return [
      (m[4]*m[8] - m[5]*m[7])/det, (m[2]*m[7] - m[1]*m[8])/det, (m[1]*m[5] - m[2]*m[4])/det,
      (m[5]*m[6] - m[3]*m[8])/det, (m[0]*m[8] - m[2]*m[6])/det, (m[2]*m[3] - m[0]*m[5])/det,
      (m[3]*m[7] - m[4]*m[6])/det, (m[1]*m[6] - m[0]*m[7])/det, (m[0]*m[4] - m[1]*m[3])/det,
    ];
  };

  // Maps the given points according to the passed in matrix.
  // Results are done in place.
  // See SkMatrix.h::mapPoints for the docs on the math.
  CanvasKit.SkMatrix.mapPoints = function(matrix, ptArr) {
    if (ptArr.length % 2) {
      throw 'mapPoints requires an even length arr';
    }
    for (var i = 0; i < ptArr.length; i+=2) {
      var x = ptArr[i], y = ptArr[i+1];
      // Gx+Hy+I
      var denom  = matrix[6]*x + matrix[7]*y + matrix[8];
      // Ax+By+C
      var xTrans = matrix[0]*x + matrix[1]*y + matrix[2];
      // Dx+Ey+F
      var yTrans = matrix[3]*x + matrix[4]*y + matrix[5];
      ptArr[i]   = xTrans/denom;
      ptArr[i+1] = yTrans/denom;
    }
    return ptArr;
  };

  CanvasKit.SkMatrix.multiply = function(m1, m2) {
    var result = [0,0,0, 0,0,0, 0,0,0];
    for (var r = 0; r < 3; r++) {
      for (var c = 0; c < 3; c++) {
        // m1 and m2 are 1D arrays pretending to be 2D arrays
        result[3*r + c] = sdot(m1[3*r + 0], m2[3*0 + c],
                               m1[3*r + 1], m2[3*1 + c],
                               m1[3*r + 2], m2[3*2 + c]);
      }
    }
    return result;
  }

  // Return a matrix representing a rotation by n radians.
  // px, py optionally say which point the rotation should be around
  // with the default being (0, 0);
  CanvasKit.SkMatrix.rotated = function(radians, px, py) {
    px = px || 0;
    py = py || 0;
    var sinV = Math.sin(radians);
    var cosV = Math.cos(radians);
    return [
      cosV, -sinV, sdot( sinV, py, 1 - cosV, px),
      sinV,  cosV, sdot(-sinV, px, 1 - cosV, py),
      0,        0,                             1,
    ];
  };

  CanvasKit.SkMatrix.scaled = function(sx, sy, px, py) {
    px = px || 0;
    py = py || 0;
    return [
      sx, 0, px - sx * px,
      0, sy, py - sy * py,
      0,  0,            1,
    ];
  };

  CanvasKit.SkMatrix.skewed = function(kx, ky, px, py) {
    px = px || 0;
    py = py || 0;
    return [
      1, kx, -kx * px,
      ky, 1, -ky * py,
      0,  0,        1,
    ];
  };

  CanvasKit.SkMatrix.translated = function(dx, dy) {
    return [
      1, 0, dx,
      0, 1, dy,
      0, 0,  1,
    ];
  };

  CanvasKit.SkPath.prototype.addArc = function(oval, startAngle, sweepAngle) {
    // see arc() for the HTMLCanvas version
    // note input angles are degrees.
    this._addArc(oval, startAngle, sweepAngle);
    return this;
  };

  CanvasKit.SkPath.prototype.addPath = function() {
    // Takes 1, 2, 7, or 10 required args, where the first arg is always the path.
    // The last arg is optional and chooses between add or extend mode.
    // The options for the remaining args are:
    //   - an array of 6 or 9 parameters (perspective is optional)
    //   - the 9 parameters of a full matrix or
    //     the 6 non-perspective params of a matrix.
    var args = Array.prototype.slice.call(arguments);
    var path = args[0];
    var extend = false;
    if (typeof args[args.length-1] === "boolean") {
      extend = args.pop();
    }
    if (args.length === 1) {
      // Add path, unchanged.  Use identity matrix
      this._addPath(path, 1, 0, 0,
                          0, 1, 0,
                          0, 0, 1,
                          extend);
    } else if (args.length === 2) {
      // User provided the 9 params of a full matrix as an array.
      var a = args[1];
      this._addPath(path, a[0],      a[1],      a[2],
                          a[3],      a[4],      a[5],
                          a[6] || 0, a[7] || 0, a[8] || 1,
                          extend);
    } else if (args.length === 7 || args.length === 10) {
      // User provided the 9 params of a (full) matrix directly.
      // (or just the 6 non perspective ones)
      // These are in the same order as what Skia expects.
      var a = args;
      this._addPath(path, a[1],      a[2],      a[3],
                          a[4],      a[5],      a[6],
                          a[7] || 0, a[8] || 0, a[9] || 1,
                          extend);
    } else {
      SkDebug('addPath expected to take 1, 2, 7, or 10 required args. Got ' + args.length);
      return null;
    }
    return this;
  };

  CanvasKit.SkPath.prototype.addRect = function() {
    // Takes 1, 2, 4 or 5 args
    //  - SkRect
    //  - SkRect, isCCW
    //  - left, top, right, bottom
    //  - left, top, right, bottom, isCCW
    if (arguments.length === 1 || arguments.length === 2) {
      var r = arguments[0];
      var ccw = arguments[1] || false;
      this._addRect(r.fLeft, r.fTop, r.fRight, r.fBottom, ccw);
    } else if (arguments.length === 4 || arguments.length === 5) {
      var a = arguments;
      this._addRect(a[0], a[1], a[2], a[3], a[4] || false);
    } else {
      SkDebug('addRect expected to take 1, 2, 4, or 5 args. Got ' + arguments.length);
      return null;
    }
    return this;
  };

  CanvasKit.SkPath.prototype.addRoundRect = function() {
    // Takes 3, 4, 6 or 7 args
    //  - SkRect, radii, ccw
    //  - SkRect, rx, ry, ccw
    //  - left, top, right, bottom, radii, ccw
    //  - left, top, right, bottom, rx, ry, ccw
    var args = arguments;
    if (args.length === 3 || args.length === 6) {
      var radii = args[args.length-2];
    } else if (args.length === 6 || args.length === 7){
      // duplicate the given (rx, ry) pairs for each corner.
      var rx = args[args.length-3];
      var ry = args[args.length-2];
      var radii = [rx, ry, rx, ry, rx, ry, rx, ry];
    } else {
      SkDebug('addRoundRect expected to take 3, 4, 6, or 7 args. Got ' + args.length);
      return null;
    }
    if (radii.length !== 8) {
      SkDebug('addRoundRect needs 8 radii provided. Got ' + radii.length);
      return null;
    }
    var rptr = copy1dArray(radii, CanvasKit.HEAPF32);
    if (args.length === 3 || args.length === 4) {
      var r = args[0];
      var ccw = args[args.length - 1];
      this._addRoundRect(r.fLeft, r.fTop, r.fRight, r.fBottom, rptr, ccw);
    } else if (args.length === 6 || args.length === 7) {
      var a = args;
      this._addRoundRect(a[0], a[1], a[2], a[3], rptr, ccw);
    }
    CanvasKit._free(rptr);
    return this;
  };

  CanvasKit.SkPath.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    // emulates the HTMLCanvas behavior.  See addArc() for the SkPath version.
    // Note input angles are radians.
    var bounds = CanvasKit.LTRBRect(x-radius, y-radius, x+radius, y+radius);
    var sweep = radiansToDegrees(endAngle - startAngle) - (360 * !!ccw);
    var temp = new CanvasKit.SkPath();
    temp.addArc(bounds, radiansToDegrees(startAngle), sweep);
    this.addPath(temp, true);
    temp.delete();
    return this;
  };

  CanvasKit.SkPath.prototype.arcTo = function() {
    // takes 4, 5 or 7 args
    // - 5 x1, y1, x2, y2, radius
    // - 4 oval (as Rect), startAngle, sweepAngle, forceMoveTo
    // - 7 x1, y1, x2, y2, startAngle, sweepAngle, forceMoveTo
    var args = arguments;
    if (args.length === 5) {
      this._arcTo(args[0], args[1], args[2], args[3], args[4]);
    } else if (args.length === 4) {
      this._arcTo(args[0], args[1], args[2], args[3]);
    } else if (args.length === 7) {
      this._arcTo(CanvasKit.LTRBRect(args[0], args[1], args[2], args[3]),
                  args[4], args[5], args[6]);
    } else {
      throw 'Invalid args for arcTo. Expected 4, 5, or 7, got '+ args.length;
    }

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

  CanvasKit.SkPath.prototype.dash = function(on, off, phase) {
    if (this._dash(on, off, phase)) {
      return this;
    }
    return null;
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

  CanvasKit.SkPath.prototype.stroke = function(opts) {
    // Fill out any missing values with the default values.
    /**
     * See externs.js for this definition
     * @type {StrokeOpts}
     */
    opts = opts || {};
    opts.width = opts.width || 1;
    opts.miter_limit = opts.miter_limit || 4;
    opts.cap = opts.cap || CanvasKit.StrokeCap.Butt;
    opts.join = opts.join || CanvasKit.StrokeJoin.Miter;
    opts.precision = opts.precision || 1;
    if (this._stroke(opts)) {
      return this;
    }
    return null;
  };

  CanvasKit.SkPath.prototype.transform = function() {
    // Takes 1 or 9 args
    if (arguments.length === 1) {
      // argument 1 should be a 6 or 9 element array.
      var a = arguments[0];
      this._transform(a[0], a[1], a[2],
                      a[3], a[4], a[5],
                      a[6] || 0, a[7] || 0, a[8] || 1);
    } else if (arguments.length === 6 || arguments.length === 9) {
      // these arguments are the 6 or 9 members of the matrix
      var a = arguments;
      this._transform(a[0], a[1], a[2],
                      a[3], a[4], a[5],
                      a[6] || 0, a[7] || 0, a[8] || 1);
    } else {
      throw 'transform expected to take 1 or 9 arguments. Got ' + arguments.length;
    }
    return this;
  };
  // isComplement is optional, defaults to false
  CanvasKit.SkPath.prototype.trim = function(startT, stopT, isComplement) {
    if (this._trim(startT, stopT, !!isComplement)) {
      return this;
    }
    return null;
  };

  // bones should be a 3d array.
  // Each bone is a 3x2 transformation matrix in column major order:
  // | scaleX   skewX transX |
  // |  skewY  scaleY transY |
  // and bones is an array of those matrices.
  // Returns a copy of this (SkVertices) with the bones applied.
  CanvasKit.SkVertices.prototype.applyBones = function(bones) {
    var bPtr = copy3dArray(bones, CanvasKit.HEAPF32);
    var vert = this._applyBones(bPtr, bones.length);
    CanvasKit._free(bPtr);
    return vert;
  }

  CanvasKit.SkImage.prototype.encodeToData = function() {
    if (!arguments.length) {
      return this._encodeToData();
    }

    if (arguments.length === 2) {
      var a = arguments;
      return this._encodeToDataWithFormat(a[0], a[1]);
    }

    throw 'encodeToData expected to take 0 or 2 arguments. Got ' + arguments.length;
  }

  CanvasKit.SkImage.prototype.makeShader = function(xTileMode, yTileMode, localMatrix) {
    if (localMatrix) {
      // Add perspective args if not provided.
      if (localMatrix.length === 6) {
        localMatrix.push(0, 0, 1);
      }
      return this._makeShader(xTileMode, yTileMode, localMatrix);
    } else {
      return this._makeShader(xTileMode, yTileMode);
    }
  }

  CanvasKit.SkImage.prototype.readPixels = function(imageInfo, srcX, srcY) {
    var rowBytes;
    switch (imageInfo.colorType){
      case CanvasKit.ColorType.RGBA_8888:
        rowBytes = imageInfo.width * 4; // 1 byte per channel == 4 bytes per pixel in 8888
        break;
      case CanvasKit.ColorType.RGBA_F32:
        rowBytes = imageInfo.width * 16; // 4 bytes per channel == 16 bytes per pixel in F32
        break;
      default:
        SkDebug("Colortype not yet supported");
        return;
    }
    var pBytes = rowBytes * imageInfo.height;
    var pPtr = CanvasKit._malloc(pBytes);

    if (!this._readPixels(imageInfo, pPtr, rowBytes, srcX, srcY)) {
      SkDebug("Could not read pixels with the given inputs");
      return null;
    }

    // Put those pixels into a typed array of the right format and then
    // make a copy with slice() that we can return.
    var retVal = null;
    switch (imageInfo.colorType){
      case CanvasKit.ColorType.RGBA_8888:
        retVal = new Uint8Array(CanvasKit.buffer, pPtr, pBytes).slice();
        break;
      case CanvasKit.ColorType.RGBA_F32:
        retVal = new Float32Array(CanvasKit.buffer, pPtr, pBytes).slice();
        break;
    }

    // Free the allocated pixels in the WASM memory
    CanvasKit._free(pPtr);
    return retVal;

  }

  // atlas is an SkImage, e.g. from CanvasKit.MakeImageFromEncoded
  // srcRects and dstXforms should be CanvasKit.SkRectBuilder and CanvasKit.RSXFormBuilder
  // or just arrays of floats in groups of 4.
  // colors, if provided, should be a CanvasKit.SkColorBuilder or array of SkColor
  // (from CanvasKit.Color)
  CanvasKit.SkCanvas.prototype.drawAtlas = function(atlas, srcRects, dstXforms, paint,
                                       /*optional*/ blendMode, colors) {
    if (!atlas || !paint || !srcRects || !dstXforms) {
      SkDebug('Doing nothing since missing a required input');
      return;
    }
    if (srcRects.length !== dstXforms.length || (colors && colors.length !== dstXforms.length)) {
      SkDebug('Doing nothing since input arrays length mismatches');
    }
    if (!blendMode) {
      blendMode = CanvasKit.BlendMode.SrcOver;
    }

    var srcRectPtr;
    if (srcRects.build) {
      srcRectPtr = srcRects.build();
    } else {
      srcRectPtr = copy1dArray(srcRects, CanvasKit.HEAPF32);
    }

    var dstXformPtr;
    if (dstXforms.build) {
      dstXformPtr = dstXforms.build();
    } else {
      dstXformPtr = copy1dArray(dstXforms, CanvasKit.HEAPF32);
    }

    var colorPtr = 0; // enscriptem doesn't like undefined for nullptr
    if (colors) {
      if (colors.build) {
        colorPtr = colors.build();
      } else {
        colorPtr = copy1dArray(colors, CanvasKit.HEAPU32);
      }
    }

    this._drawAtlas(atlas, dstXformPtr, srcRectPtr, colorPtr, dstXforms.length,
                    blendMode, paint);

    if (srcRectPtr && !srcRects.build) {
      CanvasKit._free(srcRectPtr);
    }
    if (dstXformPtr && !dstXforms.build) {
      CanvasKit._free(dstXformPtr);
    }
    if (colorPtr && !colors.build) {
      CanvasKit._free(colorPtr);
    }

  }

  // str can be either a text string or a ShapedText object
  CanvasKit.SkCanvas.prototype.drawText = function(str, x, y, paint, font) {
    if (typeof str === 'string') {
      // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
      // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
      // Add 1 for null terminator
      var strLen = lengthBytesUTF8(str) + 1;
      var strPtr = CanvasKit._malloc(strLen);

      stringToUTF8(str, strPtr, strLen);
      this._drawSimpleText(strPtr, strLen, x, y, font, paint);
    } else {
      this._drawShapedText(str, x, y, paint);
    }
  }

  // returns Uint8Array
  CanvasKit.SkCanvas.prototype.readPixels = function(x, y, w, h, alphaType,
                                                     colorType, dstRowBytes) {
    // supply defaults (which are compatible with HTMLCanvas's getImageData)
    alphaType = alphaType || CanvasKit.AlphaType.Unpremul;
    colorType = colorType || CanvasKit.ColorType.RGBA_8888;
    dstRowBytes = dstRowBytes || (4 * w);

    var len = h * dstRowBytes
    var pptr = CanvasKit._malloc(len);
    var ok = this._readPixels({
      'width': w,
      'height': h,
      'colorType': colorType,
      'alphaType': alphaType,
    }, pptr, dstRowBytes, x, y);
    if (!ok) {
      CanvasKit._free(pptr);
      return null;
    }

    // The first typed array is just a view into memory. Because we will
    // be free-ing that, we call slice to make a persistent copy.
    var pixels = new Uint8Array(CanvasKit.HEAPU8.buffer, pptr, len).slice();
    CanvasKit._free(pptr);
    return pixels;
  }

  // pixels is a TypedArray. No matter the input size, it will be treated as
  // a Uint8Array (essentially, a byte array).
  CanvasKit.SkCanvas.prototype.writePixels = function(pixels, srcWidth, srcHeight,
                                                      destX, destY, alphaType, colorType) {
    if (pixels.byteLength % (srcWidth * srcHeight)) {
      throw 'pixels length must be a multiple of the srcWidth * srcHeight';
    }
    var bytesPerPixel = pixels.byteLength / (srcWidth * srcHeight);
    // supply defaults (which are compatible with HTMLCanvas's putImageData)
    alphaType = alphaType || CanvasKit.AlphaType.Unpremul;
    colorType = colorType || CanvasKit.ColorType.RGBA_8888;
    var srcRowBytes = bytesPerPixel * srcWidth;

    var pptr = CanvasKit._malloc(pixels.byteLength);
    CanvasKit.HEAPU8.set(pixels, pptr);

    var ok = this._writePixels({
      'width': srcWidth,
      'height': srcHeight,
      'colorType': colorType,
      'alphaType': alphaType,
    }, pptr, srcRowBytes, destX, destY);

    CanvasKit._free(pptr);
    return ok;
  }

  // Returns an array of the widths of the glyphs in this string.
  CanvasKit.SkFont.prototype.getWidths = function(str) {
    // add 1 for null terminator
    var codePoints = str.length + 1;
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strBytes = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strBytes);
    stringToUTF8(str, strPtr, strBytes);

    var bytesPerFloat = 4;
    // allocate widths == numCodePoints
    var widthPtr = CanvasKit._malloc(codePoints * bytesPerFloat);
    if (!this._getWidths(strPtr, strBytes, codePoints, widthPtr)) {
      SkDebug('Could not compute widths');
      CanvasKit._free(strPtr);
      CanvasKit._free(widthPtr);
      return null;
    }
    // reminder, this shouldn't copy the data, just is a nice way to
    // wrap 4 bytes together into a float.
    var widths = new Float32Array(CanvasKit.buffer, widthPtr, codePoints);
    // This copies the data so we can free the CanvasKit memory
    var retVal = Array.from(widths);
    CanvasKit._free(strPtr);
    CanvasKit._free(widthPtr);
    return retVal;
  }

  // fontData should be an arrayBuffer
  CanvasKit.SkFontMgr.prototype.MakeTypefaceFromData = function(fontData) {
    var data = new Uint8Array(fontData);

    var fptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, fptr);
    var font = this._makeTypefaceFromData(fptr, data.byteLength);
    if (!font) {
      SkDebug('Could not decode font data');
      // We do not need to free the data since the C++ will do that for us
      // when the font is deleted (or fails to decode);
      return null;
    }
    return font;
  }

  // The serialized format of an SkPicture (informally called an "skp"), is not something
  // that clients should ever rely on. It is useful when filing bug reports, but that's
  // about it. The format may change at anytime and no promises are made for backwards
  // or forward compatibility.
  CanvasKit.SkPicture.prototype.DEBUGONLY_saveAsFile = function(skpName) {
    var data = this.DEBUGONLY_serialize();
    if (!data) {
      SkDebug('Could not serialize to skpicture.');
      return;
    }
    var bytes = CanvasKit.getSkDataBytes(data);
    saveBytesToFile(bytes, skpName);
    data.delete();
  }

  CanvasKit.SkSurface.prototype.captureFrameAsSkPicture = function(drawFrame) {
    // Set up SkPictureRecorder
    var spr = new CanvasKit.SkPictureRecorder();
    var canvas = spr.beginRecording(
                    CanvasKit.LTRBRect(0, 0, this.width(), this.height()));
    drawFrame(canvas);
    var pic = spr.finishRecordingAsPicture();
    spr.delete();
    // TODO: do we need to clean up the memory for canvas?
    // If we delete it here, saveAsFile doesn't work correctly.
    return pic;
  }

  CanvasKit.SkSurface.prototype.requestAnimationFrame = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    window.requestAnimationFrame(function() {
      if (this._context !== undefined) {
        CanvasKit.setCurrentContext(this._context);
      }

      callback(this._cached_canvas);

      this.flush();
    }.bind(this));
  }

  CanvasKit.SkTextBlob.MakeOnPath = function(str, path, font, initialOffset) {
    if (!str || !str.length) {
      SkDebug('ignoring 0 length string');
      return;
    }
    if (!path || !path.countPoints()) {
      SkDebug('ignoring empty path');
      return;
    }
    if (path.countPoints() === 1) {
      SkDebug('path has 1 point, returning normal textblob');
      return this.MakeFromText(str, font);
    }

    if (!initialOffset) {
      initialOffset = 0;
    }

    var widths = font.getWidths(str);

    var rsx = new CanvasKit.RSXFormBuilder();
    var meas = new CanvasKit.SkPathMeasure(path, false, 1);
    var dist = initialOffset;
    for (var i = 0; i < str.length; i++) {
      var width = widths[i];
      dist += width/2;
      if (dist > meas.getLength()) {
        // jump to next contour
        if (!meas.nextContour()) {
          // We have come to the end of the path - terminate the string
          // right here.
          str = str.substring(0, i);
          break;
        }
        dist = width/2;
      }

      // Gives us the (x, y) coordinates as well as the cos/sin of the tangent
      // line at that position.
      var xycs = meas.getPosTan(dist);
      var cx = xycs[0];
      var cy = xycs[1];
      var cosT = xycs[2];
      var sinT = xycs[3];

      var adjustedX = cx - (width/2 * cosT);
      var adjustedY = cy - (width/2 * sinT);

      rsx.push(cosT, sinT, adjustedX, adjustedY);
      dist += width/2;
    }
    var retVal = this.MakeFromRSXform(str, rsx, font);
    rsx.delete();
    meas.delete();
    return retVal;
  }

  CanvasKit.SkTextBlob.MakeFromRSXform = function(str, rsxBuilder, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);
    var rptr = rsxBuilder.build();

    var blob = CanvasKit.SkTextBlob._MakeFromRSXform(strPtr, strLen - 1,
                          rptr, font, CanvasKit.TextEncoding.UTF8);
    if (!blob) {
      SkDebug('Could not make textblob from string "' + str + '"');
      return null;
    }

    var origDelete = blob.delete.bind(blob);
    blob.delete = function() {
      CanvasKit._free(strPtr);
      origDelete();
    }
    return blob;
  }

  CanvasKit.SkTextBlob.MakeFromText = function(str, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);

    var blob = CanvasKit.SkTextBlob._MakeFromText(strPtr, strLen - 1, font, CanvasKit.TextEncoding.UTF8);
    if (!blob) {
      SkDebug('Could not make textblob from string "' + str + '"');
      return null;
    }

    var origDelete = blob.delete.bind(blob);
    blob.delete = function() {
      CanvasKit._free(strPtr);
      origDelete();
    }
    return blob;
  }

  // Run through the JS files that are added at compile time.
  if (CanvasKit._extraInitializations) {
    CanvasKit._extraInitializations.forEach(function(init) {
      init();
    });
  }
}; // end CanvasKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.

CanvasKit.LTRBRect = function(l, t, r, b) {
  return {
    fLeft: l,
    fTop: t,
    fRight: r,
    fBottom: b,
  };
}

CanvasKit.XYWHRect = function(x, y, w, h) {
  return {
    fLeft: x,
    fTop: y,
    fRight: x+w,
    fBottom: y+h,
  };
}

CanvasKit.MakePathFromCmds = function(cmds) {
  var ptrLen = loadCmdsTypedArray(cmds);
  var path = CanvasKit._MakePathFromCmds(ptrLen[0], ptrLen[1]);
  CanvasKit._free(ptrLen[0]);
  return path;
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

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeImage(iptr, data.byteLength);
  if (!img) {
    SkDebug('Could not decode image');
    return null;
  }
  return img;
}

// pixels is a Uint8Array
CanvasKit.MakeImage = function(pixels, width, height, alphaType, colorType) {
  var bytesPerPixel = pixels.byteLength / (width * height);
  var info = {
    'width': width,
    'height': height,
    'alphaType': alphaType,
    'colorType': colorType,
  };
  var pptr = CanvasKit._malloc(pixels.byteLength);
  CanvasKit.HEAPU8.set(pixels, pptr);
  // No need to _free iptr, Image takes it with SkData::MakeFromMalloc

  return CanvasKit._MakeImage(info, pptr, pixels.byteLength, width * bytesPerPixel);
}

CanvasKit.MakeLinearGradientShader = function(start, end, colors, pos, mode, localMatrix, flags) {
  var colorPtr = copy1dArray(colors, CanvasKit.HEAPU32);
  var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);
  flags = flags || 0;

  if (localMatrix) {
    // Add perspective args if not provided.
    if (localMatrix.length === 6) {
      localMatrix.push(0, 0, 1);
    }
    var lgs = CanvasKit._MakeLinearGradientShader(start, end, colorPtr, posPtr,
                                                  colors.length, mode, flags, localMatrix);
  } else {
    var lgs = CanvasKit._MakeLinearGradientShader(start, end, colorPtr, posPtr,
                                                  colors.length, mode, flags);
  }

  CanvasKit._free(colorPtr);
  CanvasKit._free(posPtr);
  return lgs;
}

CanvasKit.MakeRadialGradientShader = function(center, radius, colors, pos, mode, localMatrix, flags) {
  var colorPtr = copy1dArray(colors, CanvasKit.HEAPU32);
  var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);
  flags = flags || 0;

  if (localMatrix) {
    // Add perspective args if not provided.
    if (localMatrix.length === 6) {
      localMatrix.push(0, 0, 1);
    }
    var rgs = CanvasKit._MakeRadialGradientShader(center, radius, colorPtr, posPtr,
                                                  colors.length, mode, flags, localMatrix);
  } else {
    var rgs = CanvasKit._MakeRadialGradientShader(center, radius, colorPtr, posPtr,
                                                  colors.length, mode, flags);
  }

  CanvasKit._free(colorPtr);
  CanvasKit._free(posPtr);
  return rgs;
}

CanvasKit.MakeTwoPointConicalGradientShader = function(start, startRadius, end, endRadius,
                                                       colors, pos, mode, localMatrix, flags) {
  var colorPtr = copy1dArray(colors, CanvasKit.HEAPU32);
  var posPtr =   copy1dArray(pos,    CanvasKit.HEAPF32);
  flags = flags || 0;

  if (localMatrix) {
    // Add perspective args if not provided.
    if (localMatrix.length === 6) {
      localMatrix.push(0, 0, 1);
    }
    var rgs = CanvasKit._MakeTwoPointConicalGradientShader(
                        start, startRadius, end, endRadius,
                        colorPtr, posPtr, colors.length, mode, flags, localMatrix);
  } else {
    var rgs = CanvasKit._MakeTwoPointConicalGradientShader(
                        start, startRadius, end, endRadius,
                        colorPtr, posPtr, colors.length, mode, flags);
  }

  CanvasKit._free(colorPtr);
  CanvasKit._free(posPtr);
  return rgs;
}

CanvasKit.MakeSkVertices = function(mode, positions, textureCoordinates, colors,
                                    boneIndices, boneWeights, indices, isVolatile) {
  // Default isVolitile to true if not set
  isVolatile = isVolatile === undefined ? true : isVolatile;
  var idxCount = (indices && indices.length) || 0;

  var flags = 0;
  // These flags are from SkVertices.h and should be kept in sync with those.
  if (textureCoordinates && textureCoordinates.length) {
    flags |= (1 << 0);
  }
  if (colors && colors.length) {
    flags |= (1 << 1);
  }
  if (boneIndices && boneIndices.length) {
    flags |= (1 << 2);
  }
  if (!isVolatile) {
    flags |= (1 << 3);
  }

  var builder = new CanvasKit._SkVerticesBuilder(mode,  positions.length, idxCount, flags);

  copy2dArray(positions,            CanvasKit.HEAPF32, builder.positions());
  if (builder.texCoords()) {
    copy2dArray(textureCoordinates, CanvasKit.HEAPF32, builder.texCoords());
  }
  if (builder.colors()) {
    copy1dArray(colors,             CanvasKit.HEAPU32, builder.colors());
  }
  if (builder.boneIndices()) {
    copy2dArray(boneIndices,        CanvasKit.HEAP32, builder.boneIndices());
  }
  if (builder.boneWeights()) {
    copy2dArray(boneWeights,        CanvasKit.HEAPF32, builder.boneWeights());
  }
  if (builder.indices()) {
    copy1dArray(indices,            CanvasKit.HEAPU16, builder.indices());
  }

  var idxCount = (indices && indices.length) || 0;
  // Create the vertices, which owns the memory that the builder had allocated.
  return builder.detach();
};
