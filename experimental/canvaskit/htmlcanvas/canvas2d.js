// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which may be called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function(CanvasKit) {

  // See https://www.npmjs.com/package/detect-node
  const isNode = Object.prototype.toString.call(global.process) === '[object process]';

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

      const img = this._surface.makeImageSnapshot()
      if (!img) {
        console.error('no snapshot');
        return;
      }
      const png = img.encodeToData()
      if (!png) {
        console.error('encoding failure');
        return
      }
      const pngBytes = CanvasKit.getSkDataBytes(png);
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
    this._currentPath = null;

    Object.defineProperty(this, 'strokeStyle', {
      enumerable: true,
      writeable: true,
      set: function(newStyle) {
        //console.log('should set new stroke style', this);
      }
    });

    this.beginPath = function() {
      if (this._currentPath) {
        this._currentPath.delete();
      }
      this._currentPath = new CanvasKit.SkPath();
    }

    this.fillText = function(text, x, y, maxWidth) {
      // TODO
    }

    this.lineTo = function(x, y) {
      if (this._currentPath) {
        this._currentPath.lineTo(x, y);
      }
    }

    this.measureText = function(text) {
      // TODO
      return {
        width: (text.length || 0) * 12 // dummy value
      }
    }

    this.rotate = function(angle) {
      // TODO
    }

    this.stroke = function() {
      if (this._currentPath) {
        this._paint.setStyle(CanvasKit.PaintStyle.STROKE);
        this._canvas.drawPath(this._currentPath, this._paint);
      }
    }
  }

  CanvasKit.MakeCanvas = function(width, height) {
    var surf = CanvasKit.MakeSurface(width, height);
    if (surf) {
      return new HTMLCanvas(surf);
    }
    return null;
  }

}(Module)); // When this file is loaded in, the high level object is "Module";
