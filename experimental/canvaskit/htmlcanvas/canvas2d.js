// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which may be called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function(CanvasKit) {

  var isNode = typeof btoa === undefined;

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

    this.toDataURL = function() {
      this._surface.flush();

      var img = this._surface.makeImageSnapshot();
      if (!img) {
        console.error('no snapshot');
        return;
      }
      var png = img.encodeToData();
      if (!png) {
        console.error('encoding failure');
        return
      }
      // TODO(kjlubick): clean this up a bit - maybe better naming?
      var pngBytes = CanvasKit.getSkDataBytes(png);
      if (isNode) {
        // See https://stackoverflow.com/a/12713326
        var b64encoded = Buffer.from(pngBytes).toString('base64');
      } else {
        var b64encoded = btoa(String.fromCharCode.apply(null, pngBytes));
      }
      return 'data:image/png;base64,' + b64encoded;
    }
  }

  function CanvasRenderingContext2D(skcanvas) {
    this._canvas = skcanvas;
    this._paint = new CanvasKit.SkPaint();
    this._paint.setAntiAlias(true);
    this._currentPath = null;
    this._pathStarted = false;

    Object.defineProperty(this, 'font', {
      enumerable: true,
      set: function(newStyle) {
        var size = parseFontSize(newStyle);
        // TODO styles
        this._paint.setTextSize(size);
      }
    });

    Object.defineProperty(this, 'strokeStyle', {
      enumerable: true,
      set: function(newStyle) {
        this._paint.setColor(parseColor(newStyle));
      }
    });

    this.beginPath = function() {
      if (this._currentPath) {
        this._currentPath.delete();
      }
      this._currentPath = new CanvasKit.SkPath();
      this._pathStarted = false;
    }

    // ensureSubpath makes SkPath behave like the browser's path object
    // in that the first lineTo/cubicTo, etc, really acts like a moveTo.
    // ensureSubpath is the term used in the canvas spec:
    // https://html.spec.whatwg.org/multipage/canvas.html#ensure-there-is-a-subpath
    // ensureSubpath returns true if the drawing command can proceed,
    // false otherwise (i.e. it was the first command and replaced
    // with a moveTo).
    this._ensureSubpath = function(x, y) {
      if (!this._currentPath) {
        this.beginPath();
      }
      if (!this._pathStarted) {
        this._pathStarted = true;
        this.moveTo(x, y);
        return false;
      }
      return true;
    }

    this.fillText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      this._canvas.drawText(text, x, y, this._paint);
    }

    this.lineTo = function(x, y) {
      if (this._ensureSubpath(x, y)) {
        this._currentPath.lineTo(x, y);
      }
    }

    this.measureText = function(text) {
      return {
        width: this._paint.measureText(text),
        // TODO other measurements?
      }
    }

    this.moveTo = function(x, y) {
      if (this._ensureSubpath(x, y)) {
        this._currentPath.moveTo(x, y);
      }
    }

    this.resetTransform = function() {
      this.setTransform(1, 0, 0, 1, 0, 0);
    }

    this.rotate = function(radians, px, py) {
      // bindings can't turn undefined into floats
      this._canvas.rotate(radians * 180/Math.PI, px || 0, py || 0);
    }

    this.scale = function(sx, sy) {
      this._canvas.scale(sx, sy);
    }

    this.setTransform = function(a, b, c, d, e, f) {
      this._canvas.setMatrix([a, c, e,
                              b, d, f,
                              0, 0, 1]);
    }

    this.skew = function(sx, sy) {
      this._canvas.skew(sx, sy);
    }

    this.stroke = function() {
      if (this._currentPath) {
        this._paint.setStyle(CanvasKit.PaintStyle.Stroke);
        this._canvas.drawPath(this._currentPath, this._paint);
      }
    }

    this.strokeText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      this._paint.setStyle(CanvasKit.PaintStyle.Stroke);
      this._canvas.drawText(text, x, y, this._paint);
    }

    this.translate = function(dx, dy) {
      this._canvas.translate(dx, dy);
    }
  }

  CanvasKit.MakeCanvas = function(width, height) {
    // TODO do fonts the "correct" way
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
      console.err('Could not parse font size', fontStr);
      return 16;
    }
    var size = fontSize[1];
    var unit = fontSize[2];
    switch (unit) {
      // TODO are these correctly in points?
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

  function valueOrPercent(aStr) {
    var a = parseFloat(aStr) || 1;
    if (aStr && aStr.indexOf('%') !== -1) {
      return a / 100;
    }
    return a;
  }

  function parseColor(colorStr) {
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
    } //TODO grey(), hsl[a], hwb, named-color
  }

  CanvasKit._testing['parseColor'] = parseColor;

}(Module)); // When this file is loaded in, the high level object is "Module";
