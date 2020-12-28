// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the GPU version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      function get(obj, attr, defaultValue) {
        if (obj && obj.hasOwnProperty(attr)) {
          return obj[attr];
        }
        return defaultValue;
      }

      CanvasKit.GetWebGLContext = function(canvas, attrs) {
        if (!canvas) {
          throw 'null canvas passed into makeWebGLContext';
        }
        var contextAttributes = {
          'alpha': get(attrs, 'alpha', 1),
          'depth': get(attrs, 'depth', 1),
          'stencil': get(attrs, 'stencil', 8),
          'antialias': get(attrs, 'antialias', 0),
          'premultipliedAlpha': get(attrs, 'premultipliedAlpha', 1),
          'preserveDrawingBuffer': get(attrs, 'preserveDrawingBuffer', 0),
          'preferLowPowerToHighPerformance': get(attrs, 'preferLowPowerToHighPerformance', 0),
          'failIfMajorPerformanceCaveat': get(attrs, 'failIfMajorPerformanceCaveat', 0),
          'enableExtensionsByDefault': get(attrs, 'enableExtensionsByDefault', 1),
          'explicitSwapControl': get(attrs, 'explicitSwapControl', 0),
          'renderViaOffscreenBackBuffer': get(attrs, 'renderViaOffscreenBackBuffer', 0),
        };

        if (attrs && attrs['majorVersion']) {
          contextAttributes['majorVersion'] = attrs['majorVersion']
        } else {
          // Default to WebGL 2 if available and not specified.
          contextAttributes['majorVersion'] = (typeof WebGL2RenderingContext !== 'undefined') ? 2 : 1;
        }

        // This check is from the emscripten version
        if (contextAttributes['explicitSwapControl']) {
          throw 'explicitSwapControl is not supported';
        }
        // Creates a WebGL context and sets it to be the current context.
        // These functions are defined in emscripten's library_webgl.js
        var handle = GL.createContext(canvas, contextAttributes);
        if (!handle) {
          return 0;
        }
        GL.makeContextCurrent(handle);
        return handle;
      };

      CanvasKit.deleteContext = function(handle) {
        GL.deleteContext(handle);
      };

      // idOrElement can be of types:
      //  - String - in which case it is interpreted as an id of a
      //          canvas element.
      //  - HTMLCanvasElement - in which the provided canvas element will
      //          be used directly.
      // colorSpace - sk_sp<ColorSpace> - one of the supported color spaces:
      //          CanvasKit.ColorSpace.SRGB
      //          CanvasKit.ColorSpace.DISPLAY_P3
      //          CanvasKit.ColorSpace.ADOBE_RGB
      CanvasKit.MakeWebGLCanvasSurface = function(idOrElement, colorSpace, attrs) {
        colorSpace = colorSpace || null;
        var canvas = idOrElement;
        var isHTMLCanvas = typeof HTMLCanvasElement !== 'undefined' && canvas instanceof HTMLCanvasElement;
        var isOffscreenCanvas = typeof OffscreenCanvas !== 'undefined' && canvas instanceof OffscreenCanvas;
        if (!isHTMLCanvas && !isOffscreenCanvas) {
          canvas = document.getElementById(idOrElement);
          if (!canvas) {
            throw 'Canvas with id ' + idOrElement + ' was not found';
          }
        }

        var ctx = this.GetWebGLContext(canvas, attrs);
        if (!ctx || ctx < 0) {
          throw 'failed to create webgl context: err ' + ctx;
        }

        var grcontext = this.MakeGrContext(ctx);

        // Note that canvas.width/height here is used because it gives the size of the buffer we're
        // rendering into. This may not be the same size the element is displayed on the page, which
        // constrolled by css, and available in canvas.clientWidth/height.
        var surface = this.MakeOnScreenGLSurface(grcontext, canvas.width, canvas.height, colorSpace);
        if (!surface) {
          Debug('falling back from GPU implementation to a SW based one');
          // we need to throw away the old canvas (which was locked to
          // a webGL context) and create a new one so we can
          var newCanvas = canvas.cloneNode(true);
          var parent = canvas.parentNode;
          parent.replaceChild(newCanvas, canvas);
          // add a class so the user can detect that it was replaced.
          newCanvas.classList.add('ck-replaced');

          return CanvasKit.MakeSWCanvasSurface(newCanvas);
        }
        surface._context = ctx;
        surface.grContext = grcontext;
        surface.openGLversion = canvas.GLctxObject.version;
        return surface;
      };
      // Default to trying WebGL first.
      CanvasKit.MakeCanvasSurface = CanvasKit.MakeWebGLCanvasSurface;
    });
}(Module)); // When this file is loaded in, the high level object is "Module";
