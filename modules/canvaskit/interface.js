// Adds JS functions to augment the CanvasKit interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.

// CanvasKit.onRuntimeInitialized is called after the WASM library has loaded.
// Anything that modifies an exposed class (e.g. Path) should be set
// after onRuntimeInitialized, otherwise, it can happen outside of that scope.
CanvasKit.onRuntimeInitialized = function() {
  // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

  _scratchColor = CanvasKit.Malloc(Float32Array, 4); // 4 color scalars.
  _scratchColorPtr = _scratchColor['byteOffset'];

  _scratch4x4Matrix = CanvasKit.Malloc(Float32Array, 16); // 16 matrix scalars.
  _scratch4x4MatrixPtr = _scratch4x4Matrix['byteOffset'];

  _scratch3x3Matrix = CanvasKit.Malloc(Float32Array, 9); // 9 matrix scalars.
  _scratch3x3MatrixPtr = _scratch3x3Matrix['byteOffset'];

  _scratchRRect = CanvasKit.Malloc(Float32Array, 12); // 4 scalars for rrect, 8 for radii.
  _scratchRRectPtr = _scratchRRect['byteOffset'];

  _scratchRRect2 = CanvasKit.Malloc(Float32Array, 12); // 4 scalars for rrect, 8 for radii.
  _scratchRRect2Ptr = _scratchRRect2['byteOffset'];

  _scratchFourFloatsA = CanvasKit.Malloc(Float32Array, 4);
  _scratchFourFloatsAPtr = _scratchFourFloatsA['byteOffset'];

  _scratchFourFloatsB = CanvasKit.Malloc(Float32Array, 4);
  _scratchFourFloatsBPtr = _scratchFourFloatsB['byteOffset'];

  _scratchThreeFloatsA = CanvasKit.Malloc(Float32Array, 3); // 3 floats to represent SkVector3
  _scratchThreeFloatsAPtr = _scratchThreeFloatsA['byteOffset'];

  _scratchThreeFloatsB = CanvasKit.Malloc(Float32Array, 3); // 3 floats to represent SkVector3
  _scratchThreeFloatsBPtr = _scratchThreeFloatsB['byteOffset'];

  _scratchIRect = CanvasKit.Malloc(Int32Array, 4);
  _scratchIRectPtr = _scratchIRect['byteOffset'];

  // Create single copies of all three supported color spaces
  // These are sk_sp<ColorSpace>
  CanvasKit.ColorSpace.SRGB = CanvasKit.ColorSpace._MakeSRGB();
  CanvasKit.ColorSpace.DISPLAY_P3 = CanvasKit.ColorSpace._MakeDisplayP3();
  CanvasKit.ColorSpace.ADOBE_RGB = CanvasKit.ColorSpace._MakeAdobeRGB();

  // Use quotes to tell closure compiler not to minify the names
  CanvasKit['GlyphRunFlags'] = {
    'IsWhiteSpace': CanvasKit['_GlyphRunFlags_isWhiteSpace'],
  };

  CanvasKit.Path.MakeFromCmds = function(cmds) {
    var cmdPtr = copy1dArray(cmds, 'HEAPF32');
    var path = CanvasKit.Path._MakeFromCmds(cmdPtr, cmds.length);
    freeArraysThatAreNotMallocedByUsers(cmdPtr, cmds);
    return path;
  };

  // The weights array is optional (only used for conics).
  CanvasKit.Path.MakeFromVerbsPointsWeights = function(verbs, pts, weights) {
    var verbsPtr = copy1dArray(verbs, 'HEAPU8');
    var pointsPtr = copy1dArray(pts, 'HEAPF32');
    var weightsPtr = copy1dArray(weights, 'HEAPF32');
    var numWeights = (weights && weights.length) || 0;
    var path = CanvasKit.Path._MakeFromVerbsPointsWeights(
        verbsPtr, verbs.length, pointsPtr, pts.length, weightsPtr, numWeights);
    freeArraysThatAreNotMallocedByUsers(verbsPtr, verbs);
    freeArraysThatAreNotMallocedByUsers(pointsPtr, pts);
    freeArraysThatAreNotMallocedByUsers(weightsPtr, weights);
    return path;
  };

  CanvasKit.Path.prototype.addArc = function(oval, startAngle, sweepAngle) {
    // see arc() for the HTMLCanvas version
    // note input angles are degrees.
    var oPtr = copyRectToWasm(oval);
    this._addArc(oPtr, startAngle, sweepAngle);
    return this;
  };

  CanvasKit.Path.prototype.addOval = function(oval, isCCW, startIndex) {
    if (startIndex === undefined) {
      startIndex = 1;
    }
    var oPtr = copyRectToWasm(oval);
    this._addOval(oPtr, !!isCCW, startIndex);
    return this;
  };

  // TODO(kjlubick) clean up this API - split it apart if necessary
  CanvasKit.Path.prototype.addPath = function() {
    // Takes 1, 2, 7, or 10 required args, where the first arg is always the path.
    // The last arg is optional and chooses between add or extend mode.
    // The options for the remaining args are:
    //   - an array of 6 or 9 parameters (perspective is optional)
    //   - the 9 parameters of a full matrix or
    //     the 6 non-perspective params of a matrix.
    var args = Array.prototype.slice.call(arguments);
    var path = args[0];
    var extend = false;
    if (typeof args[args.length-1] === 'boolean') {
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
      Debug('addPath expected to take 1, 2, 7, or 10 required args. Got ' + args.length);
      return null;
    }
    return this;
  };

  // points is a 1d array of length 2n representing n points where the even indices
  // will be treated as x coordinates and the odd indices will be treated as y coordinates.
  // Like other APIs, this accepts a malloced type array or malloc obj.
  CanvasKit.Path.prototype.addPoly = function(points, close) {
    var ptr = copy1dArray(points, 'HEAPF32');
    this._addPoly(ptr, points.length / 2, close);
    freeArraysThatAreNotMallocedByUsers(ptr, points);
    return this;
  };

  CanvasKit.Path.prototype.addRect = function(rect, isCCW) {
    var rPtr = copyRectToWasm(rect);
    this._addRect(rPtr, !!isCCW);
    return this;
  };

  CanvasKit.Path.prototype.addRRect = function(rrect, isCCW) {
    var rPtr = copyRRectToWasm(rrect);
    this._addRRect(rPtr, !!isCCW);
    return this;
  };

  // The weights array is optional (only used for conics).
  CanvasKit.Path.prototype.addVerbsPointsWeights = function(verbs, points, weights) {
    var verbsPtr = copy1dArray(verbs, 'HEAPU8');
    var pointsPtr = copy1dArray(points, 'HEAPF32');
    var weightsPtr = copy1dArray(weights, 'HEAPF32');
    var numWeights = (weights && weights.length) || 0;
    this._addVerbsPointsWeights(verbsPtr, verbs.length, pointsPtr, points.length,
                                weightsPtr, numWeights);
    freeArraysThatAreNotMallocedByUsers(verbsPtr, verbs);
    freeArraysThatAreNotMallocedByUsers(pointsPtr, points);
    freeArraysThatAreNotMallocedByUsers(weightsPtr, weights);
  };

  CanvasKit.Path.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    // emulates the HTMLCanvas behavior.  See addArc() for the Path version.
    // Note input angles are radians.
    var bounds = CanvasKit.LTRBRect(x-radius, y-radius, x+radius, y+radius);
    var sweep = radiansToDegrees(endAngle - startAngle) - (360 * !!ccw);
    var temp = new CanvasKit.Path();
    temp.addArc(bounds, radiansToDegrees(startAngle), sweep);
    this.addPath(temp, true);
    temp.delete();
    return this;
  };

  // Appends arc to Path. Arc added is part of ellipse
  // bounded by oval, from startAngle through sweepAngle. Both startAngle and
  // sweepAngle are measured in degrees, where zero degrees is aligned with the
  // positive x-axis, and positive sweeps extends arc clockwise.
  CanvasKit.Path.prototype.arcToOval = function(oval, startAngle, sweepAngle, forceMoveTo) {
    var oPtr = copyRectToWasm(oval);
    this._arcToOval(oPtr, startAngle, sweepAngle, forceMoveTo);
    return this;
  };

  // Appends arc to Path. Arc is implemented by one or more conics weighted to
  // describe part of oval with radii (rx, ry) rotated by xAxisRotate degrees. Arc
  // curves from last point to (x, y), choosing one of four possible routes:
  // clockwise or counterclockwise, and smaller or larger.

  // Arc sweep is always less than 360 degrees. arcTo() appends line to (x, y) if
  // either radii are zero, or if last point equals (x, y). arcTo() scales radii
  // (rx, ry) to fit last point and (x, y) if both are greater than zero but
  // too small.

  // arcToRotated() appends up to four conic curves.
  // arcToRotated() implements the functionality of SVG arc, although SVG sweep-flag value
  // is opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise,
  // while kCW_Direction cast to int is zero.
  CanvasKit.Path.prototype.arcToRotated = function(rx, ry, xAxisRotate, useSmallArc, isCCW, x, y) {
    this._arcToRotated(rx, ry, xAxisRotate, !!useSmallArc, !!isCCW, x, y);
    return this;
  };

  // Appends arc to Path, after appending line if needed. Arc is implemented by conic
  // weighted to describe part of circle. Arc is contained by tangent from
  // last Path point to (x1, y1), and tangent from (x1, y1) to (x2, y2). Arc
  // is part of circle sized to radius, positioned so it touches both tangent lines.

  // If last Path Point does not start Arc, arcTo appends connecting Line to Path.
  // The length of Vector from (x1, y1) to (x2, y2) does not affect Arc.

  // Arc sweep is always less than 180 degrees. If radius is zero, or if
  // tangents are nearly parallel, arcTo appends Line from last Path Point to (x1, y1).

  // arcToTangent appends at most one Line and one conic.
  // arcToTangent implements the functionality of PostScript arct and HTML Canvas arcTo.
  CanvasKit.Path.prototype.arcToTangent = function(x1, y1, x2, y2, radius) {
    this._arcToTangent(x1, y1, x2, y2, radius);
    return this;
  };

  CanvasKit.Path.prototype.close = function() {
    this._close();
    return this;
  };

  CanvasKit.Path.prototype.conicTo = function(x1, y1, x2, y2, w) {
    this._conicTo(x1, y1, x2, y2, w);
    return this;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Path.prototype.computeTightBounds = function(optionalOutputArray) {
    this._computeTightBounds(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  CanvasKit.Path.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.Path.prototype.dash = function(on, off, phase) {
    if (this._dash(on, off, phase)) {
      return this;
    }
    return null;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Path.prototype.getBounds = function(optionalOutputArray) {
    this._getBounds(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  CanvasKit.Path.prototype.lineTo = function(x, y) {
    this._lineTo(x, y);
    return this;
  };

  CanvasKit.Path.prototype.moveTo = function(x, y) {
    this._moveTo(x, y);
    return this;
  };

  CanvasKit.Path.prototype.offset = function(dx, dy) {
    this._transform(1, 0, dx,
                    0, 1, dy,
                    0, 0, 1);
    return this;
  };

  CanvasKit.Path.prototype.quadTo = function(cpx, cpy, x, y) {
    this._quadTo(cpx, cpy, x, y);
    return this;
  };

 CanvasKit.Path.prototype.rArcTo = function(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy) {
    this._rArcTo(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy);
    return this;
  };

  CanvasKit.Path.prototype.rConicTo = function(dx1, dy1, dx2, dy2, w) {
    this._rConicTo(dx1, dy1, dx2, dy2, w);
    return this;
  };

  // These params are all relative
  CanvasKit.Path.prototype.rCubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._rCubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.Path.prototype.rLineTo = function(dx, dy) {
    this._rLineTo(dx, dy);
    return this;
  };

  CanvasKit.Path.prototype.rMoveTo = function(dx, dy) {
    this._rMoveTo(dx, dy);
    return this;
  };

  // These params are all relative
  CanvasKit.Path.prototype.rQuadTo = function(cpx, cpy, x, y) {
    this._rQuadTo(cpx, cpy, x, y);
    return this;
  };

  CanvasKit.Path.prototype.stroke = function(opts) {
    // Fill out any missing values with the default values.
    opts = opts || {};
    opts['width'] = opts['width'] || 1;
    opts['miter_limit'] = opts['miter_limit'] || 4;
    opts['cap'] = opts['cap'] || CanvasKit.StrokeCap.Butt;
    opts['join'] = opts['join'] || CanvasKit.StrokeJoin.Miter;
    opts['precision'] = opts['precision'] || 1;
    if (this._stroke(opts)) {
      return this;
    }
    return null;
  };

  // TODO(kjlubick) Change this to take a 3x3 or 4x4 matrix (optionally malloc'd)
  CanvasKit.Path.prototype.transform = function() {
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
  CanvasKit.Path.prototype.trim = function(startT, stopT, isComplement) {
    if (this._trim(startT, stopT, !!isComplement)) {
      return this;
    }
    return null;
  };

  // makeShaderCubic returns a shader for a given image, allowing it to be used on
  // a paint as well as other purposes. This shader will be higher quality than
  // other shader functions. See CubicResampler in SkSamplingOptions.h for more information
  // on the cubicResampler params.
  CanvasKit.Image.prototype.makeShaderCubic = function(xTileMode, yTileMode,
                                                       cubicResamplerB, cubicResamplerC,
                                                       localMatrix) {
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    return this._makeShaderCubic(xTileMode, yTileMode, cubicResamplerB,
                                 cubicResamplerC, localMatrixPtr);
  };

  // makeShaderCubic returns a shader for a given image, allowing it to be used on
  // a paint as well as other purposes. This shader will draw more quickly than
  // other shader functions, but at a lower quality.
  CanvasKit.Image.prototype.makeShaderOptions = function(xTileMode, yTileMode,
                                                         filterMode, mipmapMode,
                                                         localMatrix) {
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    return this._makeShaderOptions(xTileMode, yTileMode, filterMode, mipmapMode, localMatrixPtr);
  };

  function readPixels(source, srcX, srcY, imageInfo, destMallocObj, bytesPerRow) {
    if (!bytesPerRow) {
      bytesPerRow = 4 * imageInfo['width'];
      if (imageInfo['colorType'] === CanvasKit.ColorType.RGBA_F16) {
        bytesPerRow *= 2;
      }
      else if (imageInfo['colorType'] === CanvasKit.ColorType.RGBA_F32) {
        bytesPerRow *= 4;
      }
    }
    var pBytes = bytesPerRow * imageInfo.height;
    var pPtr;
    if (destMallocObj) {
      pPtr = destMallocObj['byteOffset'];
    } else {
      pPtr = CanvasKit._malloc(pBytes);
    }

    if (!source._readPixels(imageInfo, pPtr, bytesPerRow, srcX, srcY)) {
      Debug('Could not read pixels with the given inputs');
      if (!destMallocObj) {
        CanvasKit._free(pPtr);
      }
      return null;
    }

    // If the user provided us a buffer to copy into, we don't need to allocate a new TypedArray.
    if (destMallocObj) {
      return destMallocObj['toTypedArray'](); // Return the typed array wrapper w/o allocating.
    }

    // Put those pixels into a typed array of the right format and then
    // make a copy with slice() that we can return.
    var retVal = null;
    switch (imageInfo['colorType']) {
      case CanvasKit.ColorType.RGBA_8888:
      case CanvasKit.ColorType.RGBA_F16: // there is no half-float JS type, so we return raw bytes.
        retVal = new Uint8Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
      case CanvasKit.ColorType.RGBA_F32:
        retVal = new Float32Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
      default:
        Debug('ColorType not yet supported');
        return null;
    }

    // Free the allocated pixels in the WASM memory
    CanvasKit._free(pPtr);
    return retVal;
  }

  CanvasKit.Image.prototype.readPixels = function(srcX, srcY, imageInfo, destMallocObj,
                                                  bytesPerRow) {
    return readPixels(this, srcX, srcY, imageInfo, destMallocObj, bytesPerRow);
  };

  // Accepts an array of four numbers in the range of 0-1 representing a 4f color
  CanvasKit.Canvas.prototype.clear = function(color4f) {
    CanvasKit.setCurrentContext(this._context);
    var cPtr = copyColorToWasm(color4f);
    this._clear(cPtr);
  };

  CanvasKit.Canvas.prototype.clipRRect = function(rrect, op, antialias) {
    CanvasKit.setCurrentContext(this._context);
    var rPtr = copyRRectToWasm(rrect);
    this._clipRRect(rPtr, op, antialias);
  };

  CanvasKit.Canvas.prototype.clipRect = function(rect, op, antialias) {
    CanvasKit.setCurrentContext(this._context);
    var rPtr = copyRectToWasm(rect);
    this._clipRect(rPtr, op, antialias);
  };

  // concat takes a 3x2, a 3x3, or a 4x4 matrix and upscales it (if needed) to 4x4. This is because
  // under the hood, SkCanvas uses a 4x4 matrix.
  CanvasKit.Canvas.prototype.concat = function(matr) {
    CanvasKit.setCurrentContext(this._context);
    var matrPtr = copy4x4MatrixToWasm(matr);
    this._concat(matrPtr);
  };

  CanvasKit.Canvas.prototype.drawArc = function(oval, startAngle, sweepAngle, useCenter, paint) {
    CanvasKit.setCurrentContext(this._context);
    var oPtr = copyRectToWasm(oval);
    this._drawArc(oPtr, startAngle, sweepAngle, useCenter, paint);
  };

  // atlas is an Image, e.g. from CanvasKit.MakeImageFromEncoded
  // srcRects, dstXformsshould be arrays of floats of length 4*number of destinations.
  // The colors param is optional and is used to tint the drawn images using the optional blend
  // mode. Colors can be a Uint32Array of int colors or a flat Float32Array of float colors.
  CanvasKit.Canvas.prototype.drawAtlas = function(atlas, srcRects, dstXforms, paint,
                                       /* optional */ blendMode, /* optional */ colors,
                                       /* optional */ sampling) {
    if (!atlas || !paint || !srcRects || !dstXforms) {
      Debug('Doing nothing since missing a required input');
      return;
    }

    // builder arguments report the length as the number of rects, but when passed as arrays
    // their.length attribute is 4x higher because it's the number of total components of all rects.
    // colors is always going to report the same length, at least until floats colors are supported
    // by this function.
    if (srcRects.length !== dstXforms.length) {
      Debug('Doing nothing since input arrays length mismatches');
      return;
    }
    CanvasKit.setCurrentContext(this._context);
    if (!blendMode) {
      blendMode = CanvasKit.BlendMode.SrcOver;
    }

    var srcRectPtr = copy1dArray(srcRects, 'HEAPF32');

    var dstXformPtr = copy1dArray(dstXforms, 'HEAPF32');
    var count = dstXforms.length / 4;

    var colorPtr = copy1dArray(assureIntColors(colors), 'HEAPU32');

    // We require one of these:
    // 1. sampling is null (we default to linear/none)
    // 2. sampling.B and sampling.C --> CubicResampler
    // 3. sampling.filter [and sampling.mipmap] --> FilterOptions
    //
    // Thus if all fields are available, we will choose cubic (since we search for B,C first)

    if (sampling && ('B' in sampling) && ('C' in sampling)) {
        this._drawAtlasCubic(atlas, dstXformPtr, srcRectPtr, colorPtr, count, blendMode,
                             sampling['B'], sampling['C'], paint);
    } else {
        let filter = CanvasKit.FilterMode.Linear;
        let mipmap = CanvasKit.MipmapMode.None;
        if (sampling) {
            filter = sampling['filter'];    // 'filter' is a required field
            if ('mipmap' in sampling) {     // 'mipmap' is optional
                mipmap = sampling['mipmap'];
            }
        }
        this._drawAtlasOptions(atlas, dstXformPtr, srcRectPtr, colorPtr, count, blendMode,
                               filter, mipmap, paint);
    }

    freeArraysThatAreNotMallocedByUsers(srcRectPtr, srcRects);
    freeArraysThatAreNotMallocedByUsers(dstXformPtr, dstXforms);
    freeArraysThatAreNotMallocedByUsers(colorPtr, colors);
  };

  CanvasKit.Canvas.prototype.drawCircle = function(cx, cy, r, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawCircle(cx, cy, r, paint);
  }

  CanvasKit.Canvas.prototype.drawColor = function(color4f, mode) {
    CanvasKit.setCurrentContext(this._context);
    var cPtr = copyColorToWasm(color4f);
    if (mode !== undefined) {
      this._drawColor(cPtr, mode);
    } else {
      this._drawColor(cPtr);
    }
  };

  CanvasKit.Canvas.prototype.drawColorInt = function(color, mode) {
    CanvasKit.setCurrentContext(this._context);
    this._drawColorInt(color, mode || CanvasKit.BlendMode.SrcOver);
  }

  CanvasKit.Canvas.prototype.drawColorComponents = function(r, g, b, a, mode) {
    CanvasKit.setCurrentContext(this._context);
    var cPtr = copyColorComponentsToWasm(r, g, b, a);
    if (mode !== undefined) {
      this._drawColor(cPtr, mode);
    } else {
      this._drawColor(cPtr);
    }
  };

  CanvasKit.Canvas.prototype.drawDRRect = function(outer, inner, paint) {
    CanvasKit.setCurrentContext(this._context);
    var oPtr = copyRRectToWasm(outer, _scratchRRectPtr);
    var iPtr = copyRRectToWasm(inner, _scratchRRect2Ptr);
    this._drawDRRect(oPtr, iPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawGlyphs = function(glyphs, positions, x, y, font, paint) {
    if (!(glyphs.length*2 <= positions.length)) {
        throw 'Not enough positions for the array of gyphs';
    }
    CanvasKit.setCurrentContext(this._context);
    const glyphs_ptr    = copy1dArray(glyphs, 'HEAPU16');
    const positions_ptr = copy1dArray(positions, 'HEAPF32');

    this._drawGlyphs(glyphs.length, glyphs_ptr, positions_ptr, x, y, font, paint);

    freeArraysThatAreNotMallocedByUsers(positions_ptr, positions);
    freeArraysThatAreNotMallocedByUsers(glyphs_ptr,    glyphs);
  };

  CanvasKit.Canvas.prototype.drawImage = function(img, x, y, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawImage(img, x, y, paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageCubic = function(img, x, y, b, c, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawImageCubic(img, x, y, b, c, paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageOptions = function(img, x, y, filter, mipmap, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawImageOptions(img, x, y, filter, mipmap, paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageNine = function(img, center, dest, filter, paint) {
    CanvasKit.setCurrentContext(this._context);
    var cPtr = copyIRectToWasm(center);
    var dPtr = copyRectToWasm(dest);
    this._drawImageNine(img, cPtr, dPtr, filter, paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageRect = function(img, src, dest, paint, fastSample) {
    CanvasKit.setCurrentContext(this._context);
    copyRectToWasm(src,  _scratchFourFloatsAPtr);
    copyRectToWasm(dest, _scratchFourFloatsBPtr);
    this._drawImageRect(img, _scratchFourFloatsAPtr, _scratchFourFloatsBPtr, paint, !!fastSample);
  };

  CanvasKit.Canvas.prototype.drawImageRectCubic = function(img, src, dest, B, C, paint) {
    CanvasKit.setCurrentContext(this._context);
    copyRectToWasm(src,  _scratchFourFloatsAPtr);
    copyRectToWasm(dest, _scratchFourFloatsBPtr);
    this._drawImageRectCubic(img, _scratchFourFloatsAPtr, _scratchFourFloatsBPtr, B, C,
      paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageRectOptions = function(img, src, dest, filter, mipmap, paint) {
    CanvasKit.setCurrentContext(this._context);
    copyRectToWasm(src,  _scratchFourFloatsAPtr);
    copyRectToWasm(dest, _scratchFourFloatsBPtr);
    this._drawImageRectOptions(img, _scratchFourFloatsAPtr, _scratchFourFloatsBPtr, filter, mipmap,
      paint || null);
  };

  CanvasKit.Canvas.prototype.drawLine = function(x1, y1, x2, y2, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawLine(x1, y1, x2, y2, paint);
  }

  CanvasKit.Canvas.prototype.drawOval = function(oval, paint) {
    CanvasKit.setCurrentContext(this._context);
    var oPtr = copyRectToWasm(oval);
    this._drawOval(oPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawPaint = function(paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawPaint(paint);
  }

  CanvasKit.Canvas.prototype.drawParagraph = function(p, x, y) {
    CanvasKit.setCurrentContext(this._context);
    this._drawParagraph(p, x, y);
  }

  CanvasKit.Canvas.prototype.drawPatch = function(cubics, colors, texs, mode, paint) {
    if (cubics.length < 24) {
        throw 'Need 12 cubic points';
    }
    if (colors && colors.length < 4) {
        throw 'Need 4 colors';
    }
    if (texs && texs.length < 8) {
        throw 'Need 4 shader coordinates';
    }
    CanvasKit.setCurrentContext(this._context);

    const cubics_ptr =          copy1dArray(cubics, 'HEAPF32');
    const colors_ptr = colors ? copy1dArray(assureIntColors(colors), 'HEAPU32') : nullptr;
    const texs_ptr   = texs   ? copy1dArray(texs,   'HEAPF32') : nullptr;
    if (!mode) {
        mode = CanvasKit.BlendMode.Modulate;
    }

    this._drawPatch(cubics_ptr, colors_ptr, texs_ptr, mode, paint);

    freeArraysThatAreNotMallocedByUsers(texs_ptr,   texs);
    freeArraysThatAreNotMallocedByUsers(colors_ptr, colors);
    freeArraysThatAreNotMallocedByUsers(cubics_ptr, cubics);
  };

  CanvasKit.Canvas.prototype.drawPath = function(path, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawPath(path, paint);
  }

  CanvasKit.Canvas.prototype.drawPicture = function(pic) {
    CanvasKit.setCurrentContext(this._context);
    this._drawPicture(pic);
  }

  // points is a 1d array of length 2n representing n points where the even indices
  // will be treated as x coordinates and the odd indices will be treated as y coordinates.
  // Like other APIs, this accepts a malloced type array or malloc obj.
  CanvasKit.Canvas.prototype.drawPoints = function(mode, points, paint) {
    CanvasKit.setCurrentContext(this._context);
    var ptr = copy1dArray(points, 'HEAPF32');
    this._drawPoints(mode, ptr, points.length / 2, paint);
    freeArraysThatAreNotMallocedByUsers(ptr, points);
  };

  CanvasKit.Canvas.prototype.drawRRect = function(rrect, paint) {
    CanvasKit.setCurrentContext(this._context);
    var rPtr = copyRRectToWasm(rrect);
    this._drawRRect(rPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawRect = function(rect, paint) {
    CanvasKit.setCurrentContext(this._context);
    var rPtr = copyRectToWasm(rect);
    this._drawRect(rPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawRect4f = function(l, t, r, b, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawRect4f(l, t, r, b, paint);
  }

  CanvasKit.Canvas.prototype.drawShadow = function(path, zPlaneParams, lightPos, lightRadius,
                                                   ambientColor, spotColor, flags) {
    CanvasKit.setCurrentContext(this._context);
    var ambiPtr = copyColorToWasmNoScratch(ambientColor);
    var spotPtr = copyColorToWasmNoScratch(spotColor);
    // We use the return value from copy1dArray in case the passed in arrays are malloc'd.
    var zPlanePtr = copy1dArray(zPlaneParams, 'HEAPF32', _scratchThreeFloatsAPtr);
    var lightPosPtr = copy1dArray(lightPos, 'HEAPF32', _scratchThreeFloatsBPtr);
    this._drawShadow(path, zPlanePtr, lightPosPtr, lightRadius, ambiPtr, spotPtr, flags);
    freeArraysThatAreNotMallocedByUsers(ambiPtr, ambientColor);
    freeArraysThatAreNotMallocedByUsers(spotPtr, spotColor);
  };

  CanvasKit.getShadowLocalBounds = function(ctm, path, zPlaneParams, lightPos, lightRadius,
                                            flags, optOutputRect) {
    var ctmPtr = copy3x3MatrixToWasm(ctm);
    // We use the return value from copy1dArray in case the passed in arrays are malloc'd.
    var zPlanePtr = copy1dArray(zPlaneParams, 'HEAPF32', _scratchThreeFloatsAPtr);
    var lightPosPtr = copy1dArray(lightPos, 'HEAPF32', _scratchThreeFloatsBPtr);
    var ok = this._getShadowLocalBounds(ctmPtr, path, zPlanePtr, lightPosPtr, lightRadius,
                                        flags, _scratchFourFloatsAPtr);
    if (!ok) {
      return null;
    }
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optOutputRect) {
      optOutputRect.set(ta);
      return optOutputRect;
    }
    return ta.slice();
  };

  CanvasKit.Canvas.prototype.drawTextBlob = function(blob, x, y, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawTextBlob(blob, x, y, paint);
  }

  CanvasKit.Canvas.prototype.drawVertices = function(verts, mode, paint) {
    CanvasKit.setCurrentContext(this._context);
    this._drawVertices(verts, mode, paint);
  }

  // getLocalToDevice returns a 4x4 matrix.
  CanvasKit.Canvas.prototype.getLocalToDevice = function() {
    // _getLocalToDevice will copy the values into the pointer.
    this._getLocalToDevice(_scratch4x4MatrixPtr);
    return copy4x4MatrixFromWasm(_scratch4x4MatrixPtr);
  };

  // findMarkedCTM returns a 4x4 matrix, or null if a matrix was not found at
  // the provided marker.
  CanvasKit.Canvas.prototype.findMarkedCTM = function(marker) {
    // _getLocalToDevice will copy the values into the pointer.
    var found = this._findMarkedCTM(marker, _scratch4x4MatrixPtr);
    if (!found) {
      return null;
    }
    return copy4x4MatrixFromWasm(_scratch4x4MatrixPtr);
  };

  // getTotalMatrix returns the current matrix as a 3x3 matrix.
  CanvasKit.Canvas.prototype.getTotalMatrix = function() {
    // _getTotalMatrix will copy the values into the pointer.
    this._getTotalMatrix(_scratch3x3MatrixPtr);
    // read them out into an array. TODO(kjlubick): If we change Matrix to be
    // typedArrays, then we should return a typed array here too.
    var rv = new Array(9);
    for (var i = 0; i < 9; i++) {
      rv[i] = CanvasKit.HEAPF32[_scratch3x3MatrixPtr/4 + i]; // divide by 4 to "cast" to float.
    }
    return rv;
  };

  CanvasKit.Canvas.prototype.makeSurface = function(imageInfo) {
    var s = this._makeSurface(imageInfo);
    s._context = this._context;
    return s;
  };

  CanvasKit.Canvas.prototype.readPixels = function(srcX, srcY, imageInfo, destMallocObj,
                                                   bytesPerRow) {
    CanvasKit.setCurrentContext(this._context);
    return readPixels(this, srcX, srcY, imageInfo, destMallocObj, bytesPerRow);
  };

  CanvasKit.Canvas.prototype.saveLayer = function(paint, boundsRect, backdrop, flags) {
    // bPtr will be 0 (nullptr) if boundsRect is undefined/null.
    var bPtr = copyRectToWasm(boundsRect);
    // These or clauses help emscripten, which does not deal with undefined well.
    return this._saveLayer(paint || null, bPtr, backdrop || null, flags || 0);
  };

  // pixels should be a Uint8Array or a plain JS array.
  CanvasKit.Canvas.prototype.writePixels = function(pixels, srcWidth, srcHeight,
                                                      destX, destY, alphaType, colorType, colorSpace) {
    if (pixels.byteLength % (srcWidth * srcHeight)) {
      throw 'pixels length must be a multiple of the srcWidth * srcHeight';
    }
    CanvasKit.setCurrentContext(this._context);
    var bytesPerPixel = pixels.byteLength / (srcWidth * srcHeight);
    // supply defaults (which are compatible with HTMLCanvas's putImageData)
    alphaType = alphaType || CanvasKit.AlphaType.Unpremul;
    colorType = colorType || CanvasKit.ColorType.RGBA_8888;
    colorSpace = colorSpace || CanvasKit.ColorSpace.SRGB;
    var srcRowBytes = bytesPerPixel * srcWidth;

    var pptr = copy1dArray(pixels, 'HEAPU8');
    var ok = this._writePixels({
      'width': srcWidth,
      'height': srcHeight,
      'colorType': colorType,
      'alphaType': alphaType,
      'colorSpace': colorSpace,
    }, pptr, srcRowBytes, destX, destY);

    freeArraysThatAreNotMallocedByUsers(pptr, pixels);
    return ok;
  };

  CanvasKit.ColorFilter.MakeBlend = function(color4f, mode) {
    var cPtr = copyColorToWasm(color4f);
    return CanvasKit.ColorFilter._MakeBlend(cPtr, mode);
  };

  // colorMatrix is an ColorMatrix (e.g. Float32Array of length 20)
  CanvasKit.ColorFilter.MakeMatrix = function(colorMatrix) {
    if (!colorMatrix || colorMatrix.length !== 20) {
      throw 'invalid color matrix';
    }
    var fptr = copy1dArray(colorMatrix, 'HEAPF32');
    // We know skia memcopies the floats, so we can free our memory after the call returns.
    var m = CanvasKit.ColorFilter._makeMatrix(fptr);
    freeArraysThatAreNotMallocedByUsers(fptr, colorMatrix);
    return m;
  };

  CanvasKit.ContourMeasure.prototype.getPosTan = function(distance, optionalOutput) {
    this._getPosTan(distance, _scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutput) {
      optionalOutput.set(ta);
      return optionalOutput;
    }
    return ta.slice();
  };

  CanvasKit.ImageFilter.MakeMatrixTransform = function(matrix, sampling, input) {
    var matrPtr = copy3x3MatrixToWasm(matrix);

    if ('B' in sampling && 'C' in sampling) {
        return CanvasKit.ImageFilter._MakeMatrixTransformCubic(matrPtr,
                                                               sampling.B, sampling.C,
                                                               input);
    } else {
        const filter = sampling['filter'];  // 'filter' is a required field
        let mipmap = CanvasKit.MipmapMode.None;
        if ('mipmap' in sampling) {         // 'mipmap' is optional
            mipmap = sampling['mipmap'];
        }
        return CanvasKit.ImageFilter._MakeMatrixTransformOptions(matrPtr,
                                                                 filter, mipmap,
                                                                 input);
    }
  };

  CanvasKit.Paint.prototype.getColor = function() {
    this._getColor(_scratchColorPtr);
    return copyColorFromWasm(_scratchColorPtr);
  };

  CanvasKit.Paint.prototype.setColor = function(color4f, colorSpace) {
    colorSpace = colorSpace || null; // null will be replaced with sRGB in the C++ method.
    // emscripten wouldn't bind undefined to the sk_sp<ColorSpace> expected here.
    var cPtr = copyColorToWasm(color4f);
    this._setColor(cPtr, colorSpace);
  };

  // The color components here are expected to be floating point values (nominally between
  // 0.0 and 1.0, but with wider color gamuts, the values could exceed this range). To convert
  // between standard 8 bit colors and floats, just divide by 255 before passing them in.
  CanvasKit.Paint.prototype.setColorComponents = function(r, g, b, a, colorSpace) {
    colorSpace = colorSpace || null; // null will be replaced with sRGB in the C++ method.
    // emscripten wouldn't bind undefined to the sk_sp<ColorSpace> expected here.
    var cPtr = copyColorComponentsToWasm(r, g, b, a);
    this._setColor(cPtr, colorSpace);
  };

  CanvasKit.Path.prototype.getPoint = function(idx, optionalOutput) {
    // This will copy 2 floats into a space for 4 floats
    this._getPoint(idx, _scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutput) {
      // We cannot call optionalOutput.set() because it is an error to call .set() with
      // a source bigger than the destination.
      optionalOutput[0] = ta[0];
      optionalOutput[1] = ta[1];
      return optionalOutput;
    }
    // Be sure to return a copy of just the first 2 values.
    return ta.slice(0, 2);
  };

  CanvasKit.PictureRecorder.prototype.beginRecording = function(bounds) {
    var bPtr = copyRectToWasm(bounds);
    return this._beginRecording(bPtr);
  };

  CanvasKit.Surface.prototype.getCanvas = function() {
    var c = this._getCanvas();
    c._context = this._context;
    return c;
  };

  CanvasKit.Surface.prototype.makeImageSnapshot = function(optionalBoundsRect) {
    var bPtr = copyIRectToWasm(optionalBoundsRect);
    return this._makeImageSnapshot(bPtr);
  };

  CanvasKit.Surface.prototype.makeSurface = function(imageInfo) {
    var s = this._makeSurface(imageInfo);
    s._context = this._context;
    return s;
  };

  CanvasKit.Surface.prototype.requestAnimationFrame = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    requestAnimationFrame(function() {
      CanvasKit.setCurrentContext(this._context);

      callback(this._cached_canvas);

      // We do not dispose() of the Surface here, as the client will typically
      // call requestAnimationFrame again from within the supplied callback.
      // For drawing a single frame, prefer drawOnce().
      this.flush(dirtyRect);
    }.bind(this));
  };

  // drawOnce will dispose of the surface after drawing the frame using the provided
  // callback.
  CanvasKit.Surface.prototype.drawOnce = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    requestAnimationFrame(function() {
      CanvasKit.setCurrentContext(this._context);
      callback(this._cached_canvas);

      this.flush(dirtyRect);
      this.dispose();
    }.bind(this));
  };

  CanvasKit.PathEffect.MakeDash = function(intervals, phase) {
    if (!phase) {
      phase = 0;
    }
    if (!intervals.length || intervals.length % 2 === 1) {
      throw 'Intervals array must have even length';
    }
    var ptr = copy1dArray(intervals, 'HEAPF32');
    var dpe = CanvasKit.PathEffect._MakeDash(ptr, intervals.length, phase);
    freeArraysThatAreNotMallocedByUsers(ptr, intervals);
    return dpe;
  };

  CanvasKit.Shader.MakeColor = function(color4f, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtr = copyColorToWasm(color4f);
    return CanvasKit.Shader._MakeColor(cPtr, colorSpace);
  };

  // TODO(kjlubick) remove deprecated names.
  CanvasKit.Shader.Blend = CanvasKit.Shader.MakeBlend;
  CanvasKit.Shader.Color = CanvasKit.Shader.MakeColor;

  CanvasKit.Shader.MakeLinearGradient = function(start, end, colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    // Copy start and end to _scratchFourFloatsAPtr.
    var startEndPts = _scratchFourFloatsA['toTypedArray']();
    startEndPts.set(start);
    startEndPts.set(end, 2);

    var lgs = CanvasKit.Shader._MakeLinearGradient(_scratchFourFloatsAPtr, cPtrInfo.colorPtr, cPtrInfo.colorType, posPtr,
                                                   cPtrInfo.count, mode, flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return lgs;
  };

  CanvasKit.Shader.MakeRadialGradient = function(center, radius, colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var rgs = CanvasKit.Shader._MakeRadialGradient(center[0], center[1], radius, cPtrInfo.colorPtr,
                                                   cPtrInfo.colorType, posPtr, cPtrInfo.count, mode,
                                                   flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return rgs;
  };

  CanvasKit.Shader.MakeSweepGradient = function(cx, cy, colors, pos, mode, localMatrix, flags, startAngle, endAngle, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    startAngle = startAngle || 0;
    endAngle = endAngle || 360;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var sgs = CanvasKit.Shader._MakeSweepGradient(cx, cy, cPtrInfo.colorPtr, cPtrInfo.colorType, posPtr,
                                                  cPtrInfo.count, mode,
                                                  startAngle, endAngle, flags,
                                                  localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return sgs;
  };

  CanvasKit.Shader.MakeTwoPointConicalGradient = function(start, startRadius, end, endRadius,
                                                          colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr =   copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    // Copy start and end to _scratchFourFloatsAPtr.
    var startEndPts = _scratchFourFloatsA['toTypedArray']();
    startEndPts.set(start);
    startEndPts.set(end, 2);

    var rgs = CanvasKit.Shader._MakeTwoPointConicalGradient(_scratchFourFloatsAPtr,
                          startRadius, endRadius, cPtrInfo.colorPtr, cPtrInfo.colorType,
                          posPtr, cPtrInfo.count, mode, flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return rgs;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Vertices.prototype.bounds = function(optionalOutputArray) {
    this._bounds(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  // Run through the JS files that are added at compile time.
  if (CanvasKit._extraInitializations) {
    CanvasKit._extraInitializations.forEach(function(init) {
      init();
    });
  }
}; // end CanvasKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.

// Accepts an object holding two canvaskit colors.
// {
//    ambient: [r, g, b, a],
//    spot: [r, g, b, a],
// }
// Returns the same format. Note, if malloced colors are passed in, the memory
// housing the passed in colors passed in will be overwritten with the computed
// tonal colors.
CanvasKit.computeTonalColors = function(tonalColors) {
    // copy the colors into WASM
    var cPtrAmbi = copyColorToWasmNoScratch(tonalColors['ambient']);
    var cPtrSpot = copyColorToWasmNoScratch(tonalColors['spot']);
    // The output of this function will be the same pointers we passed in.
    this._computeTonalColors(cPtrAmbi, cPtrSpot);
    // Read the results out.
    var result =  {
      'ambient': copyColorFromWasm(cPtrAmbi),
      'spot': copyColorFromWasm(cPtrSpot),
    };
    // If the user passed us malloced colors in here, we don't want to clean them up.
    freeArraysThatAreNotMallocedByUsers(cPtrAmbi, tonalColors['ambient']);
    freeArraysThatAreNotMallocedByUsers(cPtrSpot, tonalColors['spot']);
    return result;
};

CanvasKit.LTRBRect = function(l, t, r, b) {
  return Float32Array.of(l, t, r, b);
};

CanvasKit.XYWHRect = function(x, y, w, h) {
  return Float32Array.of(x, y, x+w, y+h);
};

CanvasKit.LTRBiRect = function(l, t, r, b) {
  return Int32Array.of(l, t, r, b);
};

CanvasKit.XYWHiRect = function(x, y, w, h) {
  return Int32Array.of(x, y, x+w, y+h);
};

// RRectXY returns a TypedArray representing an RRect with the given rect and a radiusX and
// radiusY for all 4 corners.
CanvasKit.RRectXY = function(rect, rx, ry) {
  return Float32Array.of(
    rect[0], rect[1], rect[2], rect[3],
    rx, ry,
    rx, ry,
    rx, ry,
    rx, ry,
  );
};

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeAnimatedImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeAnimatedImage(iptr, data.byteLength);
  if (!img) {
    Debug('Could not decode animated image');
    return null;
  }
  return img;
};

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeImage(iptr, data.byteLength);
  if (!img) {
    Debug('Could not decode image');
    return null;
  }
  return img;
};

// A variable to hold a canvasElement which can be reused once created the first time.
var memoizedCanvas2dElement = null;

// Alternative to CanvasKit.MakeImageFromEncoded. Allows for CanvasKit users to take advantage of
// browser APIs to decode images instead of using codecs included in the CanvasKit wasm binary.
// Expects that the canvasImageSource has already loaded/decoded.
// CanvasImageSource reference: https://developer.mozilla.org/en-US/docs/Web/API/CanvasImageSource
CanvasKit.MakeImageFromCanvasImageSource = function(canvasImageSource) {
  var width = canvasImageSource.width;
  var height = canvasImageSource.height;

  if (!memoizedCanvas2dElement) {
    memoizedCanvas2dElement = document.createElement('canvas');
  }
  memoizedCanvas2dElement.width = width;
  memoizedCanvas2dElement.height = height;

  var ctx2d = memoizedCanvas2dElement.getContext('2d');
  ctx2d.drawImage(canvasImageSource, 0, 0);

  var imageData = ctx2d.getImageData(0, 0, width, height);

  return CanvasKit.MakeImage({
      'width': width,
      'height': height,
      'alphaType': CanvasKit.AlphaType.Unpremul,
      'colorType': CanvasKit.ColorType.RGBA_8888,
      'colorSpace': CanvasKit.ColorSpace.SRGB
    }, imageData.data, 4 * width);
};

// pixels may be an array but Uint8Array or Uint8ClampedArray is recommended,
// with the bytes representing the pixel values.
// (e.g. each set of 4 bytes could represent RGBA values for a single pixel).
CanvasKit.MakeImage = function(info, pixels, bytesPerRow) {
  var pptr = CanvasKit._malloc(pixels.length);
  CanvasKit.HEAPU8.set(pixels, pptr); // We always want to copy the bytes into the WASM heap.
  // No need to _free pptr, Image takes it with SkData::MakeFromMalloc
  return CanvasKit._MakeImage(info, pptr, pixels.length, bytesPerRow);
};

// Colors may be a Uint32Array of int colors, a Flat Float32Array of float colors
// or a 2d Array of Float32Array(4) (deprecated)
// the underlying Skia function accepts only int colors so it is recommended
// to pass an array of int colors to avoid an extra conversion.
CanvasKit.MakeVertices = function(mode, positions, textureCoordinates, colors,
                                  indices, isVolatile) {
  // Default isVolatile to true if not set
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
  if (!isVolatile) {
    flags |= (1 << 2);
  }

  var builder = new CanvasKit._VerticesBuilder(mode, positions.length / 2, idxCount, flags);

  copy1dArray(positions, 'HEAPF32', builder.positions());
  if (builder.texCoords()) {
    copy1dArray(textureCoordinates, 'HEAPF32', builder.texCoords());
  }
  if (builder.colors()) {
      copy1dArray(assureIntColors(colors), 'HEAPU32', builder.colors());
  }
  if (builder.indices()) {
    copy1dArray(indices, 'HEAPU16', builder.indices());
  }

  // Create the vertices, which owns the memory that the builder had allocated.
  return builder.detach();
};
