// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which may be called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function(CanvasKit) {

  function allAreFinite(args) {
    for (var i = 0; i < args.length; i++) {
      if (args[i] !== undefined && !Number.isFinite(args[i])) {
        return false;
      }
    }
    return true;
  }

  function toBase64String(bytes) {
    if (isNode) {
      return Buffer.from(bytes).toString('base64');
    } else {
      return btoa(String.fromCharCode.apply(null, bytes));
    }
  }

  CanvasKit._testing = {};

  function HTMLCanvas(skSurface) {
    this._surface = skSurface;
    this._context = new CanvasRenderingContext2D(skSurface.getCanvas());

    // A normal <canvas> requires that clients call getContext
    this.getContext = function(type) {
      if (type === '2d') {
        return this._context;
      }
      return null;
    }

    this.toDataURL = function(codec, quality) {
      // TODO(kjlubick): maybe support other codecs (webp?)
      // For now, just to png and jpeg
      this._surface.flush();

      var img = this._surface.makeImageSnapshot();
      if (!img) {
        SkDebug('no snapshot');
        return;
      }
      var codec = codec || 'image/png';
      var format = CanvasKit.ImageFormat.PNG;
      if (codec === 'image/jpeg') {
        format = CanvasKit.ImageFormat.JPEG;
      }
      var quality = quality || 0.92;
      var skimg = img.encodeToData(format, quality);
      if (!skimg) {
        SkDebug('encoding failure');
        return
      }
      var imgBytes = CanvasKit.getSkDataBytes(skimg);
      return 'data:' + codec + ';base64,' + toBase64String(imgBytes);
    }

    this.dispose = function() {
      this._context._dispose();
      this._surface.dispose();
    }
  }

  function CanvasRenderingContext2D(skcanvas) {
    this._canvas = skcanvas;
    this._paint = new CanvasKit.SkPaint();
    this._paint.setAntiAlias(true);

    this._paint.setStrokeMiter(10);
    this._paint.setStrokeCap(CanvasKit.StrokeCap.Butt);
    this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Miter);

    this._strokeColor    = CanvasKit.BLACK;
    this._fillColor      = CanvasKit.BLACK;
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

    this._paint.setStrokeWidth(this._strokeWidth);
    this._paint.setBlendMode(this._globalCompositeOperation);

    this._currentPath = new CanvasKit.SkPath();
    this._currentSubpath = null;
    this._currentTransform = CanvasKit.SkMatrix.identity();

    this._canvasStateStack = [];

    this._dispose = function() {
      this._currentPath.delete();
      this._currentSubpath && this._currentSubpath.delete();
      this._paint.delete();
      // Don't delete this._canvas as it will be disposed
      // by the surface of which it is based.
    }

    // This always accepts DOMMatrix/SVGMatrix or any other
    // object that has properties a,b,c,d,e,f defined.
    // It will return DOMMatrix if the constructor is defined
    // (e.g. we are in a browser), otherwise it will return a
    // flattened 9-element matrix. That 9 element matrix
    // can also be accepted on all platforms (somewhat
    //  contrary to the specification, but at least keeps it usable
    // on Node)
    Object.defineProperty(this, 'currentTransform', {
      enumerable: true,
      get: function() {
        if (isNode) {
          return this._currentTransform.slice();
        } else {
          // a-f aren't in the order as we have them. a-f are in "SVG order".
          var m = new DOMMatrix();
          m.a = this._currentTransform[0];
          m.c = this._currentTransform[1];
          m.e = this._currentTransform[2];
          m.b = this._currentTransform[3];
          m.d = this._currentTransform[4];
          m.f = this._currentTransform[5];
          return m;
        }
      },
      set: function(matrix) {
        if (matrix.a) {
          // if we see a property named 'a', guess that b-f will
          // also be there.
          this._currentTransform = [matrix.a, matrix.c, matrix.e,
                                    matrix.b, matrix.d, matrix.f,
                                           0,        0,        1];
        } else if (matrix.length === 9) {
          this._currentTransform = matrix;
        }
      }
    });

    Object.defineProperty(this, 'fillStyle', {
      enumerable: true,
      get: function() {
        return colorToString(this._fillColor);
      },
      set: function(newStyle) {
        this._fillColor = parseColor(newStyle);
      }
    });

    Object.defineProperty(this, 'font', {
      enumerable: true,
      get: function(newStyle) {
        // TODO generate this
        return '10px sans-serif';
      },
      set: function(newStyle) {
        var size = parseFontSize(newStyle);
        // TODO(kjlubick) styles, font name
        this._paint.setTextSize(size);
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
        return colorToString(this._strokeColor);
      },
      set: function(newStyle) {
        this._strokeColor = parseColor(newStyle);
      }
    });

    this.arc = function(x, y, radius, startAngle, endAngle, ccw) {
      // As per  https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-arc
      // arc is essentially a simpler version of ellipse.
      this.ellipse(x, y, radius, radius, 0, startAngle, endAngle, ccw);
    }

    this.arcTo = function(x1, y1, x2, y2, radius) {
      if (!allAreFinite(arguments)) {
        return;
      }
      if (radius < 0) {
        throw 'radii cannot be negative';
      }
      var pts = [x1, y1, x2, y2];
      CanvasKit.SkMatrix.mapPoints(this._currentTransform, pts);
      x1 = pts[0];
      y1 = pts[1];
      x2 = pts[2];
      y2 = pts[3];
      if (!this._currentSubpath) {
        this._newSubpath(x1, y1);
      }
      this._currentSubpath.arcTo(x1, y1, x2, y2, radius * this._scalefactor());
    }

    // As per the spec this doesn't begin any paths, it only
    // clears out any previous subpaths.
    this.beginPath = function() {
      this._currentPath.delete();
      this._currentPath = new CanvasKit.SkPath();
      this._currentSubpath && this._currentSubpath.delete();
      this._currentSubpath = null;
    }

    this.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
      if (!allAreFinite(arguments)) {
        return;
      }
      var pts = [cp1x, cp1y, cp2x, cp2y, x, y];
      CanvasKit.SkMatrix.mapPoints(this._currentTransform, pts);
      cp1x = pts[0];
      cp1y = pts[1];
      cp2x = pts[2];
      cp2y = pts[3];
      x    = pts[4];
      y    = pts[5];
      if (!this._currentSubpath) {
        this._newSubpath(cp1x, cp1y);
      }
      this._currentSubpath.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    }

    this.clearRect = function(x, y, width, height) {
      this._canvas.setMatrix(this._currentTransform);
      this._paint.setStyle(CanvasKit.PaintStyle.Fill);
      this._paint.setBlendMode(CanvasKit.BlendMode.Clear);
      this._canvas.drawRect(CanvasKit.LTRBRect(x, y, x+width, y+height), this._paint);
      this._canvas.setMatrix(CanvasKit.SkMatrix.identity());
      this._paint.setBlendMode(this._globalCompositeOperation);
    }

    this.closePath = function() {
      if (this._currentSubpath) {
        this._currentSubpath.close();
        var lastPt = this._currentSubpath.getPoint(0);
        this._newSubpath(lastPt[0], lastPt[1]);
      }
    }

    this._commitSubpath = function () {
      if (this._currentSubpath) {
        this._currentPath.addPath(this._currentSubpath, false);
        this._currentSubpath.delete();
        this._currentSubpath = null;
      }
    }

    this.ellipse = function(x, y, radiusX, radiusY, rotation,
                            startAngle, endAngle, ccw) {
      if (!allAreFinite(arguments)) {
        return;
      }
      if (radiusX < 0 || radiusY < 0) {
        throw 'radii cannot be negative';
      }

      if (!this._currentSubpath) {
        // Don't use newSubpath here because calculating the starting
        // point in the arc is non-trivial. Just make a new, empty
        // subpath to append to.
        this._currentSubpath = new CanvasKit.SkPath();
      }
      var bounds = CanvasKit.LTRBRect(x-radiusX, y-radiusY, x+radiusX, y+radiusY);
      var sweep = radiansToDegrees(endAngle - startAngle) - (360 * !!ccw);
      var temp = new CanvasKit.SkPath();
      // Skia takes degrees. JS tends to be radians.
      temp.addArc(bounds, radiansToDegrees(startAngle), sweep);
      var m = CanvasKit.SkMatrix.multiply(
                  this._currentTransform,
                  CanvasKit.SkMatrix.rotated(rotation, x, y));

      this._currentSubpath.addPath(temp, m, true);
      temp.delete();
    }

    // A helper to copy the current paint, ready for filling
    // This applies the global alpha.
    // Call dispose() after to clean up.
    this._fillPaint = function() {
      var paint = this._paint.copy();
      paint.setStyle(CanvasKit.PaintStyle.Fill);
      var alphaColor = CanvasKit.multiplyByAlpha(this._fillColor, this._globalAlpha);
      paint.setColor(alphaColor);

      paint.dispose = function() {
        // If there are some helper effects in the future, clean them up
        // here. In any case, we have .dispose() to make _fillPaint behave
        // like _strokePaint and _shadowPaint.
        this.delete();
      }
      return paint;
    }

    this.fill = function() {
      this._commitSubpath();
      var fillPaint = this._fillPaint();

      var shadowPaint = this._shadowPaint(fillPaint);
      if (shadowPaint) {
        var offsetMatrix = CanvasKit.SkMatrix.multiply(
          this._currentTransform,
          CanvasKit.SkMatrix.translated(this._shadowOffsetX, this._shadowOffsetY)
        );
        this._canvas.setMatrix(offsetMatrix);
        this._canvas.drawPath(this._currentPath, shadowPaint);
        this._canvas.setMatrix(CanvasKit.SkMatrix.identity());
        shadowPaint.dispose();
      }

      this._canvas.drawPath(this._currentPath, fillPaint);
      fillPaint.dispose();
    }

    this.fillRect = function(x, y, width, height) {
      var fillPaint = this._fillPaint();
      this._canvas.setMatrix(this._currentTransform);
      this._canvas.drawRect(CanvasKit.LTRBRect(x, y, x+width, y+height), fillPaint);
      this._canvas.setMatrix(CanvasKit.SkMatrix.identity());
      fillPaint.dispose();
    }

    this.fillText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      var fillPaint = this._fillPaint()
      var shadowPaint = this._shadowPaint(fillPaint);
      if (shadowPaint) {
        var offsetMatrix = CanvasKit.SkMatrix.multiply(
          this._currentTransform,
          CanvasKit.SkMatrix.translated(this._shadowOffsetX, this._shadowOffsetY)
        );
        this._canvas.setMatrix(offsetMatrix);
        this._canvas.drawText(text, x, y, shadowPaint);
        shadowPaint.dispose();
        // Don't need to setMatrix back, it will be handled by the next few lines.
      }
      this._canvas.setMatrix(this._currentTransform);
      this._canvas.drawText(text, x, y, fillPaint);
      this._canvas.setMatrix(CanvasKit.SkMatrix.identity());
      fillPaint.dispose();
    }

    this.getLineDash = function() {
      return this._lineDashList.slice();
    }

    this.lineTo = function(x, y) {
      if (!allAreFinite(arguments)) {
        return;
      }
      var pts = [x, y];
      CanvasKit.SkMatrix.mapPoints(this._currentTransform, pts);
      x = pts[0];
      y = pts[1];
      // A lineTo without a previous subpath is turned into a moveTo
      if (!this._currentSubpath) {
        this._newSubpath(x, y);
      } else {
        this._currentSubpath.lineTo(x, y);
      }
    }

    this.measureText = function(text) {
      return {
        width: this._paint.measureText(text),
        // TODO other measurements?
      }
    }

    this.moveTo = function(x, y) {
      if (!allAreFinite(arguments)) {
        return;
      }
      var pts = [x, y];
      CanvasKit.SkMatrix.mapPoints(this._currentTransform, pts);
      x = pts[0];
      y = pts[1];
      this._newSubpath(x, y);
    }

    this._newSubpath = function(x, y) {
      this._commitSubpath();
      this._currentSubpath = new CanvasKit.SkPath();
      this._currentSubpath.moveTo(x, y);
    }

    this.quadraticCurveTo = function(cpx, cpy, x, y) {
      if (!allAreFinite(arguments)) {
        return;
      }
      var pts = [cpx, cpy, x, y];
      CanvasKit.SkMatrix.mapPoints(this._currentTransform, pts);
      cpx = pts[0];
      cpy = pts[1];
      x   = pts[2];
      y   = pts[3];
      if (!this._currentSubpath) {
        this._newSubpath(cpx, cpy);
      }
      this._currentSubpath.quadTo(cpx, cpy, x, y);
    }

    this.rect = function(x, y, width, height) {
      if (!allAreFinite(arguments)) {
        return;
      }
      var pts = [x, y];
      CanvasKit.SkMatrix.mapPoints(this._currentTransform, pts);
      // https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-rect
      this._newSubpath(x, y);
      var scale = this._scalefactor();
      this._currentSubpath.addRect(x, y, x+width, y+height);
      this._currentSubpath.transform(this._currentTransform);
      this._newSubpath(pts[0], pts[1]);
    }

    this.resetTransform = function() {
      this._currentTransform = CanvasKit.SkMatrix.identity();
    }

    this.restore = function() {
      var newState = this._canvasStateStack.pop();
      if (!newState) {
        return;
      }
      this._currentTransform = newState.ctm;
      // TODO(kjlubick): clipping region
      this._lineDashList = newState.ldl;
      this._strokeWidth = newState.sw;
      this._paint.setStrokeWidth(this._strokeWidth);
      this._strokeColor = newState.sc;
      this._fillColor = newState.fc;
      this._paint.setStrokeCap(newState.cap);
      this._paint.setStrokeJoin(newState.jn);
      this._paint.setStrokeMiter(newState.mtr);
      this._shadowOffsetX = newState.sox;
      this._shadowOffsetY = newState.soy;
      this._shadowBlur = newState.sb;
      this._shadowColor = newState.shc;
      this._globalAlpha = newState.ga;
      this._globalCompositeOperation = newState.gco;
      this._paint.setBlendMode(this._globalCompositeOperation);
      this._lineDashOffset = newState.ldo;
      //TODO: filter, font, textAlign, textBaseline, direction, imageSmoothingEnabled, imageSmoothingQuality.
    }

    this.rotate = function(radians, px, py) {
      this._currentTransform = CanvasKit.SkMatrix.multiply(
                                  this._currentTransform,
                                  CanvasKit.SkMatrix.rotated(radians, px, py));
    }

    this.save = function() {
      this._canvasStateStack.push({
        ctm:  this._currentTransform.slice(),
        // TODO(kjlubick): clipping region
        ldl: this._lineDashList.slice(),
        sw:  this._strokeWidth,
        sc:  this._strokeColor,
        fc:  this._fillColor,
        cap: this._paint.getStrokeCap(),
        jn:  this._paint.getStrokeJoin(),
        mtr: this._paint.getStrokeMiter(),
        sox: this._shadowOffsetX,
        soy: this._shadowOffsetY,
        sb:  this._shadowBlur,
        shc: this._shadowColor,
        ga:  this._globalAlpha,
        ldo: this._lineDashOffset,
        gco: this._globalCompositeOperation,
        //TODO: filter, font, textAlign, textBaseline, direction, imageSmoothingEnabled, imageSmoothingQuality.
      });
    }

    this.scale = function(sx, sy) {
      this._currentTransform = CanvasKit.SkMatrix.multiply(
                                  this._currentTransform,
                                  CanvasKit.SkMatrix.scaled(sx, sy));
    }

    this._scalefactor = function() {
      // This is an approximation of what Chrome does when scaling up
      // line width.
      var m = this._currentTransform;
      var sx = m[0];
      var sy = m[4];
      return (Math.abs(sx) + Math.abs(sy))/2;
    }

    this.setLineDash = function(dashes) {
      for (var i = 0; i < dashes.length; i++) {
        if (!Number.isFinite(dashes[i]) || dashes[i] < 0) {
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
      this._currentTransform = [a, c, e,
                                b, d, f,
                                0, 0, 1];
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
        Math.max(1, this._shadowBlur/2), // very little blur when < 1
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
      var alphaColor = CanvasKit.multiplyByAlpha(this._strokeColor, this._globalAlpha);
      paint.setColor(alphaColor);
      // This is not in the spec, but it appears Chrome scales up
      // the line width by some amount when stroking (and filling?).
      var scaledWidth = this._strokeWidth * this._scalefactor();
      paint.setStrokeWidth(scaledWidth);

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

    this.stroke = function() {
      this._commitSubpath();
      var strokePaint = this._strokePaint();

      var shadowPaint = this._shadowPaint(strokePaint);
      if (shadowPaint) {
        var offsetMatrix = CanvasKit.SkMatrix.multiply(
          this._currentTransform,
          CanvasKit.SkMatrix.translated(this._shadowOffsetX, this._shadowOffsetY)
        );
        this._canvas.setMatrix(offsetMatrix);
        this._canvas.drawPath(this._currentPath, shadowPaint);
        this._canvas.setMatrix(CanvasKit.SkMatrix.identity());
        shadowPaint.dispose();
      }

      this._canvas.drawPath(this._currentPath, strokePaint);
      strokePaint.dispose();
    }

    this.strokeRect = function(x, y, width, height) {
      var strokePaint = this._strokePaint();
      this._canvas.setMatrix(this._currentTransform);
      this._canvas.drawRect(CanvasKit.LTRBRect(x, y, x+width, y+height), strokePaint);
      this._canvas.setMatrix(CanvasKit.SkMatrix.identity());
      strokePaint.dispose();
    }

    this.strokeText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      var strokePaint = this._strokePaint();

      var shadowPaint = this._shadowPaint(strokePaint);
      if (shadowPaint) {
        var offsetMatrix = CanvasKit.SkMatrix.multiply(
          this._currentTransform,
          CanvasKit.SkMatrix.translated(this._shadowOffsetX, this._shadowOffsetY)
        );
        this._canvas.setMatrix(offsetMatrix);
        this._canvas.drawText(text, x, y, shadowPaint);
        shadowPaint.dispose();
        // Don't need to setMatrix back, it will be handled by the next few lines.
      }
      this._canvas.setMatrix(this._currentTransform);
      this._canvas.drawText(text, x, y, strokePaint);
      this._canvas.setMatrix(CanvasKit.SkMatrix.identity());
      strokePaint.dispose();
    }

    this.translate = function(dx, dy) {
      this._currentTransform = CanvasKit.SkMatrix.multiply(
                                  this._currentTransform,
                                  CanvasKit.SkMatrix.translated(dx, dy));
    }

    this.transform = function(a, b, c, d, e, f) {
      this._currentTransform = CanvasKit.SkMatrix.multiply(
                                  this._currentTransform,
                                  [a, c, e,
                                   b, d, f,
                                   0, 0, 1]);
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

  CanvasKit.MakeCanvas = function(width, height) {
    // TODO(kjlubick) do fonts the "correct" way
    CanvasKit.initFonts();
    var surf = CanvasKit.MakeSurface(width, height);
    if (surf) {
      return new HTMLCanvas(surf);
    }
    return null;
  }

  var units = 'px|pt|pc|in|cm|mm|%|em|ex|ch|rem|q';
  var fontSizeRegex = new RegExp('([\\d\\.]+)(' + units + ')');
  var defaultHeight = 12;
  // Based off of node-canvas's parseFont
  // returns font size in *points* (original impl was in px);
  function parseFontSize(fontStr) {
    // This is naive and doesn't account for line-height yet
    // (but neither does node-canvas's?)
    var fontSize = fontSizeRegex.exec(fontStr);
    if (!fontSize) {
      SkDebug('Could not parse font size' + fontStr);
      return 16;
    }
    var size = parseFloat(fontSize[1]);
    var unit = fontSize[2];
    switch (unit) {
      case 'pt':
        return size;
      case 'px':
        return size * 3/4;
      case 'pc':
        return size * 12;
      case 'in':
        return size * 72;
      case 'cm':
        return size * 72.0 / 2.54;
      case 'mm':
        return size * (72.0 / 25.4);
      case '%':
        return size * (defaultHeight / 100);
      case 'em':
      case 'rem':
        return size * defaultHeight;
      case 'q':
        return size * (96 / 25.4 / 3);
    }
  }

  function colorToString(skcolor) {
    // https://html.spec.whatwg.org/multipage/canvas.html#serialisation-of-a-color
    var components = CanvasKit.getColorComponents(skcolor);
    var r = components[0];
    var g = components[1];
    var b = components[2];
    var a = components[3];
    if (a === 1.0) {
      // hex
      r = r.toString(16).toLowerCase();
      g = g.toString(16).toLowerCase();
      b = b.toString(16).toLowerCase();
      r = (r.length === 1 ? '0'+r: r);
      g = (g.length === 1 ? '0'+g: g);
      b = (b.length === 1 ? '0'+b: b);
      return '#'+r+g+b;
    } else {
      a = (a === 0 || a === 1) ? a : a.toFixed(8);
      return 'rgba('+r+', '+g+', '+b+', '+a+')';
    }
  }

  function valueOrPercent(aStr) {
    var a = parseFloat(aStr) || 1;
    if (aStr && aStr.indexOf('%') !== -1) {
      return a / 100;
    }
    return a;
  }

  function parseColor(colorStr) {
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
    } else {
      // Try for named color
      var nc = colorMap[colorStr];
      if (nc !== undefined) {
        return nc;
      }
    }
    SkDebug('unrecognized color ' + colorStr);
    return CanvasKit.BLACK;
  }

  CanvasKit._testing['parseColor'] = parseColor;
  CanvasKit._testing['colorToString'] = colorToString;

  // Create the following with
  // node ./htmlcanvas/_namedcolors.js --expose-wasm
  // JS/closure doesn't have a constexpr like thing which
  // would really help here. Since we don't, we pre-compute
  // the map, which saves (a tiny amount of) startup time
  // and (a small amount of) code size.
  /* @dict */
  var colorMap = {"aliceblue":-984833,"antiquewhite":-332841,"aqua":-16711681,"aquamarine":-8388652,"azure":-983041,"beige":-657956,"bisque":-6972,"black":-16777216,"blanchedalmond":-5171,"blue":-16776961,"blueviolet":-7722014,"brown":-5952982,"burlywood":-2180985,"cadetblue":-10510688,"chartreuse":-8388864,"chocolate":-2987746,"coral":-32944,"cornflowerblue":-10185235,"cornsilk":-1828,"crimson":-2354116,"cyan":-16711681,"darkblue":-16777077,"darkcyan":-16741493,"darkgoldenrod":-4684277,"darkgray":-5658199,"darkgreen":-16751616,"darkgrey":-5658199,"darkkhaki":-4343957,"darkmagenta":-7667573,"darkolivegreen":-11179217,"darkorange":-29696,"darkorchid":-6737204,"darkred":-7667712,"darksalmon":-1468806,"darkseagreen":-7357297,"darkslateblue":-12042869,"darkslategray":-13676721,"darkslategrey":-13676721,"darkturquoise":-16724271,"darkviolet":-7077677,"deeppink":-60269,"deepskyblue":-16728065,"dimgray":-9868951,"dimgrey":-9868951,"dodgerblue":-14774017,"firebrick":-5103070,"floralwhite":-1296,"forestgreen":-14513374,"fuchsia":-65281,"gainsboro":-2302756,"ghostwhite":-460545,"gold":-10496,"goldenrod":-2448096,"gray":-8355712,"green":-16744448,"greenyellow":-5374161,"grey":-8355712,"honeydew":-983056,"hotpink":-38476,"indianred":-3318692,"indigo":-11861886,"ivory":-16,"khaki":-989556,"lavender":-1644806,"lavenderblush":-3851,"lawngreen":-8586240,"lemonchiffon":-1331,"lightblue":-5383962,"lightcoral":-1015680,"lightcyan":-2031617,"lightgoldenrodyellow":-329006,"lightgray":-2894893,"lightgreen":-7278960,"lightgrey":-2894893,"lightpink":-18751,"lightsalmon":-24454,"lightseagreen":-14634326,"lightskyblue":-7876870,"lightslategray":-8943463,"lightslategrey":-8943463,"lightsteelblue":-5192482,"lightyellow":-32,"lime":-16711936,"limegreen":-13447886,"linen":-331546,"magenta":-65281,"maroon":-8388608,"mediumaquamarine":-10039894,"mediumblue":-16777011,"mediumorchid":-4565549,"mediumpurple":-7114533,"mediumseagreen":-12799119,"mediumslateblue":-8689426,"mediumspringgreen":-16713062,"mediumturquoise":-12004916,"mediumvioletred":-3730043,"midnightblue":-15132304,"mintcream":-655366,"mistyrose":-6943,"moccasin":-6987,"navajowhite":-8531,"navy":-16777088,"oldlace":-133658,"olive":-8355840,"olivedrab":-9728477,"orange":-23296,"orangered":-47872,"orchid":-2461482,"palegoldenrod":-1120086,"palegreen":-6751336,"paleturquoise":-5247250,"palevioletred":-2396013,"papayawhip":-4139,"peachpuff":-9543,"peru":-3308225,"pink":-16181,"plum":-2252579,"powderblue":-5185306,"purple":-8388480,"rebeccapurple":-10079335,"red":-65536,"rosybrown":-4419697,"royalblue":-12490271,"saddlebrown":-7650029,"salmon":-360334,"sandybrown":-744352,"seagreen":-13726889,"seashell":-2578,"sienna":-6270419,"silver":-4144960,"skyblue":-7876885,"slateblue":-9807155,"slategray":-9404272,"slategrey":-9404272,"snow":-1286,"springgreen":-16711809,"steelblue":-12156236,"tan":-2968436,"teal":-16744320,"thistle":-2572328,"transparent":0,"tomato":-40121,"turquoise":-12525360,"violet":-1146130,"wheat":-663885,"white":-1,"whitesmoke":-657931,"yellow":-256,"yellowgreen":-6632142};

}(Module)); // When this file is loaded in, the high level object is "Module";
