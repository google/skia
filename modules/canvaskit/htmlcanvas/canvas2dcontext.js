function CanvasRenderingContext2D(skcanvas) {
  this._canvas = skcanvas;
  this._paint = new CanvasKit.SkPaint();
  this._paint.setAntiAlias(true);

  this._paint.setStrokeMiter(10);
  this._paint.setStrokeCap(CanvasKit.StrokeCap.Butt);
  this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Miter);
  this._fontString = '10px monospace';

  this._font = new CanvasKit.SkFont(null, 10);

  this._strokeStyle    = CanvasKit.BLACK;
  this._fillStyle      = CanvasKit.BLACK;
  this._shadowBlur     = 0;
  this._shadowColor    = CanvasKit.TRANSPARENT;
  this._shadowOffsetX  = 0;
  this._shadowOffsetY  = 0;
  this._globalAlpha    = 1;
  this._strokeWidth    = 1;
  this._lineDashOffset = 0;
  this._lineDashList   = [];
  // aka SkBlendMode
  this._globalCompositeOperation = CanvasKit.BlendMode.SrcOver;
  this._imageFilterQuality = CanvasKit.FilterQuality.Low;
  this._imageSmoothingEnabled = true;


  this._paint.setStrokeWidth(this._strokeWidth);
  this._paint.setBlendMode(this._globalCompositeOperation);

  this._currentPath = new CanvasKit.SkPath();
  this._currentTransform = CanvasKit.SkMatrix.identity();

  // Use this for save/restore
  this._canvasStateStack = [];
  // Keep a reference to all the effects (e.g. gradients, patterns)
  // that were allocated for cleanup in _dispose.
  this._toCleanUp = [];

  this._dispose = function() {
    this._currentPath.delete();
    this._paint.delete();
    this._font.delete();
    this._toCleanUp.forEach(function(c) {
      c._dispose();
    });
    // Don't delete this._canvas as it will be disposed
    // by the surface of which it is based.
  }

  // This always accepts DOMMatrix/SVGMatrix or any other
  // object that has properties a,b,c,d,e,f defined.
  // Returns a DOM-Matrix like dictionary
  Object.defineProperty(this, 'currentTransform', {
    enumerable: true,
    get: function() {
      return {
        'a' : this._currentTransform[0],
        'c' : this._currentTransform[1],
        'e' : this._currentTransform[2],
        'b' : this._currentTransform[3],
        'd' : this._currentTransform[4],
        'f' : this._currentTransform[5],
      };
    },
    // @param {DOMMatrix} matrix
    set: function(matrix) {
      if (matrix.a) {
        // if we see a property named 'a', guess that b-f will
        // also be there.
        this.setTransform(matrix.a, matrix.b, matrix.c,
                          matrix.d, matrix.e, matrix.f);
      }
    }
  });

  Object.defineProperty(this, 'fillStyle', {
    enumerable: true,
    get: function() {
      if (Number.isInteger(this._fillStyle)) {
        return colorToString(this._fillStyle);
      }
      return this._fillStyle;
    },
    set: function(newStyle) {
      if (typeof newStyle === 'string') {
        this._fillStyle = parseColor(newStyle);
      } else if (newStyle._getShader) {
        // It's an effect that has a shader.
        this._fillStyle = newStyle
      }
    }
  });

  Object.defineProperty(this, 'font', {
    enumerable: true,
    get: function() {
      return this._fontString;
    },
    set: function(newFont) {
      var tf = getTypeface(newFont);
      if (tf) {
        // tf is a "dict" according to closure, that is, the field
        // names are not minified. Thus, we need to access it via
        // bracket notation to tell closure not to minify these names.
        this._font.setSize(tf['sizePx']);
        this._font.setTypeface(tf['typeface']);
        this._fontString = newFont;
      }
    }
  });

  Object.defineProperty(this, 'globalAlpha', {
    enumerable: true,
    get: function() {
      return this._globalAlpha;
    },
    set: function(newAlpha) {
      // ignore invalid values, as per the spec
      if (!isFinite(newAlpha) || newAlpha < 0 || newAlpha > 1) {
        return;
      }
      this._globalAlpha = newAlpha;
    }
  });

  Object.defineProperty(this, 'globalCompositeOperation', {
    enumerable: true,
    get: function() {
      switch (this._globalCompositeOperation) {
        // composite-mode
        case CanvasKit.BlendMode.SrcOver:
          return 'source-over';
        case CanvasKit.BlendMode.DstOver:
          return 'destination-over';
        case CanvasKit.BlendMode.Src:
          return 'copy';
        case CanvasKit.BlendMode.Dst:
          return 'destination';
        case CanvasKit.BlendMode.Clear:
          return 'clear';
        case CanvasKit.BlendMode.SrcIn:
          return 'source-in';
        case CanvasKit.BlendMode.DstIn:
          return 'destination-in';
        case CanvasKit.BlendMode.SrcOut:
          return 'source-out';
        case CanvasKit.BlendMode.DstOut:
          return 'destination-out';
        case CanvasKit.BlendMode.SrcATop:
          return 'source-atop';
        case CanvasKit.BlendMode.DstATop:
          return 'destination-atop';
        case CanvasKit.BlendMode.Xor:
          return 'xor';
        case CanvasKit.BlendMode.Plus:
          return 'lighter';

        case CanvasKit.BlendMode.Multiply:
          return 'multiply';
        case CanvasKit.BlendMode.Screen:
          return 'screen';
        case CanvasKit.BlendMode.Overlay:
          return 'overlay';
        case CanvasKit.BlendMode.Darken:
          return 'darken';
        case CanvasKit.BlendMode.Lighten:
          return 'lighten';
        case CanvasKit.BlendMode.ColorDodge:
          return 'color-dodge';
        case CanvasKit.BlendMode.ColorBurn:
          return 'color-burn';
        case CanvasKit.BlendMode.HardLight:
          return 'hard-light';
        case CanvasKit.BlendMode.SoftLight:
          return 'soft-light';
        case CanvasKit.BlendMode.Difference:
          return 'difference';
        case CanvasKit.BlendMode.Exclusion:
          return 'exclusion';
        case CanvasKit.BlendMode.Hue:
          return 'hue';
        case CanvasKit.BlendMode.Saturation:
          return 'saturation';
        case CanvasKit.BlendMode.Color:
          return 'color';
        case CanvasKit.BlendMode.Luminosity:
          return 'luminosity';
      }
    },
    set: function(newMode) {
      switch (newMode) {
        // composite-mode
        case 'source-over':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcOver;
          break;
        case 'destination-over':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstOver;
          break;
        case 'copy':
          this._globalCompositeOperation = CanvasKit.BlendMode.Src;
          break;
        case 'destination':
          this._globalCompositeOperation = CanvasKit.BlendMode.Dst;
          break;
        case 'clear':
          this._globalCompositeOperation = CanvasKit.BlendMode.Clear;
          break;
        case 'source-in':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcIn;
          break;
        case 'destination-in':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstIn;
          break;
        case 'source-out':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcOut;
          break;
        case 'destination-out':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstOut;
          break;
        case 'source-atop':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcATop;
          break;
        case 'destination-atop':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstATop;
          break;
        case 'xor':
          this._globalCompositeOperation = CanvasKit.BlendMode.Xor;
          break;
        case 'lighter':
          this._globalCompositeOperation = CanvasKit.BlendMode.Plus;
          break;
        case 'plus-lighter':
          this._globalCompositeOperation = CanvasKit.BlendMode.Plus;
          break;
        case 'plus-darker':
          throw 'plus-darker is not supported';

        // blend-mode
        case 'multiply':
          this._globalCompositeOperation = CanvasKit.BlendMode.Multiply;
          break;
        case 'screen':
          this._globalCompositeOperation = CanvasKit.BlendMode.Screen;
          break;
        case 'overlay':
          this._globalCompositeOperation = CanvasKit.BlendMode.Overlay;
          break;
        case 'darken':
          this._globalCompositeOperation = CanvasKit.BlendMode.Darken;
          break;
        case 'lighten':
          this._globalCompositeOperation = CanvasKit.BlendMode.Lighten;
          break;
        case 'color-dodge':
          this._globalCompositeOperation = CanvasKit.BlendMode.ColorDodge;
          break;
        case 'color-burn':
          this._globalCompositeOperation = CanvasKit.BlendMode.ColorBurn;
          break;
        case 'hard-light':
          this._globalCompositeOperation = CanvasKit.BlendMode.HardLight;
          break;
        case 'soft-light':
          this._globalCompositeOperation = CanvasKit.BlendMode.SoftLight;
          break;
        case 'difference':
          this._globalCompositeOperation = CanvasKit.BlendMode.Difference;
          break;
        case 'exclusion':
          this._globalCompositeOperation = CanvasKit.BlendMode.Exclusion;
          break;
        case 'hue':
          this._globalCompositeOperation = CanvasKit.BlendMode.Hue;
          break;
        case 'saturation':
          this._globalCompositeOperation = CanvasKit.BlendMode.Saturation;
          break;
        case 'color':
          this._globalCompositeOperation = CanvasKit.BlendMode.Color;
          break;
        case 'luminosity':
          this._globalCompositeOperation = CanvasKit.BlendMode.Luminosity;
          break;
        default:
          return;
      }
      this._paint.setBlendMode(this._globalCompositeOperation);
    }
  });

  Object.defineProperty(this, 'imageSmoothingEnabled', {
    enumerable: true,
    get: function() {
      return this._imageSmoothingEnabled;
    },
    set: function(newVal) {
      this._imageSmoothingEnabled = !!newVal;
    }
  });

  Object.defineProperty(this, 'imageSmoothingQuality', {
    enumerable: true,
    get: function() {
      switch (this._imageFilterQuality) {
        case CanvasKit.FilterQuality.Low:
          return 'low';
        case CanvasKit.FilterQuality.Medium:
          return 'medium';
        case CanvasKit.FilterQuality.High:
          return 'high';
      }
    },
    set: function(newQuality) {
      switch (newQuality) {
        case 'low':
          this._imageFilterQuality = CanvasKit.FilterQuality.Low;
          return;
        case 'medium':
          this._imageFilterQuality = CanvasKit.FilterQuality.Medium;
          return;
        case 'high':
          this._imageFilterQuality = CanvasKit.FilterQuality.High;
          return;
      }
    }
  });

  Object.defineProperty(this, 'lineCap', {
    enumerable: true,
    get: function() {
      switch (this._paint.getStrokeCap()) {
        case CanvasKit.StrokeCap.Butt:
          return 'butt';
        case CanvasKit.StrokeCap.Round:
          return 'round';
        case CanvasKit.StrokeCap.Square:
          return 'square';
      }
    },
    set: function(newCap) {
      switch (newCap) {
        case 'butt':
          this._paint.setStrokeCap(CanvasKit.StrokeCap.Butt);
          return;
        case 'round':
          this._paint.setStrokeCap(CanvasKit.StrokeCap.Round);
          return;
        case 'square':
          this._paint.setStrokeCap(CanvasKit.StrokeCap.Square);
          return;
      }
    }
  });

  Object.defineProperty(this, 'lineDashOffset', {
    enumerable: true,
    get: function() {
      return this._lineDashOffset;
    },
    set: function(newOffset) {
      if (!isFinite(newOffset)) {
        return;
      }
      this._lineDashOffset = newOffset;
    }
  });

  Object.defineProperty(this, 'lineJoin', {
    enumerable: true,
    get: function() {
      switch (this._paint.getStrokeJoin()) {
        case CanvasKit.StrokeJoin.Miter:
          return 'miter';
        case CanvasKit.StrokeJoin.Round:
          return 'round';
        case CanvasKit.StrokeJoin.Bevel:
          return 'bevel';
      }
    },
    set: function(newJoin) {
      switch (newJoin) {
        case 'miter':
          this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Miter);
          return;
        case 'round':
          this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Round);
          return;
        case 'bevel':
          this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Bevel);
          return;
      }
    }
  });

  Object.defineProperty(this, 'lineWidth', {
    enumerable: true,
    get: function() {
      return this._paint.getStrokeWidth();
    },
    set: function(newWidth) {
      if (newWidth <= 0 || !newWidth) {
        // Spec says to ignore NaN/Inf/0/negative values
        return;
      }
      this._strokeWidth = newWidth;
      this._paint.setStrokeWidth(newWidth);
    }
  });

  Object.defineProperty(this, 'miterLimit', {
    enumerable: true,
    get: function() {
      return this._paint.getStrokeMiter();
    },
    set: function(newLimit) {
      if (newLimit <= 0 || !newLimit) {
        // Spec says to ignore NaN/Inf/0/negative values
        return;
      }
      this._paint.setStrokeMiter(newLimit);
    }
  });

  Object.defineProperty(this, 'shadowBlur', {
    enumerable: true,
    get: function() {
      return this._shadowBlur;
    },
    set: function(newBlur) {
      // ignore negative, inf and NAN (but not 0) as per the spec.
      if (newBlur < 0 || !isFinite(newBlur)) {
        return;
      }
      this._shadowBlur = newBlur;
    }
  });

  Object.defineProperty(this, 'shadowColor', {
    enumerable: true,
    get: function() {
      return colorToString(this._shadowColor);
    },
    set: function(newColor) {
      this._shadowColor = parseColor(newColor);
    }
  });

  Object.defineProperty(this, 'shadowOffsetX', {
    enumerable: true,
    get: function() {
      return this._shadowOffsetX;
    },
    set: function(newOffset) {
      if (!isFinite(newOffset)) {
        return;
      }
      this._shadowOffsetX = newOffset;
    }
  });

  Object.defineProperty(this, 'shadowOffsetY', {
    enumerable: true,
    get: function() {
      return this._shadowOffsetY;
    },
    set: function(newOffset) {
      if (!isFinite(newOffset)) {
        return;
      }
      this._shadowOffsetY = newOffset;
    }
  });

  Object.defineProperty(this, 'strokeStyle', {
    enumerable: true,
    get: function() {
      return colorToString(this._strokeStyle);
    },
    set: function(newStyle) {
      if (typeof newStyle === 'string') {
        this._strokeStyle = parseColor(newStyle);
      } else if (newStyle._getShader) {
        // It's probably an effect.
        this._strokeStyle = newStyle
      }
    }
  });

  this.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    arc(this._currentPath, x, y, radius, startAngle, endAngle, ccw);
  }

  this.arcTo = function(x1, y1, x2, y2, radius) {
    arcTo(this._currentPath, x1, y1, x2, y2, radius);
  }

  // As per the spec this doesn't begin any paths, it only
  // clears out any previous paths.
  this.beginPath = function() {
    this._currentPath.delete();
    this._currentPath = new CanvasKit.SkPath();
  }

  this.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    bezierCurveTo(this._currentPath, cp1x, cp1y, cp2x, cp2y, x, y);
  }

  this.clearRect = function(x, y, width, height) {
    this._paint.setStyle(CanvasKit.PaintStyle.Fill);
    this._paint.setBlendMode(CanvasKit.BlendMode.Clear);
    this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), this._paint);
    this._paint.setBlendMode(this._globalCompositeOperation);
  }

  this.clip = function(path, fillRule) {
    if (typeof path === 'string') {
      // shift the args if a Path2D is supplied
      fillRule = path;
      path = this._currentPath;
    } else if (path && path._getPath) {
      path = path._getPath();
    }
    if (!path) {
      path = this._currentPath;
    }

    var clip = path.copy();
    if (fillRule && fillRule.toLowerCase() === 'evenodd') {
      clip.setFillType(CanvasKit.FillType.EvenOdd);
    } else {
      clip.setFillType(CanvasKit.FillType.Winding);
    }
    this._canvas.clipPath(clip, CanvasKit.ClipOp.Intersect, true);
    clip.delete();
  }

  this.closePath = function() {
    closePath(this._currentPath);
  }

  this.createImageData = function() {
    // either takes in 1 or 2 arguments:
    //  - imagedata on which to copy *width* and *height* only
    //  - width, height
    if (arguments.length === 1) {
      var oldData = arguments[0];
      var byteLength = 4 * oldData.width * oldData.height;
      return new ImageData(new Uint8ClampedArray(byteLength),
                           oldData.width, oldData.height);
    } else if (arguments.length === 2) {
      var width = arguments[0];
      var height = arguments[1];
      var byteLength = 4 * width * height;
      return new ImageData(new Uint8ClampedArray(byteLength),
                           width, height);
    } else {
      throw 'createImageData expects 1 or 2 arguments, got '+arguments.length;
    }
  }

  this.createLinearGradient = function(x1, y1, x2, y2) {
    if (!allAreFinite(arguments)) {
      return;
    }
    var lcg = new LinearCanvasGradient(x1, y1, x2, y2);
    this._toCleanUp.push(lcg);
    return lcg;
  }

  this.createPattern = function(image, repetition) {
    var cp = new CanvasPattern(image, repetition);
    this._toCleanUp.push(cp);
    return cp;
  }

  this.createRadialGradient = function(x1, y1, r1, x2, y2, r2) {
    if (!allAreFinite(arguments)) {
      return;
    }
    var rcg = new RadialCanvasGradient(x1, y1, r1, x2, y2, r2);
    this._toCleanUp.push(rcg);
    return rcg;
  }

  this._imagePaint = function() {
    var iPaint = this._fillPaint();
    if (!this._imageSmoothingEnabled) {
      iPaint.setFilterQuality(CanvasKit.FilterQuality.None);
    } else {
      iPaint.setFilterQuality(this._imageFilterQuality);
    }
    return iPaint;
  }

  this.drawImage = function(img) {
    // 3 potential sets of arguments
    // - image, dx, dy
    // - image, dx, dy, dWidth, dHeight
    // - image, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight
    // use the fillPaint, which has the globalAlpha in it
    // which drawImageRect will use.
    var iPaint = this._imagePaint();
    if (arguments.length === 3 || arguments.length === 5) {
      var destRect = CanvasKit.XYWHRect(arguments[1], arguments[2],
                        arguments[3] || img.width(), arguments[4] || img.height());
      var srcRect = CanvasKit.XYWHRect(0, 0, img.width(), img.height());
    } else if (arguments.length === 9){
      var destRect = CanvasKit.XYWHRect(arguments[5], arguments[6],
                                        arguments[7], arguments[8]);
      var srcRect = CanvasKit.XYWHRect(arguments[1], arguments[2],
                                       arguments[3], arguments[4]);
    } else {
      throw 'invalid number of args for drawImage, need 3, 5, or 9; got '+ arguments.length;
    }
    this._canvas.drawImageRect(img, srcRect, destRect, iPaint, false);

    iPaint.dispose();
  }

  this.ellipse = function(x, y, radiusX, radiusY, rotation,
                          startAngle, endAngle, ccw) {
    ellipse(this._currentPath, x, y, radiusX, radiusY, rotation,
            startAngle, endAngle, ccw);
  }

  // A helper to copy the current paint, ready for filling
  // This applies the global alpha.
  // Call dispose() after to clean up.
  this._fillPaint = function() {
    var paint = this._paint.copy();
    paint.setStyle(CanvasKit.PaintStyle.Fill);
    if (Number.isInteger(this._fillStyle)) {
      var alphaColor = CanvasKit.multiplyByAlpha(this._fillStyle, this._globalAlpha);
      paint.setColor(alphaColor);
    } else {
      var shader = this._fillStyle._getShader(this._currentTransform);
      paint.setColor(CanvasKit.Color(0,0,0, this._globalAlpha));
      paint.setShader(shader);
    }

    paint.dispose = function() {
      // If there are some helper effects in the future, clean them up
      // here. In any case, we have .dispose() to make _fillPaint behave
      // like _strokePaint and _shadowPaint.
      this.delete();
    }
    return paint;
  }

  this.fill = function(path, fillRule) {
    if (typeof path === 'string') {
      // shift the args if a Path2D is supplied
      fillRule = path;
      path = this._currentPath;
    } else if (path && path._getPath) {
      path = path._getPath();
    }
    if (fillRule === 'evenodd') {
      this._currentPath.setFillType(CanvasKit.FillType.EvenOdd);
    } else if (fillRule === 'nonzero' || !fillRule) {
      this._currentPath.setFillType(CanvasKit.FillType.Winding);
    } else {
      throw 'invalid fill rule';
    }
    if (!path) {
      path = this._currentPath;
    }

    var fillPaint = this._fillPaint();

    var shadowPaint = this._shadowPaint(fillPaint);
    if (shadowPaint) {
      this._canvas.save();
      this._canvas.concat(this._shadowOffsetMatrix());
      this._canvas.drawPath(path, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }
    this._canvas.drawPath(path, fillPaint);
    fillPaint.dispose();
  }

  this.fillRect = function(x, y, width, height) {
    var fillPaint = this._fillPaint();
    this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), fillPaint);
    fillPaint.dispose();
  }

  this.fillText = function(text, x, y, maxWidth) {
    // TODO do something with maxWidth, probably involving measure
    var fillPaint = this._fillPaint();
    var blob = CanvasKit.SkTextBlob.MakeFromText(text, this._font);

    var shadowPaint = this._shadowPaint(fillPaint);
    if (shadowPaint) {
      this._canvas.save();
      this._canvas.concat(this._shadowOffsetMatrix());
      this._canvas.drawTextBlob(blob, x, y, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }
    this._canvas.drawTextBlob(blob, x, y, fillPaint);
    blob.delete();
    fillPaint.dispose();
  }

  this.getImageData = function(x, y, w, h) {
    var pixels = this._canvas.readPixels(x, y, w, h);
    if (!pixels) {
      return null;
    }
    // This essentially re-wraps the pixels from a Uint8Array to
    // a Uint8ClampedArray (without making a copy of pixels).
    return new ImageData(
      new Uint8ClampedArray(pixels.buffer),
      w, h);
  }

  this.getLineDash = function() {
    return this._lineDashList.slice();
  }

  this._mapToLocalCoordinates = function(pts) {
    var inverted = CanvasKit.SkMatrix.invert(this._currentTransform);
    CanvasKit.SkMatrix.mapPoints(inverted, pts);
    return pts;
  }

  this.isPointInPath = function(x, y, fillmode) {
    var args = arguments;
    if (args.length === 3) {
      var path = this._currentPath;
    } else if (args.length === 4) {
      var path = args[0];
      x = args[1];
      y = args[2];
      fillmode = args[3];
    } else {
      throw 'invalid arg count, need 3 or 4, got ' + args.length;
    }
    if (!isFinite(x) || !isFinite(y)) {
      return false;
    }
    fillmode = fillmode || 'nonzero';
    if (!(fillmode === 'nonzero' || fillmode === 'evenodd')) {
      return false;
    }
    // x and y are in canvas coordinates (i.e. unaffected by CTM)
    var pts = this._mapToLocalCoordinates([x, y]);
    x = pts[0];
    y = pts[1];
    path.setFillType(fillmode === 'nonzero' ?
                                  CanvasKit.FillType.Winding :
                                  CanvasKit.FillType.EvenOdd);
    return path.contains(x, y);
  }

  this.isPointInStroke = function(x, y) {
    var args = arguments;
    if (args.length === 2) {
      var path = this._currentPath;
    } else if (args.length === 3) {
      var path = args[0];
      x = args[1];
      y = args[2];
    } else {
      throw 'invalid arg count, need 2 or 3, got ' + args.length;
    }
    if (!isFinite(x) || !isFinite(y)) {
      return false;
    }
    var pts = this._mapToLocalCoordinates([x, y]);
    x = pts[0];
    y = pts[1];
    var temp = path.copy();
    // fillmode is always nonzero
    temp.setFillType(CanvasKit.FillType.Winding);
    temp.stroke({'width': this.lineWidth, 'miter_limit': this.miterLimit,
                 'cap': this._paint.getStrokeCap(), 'join': this._paint.getStrokeJoin(),
                 'precision': 0.3, // this is what Chrome uses to compute this
                });
    var retVal = temp.contains(x, y);
    temp.delete();
    return retVal;
  }

  this.lineTo = function(x, y) {
    lineTo(this._currentPath, x, y);
  }

  this.measureText = function(text) {
    return {
      width: this._font.measureText(text),
      // TODO other measurements?
    }
  }

  this.moveTo = function(x, y) {
    moveTo(this._currentPath, x, y);
  }

  this.putImageData = function(imageData, x, y, dirtyX, dirtyY, dirtyWidth, dirtyHeight) {
    if (!allAreFinite([x, y, dirtyX, dirtyY, dirtyWidth, dirtyHeight])) {
      return;
    }
    if (dirtyX === undefined) {
      // fast, simple path for basic call
      this._canvas.writePixels(imageData.data, imageData.width, imageData.height, x, y);
      return;
    }
    dirtyX = dirtyX || 0;
    dirtyY = dirtyY || 0;
    dirtyWidth = dirtyWidth || imageData.width;
    dirtyHeight = dirtyHeight || imageData.height;

    // as per https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-putimagedata
    if (dirtyWidth < 0) {
      dirtyX = dirtyX+dirtyWidth;
      dirtyWidth = Math.abs(dirtyWidth);
    }
    if (dirtyHeight < 0) {
      dirtyY = dirtyY+dirtyHeight;
      dirtyHeight = Math.abs(dirtyHeight);
    }
    if (dirtyX < 0) {
      dirtyWidth = dirtyWidth + dirtyX;
      dirtyX = 0;
    }
    if (dirtyY < 0) {
      dirtyHeight = dirtyHeight + dirtyY;
      dirtyY = 0;
    }
    if (dirtyWidth <= 0 || dirtyHeight <= 0) {
      return;
    }
    var img = CanvasKit.MakeImage(imageData.data, imageData.width, imageData.height,
                                  CanvasKit.AlphaType.Unpremul,
                                  CanvasKit.ColorType.RGBA_8888);
    var src = CanvasKit.XYWHRect(dirtyX, dirtyY, dirtyWidth, dirtyHeight);
    var dst = CanvasKit.XYWHRect(x+dirtyX, y+dirtyY, dirtyWidth, dirtyHeight);
    var inverted = CanvasKit.SkMatrix.invert(this._currentTransform);
    this._canvas.save();
    // putImageData() operates in device space.
    this._canvas.concat(inverted);
    this._canvas.drawImageRect(img, src, dst, null, false);
    this._canvas.restore();
    img.delete();
  }

  this.quadraticCurveTo = function(cpx, cpy, x, y) {
    quadraticCurveTo(this._currentPath, cpx, cpy, x, y);
  }

  this.rect = function(x, y, width, height) {
    rect(this._currentPath, x, y, width, height);
  }

  this.resetTransform = function() {
    // Apply the current transform to the path and then reset
    // to the identity. Essentially "commit" the transform.
    this._currentPath.transform(this._currentTransform);
    var inverted = CanvasKit.SkMatrix.invert(this._currentTransform);
    this._canvas.concat(inverted);
    // This should be identity, modulo floating point drift.
    this._currentTransform = this._canvas.getTotalMatrix();
  }

  this.restore = function() {
    var newState = this._canvasStateStack.pop();
    if (!newState) {
      return;
    }
    // "commit" the current transform. We pop, then apply the inverse of the
    // popped state, which has the effect of applying just the delta of
    // transforms between old and new.
    var combined = CanvasKit.SkMatrix.multiply(
      this._currentTransform,
      CanvasKit.SkMatrix.invert(newState.ctm)
    );
    this._currentPath.transform(combined);
    this._paint.delete();
    this._paint = newState.paint;

    this._lineDashList = newState.ldl;
    this._strokeWidth = newState.sw;
    this._strokeStyle = newState.ss;
    this._fillStyle = newState.fs;
    this._shadowOffsetX = newState.sox;
    this._shadowOffsetY = newState.soy;
    this._shadowBlur = newState.sb;
    this._shadowColor = newState.shc;
    this._globalAlpha = newState.ga;
    this._globalCompositeOperation = newState.gco;
    this._lineDashOffset = newState.ldo;
    this._imageSmoothingEnabled = newState.ise;
    this._imageFilterQuality = newState.isq;
    this._fontString = newState.fontstr;

    //TODO: textAlign, textBaseline

    // restores the clip and ctm
    this._canvas.restore();
    this._currentTransform = this._canvas.getTotalMatrix();
  }

  this.rotate = function(radians) {
    if (!isFinite(radians)) {
      return;
    }
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.SkMatrix.rotated(-radians);
    this._currentPath.transform(inverted);
    this._canvas.rotate(radiansToDegrees(radians), 0, 0);
    this._currentTransform = this._canvas.getTotalMatrix();
  }

  this.save = function() {
    if (this._fillStyle._copy) {
      var fs = this._fillStyle._copy();
      this._toCleanUp.push(fs);
    } else {
      var fs = this._fillStyle;
    }

    if (this._strokeStyle._copy) {
      var ss = this._strokeStyle._copy();
      this._toCleanUp.push(ss);
    } else {
      var ss = this._strokeStyle;
    }

    this._canvasStateStack.push({
      ctm:     this._currentTransform.slice(),
      ldl:     this._lineDashList.slice(),
      sw:      this._strokeWidth,
      ss:      ss,
      fs:      fs,
      sox:     this._shadowOffsetX,
      soy:     this._shadowOffsetY,
      sb:      this._shadowBlur,
      shc:     this._shadowColor,
      ga:      this._globalAlpha,
      ldo:     this._lineDashOffset,
      gco:     this._globalCompositeOperation,
      ise:     this._imageSmoothingEnabled,
      isq:     this._imageFilterQuality,
      paint:   this._paint.copy(),
      fontstr: this._fontString,
      //TODO: textAlign, textBaseline
    });
    // Saves the clip
    this._canvas.save();
  }

  this.scale = function(sx, sy) {
    if (!allAreFinite(arguments)) {
      return;
    }
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.SkMatrix.scaled(1/sx, 1/sy);
    this._currentPath.transform(inverted);
    this._canvas.scale(sx, sy);
    this._currentTransform = this._canvas.getTotalMatrix();
  }

  this.setLineDash = function(dashes) {
    for (var i = 0; i < dashes.length; i++) {
      if (!isFinite(dashes[i]) || dashes[i] < 0) {
        SkDebug('dash list must have positive, finite values');
        return;
      }
    }
    if (dashes.length % 2 === 1) {
      // as per the spec, concatenate 2 copies of dashes
      // to give it an even number of elements.
      Array.prototype.push.apply(dashes, dashes);
    }
    this._lineDashList = dashes;
  }

  this.setTransform = function(a, b, c, d, e, f) {
    if (!(allAreFinite(arguments))) {
      return;
    }
    this.resetTransform();
    this.transform(a, b, c, d, e, f);
  }

  // Returns the matrix representing the offset of the shadows. This unapplies
  // the effects of the scale, which should not affect the shadow offsets.
  this._shadowOffsetMatrix = function() {
    var sx = this._currentTransform[0];
    var sy = this._currentTransform[4];
    return CanvasKit.SkMatrix.translated(this._shadowOffsetX/sx, this._shadowOffsetY/sy);
  }

  // Returns the shadow paint for the current settings or null if there
  // should be no shadow. This ends up being a copy of the given
  // paint with a blur maskfilter and the correct color.
  this._shadowPaint = function(basePaint) {
    // multiply first to see if the alpha channel goes to 0 after multiplication.
    var alphaColor = CanvasKit.multiplyByAlpha(this._shadowColor, this._globalAlpha);
    // if alpha is zero, no shadows
    if (!CanvasKit.getColorComponents(alphaColor)[3]) {
      return null;
    }
    // one of these must also be non-zero (otherwise the shadow is
    // completely hidden.  And the spec says so).
    if (!(this._shadowBlur || this._shadowOffsetY || this._shadowOffsetX)) {
      return null;
    }
    var shadowPaint = basePaint.copy();
    shadowPaint.setColor(alphaColor);
    var blurEffect = CanvasKit.MakeBlurMaskFilter(CanvasKit.BlurStyle.Normal,
      SkBlurRadiusToSigma(this._shadowBlur),
      false);
    shadowPaint.setMaskFilter(blurEffect);

    // hack up a "destructor" which also cleans up the blurEffect. Otherwise,
    // we leak the blurEffect (since smart pointers don't help us in JS land).
    shadowPaint.dispose = function() {
      blurEffect.delete();
      this.delete();
    };
    return shadowPaint;
  }

  // A helper to get a copy of the current paint, ready for stroking.
  // This applies the global alpha and the dashedness.
  // Call dispose() after to clean up.
  this._strokePaint = function() {
    var paint = this._paint.copy();
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    if (Number.isInteger(this._strokeStyle)) {
      var alphaColor = CanvasKit.multiplyByAlpha(this._strokeStyle, this._globalAlpha);
      paint.setColor(alphaColor);
    } else {
      var shader = this._strokeStyle._getShader(this._currentTransform);
      paint.setColor(CanvasKit.Color(0,0,0, this._globalAlpha));
      paint.setShader(shader);
    }

    paint.setStrokeWidth(this._strokeWidth);

    if (this._lineDashList.length) {
      var dashedEffect = CanvasKit.MakeSkDashPathEffect(this._lineDashList, this._lineDashOffset);
      paint.setPathEffect(dashedEffect);
    }

    paint.dispose = function() {
      dashedEffect && dashedEffect.delete();
      this.delete();
    }
    return paint;
  }

  this.stroke = function(path) {
    path = path ? path._getPath() : this._currentPath;
    var strokePaint = this._strokePaint();

    var shadowPaint = this._shadowPaint(strokePaint);
    if (shadowPaint) {
      this._canvas.save();
      this._canvas.concat(this._shadowOffsetMatrix());
      this._canvas.drawPath(path, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }

    this._canvas.drawPath(path, strokePaint);
    strokePaint.dispose();
  }

  this.strokeRect = function(x, y, width, height) {
    var strokePaint = this._strokePaint();
    this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), strokePaint);
    strokePaint.dispose();
  }

  this.strokeText = function(text, x, y, maxWidth) {
    // TODO do something with maxWidth, probably involving measure
    var strokePaint = this._strokePaint();
    var blob = CanvasKit.SkTextBlob.MakeFromText(text, this._font);

    var shadowPaint = this._shadowPaint(strokePaint);
    if (shadowPaint) {
      this._canvas.save();
      this._canvas.concat(this._shadowOffsetMatrix());
      this._canvas.drawTextBlob(blob, x, y, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }
    this._canvas.drawTextBlob(blob, x, y, strokePaint);
    blob.delete();
    strokePaint.dispose();
  }

  this.translate = function(dx, dy) {
    if (!allAreFinite(arguments)) {
      return;
    }
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.SkMatrix.translated(-dx, -dy);
    this._currentPath.transform(inverted);
    this._canvas.translate(dx, dy);
    this._currentTransform = this._canvas.getTotalMatrix();
  }

  this.transform = function(a, b, c, d, e, f) {
    var newTransform = [a, c, e,
                        b, d, f,
                        0, 0, 1];
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.SkMatrix.invert(newTransform);
    this._currentPath.transform(inverted);
    this._canvas.concat(newTransform);
    this._currentTransform = this._canvas.getTotalMatrix();
  }

  // Not supported operations (e.g. for Web only)
  this.addHitRegion = function() {};
  this.clearHitRegions = function() {};
  this.drawFocusIfNeeded = function() {};
  this.removeHitRegion = function() {};
  this.scrollPathIntoView = function() {};

  Object.defineProperty(this, 'canvas', {
    value: null,
    writable: false
  });
}

function SkBlurRadiusToSigma(radius) {
  // Blink (Chrome) does the following, for legacy reasons, even though it
  // is against the spec. https://bugs.chromium.org/p/chromium/issues/detail?id=179006
  // This may change in future releases.
  // This code is staying here in case any clients are interested in using it
  // to match Blink "exactly".
  // if (radius <= 0)
  //   return 0;
  // return 0.288675 * radius + 0.5;
  //
  // This is what the spec says, which is how Firefox and others operate.
  return radius/2;
}