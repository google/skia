// Adds compile-time JS functions to augment the CanvasKit interface.
// Implementations in this file are considerate of GPU builds, i.e. some
// behavior is predicated on whether or not this is being compiled alongside
// gpu.js.
(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {
    // Takes in an html id or a canvas element
    CanvasKit.MakeSWCanvasSurface = function(idOrElement) {
      var canvas = idOrElement;
      if (canvas.tagName !== 'CANVAS') {
        // TODO(nifong): unit test
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

    // Note that color spaces are currently not supported in CPU surfaces. due to the limitation
    // canvas.getContext('2d').putImageData imposes a limitation of using an RGBA_8888 color type.
    // TODO(nifong): support WGC color spaces while still using an RGBA_8888 color type when
    // on a cpu backend.
    CanvasKit.MakeSurface = function(width, height) {
      var imageInfo = {
        'width':  width,
        'height': height,
        'colorType': CanvasKit.ColorType.RGBA_8888,
        // Since we are sending these pixels directly into the HTML canvas,
        // (and those pixels are un-premultiplied, i.e. straight r,g,b,a)
        'alphaType': CanvasKit.AlphaType.Unpremul,
        'colorSpace': CanvasKit.ColorSpace.SRGB,
      };
      var pixelLen = width * height * 4; // it's 8888, so 4 bytes per pixel
      // Allocate the buffer of pixels to be drawn into.
      var pixelPtr = CanvasKit._malloc(pixelLen);

      // Experiments with using RasterDirect vs Raster showed a 10% slowdown
      // over the traditional Surface::MakeRaster approach. This was exacerbated when
      // the surface was drawing to Premul and we had to convert to Unpremul each frame
      // (up to a 10x further slowdown).
      var surface = CanvasKit.Surface._makeRasterDirect(imageInfo, pixelPtr, width*4);
      if (surface) {
        surface._canvas = null;
        surface._width = width;
        surface._height = height;
        surface._pixelLen = pixelLen;

        surface._pixelPtr = pixelPtr;
        // rasterDirectSurface does not initialize the pixels, so we clear them
        // to transparent black.
        surface.getCanvas().clear(CanvasKit.TRANSPARENT);
      }
      return surface;
    };

    CanvasKit.MakeRasterDirectSurface = function(imageInfo, mallocObj, bytesPerRow) {
      return CanvasKit.Surface._makeRasterDirect(imageInfo, mallocObj['byteOffset'], bytesPerRow);
    };

    // For GPU builds, simply proxies to native code flush.  For CPU builds,
    // also updates the underlying HTML canvas, optionally with dirtyRect.
    CanvasKit.Surface.prototype.flush = function(dirtyRect) {
      CanvasKit.setCurrentContext(this._context);
      this._flush();
      // Do we have an HTML canvas to write the pixels to?
      // We will not have a canvas if this a GPU build, for example.
      if (this._canvas) {
        var pixels = new Uint8ClampedArray(CanvasKit.HEAPU8.buffer, this._pixelPtr, this._pixelLen);
        var imageData = new ImageData(pixels, this._width, this._height);

        if (!dirtyRect) {
          this._canvas.getContext('2d').putImageData(imageData, 0, 0);
        } else {
          this._canvas.getContext('2d').putImageData(imageData, 0, 0,
                                                     dirtyRect[0], dirtyRect[1],
                                                     dirtyRect[2] - dirtyRect[0],
                                                     dirtyRect[3] - dirtyRect[1]);
        }
      }
    };

    // Call dispose() instead of delete to clean up the underlying memory.
    // TODO(kjlubick) get rid of this and just wrap around delete().
    CanvasKit.Surface.prototype.dispose = function() {
      if (this._pixelPtr) {
        CanvasKit._free(this._pixelPtr);
      }
      this.delete();
    };

    CanvasKit.setCurrentContext = CanvasKit.setCurrentContext || function() {
       // no op if this is a cpu-only build.
    };
  });
}(Module)); // When this file is loaded in, the high level object is "Module";
