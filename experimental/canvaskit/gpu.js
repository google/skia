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
        // These defaults come from the emscripten _emscripten_webgl_create_context
        var contextAttributes = {
          alpha: get(attrs, 'alpha', 1),
          depth: get(attrs, 'depth', 1),
          stencil: get(attrs, 'stencil', 0),
          antialias: get(attrs, 'antialias', 1),
          premultipliedAlpha: get(attrs, 'premultipliedAlpha', 1),
          preserveDrawingBuffer: get(attrs, 'preserveDrawingBuffer', 0),
          preferLowPowerToHighPerformance: get(attrs, 'preferLowPowerToHighPerformance', 0),
          failIfMajorPerformanceCaveat: get(attrs, 'failIfMajorPerformanceCaveat', 0),
          majorVersion: get(attrs, 'majorVersion', 1),
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
        return GL.createContext(canvas, contextAttributes);
      }

      // arg can be of types:
      //  - String - in which case it is interpreted as an id of a
      //          canvas element.
      //  - HTMLCanvasElement - in which the provided canvas element will
      //          be used directly.
      //  - int - in which case it will be used as a WebGLContext. Only 1.0
      //          contexts are known to work for now.
      // Width and height can be provided to override those on the canvas
      // element, or specify a height for when a context is provided.
      CanvasKit.MakeWebGLCanvasSurface = function(arg, width, height) {
        var ctx = arg;
        // ctx is only > 0 if it's an int, and thus a valid context
        if (!(ctx > 0)) {
          var canvas = arg;
          if (canvas.tagName !== 'CANVAS') {
            canvas = document.getElementById(arg);
            if (!canvas) {
              throw 'Canvas with id ' + arg + ' was not found';
            }
          }
          // we are ok with all the defaults
          ctx = makeWebGLContext(canvas);
        }

        if (!ctx || ctx < 0) {
          throw 'failed to create webgl context: err ' + ctx;
        }

        if (!canvas && (!width || !height)) {
          throw 'height and width must be provided with context';
        }

        // Maybe better to use clientWidth/height.  See:
        // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
        var surface = this._getWebGLSurface(ctx, width || canvas.width,
                                            height || canvas.height);
        if (!surface) {
          SkDebug('falling back from GPU implementation to a SW based one');
          return CanvasKit.MakeSWCanvasSurface(arg);
        }
        return surface;
      };
      // Default to trying WebGL first.
      CanvasKit.MakeCanvasSurface = CanvasKit.MakeWebGLCanvasSurface;
    });
}(Module)); // When this file is loaded in, the high level object is "Module";
