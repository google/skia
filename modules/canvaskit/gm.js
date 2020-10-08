// When this file is loaded in, the high level object is "Module";
var WasmGMTests = Module;
WasmGMTests.onRuntimeInitialized = function() {

  WasmGMTests.GetWebGLContext = function(canvas, webGLVersion) {
    if (!canvas) {
      throw 'null canvas passed into makeWebGLContext';
    }
    if (webGLVersion !== 1 && webGLVersion !== 2 ) {
      throw 'invalid webGLVersion';
    }
    var contextAttributes = {
      'alpha': 1,
      'depth': 0, // can be 0 because off-screen.
      'stencil': 0, // can be 0 because off-screen.
      'antialias': 0,
      'premultipliedAlpha': 1,
      'preserveDrawingBuffer': 0,
      'preferLowPowerToHighPerformance': 0,
      'failIfMajorPerformanceCaveat': 0,
      'enableExtensionsByDefault': 1,
      'explicitSwapControl': 0,
      'renderViaOffscreenBackBuffer': 0,
      'majorVersion': webGLVersion,
    };

    // Creates a WebGL context and sets it to be the current context.
    // These functions are defined in emscripten's library_webgl.js
    var handle = GL.createContext(canvas, contextAttributes);
    if (!handle) {
      return 0;
    }
    GL.makeContextCurrent(handle);
    return handle;
  };

}
