// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which may be called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function(CanvasKit) {

  var isNode = typeof btoa === undefined;

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
        //TODO
        this._paint.setTextSize(30);
      }
    });

    Object.defineProperty(this, 'strokeStyle', {
      enumerable: true,
      set: function(newStyle) {
        // TODO
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
        this._paint.setStyle(CanvasKit.PaintStyle.STROKE);
        this._canvas.drawPath(this._currentPath, this._paint);
      }
    }

    this.strokeText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      this._paint.setStyle(CanvasKit.PaintStyle.STROKE);
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

}(Module)); // When this file is loaded in, the high level object is "Module";
