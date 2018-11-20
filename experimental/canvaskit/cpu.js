// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the CPU version of canvaskit.
(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {
    CanvasKit.MakeSWCanvasSurface = function(htmlID) {
      var canvas = document.getElementById(htmlID);
      if (!canvas) {
        throw 'Canvas with id ' + htmlID + ' was not found';
      }
      // Maybe better to use clientWidth/height.  See:
      // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
      var surface = this._getRasterN32PremulSurface(canvas.width, canvas.height);
      if (surface) {
        surface._canvas = canvas;
        surface._width = canvas.width;
        surface._height = canvas.height;
        surface._pixelLen = surface._width * surface._height * 4; // it's 8888
        // Allocate the buffer of pixels to be used to draw back and forth.
        surface._pixelPtr = CanvasKit._malloc(surface._pixelLen);
      }
      return surface;
    };

    // Don't over-write the MakeCanvasSurface set by gpu.js if it exists.
    if (!CanvasKit.MakeCanvasSurface) {
      CanvasKit.MakeCanvasSurface = CanvasKit.MakeSWCanvasSurface;
    }

    CanvasKit.MakeSurface = function(width, height) {
      var surface = this._getRasterN32PremulSurface(width, height);
      if (surface) {
        surface._canvas = null;
        surface._width = width;
        surface._height = height;
        surface._pixelLen = width * height * 4; // it's 8888
        // Allocate the buffer of pixels to be used to draw back and forth.
        surface._pixelPtr = CanvasKit._malloc(surface._pixelLen);
      }
      return surface;
    };

    CanvasKit.SkSurface.prototype.flush = function() {
      this._flush();
      // Do we have an HTML canvas to write the pixels to?
      // We will not if this a GPU build or a raster surface, for example.
      if (this._canvas) {
        var success = this._readPixels(this._width, this._height, this._pixelPtr);
        if (!success) {
          SkDebug('could not read pixels');
          return;
        }

        var pixels = new Uint8ClampedArray(CanvasKit.buffer, this._pixelPtr, this._pixelLen);
        var imageData = new ImageData(pixels, this._width, this._height);

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
