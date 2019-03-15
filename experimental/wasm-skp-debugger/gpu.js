// Adds compile-time JS functions to augment the DebuggerView interface.
// Specifically, anything that should only be on the GPU version of DebuggerView.
(function(DebuggerView){
    function makeWebGLContext(canvas, attrs) {
      // These defaults come from the emscripten _emscripten_webgl_create_context
      // TODO(nifong): All these settings appear to be ignored. investigate.
      var contextAttributes = {
        alpha: 1,
        depth: 1,
        stencil: 0,
        antialias: 1,
        premultipliedAlpha: 1,
        preserveDrawingBuffer: 0,
        preferLowPowerToHighPerformance: 0,
        failIfMajorPerformanceCaveat: 0,
        majorVersion: 1,
        minorVersion: 0,
        enableExtensionsByDefault: 1,
        explicitSwapControl: 0,
        renderViaOffscreenBackBuffer: 0,
      };
      if (!canvas) {
        console.log('null canvas passed into makeWebGLContext');
        return 0;
      }
      // This check is from the emscripten version
      if (contextAttributes['explicitSwapControl']) {
        console.log('explicitSwapControl is not supported');
        return 0;
      }
      // GL is an enscripten provided helper
      // See https://github.com/emscripten-core/emscripten/blob/incoming/src/library_webgl.js
      let context = GL.createContext(canvas, contextAttributes);
      if (!context) {
        console.log('Could not get a WebGL context from the canvas element.');
      }
      console.log('Made Web Gl Canvas Surface');
      return context
    }

    DebuggerView.GetWebGLContext = function(canvas, attrs) {
      return makeWebGLContext(canvas, attrs);
    };

    // arg can be of types:
    //  - String - in which case it is interpreted as an id of a
    //          canvas element.
    //  - HTMLCanvasElement - in which the provided canvas element will
    //          be used directly.
    // Width and height can be provided to override those on the canvas
    // element, or specify a height for when a context is provided.
    DebuggerView.MakeWebGLCanvasSurface = function(arg, width, height) {
      var canvas = arg;
      if (canvas.tagName !== 'CANVAS') {
        canvas = document.getElementById(arg);
        if (!canvas) {
          throw 'Canvas with id ' + arg + ' was not found';
        }
      }
      // we are ok with all the defaults
      var ctx = DebuggerView.GetWebGLContext(canvas);

      if (!ctx || ctx < 0) {
        throw 'failed to create webgl context: err ' + ctx;
      }

      if (!canvas && (!width || !height)) {
        throw 'height and width must be provided with context';
      }

      var grcontext = this.MakeGrContext(ctx);
      if (!grcontext) {
        throw (
          'failed to create grcontext. Open GL driver may not support all needed functions: err '
          + grcontext);
      }

      // Maybe better to use clientWidth/height.  See:
      // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
      var surface = this.MakeOnScreenGLSurface(grcontext,
                                               width  || canvas.width,
                                               height || canvas.height);
      if (!surface) {
        // Don't fall back silently in the debugger, the user explicitly controls which backend he
        // wants via the UI. Calling function may catch this and show the user an error.
        throw ('Failed to create OpenGL surface. GPU Backend unavailable.');
      }
      return surface;
    };
    // Default to trying WebGL first.
    DebuggerView.MakeCanvasSurface = DebuggerView.MakeWebGLCanvasSurface;
}(Module)); // When this file is loaded in, the high level object is "Module";
