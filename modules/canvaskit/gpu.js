// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the GPU version of canvaskit.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      function get(obj, attr, defaultValue) {
        if (obj && obj.hasOwnProperty(attr)) {
          return obj[attr];
        }
        return defaultValue;
      }

      function makeWebGLContext(canvas, attrs) {
        var contextAttributes = {
          alpha: get(attrs, 'alpha', 1),
          depth: get(attrs, 'depth', 1),
          stencil: get(attrs, 'stencil', 8),
          antialias: get(attrs, 'antialias', 1),
          premultipliedAlpha: get(attrs, 'premultipliedAlpha', 1),
          preserveDrawingBuffer: get(attrs, 'preserveDrawingBuffer', 0),
          preferLowPowerToHighPerformance: get(attrs, 'preferLowPowerToHighPerformance', 0),
          failIfMajorPerformanceCaveat: get(attrs, 'failIfMajorPerformanceCaveat', 0),
          majorVersion: get(attrs, 'majorVersion', 2),
          minorVersion: get(attrs, 'minorVersion', 0),
          enableExtensionsByDefault: get(attrs, 'enableExtensionsByDefault', 1),
          explicitSwapControl: get(attrs, 'explicitSwapControl', 0),
          renderViaOffscreenBackBuffer: get(attrs, 'renderViaOffscreenBackBuffer', 0),
        };
        if (!canvas) {
          SkDebug('null canvas passed into makeWebGLContext');
          return 0;
        }
        // This check is from the emscripten version
        if (contextAttributes['explicitSwapControl']) {
          SkDebug('explicitSwapControl is not supported');
          return 0;
        }
        // GL is an enscripten provided helper
        // See https://github.com/emscripten-core/emscripten/blob/incoming/src/library_webgl.js
        var ctx = GL.createContext(canvas, contextAttributes);

        if (!ctx && contextAttributes.majorVersion > 1) {
          contextAttributes.majorVersion = 1;  // fall back to WebGL 1.0
          contextAttributes.minorVersion = 0;
          ctx = GL.createContext(canvas, contextAttributes);
        }
        return ctx;
      }

      CanvasKit.GetWebGLContext = function(canvas, attrs) {
        return makeWebGLContext(canvas, attrs);
      };

      // arg can be of types:
      //  - String - in which case it is interpreted as an id of a
      //          canvas element.
      //  - HTMLCanvasElement - in which the provided canvas element will
      //          be used directly.
      // Width and height can be provided to override those on the canvas
      // element, or specify a height for when a context is provided.
      CanvasKit.MakeWebGLCanvasSurface = function(arg, width, height) {
        var canvas = arg;
        if (canvas.tagName !== 'CANVAS') {
          canvas = document.getElementById(arg);
          if (!canvas) {
            throw 'Canvas with id ' + arg + ' was not found';
          }
        }
        // we are ok with all the defaults
        var ctx = this.GetWebGLContext(canvas);

        if (!ctx || ctx < 0) {
          throw 'failed to create webgl context: err ' + ctx;
        }

        if (!canvas && (!width || !height)) {
          throw 'height and width must be provided with context';
        }

        var grcontext = this.MakeGrContext(ctx);

        if (grcontext) {
           // Bump the default resource cache limit.
          var RESOURCE_CACHE_BYTES = 256 * 1024 * 1024;
          grcontext.setResourceCacheLimitBytes(RESOURCE_CACHE_BYTES);
        }


        // Maybe better to use clientWidth/height.  See:
        // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
        var surface = this.MakeOnScreenGLSurface(grcontext,
                                                 width  || canvas.width,
                                                 height || canvas.height);
        if (!surface) {
          SkDebug('falling back from GPU implementation to a SW based one');
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
        return surface;
      };
      // Default to trying WebGL first.
      CanvasKit.MakeCanvasSurface = CanvasKit.MakeWebGLCanvasSurface;
    });
}(Module)); // When this file is loaded in, the high level object is "Module";
