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
      // From https://stackoverflow.com/a/25644409
      // because the naive solution of
      //     btoa(String.fromCharCode.apply(null, bytes));
      // would occasionally throw "Maximum call stack size exceeded"
      var CHUNK_SIZE = 0x8000; //arbitrary number
      var index = 0;
      var length = bytes.length;
      var result = '';
      var slice;
      while (index < length) {
        slice = bytes.slice(index, Math.min(index + CHUNK_SIZE, length));
        result += String.fromCharCode.apply(null, slice);
        index += CHUNK_SIZE;
      }
      return btoa(result);
    }
  }

  CanvasKit._testing = {};

  function HTMLCanvas(skSurface) {
    this._surface = skSurface;
    this._context = new CanvasRenderingContext2D(skSurface.getCanvas());
    this._imgs = [];

    // Data is either an ArrayBuffer, a TypedArray, or a Node Buffer
    this.decodeImage = function(data) {
      var img = CanvasKit.MakeImageFromEncoded(data);
      if (!img) {
        throw 'Invalid input';
      }
      this._imgs.push(img);
      return img;
    }

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
      this._imgs.forEach(function(i) {
        i.delete();
      });
      this._surface.dispose();
    }
  }

  function ImageData(arr, width, height) {
    if (!width || height === 0) {
      throw 'invalid dimensions, width and height must be non-zero';
    }
    if (arr.length % 4) {
      throw 'arr must be a multiple of 4';
    }
    height = height || arr.length/(4*width);

    Object.defineProperty(this, 'data', {
      value: arr,
      writable: false
    });
    Object.defineProperty(this, 'height', {
      value: height,
      writable: false
    });
    Object.defineProperty(this, 'width', {
      value: width,
      writable: false
    });
  }

  CanvasKit.ImageData = function() {
    if (arguments.length === 2) {
      var width = arguments[0];
      var height = arguments[1];
      var byteLength = 4 * width * height;
      return new ImageData(new Uint8ClampedArray(byteLength),
                           width, height);
    } else if (arguments.length === 3) {
      var arr = arguments[0];
      if (arr.prototype.constructor !== Uint8ClampedArray ) {
        throw 'bytes must be given as a Uint8ClampedArray';
      }
      var width = arguments[1];
      var height = arguments[2];
      if (arr % 4) {
        throw 'bytes must be given in a multiple of 4';
      }
      if (arr % width) {
        throw 'bytes must divide evenly by width';
      }
      if (height && (height !== (arr / (width * 4)))) {
        throw 'invalid height given';
      }
      height = arr / (width * 4);
      return new ImageData(arr, width, height);
    } else {
      throw 'invalid number of arguments - takes 2 or 3, saw ' + arguments.length;
    }
  }

  function LinearCanvasGradient(x1, y1, x2, y2) {
    this._shader = null;
    this._colors = [];
    this._pos = [];

    this.addColorStop = function(offset, color) {
      if (offset < 0 || offset > 1 || !isFinite(offset)) {
        throw 'offset must be between 0 and 1 inclusively';
      }

      color = parseColor(color);
      // From the spec: If multiple stops are added at the same offset on a
      // gradient, then they must be placed in the order added, with the first
      // one closest to the start of the gradient, and each subsequent one
      // infinitesimally further along towards the end point (in effect
      // causing all but the first and last stop added at each point to be
      // ignored).
      // To implement that, if an offset is already in the list,
      // we just overwrite its color (since the user can't remove Color stops
      // after the fact).
      var idx = this._pos.indexOf(offset);
      if (idx !== -1) {
        this._colors[idx] = color;
      } else {
        // insert it in sorted order
        for (idx = 0; idx < this._pos.length; idx++) {
          if (this._pos[idx] > offset) {
            break;
          }
        }
        this._pos   .splice(idx, 0, offset);
        this._colors.splice(idx, 0, color);
      }
    }

    this._copy = function() {
      var lcg = new LinearCanvasGradient(x1, y1, x2, y2);
      lcg._colors = this._colors.slice();
      lcg._pos    = this._pos.slice();
      return lcg;
    }

    this._dispose = function() {
      if (this._shader) {
        this._shader.delete();
        this._shader = null;
      }
    }

    this._getShader = function(currentTransform) {
      // From the spec: "The points in the linear gradient must be transformed
      // as described by the current transformation matrix when rendering."
      var pts = [x1, y1, x2, y2];
      CanvasKit.SkMatrix.mapPoints(currentTransform, pts);
      var sx1 = pts[0];
      var sy1 = pts[1];
      var sx2 = pts[2];
      var sy2 = pts[3];

      this._dispose();
      this._shader = CanvasKit.MakeLinearGradientShader([sx1, sy1], [sx2, sy2],
        this._colors, this._pos, CanvasKit.TileMode.Clamp);
      return this._shader;
    }
  }

  // Note, Skia has a different notion of a "radial" gradient.
  // Skia has a twoPointConical gradient that is the same as the
  // canvas's RadialGradient.
  function RadialCanvasGradient(x1, y1, r1, x2, y2, r2) {
    this._shader = null;
    this._colors = [];
    this._pos = [];

    this.addColorStop = function(offset, color) {
      if (offset < 0 || offset > 1 || !isFinite(offset)) {
        throw 'offset must be between 0 and 1 inclusively';
      }

      color = parseColor(color);
      // From the spec: If multiple stops are added at the same offset on a
      // gradient, then they must be placed in the order added, with the first
      // one closest to the start of the gradient, and each subsequent one
      // infinitesimally further along towards the end point (in effect
      // causing all but the first and last stop added at each point to be
      // ignored).
      // To implement that, if an offset is already in the list,
      // we just overwrite its color (since the user can't remove Color stops
      // after the fact).
      var idx = this._pos.indexOf(offset);
      if (idx !== -1) {
        this._colors[idx] = color;
      } else {
        // insert it in sorted order
        for (idx = 0; idx < this._pos.length; idx++) {
          if (this._pos[idx] > offset) {
            break;
          }
        }
        this._pos   .splice(idx, 0, offset);
        this._colors.splice(idx, 0, color);
      }
    }

    this._copy = function() {
      var rcg = new RadialCanvasGradient(x1, y1, r1, x2, y2, r2);
      rcg._colors = this._colors.slice();
      rcg._pos    = this._pos.slice();
      return rcg;
    }

    this._dispose = function() {
      if (this._shader) {
        this._shader.delete();
        this._shader = null;
      }
    }

    this._getShader = function(currentTransform) {
      // From the spec: "The points in the linear gradient must be transformed
      // as described by the current transformation matrix when rendering."
      var pts = [x1, y1, x2, y2];
      CanvasKit.SkMatrix.mapPoints(currentTransform, pts);
      var sx1 = pts[0];
      var sy1 = pts[1];
      var sx2 = pts[2];
      var sy2 = pts[3];

      var sx = currentTransform[0];
      var sy = currentTransform[4];
      var scaleFactor = (Math.abs(sx) + Math.abs(sy))/2;

      var sr1 = r1 * scaleFactor;
      var sr2 = r2 * scaleFactor;

      this._dispose();
      this._shader = CanvasKit.MakeTwoPointConicalGradientShader(
          [sx1, sy1], sr1, [sx2, sy2], sr2, this._colors, this._pos,
          CanvasKit.TileMode.Clamp);
      return this._shader;
    }
  }

  function CanvasPattern(image, repetition) {
    this._shader = null;
    // image should be an SkImage returned from HTMLCanvas.decodeImage()
    this._image = image;
    this._transform = CanvasKit.SkMatrix.identity();

    if (repetition === '') {
      repetition = 'repeat';
    }
    switch(repetition) {
      case 'repeat-x':
        this._tileX = CanvasKit.TileMode.Repeat;
        this._tileY = CanvasKit.TileMode.Decal;
        break;
      case 'repeat-y':
        this._tileX = CanvasKit.TileMode.Decal;
        this._tileY = CanvasKit.TileMode.Repeat;
        break;
      case 'repeat':
        this._tileX = CanvasKit.TileMode.Repeat;
        this._tileY = CanvasKit.TileMode.Repeat;
        break;
      case 'no-repeat':
        this._tileX = CanvasKit.TileMode.Decal;
        this._tileY = CanvasKit.TileMode.Decal;
        break;
      default:
        throw 'invalid repetition mode ' + repetition;
    }

    // Takes a DOMMatrix like object. e.g. the identity would be:
    // {a:1, b: 0, c: 0, d: 1, e: 0, f: 0}
    // @param {DOMMatrix} m
    this.setTransform = function(m) {
      var t = [m.a, m.c, m.e,
               m.b, m.d, m.f,
                 0,   0,   1];
      if (allAreFinite(t)) {
        this._transform = t;
      }
    }

    this._copy = function() {
      var cp = new CanvasPattern()
      cp._tileX = this._tileX;
      cp._tileY = this._tileY;
      return cp;
    }

    this._dispose = function() {
      if (this._shader) {
        this._shader.delete();
        this._shader = null;
      }
    }

    this._getShader = function(currentTransform) {
      // Ignore currentTransform since it will be applied later
      this._dispose();
      this._shader = CanvasKit.MakeImageShader(this._image, this._tileX, this._tileY,
                                               false, this._transform);
      return this._shader;
    }

  }

  function CanvasRenderingContext2D(skcanvas) {
    this._canvas = skcanvas;
    this._paint = new CanvasKit.SkPaint();
    this._paint.setAntiAlias(true);

    this._paint.setStrokeMiter(10);
    this._paint.setStrokeCap(CanvasKit.StrokeCap.Butt);
    this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Miter);

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
      if (this._currentPath.isEmpty()) {
        this.moveTo(x1, y1);
      }
      this._currentPath.arcTo(x1, y1, x2, y2, radius);
    }

    // As per the spec this doesn't begin any paths, it only
    // clears out any previous paths.
    this.beginPath = function() {
      this._currentPath.delete();
      this._currentPath = new CanvasKit.SkPath();
    }

    this.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
      if (!allAreFinite(arguments)) {
        return;
      }
      if (this._currentPath.isEmpty()) {
        this.moveTo(cp1x, cp1y);
      }
      this._currentPath.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    }

    this.clearRect = function(x, y, width, height) {
      this._paint.setStyle(CanvasKit.PaintStyle.Fill);
      this._paint.setBlendMode(CanvasKit.BlendMode.Clear);
      this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), this._paint);
      this._paint.setBlendMode(this._globalCompositeOperation);
    }

    this.clip = function(fillRule) {
      var clip = this._currentPath.copy();
      if (fillRule && fillRule.toLowerCase() === 'evenodd') {
        clip.setFillType(CanvasKit.FillType.EvenOdd);
      } else {
        clip.setFillType(CanvasKit.FillType.Winding);
      }
      this._canvas.clipPath(clip, CanvasKit.ClipOp.Intersect, true);
    }

    this.closePath = function() {
      if (this._currentPath.isEmpty()) {
        return;
      }
      // Check to see if we are not just a single point
      var bounds = this._currentPath.getBounds();
      if ((bounds.fBottom - bounds.fTop) || (bounds.fRight - bounds.fLeft)) {
        this._currentPath.close();
      }
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

    this._ellipseHelper = function(x, y, radiusX, radiusY, startAngle, endAngle) {
      var sweepDegrees = radiansToDegrees(endAngle - startAngle);
      var startDegrees = radiansToDegrees(startAngle);

      var oval = CanvasKit.LTRBRect(x - radiusX, y - radiusY, x + radiusX, y + radiusY);

      // draw in 2 180 degree segments because trying to draw all 360 degrees at once
      // draws nothing.
      if (almostEqual(Math.abs(sweepDegrees), 360)) {
        var halfSweep = sweepDegrees/2;
        this._currentPath.arcTo(oval, startDegrees, halfSweep, false);
        this._currentPath.arcTo(oval, startDegrees + halfSweep, halfSweep, false);
        return;
      }
      this._currentPath.arcTo(oval, startDegrees, sweepDegrees, false);
    }

    this.ellipse = function(x, y, radiusX, radiusY, rotation,
                            startAngle, endAngle, ccw) {
      if (!allAreFinite([x, y, radiusX, radiusY, rotation, startAngle, endAngle])) {
        return;
      }
      if (radiusX < 0 || radiusY < 0) {
        throw 'radii cannot be negative';
      }

      // based off of CanonicalizeAngle in Chrome
      var tao = 2 * Math.PI;
      var newStartAngle = startAngle % tao;
      if (newStartAngle < 0) {
        newStartAngle += tao;
      }
      var delta = newStartAngle - startAngle;
      startAngle = newStartAngle;
      endAngle += delta;

      // Based off of AdjustEndAngle in Chrome.
      if (!ccw && (endAngle - startAngle) >= tao) {
        // Draw complete ellipse
        endAngle = startAngle + tao;
      } else if (ccw && (startAngle - endAngle) >= tao) {
        // Draw complete ellipse
        endAngle = startAngle - tao;
      } else if (!ccw && startAngle > endAngle) {
        endAngle = startAngle + (tao - (startAngle - endAngle) % tao);
      } else if (ccw && startAngle < endAngle) {
        endAngle = startAngle - (tao - (endAngle - startAngle) % tao);
      }


      // Based off of Chrome's implementation in
      // https://cs.chromium.org/chromium/src/third_party/blink/renderer/platform/graphics/path.cc
      // of note, can't use addArc or addOval because they close the arc, which
      // the spec says not to do (unless the user explicitly calls closePath).
      // This throws off points being in/out of the arc.
      if (!rotation) {
        this._ellipseHelper(x, y, radiusX, radiusY, startAngle, endAngle);
        return;
      }
      var rotated = CanvasKit.SkMatrix.rotated(rotation, x, y);
      this._currentPath.transform(CanvasKit.SkMatrix.invert(rotated));
      this._ellipseHelper(x, y, radiusX, radiusY, startAngle, endAngle);
      this._currentPath.transform(rotated);
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

    this.fill = function(fillRule) {
      if (fillRule === 'evenodd') {
        this._currentPath.setFillType(CanvasKit.FillType.EvenOdd);
      } else if (fillRule === 'nonzero' || !fillRule) {
        this._currentPath.setFillType(CanvasKit.FillType.Winding);
      } else {
        throw 'invalid fill rule';
      }
      var fillPaint = this._fillPaint();

      var shadowPaint = this._shadowPaint(fillPaint);
      if (shadowPaint) {
        this._canvas.save();
        this._canvas.concat(this._shadowOffsetMatrix());
        this._canvas.drawPath(this._currentPath, shadowPaint);
        this._canvas.restore();
        shadowPaint.dispose();
      }
      this._canvas.drawPath(this._currentPath, fillPaint);
      fillPaint.dispose();
    }

    this.fillRect = function(x, y, width, height) {
      var fillPaint = this._fillPaint();
      this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), fillPaint);
      fillPaint.dispose();
    }

    this.fillText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      var fillPaint = this._fillPaint()
      var shadowPaint = this._shadowPaint(fillPaint);
      if (shadowPaint) {
        this._canvas.save();
        this._canvas.concat(this._shadowOffsetMatrix());
        this._canvas.drawText(text, x, y, shadowPaint);
        this._canvas.restore();
        shadowPaint.dispose();
      }
      this._canvas.drawText(text, x, y, fillPaint);
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
      this._currentPath.setFillType(fillmode === 'nonzero' ?
                                    CanvasKit.FillType.Winding :
                                    CanvasKit.FillType.EvenOdd);
      return this._currentPath.contains(x, y);
    }

    this.isPointInStroke = function(x, y) {
      if (!isFinite(x) || !isFinite(y)) {
        return false;
      }
      var pts = this._mapToLocalCoordinates([x, y]);
      x = pts[0];
      y = pts[1];
      var temp = this._currentPath.copy();
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
      if (!allAreFinite(arguments)) {
        return;
      }
      // A lineTo without a previous point has a moveTo inserted before it
      if (this._currentPath.isEmpty()) {
        this._currentPath.moveTo(x, y);
      }
      this._currentPath.lineTo(x, y);
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
      this._currentPath.moveTo(x, y);
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
      if (!allAreFinite(arguments)) {
        return;
      }
      if (this._currentPath.isEmpty()) {
        this._currentPath.moveTo(cpx, cpy);
      }
      this._currentPath.quadTo(cpx, cpy, x, y);
    }

    this.rect = function(x, y, width, height) {
      if (!allAreFinite(arguments)) {
        return;
      }
      // https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-rect
      this._currentPath.addRect(x, y, x+width, y+height);
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

      this._lineDashList = newState.ldl;
      this._strokeWidth = newState.sw;
      this._paint.setStrokeWidth(this._strokeWidth);
      this._strokeStyle = newState.ss;
      this._fillStyle = newState.fs;
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
      this._imageSmoothingEnabled = newState.ise;
      this._imageFilterQuality = newState.isq;
      //TODO: font, textAlign, textBaseline, direction

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
        ctm:  this._currentTransform.slice(),
        ldl: this._lineDashList.slice(),
        sw:  this._strokeWidth,
        ss:  ss,
        fs:  fs,
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
        ise: this._imageSmoothingEnabled,
        isq: this._imageFilterQuality,
        //TODO: font, textAlign, textBaseline, direction
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

    this.stroke = function() {
      var strokePaint = this._strokePaint();

      var shadowPaint = this._shadowPaint(strokePaint);
      if (shadowPaint) {
        this._canvas.save();
        this._canvas.concat(this._shadowOffsetMatrix());
        this._canvas.drawPath(this._currentPath, shadowPaint);
        this._canvas.restore();
        shadowPaint.dispose();
      }

      this._canvas.drawPath(this._currentPath, strokePaint);
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

      var shadowPaint = this._shadowPaint(strokePaint);
      if (shadowPaint) {
        this._canvas.save();
        this._canvas.concat(this._shadowOffsetMatrix());
        this._canvas.drawText(text, x, y, shadowPaint);
        this._canvas.restore();
        shadowPaint.dispose();
      }
      this._canvas.drawText(text, x, y, strokePaint);
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
