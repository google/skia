// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the CPU version of canvaskit.
(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {
    // Takes in an html id or a canvas element
    CanvasKit.MakeSWCanvasSurface = function(idOrElement) {
        var canvas = idOrElement;
        if (canvas.tagName !== 'CANVAS') {
          canvas = document.getElementById(idOrElement);
          if (!canvas) {
            throw 'Canvas with id ' + idOrElement + ' was not found';
          }
        }
      // Maybe better to use clientWidth/height.  See:
      // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
      var surface = CanvasKit.MakeSurface(canvas.width, canvas.height);
      if (surface) {
        surface._canvas = canvas;
      }
      return surface;
    };

    // Don't over-write the MakeCanvasSurface set by gpu.js if it exists.
    if (!CanvasKit.MakeCanvasSurface) {
      CanvasKit.MakeCanvasSurface = CanvasKit.MakeSWCanvasSurface;
    }

    CanvasKit.MakeSurface = function(width, height) {
      // We're going to allocate twice as much space as you'd expect,
      // one premul plane to draw into, and another unpremul plane to
      // read back from that premul plane then push to putImageData().
      var planeLen = width * height * 4; // it's 8888, so 4 bytes per pixel
      var pixelPtr = CanvasKit._malloc(2*planeLen);

      /* @dict */
      var imageInfo = {
        'width':  width,
        'height': height,
        'colorType': CanvasKit.ColorType.RGBA_8888,
        'alphaType': CanvasKit.AlphaType.Premul,
      }
      var surface = this._getRasterDirectSurface(imageInfo, pixelPtr+0*planeLen, width*4);
      if (surface) {
        surface._canvas = null;
        surface._width = width;
        surface._height = height;
        surface._planeLen = planeLen;

        surface._pixelPtr = pixelPtr;
        // rasterDirectSurface does not initialize the pixels, so we clear them
        // to transparent black.
        surface.getCanvas().clear(CanvasKit.TRANSPARENT);
      }
      return surface;
    };

    CanvasKit.SkSurface.prototype.flush = function() {
      this._flush();
      // Do we have an HTML canvas to write the pixels to?
      // We will not if this a GPU build or a raster surface, for example.
      if (this._canvas) {
        // We've drawn as premul into plane 0 of our pixel buffer.
        // We'll read that back as unpremul into plane 1, then pass that to putImageData().
        var unpremulInfo = {
            'width':  this._width,
            'height': this._height,
            'colorType': CanvasKit.ColorType.RGBA_8888,
            'alphaType': CanvasKit.AlphaType.Unpremul,
        }
        this.getCanvas()._readPixels(unpremulInfo,
                                     this._pixelPtr+1*this._planeLen,
                                     this._width*4,
                                     0,0);
        var unpremul = new Uint8ClampedArray(CanvasKit.HEAPU8.buffer,
                                             this._pixelPtr+1*this._planeLen,
                                             this._planeLen);
        var imageData = new ImageData(unpremul, this._width, this._height);

        this._canvas.getContext('2d').putImageData(imageData, 0, 0);
      }
    };

    // Call dispose() instead of delete to clean up the underlying memory
    CanvasKit.SkSurface.prototype.dispose = function() {
      if (this._pixelPtr) {
        CanvasKit._free(this._pixelPtr);
      }
      this.delete();
    }

    CanvasKit.currentContext = CanvasKit.currentContext || function() {
      // no op if this is a cpu-only build.
    };

    CanvasKit.setCurrentContext = CanvasKit.setCurrentContext || function() {
       // no op if this is a cpu-only build.
    };
  });
}(Module)); // When this file is loaded in, the high level object is "Module";
