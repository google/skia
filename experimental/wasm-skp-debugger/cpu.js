// Adds compile-time JS functions to handle creation and flushing of wasm's offscreen buffer
// to a visible element on the page.
(function(DebuggerView){
    // Takes in an html id or a canvas element
    DebuggerView.MakeSWCanvasSurface = function(idOrElement) {
        let canvas = idOrElement;
        if (canvas.tagName !== 'CANVAS') {
          canvas = document.getElementById(idOrElement);
          if (!canvas) {
            throw 'Canvas with id ' + idOrElement + ' was not found';
          }
        }
      // Maybe better to use clientWidth/height.  See:
      // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
      let surface = DebuggerView.MakeSurface(canvas.width, canvas.height);
      if (surface) {
        surface._canvas = canvas;
      }
      console.log('Made HTML Canvas Surface');
      return surface;
    };

    // Don't over-write the MakeCanvasSurface set by gpu.js if it exists.
    if (!DebuggerView.MakeCanvasSurface) {
      DebuggerView.MakeCanvasSurface = DebuggerView.MakeSWCanvasSurface;
    }

    DebuggerView.MakeSurface = function(width, height) {
      /* @dict */
      let imageInfo = {
        'width':  width,
        'height': height,
        'colorType': DebuggerView.ColorType.RGBA_8888,
        // Since we are sending these pixels directly into the HTML canvas,
        // (and those pixels are un-premultiplied, i.e. straight r,g,b,a)
        'alphaType': DebuggerView.AlphaType.Unpremul,
      }
      let pixelLen = width * height * 4; // it's 8888, so 4 bytes per pixel
      // Allocate the buffer of pixels to be drawn into.
      let pixelPtr = DebuggerView._malloc(pixelLen);

      let surface = this._getRasterDirectSurface(imageInfo, pixelPtr, width*4);
      if (surface) {
        surface._canvas = null;
        surface._width = width;
        surface._height = height;
        surface._pixelLen = pixelLen;

        surface._pixelPtr = pixelPtr;
        // rasterDirectSurface does not initialize the pixels, so we clear them
        // to transparent black.
        surface.getCanvas().clear(DebuggerView.TRANSPARENT);
      }
      return surface;
    };


    DebuggerView.onRuntimeInitialized = function() {

      DebuggerView.SkSurface.prototype.flush = function() {
        this._flush();
        // Do we have an HTML canvas to write the pixels to?
        // We will not if this a GPU build or a raster surface, for example.
        if (this._canvas) {
          let pixels = new Uint8ClampedArray(DebuggerView.buffer, this._pixelPtr, this._pixelLen);
          let imageData = new ImageData(pixels, this._width, this._height);
          this._canvas.getContext('2d').putImageData(imageData, 0, 0);
        }
      };

      // Call dispose() instead of delete to clean up the underlying memory
      DebuggerView.SkSurface.prototype.dispose = function() {
        if (this._pixelPtr) {
          DebuggerView._free(this._pixelPtr);
        }
        this.delete();
      }
    }

    DebuggerView.currentContext = DebuggerView.currentContext || function() {
      // no op if this is a cpu-only build.
    };

    DebuggerView.setCurrentContext = DebuggerView.setCurrentContext || function() {
       // no op if this is a cpu-only build.
    };
}(Module)); // When this file is loaded in, the high level object is "Module";
