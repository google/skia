// Adds compile-time JS functions to augment the SkottieKit interface.
// Specifically, anything that should only be on the CPU version of SkottieKit.
(function(SkottieKit){
SkottieKit._extraInitializations = SkottieKit._extraInitializations || [];
SkottieKit._extraInitializations.push(function() {
  // Takes in an html id or a canvas element
  SkottieKit.MakeSWCanvasSurface = function(idOrElement) {
      var canvas = idOrElement;
      if (canvas.tagName !== 'CANVAS') {
        canvas = document.getElementById(idOrElement);
        if (!canvas) {
          throw 'Canvas with id ' + idOrElement + ' was not found';
        }
      }
    // Maybe better to use clientWidth/height.  See:
    // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
    var surface = SkottieKit.MakeInMemorySurface(canvas.width, canvas.height);
    if (surface) {
      surface._canvas = canvas;
    }
    return surface;
  };

  // Don't over-write the MakeCanvasSurface set by gpu.js if it exists.
  if (!SkottieKit.MakeCanvasSurface) {
    SkottieKit.MakeCanvasSurface = SkottieKit.MakeSWCanvasSurface;
  }

  SkottieKit.MakeInMemorySurface = function(width, height) {
    /* @dict */
    var imageInfo = {
      'width':  width,
      'height': height,
      'colorType': SkottieKit.ColorType.RGBA_8888,
      // Since we are sending these pixels directly into the HTML canvas,
      // (and those pixels are un-premultiplied, i.e. straight r,g,b,a)
      'alphaType': SkottieKit.AlphaType.Unpremul,
    }
    var pixelLen = width * height * 4; // it's 8888, so 4 bytes per pixel
    // Allocate the buffer of pixels to be drawn into.
    var pixelPtr = SkottieKit._malloc(pixelLen);

    var surface = this._getRasterDirectSurface(imageInfo, pixelPtr, width*4);
    if (surface) {
      surface._canvas = null;
      surface._width = width;
      surface._height = height;
      surface._pixelLen = pixelLen;

      surface._pixelPtr = pixelPtr;
      // rasterDirectSurface does not initialize the pixels, so we clear them
      // to transparent black.
      surface.getCanvas().clear(SkottieKit.TRANSPARENT);
    }
    return surface;
  };

  SkottieKit.SkSurface.prototype.flush = function() {
    this._flush();
    // Do we have an HTML canvas to write the pixels to?
    // We will not if this a GPU build or a raster surface, for example.
    if (this._canvas) {
      var pixels = new Uint8ClampedArray(SkottieKit.HEAPU8.buffer, this._pixelPtr, this._pixelLen);
      var imageData = new ImageData(pixels, this._width, this._height);

      this._canvas.getContext('2d').putImageData(imageData, 0, 0);
    }
  };

  // Call dispose() instead of delete to clean up the underlying memory
  SkottieKit.SkSurface.prototype.dispose = function() {
    if (this._pixelPtr) {
      SkottieKit._free(this._pixelPtr);
    }
    this.delete();
  }

  SkottieKit.currentContext = SkottieKit.currentContext || function() {
    // no op if this is a cpu-only build.
  };

  SkottieKit.setCurrentContext = SkottieKit.setCurrentContext || function() {
     // no op if this is a cpu-only build.
  };
});
}(Module)); // When this file is loaded in, the high level object is "Module";

