
var CanvasKitInit = (function() {
  var _scriptDir = typeof document !== 'undefined' && document.currentScript ? document.currentScript.src : undefined;
  if (typeof __filename !== 'undefined') _scriptDir = _scriptDir || __filename;
  return (
function(CanvasKitInit) {
  CanvasKitInit = CanvasKitInit || {};



// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(Module) { ..generated code.. }
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module = typeof CanvasKitInit !== 'undefined' ? CanvasKitInit : {};

// Set up the promise that indicates the Module is initialized
var readyPromiseResolve, readyPromiseReject;
Module['ready'] = new Promise(function(resolve, reject) {
  readyPromiseResolve = resolve;
  readyPromiseReject = reject;
});

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_malloc')) {
        Object.defineProperty(Module['ready'], '_malloc', { configurable: true, get: function() { abort('You are getting _malloc on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_malloc', { configurable: true, set: function() { abort('You are setting _malloc on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_free')) {
        Object.defineProperty(Module['ready'], '_free', { configurable: true, get: function() { abort('You are getting _free on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_free', { configurable: true, set: function() { abort('You are setting _free on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_emscripten_stack_get_end')) {
        Object.defineProperty(Module['ready'], '_emscripten_stack_get_end', { configurable: true, get: function() { abort('You are getting _emscripten_stack_get_end on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_emscripten_stack_get_end', { configurable: true, set: function() { abort('You are setting _emscripten_stack_get_end on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_emscripten_stack_get_free')) {
        Object.defineProperty(Module['ready'], '_emscripten_stack_get_free', { configurable: true, get: function() { abort('You are getting _emscripten_stack_get_free on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_emscripten_stack_get_free', { configurable: true, set: function() { abort('You are setting _emscripten_stack_get_free on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_emscripten_stack_init')) {
        Object.defineProperty(Module['ready'], '_emscripten_stack_init', { configurable: true, get: function() { abort('You are getting _emscripten_stack_init on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_emscripten_stack_init', { configurable: true, set: function() { abort('You are setting _emscripten_stack_init on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '___cxa_demangle')) {
        Object.defineProperty(Module['ready'], '___cxa_demangle', { configurable: true, get: function() { abort('You are getting ___cxa_demangle on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '___cxa_demangle', { configurable: true, set: function() { abort('You are setting ___cxa_demangle on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_stackSave')) {
        Object.defineProperty(Module['ready'], '_stackSave', { configurable: true, get: function() { abort('You are getting _stackSave on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_stackSave', { configurable: true, set: function() { abort('You are setting _stackSave on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_stackRestore')) {
        Object.defineProperty(Module['ready'], '_stackRestore', { configurable: true, get: function() { abort('You are getting _stackRestore on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_stackRestore', { configurable: true, set: function() { abort('You are setting _stackRestore on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_stackAlloc')) {
        Object.defineProperty(Module['ready'], '_stackAlloc', { configurable: true, get: function() { abort('You are getting _stackAlloc on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_stackAlloc', { configurable: true, set: function() { abort('You are setting _stackAlloc on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '___wasm_call_ctors')) {
        Object.defineProperty(Module['ready'], '___wasm_call_ctors', { configurable: true, get: function() { abort('You are getting ___wasm_call_ctors on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '___wasm_call_ctors', { configurable: true, set: function() { abort('You are setting ___wasm_call_ctors on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '___errno_location')) {
        Object.defineProperty(Module['ready'], '___errno_location', { configurable: true, get: function() { abort('You are getting ___errno_location on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '___errno_location', { configurable: true, set: function() { abort('You are setting ___errno_location on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_setThrew')) {
        Object.defineProperty(Module['ready'], '_setThrew', { configurable: true, get: function() { abort('You are getting _setThrew on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_setThrew', { configurable: true, set: function() { abort('You are setting _setThrew on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '__get_tzname')) {
        Object.defineProperty(Module['ready'], '__get_tzname', { configurable: true, get: function() { abort('You are getting __get_tzname on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '__get_tzname', { configurable: true, set: function() { abort('You are setting __get_tzname on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '__get_daylight')) {
        Object.defineProperty(Module['ready'], '__get_daylight', { configurable: true, get: function() { abort('You are getting __get_daylight on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '__get_daylight', { configurable: true, set: function() { abort('You are setting __get_daylight on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '__get_timezone')) {
        Object.defineProperty(Module['ready'], '__get_timezone', { configurable: true, get: function() { abort('You are getting __get_timezone on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '__get_timezone', { configurable: true, set: function() { abort('You are setting __get_timezone on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_memalign')) {
        Object.defineProperty(Module['ready'], '_memalign', { configurable: true, get: function() { abort('You are getting _memalign on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_memalign', { configurable: true, set: function() { abort('You are setting _memalign on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_memset')) {
        Object.defineProperty(Module['ready'], '_memset', { configurable: true, get: function() { abort('You are getting _memset on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_memset', { configurable: true, set: function() { abort('You are setting _memset on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_realloc')) {
        Object.defineProperty(Module['ready'], '_realloc', { configurable: true, get: function() { abort('You are getting _realloc on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_realloc', { configurable: true, set: function() { abort('You are setting _realloc on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_testSetjmp')) {
        Object.defineProperty(Module['ready'], '_testSetjmp', { configurable: true, get: function() { abort('You are getting _testSetjmp on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_testSetjmp', { configurable: true, set: function() { abort('You are setting _testSetjmp on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_saveSetjmp')) {
        Object.defineProperty(Module['ready'], '_saveSetjmp', { configurable: true, get: function() { abort('You are getting _saveSetjmp on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_saveSetjmp', { configurable: true, set: function() { abort('You are setting _saveSetjmp on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], '_emscripten_main_thread_process_queued_calls')) {
        Object.defineProperty(Module['ready'], '_emscripten_main_thread_process_queued_calls', { configurable: true, get: function() { abort('You are getting _emscripten_main_thread_process_queued_calls on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], '_emscripten_main_thread_process_queued_calls', { configurable: true, set: function() { abort('You are setting _emscripten_main_thread_process_queued_calls on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

      if (!Object.getOwnPropertyDescriptor(Module['ready'], 'onRuntimeInitialized')) {
        Object.defineProperty(Module['ready'], 'onRuntimeInitialized', { configurable: true, get: function() { abort('You are getting onRuntimeInitialized on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
        Object.defineProperty(Module['ready'], 'onRuntimeInitialized', { configurable: true, set: function() { abort('You are setting onRuntimeInitialized on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js') } });
      }
    

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
function Debug(msg) {
  console.warn(msg);
}
/** @const */ var IsDebug = true;

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

    CanvasKit.currentContext = CanvasKit.currentContext || function() {
      // no op if this is a cpu-only build.
    };

    CanvasKit.setCurrentContext = CanvasKit.setCurrentContext || function() {
       // no op if this is a cpu-only build.
    };
  });
}(Module)); // When this file is loaded in, the high level object is "Module";

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

// Adds compile-time JS functions to augment the CanvasKit interface.
(function(CanvasKit) {


// This intentionally dangles because we want all the
// JS code to be in the same scope, but JS doesn't support
// namespaces like C++ does. Thus, we simply include this
// preamble.js file, all the source .js files and then postamble.js
// to bundle everything in the same scope.
/*
 * This file houses functions that deal with color.
 */

// Constructs a Color with the same API as CSS's rgba(), that is
// r,g,b are 0-255, and a is 0.0 to 1.0.
// if a is omitted, it will be assumed to be 1.0
// Internally, Colors are a TypedArray of four unpremultiplied 32-bit floats: a, r, g, b
// In order to construct one with more precision or in a wider gamut, use
// CanvasKit.Color4f
CanvasKit.Color = function(r, g, b, a) {
  if (a === undefined) {
      a = 1;
  }
  return CanvasKit.Color4f(clamp(r)/255, clamp(g)/255, clamp(b)/255, a);
};

// Constructs a Color as a 32 bit unsigned integer, with 8 bits assigned to each channel.
// Channels are expected to be between 0 and 255 and will be clamped as such.
CanvasKit.ColorAsInt = function(r, g, b, a) {
  // default to opaque
  if (a === undefined) {
      a = 255;
  }
  // This is consistent with how Skia represents colors in C++, as an unsigned int.
  // This is also consistent with how Flutter represents colors:
  // https://github.com/flutter/engine/blob/243bb59c7179a7e701ce478080d6ce990710ae73/lib/web_ui/lib/src/ui/painting.dart#L50
  return (((clamp(a) << 24) | (clamp(r) << 16) | (clamp(g) << 8) | (clamp(b) << 0)
   & 0xFFFFFFF) // This truncates the unsigned to 32 bits and signals to JS engines they can
                // represent the number with an int instead of a double.
    >>> 0);     // This makes the value an unsigned int.
};
// Construct a 4-float color.
// Opaque if opacity is omitted.
CanvasKit.Color4f = function(r, g, b, a) {
  if (a === undefined) {
    a = 1;
  }
  return Float32Array.of(r, g, b, a);
};

// Color constants use property getters to prevent other code from accidentally
// changing them.
Object.defineProperty(CanvasKit, 'TRANSPARENT', {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 0); }
});
Object.defineProperty(CanvasKit, 'BLACK', {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, 'WHITE', {
    get: function() { return CanvasKit.Color4f(1, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, 'RED', {
    get: function() { return CanvasKit.Color4f(1, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, 'GREEN', {
    get: function() { return CanvasKit.Color4f(0, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, 'BLUE', {
    get: function() { return CanvasKit.Color4f(0, 0, 1, 1); }
});
Object.defineProperty(CanvasKit, 'YELLOW', {
    get: function() { return CanvasKit.Color4f(1, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, 'CYAN', {
    get: function() { return CanvasKit.Color4f(0, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, 'MAGENTA', {
    get: function() { return CanvasKit.Color4f(1, 0, 1, 1); }
});

// returns a css style [r, g, b, a] from a CanvasKit.Color
// where r, g, b are returned as ints in the range [0, 255]
// where a is scaled between 0 and 1.0
CanvasKit.getColorComponents = function(color) {
  return [
    Math.floor(color[0]*255),
    Math.floor(color[1]*255),
    Math.floor(color[2]*255),
    color[3]
  ];
};

// parseColorString takes in a CSS color value and returns a CanvasKit.Color
// (which is an array of 4 floats in RGBA order). An optional colorMap
// may be provided which maps custom strings to values.
// In the CanvasKit canvas2d shim layer, we provide this map for processing
// canvas2d calls, but not here for code size reasons.
CanvasKit.parseColorString = function(colorStr, colorMap) {
  colorStr = colorStr.toLowerCase();
  // See https://drafts.csswg.org/css-color/#typedef-hex-color
  if (colorStr.startsWith('#')) {
    var r, g, b, a = 255;
    switch (colorStr.length) {
      case 9: // 8 hex chars #RRGGBBAA
        a = parseInt(colorStr.slice(7, 9), 16);
      case 7: // 6 hex chars #RRGGBB
        r = parseInt(colorStr.slice(1, 3), 16);
        g = parseInt(colorStr.slice(3, 5), 16);
        b = parseInt(colorStr.slice(5, 7), 16);
        break;
      case 5: // 4 hex chars #RGBA
        // multiplying by 17 is the same effect as
        // appending another character of the same value
        // e.g. e => ee == 14 => 238
        a = parseInt(colorStr.slice(4, 5), 16) * 17;
      case 4: // 6 hex chars #RGB
        r = parseInt(colorStr.slice(1, 2), 16) * 17;
        g = parseInt(colorStr.slice(2, 3), 16) * 17;
        b = parseInt(colorStr.slice(3, 4), 16) * 17;
        break;
    }
    return CanvasKit.Color(r, g, b, a/255);

  } else if (colorStr.startsWith('rgba')) {
    // Trim off rgba( and the closing )
    colorStr = colorStr.slice(5, -1);
    var nums = colorStr.split(',');
    return CanvasKit.Color(+nums[0], +nums[1], +nums[2],
                           valueOrPercent(nums[3]));
  } else if (colorStr.startsWith('rgb')) {
    // Trim off rgba( and the closing )
    colorStr = colorStr.slice(4, -1);
    var nums = colorStr.split(',');
    // rgb can take 3 or 4 arguments
    return CanvasKit.Color(+nums[0], +nums[1], +nums[2],
                           valueOrPercent(nums[3]));
  } else if (colorStr.startsWith('gray(')) {
    // TODO(kjlubick)
  } else if (colorStr.startsWith('hsl')) {
    // TODO(kjlubick)
  } else if (colorMap) {
    // Try for named color
    var nc = colorMap[colorStr];
    if (nc !== undefined) {
      return nc;
    }
  }
  Debug('unrecognized color ' + colorStr);
  return CanvasKit.BLACK;
};

function isCanvasKitColor(ob) {
  if (!ob) {
    return false;
  }
  return (ob.constructor === Float32Array && ob.length === 4);
}

// Warning information is lost by this conversion
function toUint32Color(c) {
  return ((clamp(c[3]*255) << 24) | (clamp(c[0]*255) << 16) | (clamp(c[1]*255) << 8) | (clamp(c[2]*255) << 0)) >>> 0;
}
// Accepts various colors representations and converts them to an array of int colors.
// Does not handle builders.
function assureIntColors(arr) {
  if (arr instanceof Float32Array) {
    var count = Math.floor(arr.length / 4);
    var result = new Uint32Array(count);
    for (var i = 0; i < count; i ++) {
      result[i] = toUint32Color(arr.slice(i*4, (i+1)*4));
    }
    return result;
  } else if (arr instanceof Uint32Array) {
    return arr;
  } else if (arr instanceof Array && arr[0] instanceof Float32Array) {
    return arr.map(toUint32Color);
  }
}

function uIntColorToCanvasKitColor(c) {
    return CanvasKit.Color(
     (c >> 16) & 0xFF,
     (c >>  8) & 0xFF,
     (c >>  0) & 0xFF,
    ((c >> 24) & 0xFF) / 255
  );
}

function valueOrPercent(aStr) {
  if (aStr === undefined) {
    return 1; // default to opaque.
  }
  var a = parseFloat(aStr);
  if (aStr && aStr.indexOf('%') !== -1) {
    return a / 100;
  }
  return a;
}

function clamp(c) {
  return Math.round(Math.max(0, Math.min(c || 0, 255)));
}

// TODO(kjlubick) delete this, as it is now trivial with 4f colors
CanvasKit.multiplyByAlpha = function(color, alpha) {
  // make a copy of the color so the function remains pure.
  var result = color.slice();
  result[3] = Math.max(0, Math.min(result[3] * alpha, 1));
  return result;
};
/*
 * This file houses utilities for copying blocks of memory to and from
 * the WASM heap.
 */

/**
 * Malloc returns a TypedArray backed by the C++ memory of the
 * given length. It should only be used by advanced users who
 * can manage memory and initialize values properly. When used
 * correctly, it can save copying of data between JS and C++.
 * When used incorrectly, it can lead to memory leaks.
 * Any memory allocated by CanvasKit.Malloc needs to be released with CanvasKit.Free.
 *
 * const mObj = CanvasKit.Malloc(Float32Array, 20);
 * Get a TypedArray view around the malloc'd memory (this does not copy anything).
 * const ta = mObj.toTypedArray();
 * // store data into ta
 * const cf = CanvasKit.ColorFilter.MakeMatrix(ta); // mObj could also be used.
 *
 * // eventually...
 * CanvasKit.Free(mObj);
 *
 * @param {TypedArray} typedArray - constructor for the typedArray.
 * @param {number} len - number of *elements* to store.
 */
CanvasKit.Malloc = function(typedArray, len) {
  var byteLen = len * typedArray.BYTES_PER_ELEMENT;
  var ptr = CanvasKit._malloc(byteLen);
  return {
    '_ck': true,
    'length': len,
    'byteOffset': ptr,
    typedArray: null,
    'subarray': function(start, end) {
      var sa = this['toTypedArray']().subarray(start, end);
      sa['_ck'] = true;
      return sa;
    },
    'toTypedArray': function() {
      // Check if the previously allocated array is still usable.
      // If it's falsy, then we haven't created an array yet.
      // If it's empty, then WASM resized memory and emptied the array.
      if (this.typedArray && this.typedArray.length) {
        return this.typedArray;
      }
      this.typedArray = new typedArray(CanvasKit.HEAPU8.buffer, ptr, len);
      // add a marker that this was allocated in C++ land
      this.typedArray['_ck'] = true;
      return this.typedArray;
    },
  };
};

/**
 * Free frees the memory returned by Malloc.
 * Any memory allocated by CanvasKit.Malloc needs to be released with CanvasKit.Free.
 */
CanvasKit.Free = function(mallocObj) {
  CanvasKit._free(mallocObj['byteOffset']);
  mallocObj['byteOffset'] = nullptr;
  // Set these to null to make sure the TypedArrays can be garbage collected.
  mallocObj['toTypedArray'] = null;
  mallocObj.typedArray = null;
};

// This helper will free the given pointer unless the provided array is one
// that was returned by CanvasKit.Malloc.
function freeArraysThatAreNotMallocedByUsers(ptr, arr) {
  if (arr && !arr['_ck']) {
    CanvasKit._free(ptr);
  }
}

// We define some "scratch" variables which will house both the pointer to
// memory we allocate at startup as well as a Malloc object, which we can
// use to get a TypedArray view of that memory.

var _scratch3x3MatrixPtr = nullptr;
var _scratch3x3Matrix;  // the result from CanvasKit.Malloc

var _scratch4x4MatrixPtr = nullptr;
var _scratch4x4Matrix;

var _scratchColorPtr = nullptr;
var _scratchColor;

var _scratchFourFloatsA;
var _scratchFourFloatsAPtr = nullptr;

var _scratchFourFloatsB;
var _scratchFourFloatsBPtr = nullptr;

var _scratchThreeFloatsA;
var _scratchThreeFloatsAPtr = nullptr;

var _scratchThreeFloatsB;
var _scratchThreeFloatsBPtr = nullptr;

var _scratchIRect;
var _scratchIRectPtr = nullptr;

var _scratchRRect;
var _scratchRRectPtr = nullptr;

var _scratchRRect2;
var _scratchRRect2Ptr = nullptr;

// arr can be a normal JS array or a TypedArray
// dest is a string like 'HEAPU32' that specifies the type the src array
// should be copied into.
// ptr can be optionally provided if the memory was already allocated.
// Callers should eventually free the data unless the C++ object owns the memory,
// or the provided pointer is a scratch pointer or a user-malloced value.
// see also freeArraysThatAreNotMallocedByUsers().
function copy1dArray(arr, dest, ptr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  // This was created with CanvasKit.Malloc, so it's already been copied.
  if (arr['_ck']) {
    return arr.byteOffset;
  }
  var bytesPerElement = CanvasKit[dest].BYTES_PER_ELEMENT;
  if (!ptr) {
    ptr = CanvasKit._malloc(arr.length * bytesPerElement);
  }
  // In c++ terms, the WASM heap is a uint8_t*, a long buffer/array of single
  // byte elements. When we run _malloc, we always get an offset/pointer into
  // that block of memory.
  // CanvasKit exposes some different views to make it easier to work with
  // different types. HEAPF32 for example, exposes it as a float*
  // However, to make the ptr line up, we have to do some pointer arithmetic.
  // Concretely, we need to convert ptr to go from an index into a 1-byte-wide
  // buffer to an index into a 4-byte-wide buffer (in the case of HEAPF32)
  // and thus we divide ptr by 4.
  // It is important to make sure we are grabbing the freshest view of the
  // memory possible because if we call _malloc and the heap needs to grow,
  // the TypedArrayView will no longer be valid.
  CanvasKit[dest].set(arr, ptr / bytesPerElement);
  return ptr;
}

// Copies an array of colors to wasm, returning an object with the pointer
// and info necessary to use the copied colors.
// Accepts either a flat Float32Array, flat Uint32Array or Array of Float32Arrays.
// If color is an object that was allocated with CanvasKit.Malloc, its pointer is
// returned and no extra copy is performed.
// TODO(nifong): have this accept color builders.
function copyFlexibleColorArray(colors) {
  var result = {
    colorPtr: nullptr,
    count: colors.length,
    colorType: CanvasKit.ColorType.RGBA_F32,
  };
  if (colors instanceof Float32Array) {
    result.colorPtr = copy1dArray(colors, 'HEAPF32');
    result.count = colors.length / 4;

  } else if (colors instanceof Uint32Array) {
    result.colorPtr = copy1dArray(colors, 'HEAPU32');
    result.colorType = CanvasKit.ColorType.RGBA_8888;

  } else if (colors instanceof Array) {
    result.colorPtr = copyColorArray(colors);
  } else {
    throw('Invalid argument to copyFlexibleColorArray, Not a color array '+typeof(colors));
  }
  return result;
}

function copyColorArray(arr) {
  if (!arr || !arr.length) {
    return nullptr;
  }
  // 4 floats per color, 4 bytes per float.
  var ptr = CanvasKit._malloc(arr.length * 4 * 4);

  var idx = 0;
  var adjustedPtr = ptr / 4; // cast the byte pointer into a float pointer.
  for (var r = 0; r < arr.length; r++) {
    for (var c = 0; c < 4; c++) {
      CanvasKit.HEAPF32[adjustedPtr + idx] = arr[r][c];
      idx++;
    }
  }
  return ptr;
}

var defaultPerspective = Float32Array.of(0, 0, 1);

// Copies the given DOMMatrix/Array/TypedArray to the CanvasKit heap and
// returns a pointer to the memory. This memory is a float* of length 9.
// If the passed in matrix is null/undefined, we return 0 (nullptr). The
// returned pointer should NOT be freed, as it is either null or a scratch
// pointer.
function copy3x3MatrixToWasm(matr) {
  if (!matr) {
    return nullptr;
  }

  if (matr.length) {
    if (matr.length === 6 || matr.length === 9) {
      // matr should be an array or typed array.
      copy1dArray(matr, 'HEAPF32', _scratch3x3MatrixPtr);
      if (matr.length === 6) {
        // Overwrite the last 3 floats with the default perspective. The divide
        // by 4 casts the pointer into a float pointer.
        CanvasKit.HEAPF32.set(defaultPerspective, 6 + _scratch3x3MatrixPtr / 4);
      }
      return _scratch3x3MatrixPtr;
    } else if (matr.length === 16) {
      // Downsample the 4x4 matrix into a 3x3
      var wasm3x3Matrix = _scratch3x3Matrix['toTypedArray']();
      wasm3x3Matrix[0] = matr[0];
      wasm3x3Matrix[1] = matr[1];
      wasm3x3Matrix[2] = matr[3];

      wasm3x3Matrix[3] = matr[4];
      wasm3x3Matrix[4] = matr[5];
      wasm3x3Matrix[5] = matr[7];

      wasm3x3Matrix[6] = matr[12];
      wasm3x3Matrix[7] = matr[13];
      wasm3x3Matrix[8] = matr[15];
      return _scratch3x3MatrixPtr;
    }
    throw 'invalid matrix size';
  }
  var wasm3x3Matrix = _scratch3x3Matrix['toTypedArray']();
  // Try as if it's a DOMMatrix. Reminder that DOMMatrix is column-major.
  wasm3x3Matrix[0] = matr.m11;
  wasm3x3Matrix[1] = matr.m21;
  wasm3x3Matrix[2] = matr.m41;

  wasm3x3Matrix[3] = matr.m12;
  wasm3x3Matrix[4] = matr.m22;
  wasm3x3Matrix[5] = matr.m42;

  wasm3x3Matrix[6] = matr.m14;
  wasm3x3Matrix[7] = matr.m24;
  wasm3x3Matrix[8] = matr.m44;
  return _scratch3x3MatrixPtr;
}


// Copies the given DOMMatrix/Array/TypedArray to the CanvasKit heap and
// returns a pointer to the memory. This memory is a float* of length 16.
// If the passed in matrix is null/undefined, we return 0 (nullptr). The
// returned pointer should NOT be freed, as it is either null or a scratch
// pointer.
function copy4x4MatrixToWasm(matr) {
  if (!matr) {
    return nullptr;
  }
  var wasm4x4Matrix = _scratch4x4Matrix['toTypedArray']();
  if (matr.length) {
    if (matr.length !== 16 && matr.length !== 6 && matr.length !== 9) {
      throw 'invalid matrix size';
    }
    if (matr.length === 16) {
      // matr should be an array or typed array.
      return copy1dArray(matr, 'HEAPF32', _scratch4x4MatrixPtr);
    }
    // Upscale the row-major 3x3 or 3x2 matrix into a 4x4 row-major matrix
    // TODO(skbug.com/10108) This will need to change when we convert our
    //   JS 4x4 to be column-major.
    // When upscaling, we need to overwrite the 3rd column and the 3rd row with
    // 0s. It's easiest to just do that with a fill command.
    wasm4x4Matrix.fill(0);
    wasm4x4Matrix[0] = matr[0];
    wasm4x4Matrix[1] = matr[1];
    // skip col 2
    wasm4x4Matrix[3] = matr[2];

    wasm4x4Matrix[4] = matr[3];
    wasm4x4Matrix[5] = matr[4];
    // skip col 2
    wasm4x4Matrix[7] = matr[5];

    // skip row 2

    wasm4x4Matrix[12] = matr[6];
    wasm4x4Matrix[13] = matr[7];
    // skip col 2
    wasm4x4Matrix[15] = matr[8];

    if (matr.length === 6) {
      // fix perspective for the 3x2 case (from above, they will be undefined).
      wasm4x4Matrix[12]=0;
      wasm4x4Matrix[13]=0;
      wasm4x4Matrix[15]=1;
    }
    return _scratch4x4MatrixPtr;
  }
  // Try as if it's a DOMMatrix. Reminder that DOMMatrix is column-major.
  wasm4x4Matrix[0] = matr.m11;
  wasm4x4Matrix[1] = matr.m21;
  wasm4x4Matrix[2] = matr.m31;
  wasm4x4Matrix[3] = matr.m41;

  wasm4x4Matrix[4] = matr.m12;
  wasm4x4Matrix[5] = matr.m22;
  wasm4x4Matrix[6] = matr.m32;
  wasm4x4Matrix[7] = matr.m42;

  wasm4x4Matrix[8] = matr.m13;
  wasm4x4Matrix[9] = matr.m23;
  wasm4x4Matrix[10] = matr.m33;
  wasm4x4Matrix[11] = matr.m43;

  wasm4x4Matrix[12] = matr.m14;
  wasm4x4Matrix[13] = matr.m24;
  wasm4x4Matrix[14] = matr.m34;
  wasm4x4Matrix[15] = matr.m44;
  return _scratch4x4MatrixPtr;
}

// copies a 4x4 matrix at the given pointer into a JS array.
function copy4x4MatrixFromWasm(matrPtr) {
  // read them out into an array. TODO(kjlubick): If we change Matrix to be
  // typedArrays, then we should return a typed array here too.
  var rv = new Array(16);
  for (var i = 0; i < 16; i++) {
    rv[i] = CanvasKit.HEAPF32[matrPtr/4 + i]; // divide by 4 to cast to float.
  }
  return rv;
}

// copies the given floats into the wasm heap as an SkColor4f. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyColorToWasm(color4f, ptr) {
  return copy1dArray(color4f, 'HEAPF32', ptr || _scratchColorPtr);
}

// copies the given color into the wasm heap. Callers do not need to free the returned pointer.
function copyColorComponentsToWasm(r, g, b, a) {
  var colors = _scratchColor['toTypedArray']();
  colors[0] = r;
  colors[1] = g;
  colors[2] = b;
  colors[3] = a;
  return _scratchColorPtr;
}

// copies the given color into the wasm heap. Callers must free the returned pointer.
function copyColorToWasmNoScratch(color4f) {
  // TODO(kjlubick): accept 4 floats or int color
  return copy1dArray(color4f, 'HEAPF32');
}

// copies the four floats at the given pointer in a js Float32Array
function copyColorFromWasm(colorPtr) {
  var rv = new Float32Array(4);
  for (var i = 0; i < 4; i++) {
    rv[i] = CanvasKit.HEAPF32[colorPtr/4 + i]; // divide by 4 to cast to float.
  }
  return rv;
}

// copies the given floats into the wasm heap as an SkRect. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyRectToWasm(fourFloats, ptr) {
  return copy1dArray(fourFloats, 'HEAPF32', ptr || _scratchFourFloatsAPtr);
}

// copies the given ints into the wasm heap as an SkIRect. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyIRectToWasm(fourInts, ptr) {
  return copy1dArray(fourInts, 'HEAP32', ptr || _scratchIRectPtr);
}

// copies the given floats into the wasm heap as an SkRRect. Unless a non-scratch pointer is
// passed into ptr, callers do NOT need to free the returned pointer.
function copyRRectToWasm(twelveFloats, ptr) {
  return copy1dArray(twelveFloats, 'HEAPF32', ptr || _scratchRRectPtr);
}
// TODO(kjlubick)
// The remaining functions here are deprecated and should be removed eventually.

// Caching the Float32Arrays can save having to reallocate them
// over and over again.
var Float32ArrayCache = {};

// Takes a 2D array of commands and puts them into the WASM heap
// as a 1D array. This allows them to referenced from the C++ code.
// Returns a 2 element array, with the first item being essentially a
// pointer to the array and the second item being the length of
// the new 1D array.
//
// Example usage:
// let cmds = [
//   [CanvasKit.MOVE_VERB, 0, 10],
//   [CanvasKit.LINE_VERB, 30, 40],
//   [CanvasKit.QUAD_VERB, 20, 50, 45, 60],
// ];
// TODO(kjlubick) remove this and Float32ArrayCache (superceded by Malloc).
function loadCmdsTypedArray(arr) {
  var len = 0;
  for (var r = 0; r < arr.length; r++) {
    len += arr[r].length;
  }

  var ta;
  if (Float32ArrayCache[len]) {
    ta = Float32ArrayCache[len];
  } else {
    ta = new Float32Array(len);
    Float32ArrayCache[len] = ta;
  }
  // Flatten into a 1d array
  var i = 0;
  for (var r = 0; r < arr.length; r++) {
    for (var c = 0; c < arr[r].length; c++) {
      var item = arr[r][c];
      ta[i] = item;
      i++;
    }
  }

  var ptr = copy1dArray(ta, 'HEAPF32');
  return [ptr, len];
}

// TODO(kjlubick) remove Builders - no longer needed now that Malloc is a thing.
/**
 * Generic helper for dealing with an array of four floats.
 */
CanvasKit.FourFloatArrayHelper = function() {
  this._floats = [];
  this._ptr = null;

  Object.defineProperty(this, 'length', {
    enumerable: true,
    get: function() {
      return this._floats.length / 4;
    },
  });
};

/**
 * push the four floats onto the end of the array - if build() has already
 * been called, the call will return without modifying anything.
 */
CanvasKit.FourFloatArrayHelper.prototype.push = function(f1, f2, f3, f4) {
  if (this._ptr) {
    Debug('Cannot push more points - already built');
    return;
  }
  this._floats.push(f1, f2, f3, f4);
};

/**
 * Set the four floats at a given index - if build() has already
 * been called, the WASM memory will be written to directly.
 */
CanvasKit.FourFloatArrayHelper.prototype.set = function(idx, f1, f2, f3, f4) {
  if (idx < 0 || idx >= this._floats.length/4) {
    Debug('Cannot set index ' + idx + ', it is out of range', this._floats.length/4);
    return;
  }
  idx *= 4;
  var BYTES_PER_ELEMENT = 4;
  if (this._ptr) {
    // convert this._ptr from uint8_t* to SkScalar* by dividing by 4
    var floatPtr = (this._ptr / BYTES_PER_ELEMENT) + idx;
    CanvasKit.HEAPF32[floatPtr]     = f1;
    CanvasKit.HEAPF32[floatPtr + 1] = f2;
    CanvasKit.HEAPF32[floatPtr + 2] = f3;
    CanvasKit.HEAPF32[floatPtr + 3] = f4;
    return;
  }
  this._floats[idx]     = f1;
  this._floats[idx + 1] = f2;
  this._floats[idx + 2] = f3;
  this._floats[idx + 3] = f4;
};

/**
 * Copies the float data to the WASM memory and returns a pointer
 * to that allocated memory. Once build has been called, this
 * float array cannot be made bigger.
 */
CanvasKit.FourFloatArrayHelper.prototype.build = function() {
  if (this._ptr) {
    return this._ptr;
  }
  this._ptr = copy1dArray(this._floats, 'HEAPF32');
  return this._ptr;
};

/**
 * Frees the wasm memory associated with this array. Of note,
 * the points are not removed, so push/set/build can all
 * be called to make a newly allocated (possibly bigger)
 * float array.
 */
CanvasKit.FourFloatArrayHelper.prototype.delete = function() {
  if (this._ptr) {
    CanvasKit._free(this._ptr);
    this._ptr = null;
  }
};

/**
 * Generic helper for dealing with an array of unsigned ints.
 */
CanvasKit.OneUIntArrayHelper = function() {
  this._uints = [];
  this._ptr = null;

  Object.defineProperty(this, 'length', {
    enumerable: true,
    get: function() {
      return this._uints.length;
    },
  });
};

/**
 * push the unsigned int onto the end of the array - if build() has already
 * been called, the call will return without modifying anything.
 */
CanvasKit.OneUIntArrayHelper.prototype.push = function(u) {
  if (this._ptr) {
    Debug('Cannot push more points - already built');
    return;
  }
  this._uints.push(u);
};

/**
 * Set the uint at a given index - if build() has already
 * been called, the WASM memory will be written to directly.
 */
CanvasKit.OneUIntArrayHelper.prototype.set = function(idx, u) {
  if (idx < 0 || idx >= this._uints.length) {
    Debug('Cannot set index ' + idx + ', it is out of range', this._uints.length);
    return;
  }
  idx *= 4;
  var BYTES_PER_ELEMENT = 4;
  if (this._ptr) {
    // convert this._ptr from uint8_t* to SkScalar* by dividing by 4
    var uintPtr = (this._ptr / BYTES_PER_ELEMENT) + idx;
    CanvasKit.HEAPU32[uintPtr] = u;
    return;
  }
  this._uints[idx] = u;
};

/**
 * Copies the uint data to the WASM memory and returns a pointer
 * to that allocated memory. Once build has been called, this
 * unit array cannot be made bigger.
 */
CanvasKit.OneUIntArrayHelper.prototype.build = function() {
  if (this._ptr) {
    return this._ptr;
  }
  this._ptr = copy1dArray(this._uints, 'HEAPU32');
  return this._ptr;
};

/**
 * Frees the wasm memory associated with this array. Of note,
 * the points are not removed, so push/set/build can all
 * be called to make a newly allocated (possibly bigger)
 * uint array.
 */
CanvasKit.OneUIntArrayHelper.prototype.delete = function() {
  if (this._ptr) {
    CanvasKit._free(this._ptr);
    this._ptr = null;
  }
};

/**
 * Helper for building an array of Rects (which are just structs
 * of 4 floats).
 *
 * It can be more performant to use this helper, as
 * the C++-side array is only allocated once (on the first call)
 * to build. Subsequent set() operations operate directly on
 * the C++-side array, avoiding having to re-allocate (and free)
 * the array every time.
 *
 * Input points are taken as left, top, right, bottom
 */
CanvasKit.RectBuilder = CanvasKit.FourFloatArrayHelper;
/**
 * Helper for building an array of RSXForms (which are just structs
 * of 4 floats).
 *
 * It can be more performant to use this helper, as
 * the C++-side array is only allocated once (on the first call)
 * to build. Subsequent set() operations operate directly on
 * the C++-side array, avoiding having to re-allocate (and free)
 * the array every time.
 *
 *  An RSXForm is a compressed form of a rotation+scale matrix.
 *
 *  [ scos    -ssin    tx ]
 *  [ ssin     scos    ty ]
 *  [    0        0     1 ]
 *
 * Input points are taken as scos, ssin, tx, ty
 */
CanvasKit.RSXFormBuilder = CanvasKit.FourFloatArrayHelper;

/**
 * Helper for building an array of Color
 *
 * It can be more performant to use this helper, as
 * the C++-side array is only allocated once (on the first call)
 * to build. Subsequent set() operations operate directly on
 * the C++-side array, avoiding having to re-allocate (and free)
 * the array every time.
 */
CanvasKit.ColorBuilder = CanvasKit.OneUIntArrayHelper;

/**
 * This file houses miscellaneous helper functions and constants.
 */

var nullptr = 0; // emscripten doesn't like to take null as uintptr_t


function radiansToDegrees(rad) {
  return (rad / Math.PI) * 180;
}

function degreesToRadians(deg) {
  return (deg / 180) * Math.PI;
}

// See https://stackoverflow.com/a/31090240
// This contraption keeps closure from minifying away the check
// if btoa is defined *and* prevents runtime 'btoa' or 'window' is not defined.
// Defined outside any scopes to make it available in all files.
var isNode = !(new Function('try {return this===window;}catch(e){ return false;}')());

function almostEqual(floata, floatb) {
  return Math.abs(floata - floatb) < 0.00001;
}

function saveBytesToFile(bytes, fileName) {
  if (!isNode) {
    // https://stackoverflow.com/a/32094834
    var blob = new Blob([bytes], {type: 'application/octet-stream'});
    url = window.URL.createObjectURL(blob);
    var a = document.createElement('a');
    document.body.appendChild(a);
    a.href = url;
    a.download = fileName;
    a.click();
    // clean up after because FF might not download it synchronously
    setTimeout(function() {
      URL.revokeObjectURL(url);
      a.remove();
    }, 50);
  } else {
    var fs = require('fs');
    // https://stackoverflow.com/a/42006750
    // https://stackoverflow.com/a/47018122
    fs.writeFile(fileName, new Buffer(bytes), function(err) {
      if (err) throw err;
    });
  }
}
// Adds JS functions to augment the CanvasKit interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.

// CanvasKit.onRuntimeInitialized is called after the WASM library has loaded.
// Anything that modifies an exposed class (e.g. Path) should be set
// after onRuntimeInitialized, otherwise, it can happen outside of that scope.
CanvasKit.onRuntimeInitialized = function() {
  // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

  _scratchColor = CanvasKit.Malloc(Float32Array, 4); // 4 color scalars.
  _scratchColorPtr = _scratchColor['byteOffset'];

  _scratch4x4Matrix = CanvasKit.Malloc(Float32Array, 16); // 16 matrix scalars.
  _scratch4x4MatrixPtr = _scratch4x4Matrix['byteOffset'];

  _scratch3x3Matrix = CanvasKit.Malloc(Float32Array, 9); // 9 matrix scalars.
  _scratch3x3MatrixPtr = _scratch3x3Matrix['byteOffset'];

  _scratchRRect = CanvasKit.Malloc(Float32Array, 12); // 4 scalars for rrect, 8 for radii.
  _scratchRRectPtr = _scratchRRect['byteOffset'];

  _scratchRRect2 = CanvasKit.Malloc(Float32Array, 12); // 4 scalars for rrect, 8 for radii.
  _scratchRRect2Ptr = _scratchRRect2['byteOffset'];

  _scratchFourFloatsA = CanvasKit.Malloc(Float32Array, 4);
  _scratchFourFloatsAPtr = _scratchFourFloatsA['byteOffset'];

  _scratchFourFloatsB = CanvasKit.Malloc(Float32Array, 4);
  _scratchFourFloatsBPtr = _scratchFourFloatsB['byteOffset'];

  _scratchThreeFloatsA = CanvasKit.Malloc(Float32Array, 3); // 3 floats to represent SkVector3
  _scratchThreeFloatsAPtr = _scratchThreeFloatsA['byteOffset'];

  _scratchThreeFloatsB = CanvasKit.Malloc(Float32Array, 3); // 3 floats to represent SkVector3
  _scratchThreeFloatsBPtr = _scratchThreeFloatsB['byteOffset'];

  _scratchIRect = CanvasKit.Malloc(Int32Array, 4);
  _scratchIRectPtr = _scratchIRect['byteOffset'];

  // Create single copies of all three supported color spaces
  // These are sk_sp<ColorSpace>
  CanvasKit.ColorSpace.SRGB = CanvasKit.ColorSpace._MakeSRGB();
  CanvasKit.ColorSpace.DISPLAY_P3 = CanvasKit.ColorSpace._MakeDisplayP3();
  CanvasKit.ColorSpace.ADOBE_RGB = CanvasKit.ColorSpace._MakeAdobeRGB();

  CanvasKit.Path.MakeFromCmds = function(cmds) {
    var ptrLen = loadCmdsTypedArray(cmds);
    var path = CanvasKit.Path._MakeFromCmds(ptrLen[0], ptrLen[1]);
    CanvasKit._free(ptrLen[0]);
    return path;
  };

  // The weights array is optional (only used for conics).
  CanvasKit.Path.MakeFromVerbsPointsWeights = function(verbs, pts, weights) {
    var verbsPtr = copy1dArray(verbs, 'HEAPU8');
    var pointsPtr = copy1dArray(pts, 'HEAPF32');
    var weightsPtr = copy1dArray(weights, 'HEAPF32');
    var numWeights = (weights && weights.length) || 0;
    var path = CanvasKit.Path._MakeFromVerbsPointsWeights(
        verbsPtr, verbs.length, pointsPtr, pts.length, weightsPtr, numWeights);
    freeArraysThatAreNotMallocedByUsers(verbsPtr, verbs);
    freeArraysThatAreNotMallocedByUsers(pointsPtr, pts);
    freeArraysThatAreNotMallocedByUsers(weightsPtr, weights);
    return path;
  };

  CanvasKit.Path.prototype.addArc = function(oval, startAngle, sweepAngle) {
    // see arc() for the HTMLCanvas version
    // note input angles are degrees.
    var oPtr = copyRectToWasm(oval);
    this._addArc(oPtr, startAngle, sweepAngle);
    return this;
  };

  CanvasKit.Path.prototype.addOval = function(oval, isCCW, startIndex) {
    if (startIndex === undefined) {
      startIndex = 1;
    }
    var oPtr = copyRectToWasm(oval);
    this._addOval(oPtr, !!isCCW, startIndex);
    return this;
  };

  // TODO(kjlubick) clean up this API - split it apart if necessary
  CanvasKit.Path.prototype.addPath = function() {
    // Takes 1, 2, 7, or 10 required args, where the first arg is always the path.
    // The last arg is optional and chooses between add or extend mode.
    // The options for the remaining args are:
    //   - an array of 6 or 9 parameters (perspective is optional)
    //   - the 9 parameters of a full matrix or
    //     the 6 non-perspective params of a matrix.
    var args = Array.prototype.slice.call(arguments);
    var path = args[0];
    var extend = false;
    if (typeof args[args.length-1] === 'boolean') {
      extend = args.pop();
    }
    if (args.length === 1) {
      // Add path, unchanged.  Use identity matrix
      this._addPath(path, 1, 0, 0,
                          0, 1, 0,
                          0, 0, 1,
                          extend);
    } else if (args.length === 2) {
      // User provided the 9 params of a full matrix as an array.
      var a = args[1];
      this._addPath(path, a[0],      a[1],      a[2],
                          a[3],      a[4],      a[5],
                          a[6] || 0, a[7] || 0, a[8] || 1,
                          extend);
    } else if (args.length === 7 || args.length === 10) {
      // User provided the 9 params of a (full) matrix directly.
      // (or just the 6 non perspective ones)
      // These are in the same order as what Skia expects.
      var a = args;
      this._addPath(path, a[1],      a[2],      a[3],
                          a[4],      a[5],      a[6],
                          a[7] || 0, a[8] || 0, a[9] || 1,
                          extend);
    } else {
      Debug('addPath expected to take 1, 2, 7, or 10 required args. Got ' + args.length);
      return null;
    }
    return this;
  };

  // points is a 1d array of length 2n representing n points where the even indices
  // will be treated as x coordinates and the odd indices will be treated as y coordinates.
  // Like other APIs, this accepts a malloced type array or malloc obj.
  CanvasKit.Path.prototype.addPoly = function(points, close) {
    var ptr = copy1dArray(points, 'HEAPF32');
    this._addPoly(ptr, points.length / 2, close);
    freeArraysThatAreNotMallocedByUsers(ptr, points);
    return this;
  };

  CanvasKit.Path.prototype.addRect = function(rect, isCCW) {
    var rPtr = copyRectToWasm(rect);
    this._addRect(rPtr, !!isCCW);
    return this;
  };

  CanvasKit.Path.prototype.addRRect = function(rrect, isCCW) {
    var rPtr = copyRRectToWasm(rrect);
    this._addRRect(rPtr, !!isCCW);
    return this;
  };

  // The weights array is optional (only used for conics).
  CanvasKit.Path.prototype.addVerbsPointsWeights = function(verbs, points, weights) {
    var verbsPtr = copy1dArray(verbs, 'HEAPU8');
    var pointsPtr = copy1dArray(points, 'HEAPF32');
    var weightsPtr = copy1dArray(weights, 'HEAPF32');
    var numWeights = (weights && weights.length) || 0;
    this._addVerbsPointsWeights(verbsPtr, verbs.length, pointsPtr, points.length,
                                weightsPtr, numWeights);
    freeArraysThatAreNotMallocedByUsers(verbsPtr, verbs);
    freeArraysThatAreNotMallocedByUsers(pointsPtr, points);
    freeArraysThatAreNotMallocedByUsers(weightsPtr, weights);
  };

  CanvasKit.Path.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    // emulates the HTMLCanvas behavior.  See addArc() for the Path version.
    // Note input angles are radians.
    var bounds = CanvasKit.LTRBRect(x-radius, y-radius, x+radius, y+radius);
    var sweep = radiansToDegrees(endAngle - startAngle) - (360 * !!ccw);
    var temp = new CanvasKit.Path();
    temp.addArc(bounds, radiansToDegrees(startAngle), sweep);
    this.addPath(temp, true);
    temp.delete();
    return this;
  };

  // Appends arc to Path. Arc added is part of ellipse
  // bounded by oval, from startAngle through sweepAngle. Both startAngle and
  // sweepAngle are measured in degrees, where zero degrees is aligned with the
  // positive x-axis, and positive sweeps extends arc clockwise.
  CanvasKit.Path.prototype.arcToOval = function(oval, startAngle, sweepAngle, forceMoveTo) {
    var oPtr = copyRectToWasm(oval);
    this._arcToOval(oPtr, startAngle, sweepAngle, forceMoveTo);
    return this;
  };

  // Appends arc to Path. Arc is implemented by one or more conics weighted to
  // describe part of oval with radii (rx, ry) rotated by xAxisRotate degrees. Arc
  // curves from last point to (x, y), choosing one of four possible routes:
  // clockwise or counterclockwise, and smaller or larger.

  // Arc sweep is always less than 360 degrees. arcTo() appends line to (x, y) if
  // either radii are zero, or if last point equals (x, y). arcTo() scales radii
  // (rx, ry) to fit last point and (x, y) if both are greater than zero but
  // too small.

  // arcToRotated() appends up to four conic curves.
  // arcToRotated() implements the functionality of SVG arc, although SVG sweep-flag value
  // is opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise,
  // while kCW_Direction cast to int is zero.
  CanvasKit.Path.prototype.arcToRotated = function(rx, ry, xAxisRotate, useSmallArc, isCCW, x, y) {
    this._arcToRotated(rx, ry, xAxisRotate, !!useSmallArc, !!isCCW, x, y);
    return this;
  };

  // Appends arc to Path, after appending line if needed. Arc is implemented by conic
  // weighted to describe part of circle. Arc is contained by tangent from
  // last Path point to (x1, y1), and tangent from (x1, y1) to (x2, y2). Arc
  // is part of circle sized to radius, positioned so it touches both tangent lines.

  // If last Path Point does not start Arc, arcTo appends connecting Line to Path.
  // The length of Vector from (x1, y1) to (x2, y2) does not affect Arc.

  // Arc sweep is always less than 180 degrees. If radius is zero, or if
  // tangents are nearly parallel, arcTo appends Line from last Path Point to (x1, y1).

  // arcToTangent appends at most one Line and one conic.
  // arcToTangent implements the functionality of PostScript arct and HTML Canvas arcTo.
  CanvasKit.Path.prototype.arcToTangent = function(x1, y1, x2, y2, radius) {
    this._arcToTangent(x1, y1, x2, y2, radius);
    return this;
  };

  CanvasKit.Path.prototype.close = function() {
    this._close();
    return this;
  };

  CanvasKit.Path.prototype.conicTo = function(x1, y1, x2, y2, w) {
    this._conicTo(x1, y1, x2, y2, w);
    return this;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Path.prototype.computeTightBounds = function(optionalOutputArray) {
    this._computeTightBounds(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  CanvasKit.Path.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.Path.prototype.dash = function(on, off, phase) {
    if (this._dash(on, off, phase)) {
      return this;
    }
    return null;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Path.prototype.getBounds = function(optionalOutputArray) {
    this._getBounds(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  CanvasKit.Path.prototype.lineTo = function(x, y) {
    this._lineTo(x, y);
    return this;
  };

  CanvasKit.Path.prototype.moveTo = function(x, y) {
    this._moveTo(x, y);
    return this;
  };

  CanvasKit.Path.prototype.offset = function(dx, dy) {
    this._transform(1, 0, dx,
                    0, 1, dy,
                    0, 0, 1);
    return this;
  };

  CanvasKit.Path.prototype.quadTo = function(cpx, cpy, x, y) {
    this._quadTo(cpx, cpy, x, y);
    return this;
  };

 CanvasKit.Path.prototype.rArcTo = function(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy) {
    this._rArcTo(rx, ry, xAxisRotate, useSmallArc, isCCW, dx, dy);
    return this;
  };

  CanvasKit.Path.prototype.rConicTo = function(dx1, dy1, dx2, dy2, w) {
    this._rConicTo(dx1, dy1, dx2, dy2, w);
    return this;
  };

  // These params are all relative
  CanvasKit.Path.prototype.rCubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    this._rCubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    return this;
  };

  CanvasKit.Path.prototype.rLineTo = function(dx, dy) {
    this._rLineTo(dx, dy);
    return this;
  };

  CanvasKit.Path.prototype.rMoveTo = function(dx, dy) {
    this._rMoveTo(dx, dy);
    return this;
  };

  // These params are all relative
  CanvasKit.Path.prototype.rQuadTo = function(cpx, cpy, x, y) {
    this._rQuadTo(cpx, cpy, x, y);
    return this;
  };

  CanvasKit.Path.prototype.stroke = function(opts) {
    // Fill out any missing values with the default values.
    opts = opts || {};
    opts['width'] = opts['width'] || 1;
    opts['miter_limit'] = opts['miter_limit'] || 4;
    opts['cap'] = opts['cap'] || CanvasKit.StrokeCap.Butt;
    opts['join'] = opts['join'] || CanvasKit.StrokeJoin.Miter;
    opts['precision'] = opts['precision'] || 1;
    if (this._stroke(opts)) {
      return this;
    }
    return null;
  };

  // TODO(kjlubick) Change this to take a 3x3 or 4x4 matrix (optionally malloc'd)
  CanvasKit.Path.prototype.transform = function() {
    // Takes 1 or 9 args
    if (arguments.length === 1) {
      // argument 1 should be a 6 or 9 element array.
      var a = arguments[0];
      this._transform(a[0], a[1], a[2],
                      a[3], a[4], a[5],
                      a[6] || 0, a[7] || 0, a[8] || 1);
    } else if (arguments.length === 6 || arguments.length === 9) {
      // these arguments are the 6 or 9 members of the matrix
      var a = arguments;
      this._transform(a[0], a[1], a[2],
                      a[3], a[4], a[5],
                      a[6] || 0, a[7] || 0, a[8] || 1);
    } else {
      throw 'transform expected to take 1 or 9 arguments. Got ' + arguments.length;
    }
    return this;
  };
  // isComplement is optional, defaults to false
  CanvasKit.Path.prototype.trim = function(startT, stopT, isComplement) {
    if (this._trim(startT, stopT, !!isComplement)) {
      return this;
    }
    return null;
  };

  CanvasKit.Image.prototype.encodeToData = function() {
    if (!arguments.length) {
      return this._encodeToData();
    }

    if (arguments.length === 2) {
      var a = arguments;
      return this._encodeToDataWithFormat(a[0], a[1]);
    }

    throw 'encodeToData expected to take 0 or 2 arguments. Got ' + arguments.length;
  };

  // makeShaderCubic returns a shader for a given image, allowing it to be used on
  // a paint as well as other purposes. This shader will be higher quality than
  // other shader functions. See CubicResampler in SkSamplingOptions.h for more information
  // on the cubicResampler params.
  CanvasKit.Image.prototype.makeShaderCubic = function(xTileMode, yTileMode,
                                                       cubicResamplerB, cubicResamplerC,
                                                       localMatrix) {
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    return this._makeShaderCubic(xTileMode, yTileMode, cubicResamplerB,
                                 cubicResamplerC, localMatrixPtr);
  };

  // makeShaderCubic returns a shader for a given image, allowing it to be used on
  // a paint as well as other purposes. This shader will draw more quickly than
  // other shader functions, but at a lower quality.
  CanvasKit.Image.prototype.makeShaderOptions = function(xTileMode, yTileMode,
                                                         filterMode, mipmapMode,
                                                         localMatrix) {
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    return this._makeShaderOptions(xTileMode, yTileMode, filterMode, mipmapMode, localMatrixPtr);
  };

  function readPixels(source, srcX, srcY, imageInfo, destMallocObj, bytesPerRow) {
    if (!bytesPerRow) {
      bytesPerRow = 4 * imageInfo['width'];
      if (imageInfo['colorType'] === CanvasKit.ColorType.RGBA_F16) {
        bytesPerRow *= 2;
      }
      else if (imageInfo['colorType'] === CanvasKit.ColorType.RGBA_F32) {
        bytesPerRow *= 4;
      }
    }
    var pBytes = bytesPerRow * imageInfo.height;
    var pPtr;
    if (destMallocObj) {
      pPtr = destMallocObj['byteOffset'];
    } else {
      pPtr = CanvasKit._malloc(pBytes);
    }

    if (!source._readPixels(imageInfo, pPtr, bytesPerRow, srcX, srcY)) {
      Debug('Could not read pixels with the given inputs');
      if (!destMallocObj) {
        CanvasKit._free(pPtr);
      }
      return null;
    }

    // If the user provided us a buffer to copy into, we don't need to allocate a new TypedArray.
    if (destMallocObj) {
      return destMallocObj['toTypedArray'](); // Return the typed array wrapper w/o allocating.
    }

    // Put those pixels into a typed array of the right format and then
    // make a copy with slice() that we can return.
    var retVal = null;
    switch (imageInfo['colorType']) {
      case CanvasKit.ColorType.RGBA_8888:
      case CanvasKit.ColorType.RGBA_F16: // there is no half-float JS type, so we return raw bytes.
        retVal = new Uint8Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
      case CanvasKit.ColorType.RGBA_F32:
        retVal = new Float32Array(CanvasKit.HEAPU8.buffer, pPtr, pBytes).slice();
        break;
      default:
        Debug('ColorType not yet supported');
        return null;
    }

    // Free the allocated pixels in the WASM memory
    CanvasKit._free(pPtr);
    return retVal;
  }

  CanvasKit.Image.prototype.readPixels = function(srcX, srcY, imageInfo, destMallocObj,
                                                  bytesPerRow) {
    return readPixels(this, srcX, srcY, imageInfo, destMallocObj, bytesPerRow);
  };

  // Accepts an array of four numbers in the range of 0-1 representing a 4f color
  CanvasKit.Canvas.prototype.clear = function(color4f) {
    var cPtr = copyColorToWasm(color4f);
    this._clear(cPtr);
  };

  CanvasKit.Canvas.prototype.clipRRect = function(rrect, op, antialias) {
    var rPtr = copyRRectToWasm(rrect);
    this._clipRRect(rPtr, op, antialias);
  };

  CanvasKit.Canvas.prototype.clipRect = function(rect, op, antialias) {
    var rPtr = copyRectToWasm(rect);
    this._clipRect(rPtr, op, antialias);
  };

  // concat takes a 3x2, a 3x3, or a 4x4 matrix and upscales it (if needed) to 4x4. This is because
  // under the hood, SkCanvas uses a 4x4 matrix.
  CanvasKit.Canvas.prototype.concat = function(matr) {
    var matrPtr = copy4x4MatrixToWasm(matr);
    this._concat(matrPtr);
  };

  CanvasKit.Canvas.prototype.drawArc = function(oval, startAngle, sweepAngle, useCenter, paint) {
    var oPtr = copyRectToWasm(oval);
    this._drawArc(oPtr, startAngle, sweepAngle, useCenter, paint);
  };

  // atlas is an Image, e.g. from CanvasKit.MakeImageFromEncoded
  // srcRects, dstXforms, and colors should be CanvasKit.RectBuilder, CanvasKit.RSXFormBuilder,
  // and CanvasKit.ColorBuilder (fastest)
  // Or they can be an array of floats of length 4*number of destinations.
  // colors are optional and used to tint the drawn images using the optional blend mode
  // Colors may be an ColorBuilder, a Uint32Array of int colors,
  // a Flat Float32Array of float colors or a 2d Array of Float32Array(4) (deprecated)
  // TODO(kjlubick) remove Builders - no longer needed now that Malloc is a thing.
  CanvasKit.Canvas.prototype.drawAtlas = function(atlas, srcRects, dstXforms, paint,
                                       /*optional*/ blendMode, colors) {
    if (!atlas || !paint || !srcRects || !dstXforms) {
      Debug('Doing nothing since missing a required input');
      return;
    }

    // builder arguments report the length as the number of rects, but when passed as arrays
    // their.length attribute is 4x higher because it's the number of total components of all rects.
    // colors is always going to report the same length, at least until floats colors are supported
    // by this function.
    if (srcRects.length !== dstXforms.length) {
      Debug('Doing nothing since input arrays length mismatches');
      return;
    }
    if (!blendMode) {
      blendMode = CanvasKit.BlendMode.SrcOver;
    }

    var srcRectPtr;
    if (srcRects.build) {
      srcRectPtr = srcRects.build();
    } else {
      srcRectPtr = copy1dArray(srcRects, 'HEAPF32');
    }

    var count = 1;
    var dstXformPtr;
    if (dstXforms.build) {
      dstXformPtr = dstXforms.build();
      count = dstXforms.length;
    } else {
      dstXformPtr = copy1dArray(dstXforms, 'HEAPF32');
      count = dstXforms.length / 4;
    }

    var colorPtr = nullptr;
    if (colors) {
      if (colors.build) {
        colorPtr = colors.build();
      } else {
        colorPtr = copy1dArray(assureIntColors(colors), 'HEAPU32');
      }
    }

    this._drawAtlas(atlas, dstXformPtr, srcRectPtr, colorPtr, count, blendMode, paint);

    if (srcRectPtr && !srcRects.build) {
      freeArraysThatAreNotMallocedByUsers(srcRectPtr, srcRects);
    }
    if (dstXformPtr && !dstXforms.build) {
      freeArraysThatAreNotMallocedByUsers(dstXformPtr, dstXforms);
    }
    if (colorPtr && !colors.build) {
      freeArraysThatAreNotMallocedByUsers(colorPtr, colors);
    }
  };

  CanvasKit.Canvas.prototype.drawColor = function (color4f, mode) {
    var cPtr = copyColorToWasm(color4f);
    if (mode !== undefined) {
      this._drawColor(cPtr, mode);
    } else {
      this._drawColor(cPtr);
    }
  };

  CanvasKit.Canvas.prototype.drawColorComponents = function (r, g, b, a, mode) {
    var cPtr = copyColorComponentsToWasm(r, g, b, a);
    if (mode !== undefined) {
      this._drawColor(cPtr, mode);
    } else {
      this._drawColor(cPtr);
    }
  };

  CanvasKit.Canvas.prototype.drawDRRect = function(outer, inner, paint) {
    var oPtr = copyRRectToWasm(outer, _scratchRRectPtr);
    var iPtr = copyRRectToWasm(inner, _scratchRRect2Ptr);
    this._drawDRRect(oPtr, iPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawImageNine = function(img, center, dest, filter, paint) {
    var cPtr = copyIRectToWasm(center);
    var dPtr = copyRectToWasm(dest);
    this._drawImageNine(img, cPtr, dPtr, filter, paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageRect = function(img, src, dest, paint, fastSample) {
    copyRectToWasm(src,  _scratchFourFloatsAPtr);
    copyRectToWasm(dest, _scratchFourFloatsBPtr);
    this._drawImageRect(img, _scratchFourFloatsAPtr, _scratchFourFloatsBPtr, paint, !!fastSample);
  };

  CanvasKit.Canvas.prototype.drawImageRectCubic = function(img, src, dest, B, C, paint) {
    copyRectToWasm(src,  _scratchFourFloatsAPtr);
    copyRectToWasm(dest, _scratchFourFloatsBPtr);
    this._drawImageRectCubic(img, _scratchFourFloatsAPtr, _scratchFourFloatsBPtr, B, C,
      paint || null);
  };

  CanvasKit.Canvas.prototype.drawImageRectOptions = function(img, src, dest, filter, mipmap, paint) {
    copyRectToWasm(src,  _scratchFourFloatsAPtr);
    copyRectToWasm(dest, _scratchFourFloatsBPtr);
    this._drawImageRectOptions(img, _scratchFourFloatsAPtr, _scratchFourFloatsBPtr, filter, mipmap,
      paint || null);
  };

  CanvasKit.Canvas.prototype.drawOval = function(oval, paint) {
    var oPtr = copyRectToWasm(oval);
    this._drawOval(oPtr, paint);
  };

  // points is a 1d array of length 2n representing n points where the even indices
  // will be treated as x coordinates and the odd indices will be treated as y coordinates.
  // Like other APIs, this accepts a malloced type array or malloc obj.
  CanvasKit.Canvas.prototype.drawPoints = function(mode, points, paint) {
    var ptr = copy1dArray(points, 'HEAPF32');
    this._drawPoints(mode, ptr, points.length / 2, paint);
    freeArraysThatAreNotMallocedByUsers(ptr, points);
  };

  CanvasKit.Canvas.prototype.drawRRect = function(rrect, paint) {
    var rPtr = copyRRectToWasm(rrect);
    this._drawRRect(rPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawRect = function(rect, paint) {
    var rPtr = copyRectToWasm(rect);
    this._drawRect(rPtr, paint);
  };

  CanvasKit.Canvas.prototype.drawShadow = function(path, zPlaneParams, lightPos, lightRadius,
                                                   ambientColor, spotColor, flags) {
    var ambiPtr = copyColorToWasmNoScratch(ambientColor);
    var spotPtr = copyColorToWasmNoScratch(spotColor);
    // We use the return value from copy1dArray in case the passed in arrays are malloc'd.
    var zPlanePtr = copy1dArray(zPlaneParams, 'HEAPF32', _scratchThreeFloatsAPtr);
    var lightPosPtr = copy1dArray(lightPos, 'HEAPF32', _scratchThreeFloatsBPtr);
    this._drawShadow(path, zPlanePtr, lightPosPtr, lightRadius, ambiPtr, spotPtr, flags);
    freeArraysThatAreNotMallocedByUsers(ambiPtr, ambientColor);
    freeArraysThatAreNotMallocedByUsers(spotPtr, spotColor);
  };

  CanvasKit.getShadowLocalBounds = function(ctm, path, zPlaneParams, lightPos, lightRadius,
                                            flags, optOutputRect) {
    var ctmPtr = copy3x3MatrixToWasm(ctm);
    // We use the return value from copy1dArray in case the passed in arrays are malloc'd.
    var zPlanePtr = copy1dArray(zPlaneParams, 'HEAPF32', _scratchThreeFloatsAPtr);
    var lightPosPtr = copy1dArray(lightPos, 'HEAPF32', _scratchThreeFloatsBPtr);
    var ok = this._getShadowLocalBounds(ctmPtr, path, zPlanePtr, lightPosPtr, lightRadius,
                                        flags, _scratchFourFloatsAPtr);
    if (!ok) {
      return null;
    }
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optOutputRect) {
      optOutputRect.set(ta);
      return optOutputRect;
    }
    return ta.slice();
  };

  // getLocalToDevice returns a 4x4 matrix.
  CanvasKit.Canvas.prototype.getLocalToDevice = function() {
    // _getLocalToDevice will copy the values into the pointer.
    this._getLocalToDevice(_scratch4x4MatrixPtr);
    return copy4x4MatrixFromWasm(_scratch4x4MatrixPtr);
  };

  // findMarkedCTM returns a 4x4 matrix, or null if a matrix was not found at
  // the provided marker.
  CanvasKit.Canvas.prototype.findMarkedCTM = function(marker) {
    // _getLocalToDevice will copy the values into the pointer.
    var found = this._findMarkedCTM(marker, _scratch4x4MatrixPtr);
    if (!found) {
      return null;
    }
    return copy4x4MatrixFromWasm(_scratch4x4MatrixPtr);
  };

  // getTotalMatrix returns the current matrix as a 3x3 matrix.
  CanvasKit.Canvas.prototype.getTotalMatrix = function() {
    // _getTotalMatrix will copy the values into the pointer.
    this._getTotalMatrix(_scratch3x3MatrixPtr);
    // read them out into an array. TODO(kjlubick): If we change Matrix to be
    // typedArrays, then we should return a typed array here too.
    var rv = new Array(9);
    for (var i = 0; i < 9; i++) {
      rv[i] = CanvasKit.HEAPF32[_scratch3x3MatrixPtr/4 + i]; // divide by 4 to "cast" to float.
    }
    return rv;
  };

  CanvasKit.Canvas.prototype.readPixels = function(srcX, srcY, imageInfo, destMallocObj,
                                                   bytesPerRow) {
    return readPixels(this, srcX, srcY, imageInfo, destMallocObj, bytesPerRow);
  };

  CanvasKit.Canvas.prototype.saveLayer = function(paint, boundsRect, backdrop, flags) {
    // bPtr will be 0 (nullptr) if boundsRect is undefined/null.
    var bPtr = copyRectToWasm(boundsRect);
    // These or clauses help emscripten, which does not deal with undefined well.
    return this._saveLayer(paint || null, bPtr, backdrop || null, flags || 0);
  };

  // pixels should be a Uint8Array or a plain JS array.
  CanvasKit.Canvas.prototype.writePixels = function(pixels, srcWidth, srcHeight,
                                                      destX, destY, alphaType, colorType, colorSpace) {
    if (pixels.byteLength % (srcWidth * srcHeight)) {
      throw 'pixels length must be a multiple of the srcWidth * srcHeight';
    }
    var bytesPerPixel = pixels.byteLength / (srcWidth * srcHeight);
    // supply defaults (which are compatible with HTMLCanvas's putImageData)
    alphaType = alphaType || CanvasKit.AlphaType.Unpremul;
    colorType = colorType || CanvasKit.ColorType.RGBA_8888;
    colorSpace = colorSpace || CanvasKit.ColorSpace.SRGB;
    var srcRowBytes = bytesPerPixel * srcWidth;

    var pptr = copy1dArray(pixels, 'HEAPU8');
    var ok = this._writePixels({
      'width': srcWidth,
      'height': srcHeight,
      'colorType': colorType,
      'alphaType': alphaType,
      'colorSpace': colorSpace,
    }, pptr, srcRowBytes, destX, destY);

    freeArraysThatAreNotMallocedByUsers(pptr, pixels);
    return ok;
  };

  CanvasKit.ColorFilter.MakeBlend = function(color4f, mode) {
    var cPtr = copyColorToWasm(color4f);
    return CanvasKit.ColorFilter._MakeBlend(cPtr, mode);
  };

  // colorMatrix is an ColorMatrix (e.g. Float32Array of length 20)
  CanvasKit.ColorFilter.MakeMatrix = function(colorMatrix) {
    if (!colorMatrix || colorMatrix.length !== 20) {
      throw 'invalid color matrix';
    }
    var fptr = copy1dArray(colorMatrix, 'HEAPF32');
    // We know skia memcopies the floats, so we can free our memory after the call returns.
    var m = CanvasKit.ColorFilter._makeMatrix(fptr);
    freeArraysThatAreNotMallocedByUsers(fptr, colorMatrix);
    return m;
  };

  CanvasKit.ContourMeasure.prototype.getPosTan = function(distance, optionalOutput) {
    this._getPosTan(distance, _scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutput) {
      optionalOutput.set(ta);
      return optionalOutput;
    }
    return ta.slice();
  };

  CanvasKit.ImageFilter.MakeMatrixTransform = function(matr, filterQuality, input) {
    var matrPtr = copy3x3MatrixToWasm(matr);
    return CanvasKit.ImageFilter._MakeMatrixTransform(matrPtr, filterQuality, input);
  };

  CanvasKit.Paint.prototype.getColor = function() {
    this._getColor(_scratchColorPtr);
    return copyColorFromWasm(_scratchColorPtr);
  };

  CanvasKit.Paint.prototype.setColor = function(color4f, colorSpace) {
    colorSpace = colorSpace || null; // null will be replaced with sRGB in the C++ method.
    // emscripten wouldn't bind undefined to the sk_sp<ColorSpace> expected here.
    var cPtr = copyColorToWasm(color4f);
    this._setColor(cPtr, colorSpace);
  };

  // The color components here are expected to be floating point values (nominally between
  // 0.0 and 1.0, but with wider color gamuts, the values could exceed this range). To convert
  // between standard 8 bit colors and floats, just divide by 255 before passing them in.
  CanvasKit.Paint.prototype.setColorComponents = function(r, g, b, a, colorSpace) {
    colorSpace = colorSpace || null; // null will be replaced with sRGB in the C++ method.
    // emscripten wouldn't bind undefined to the sk_sp<ColorSpace> expected here.
    var cPtr = copyColorComponentsToWasm(r, g, b, a);
    this._setColor(cPtr, colorSpace);
  };

  CanvasKit.Path.prototype.getPoint = function(idx, optionalOutput) {
    // This will copy 2 floats into a space for 4 floats
    this._getPoint(idx, _scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutput) {
      // We cannot call optionalOutput.set() because it is an error to call .set() with
      // a source bigger than the destination.
      optionalOutput[0] = ta[0];
      optionalOutput[1] = ta[1];
      return optionalOutput;
    }
    // Be sure to return a copy of just the first 2 values.
    return ta.slice(0, 2);
  };

  CanvasKit.PictureRecorder.prototype.beginRecording = function(bounds) {
    var bPtr = copyRectToWasm(bounds);
    return this._beginRecording(bPtr);
  };

  CanvasKit.Surface.prototype.makeImageSnapshot = function(optionalBoundsRect) {
    var bPtr = copyIRectToWasm(optionalBoundsRect);
    return this._makeImageSnapshot(bPtr);
  };

  CanvasKit.Surface.prototype.requestAnimationFrame = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    requestAnimationFrame(function() {
      if (this._context !== undefined) {
        CanvasKit.setCurrentContext(this._context);
      }

      callback(this._cached_canvas);

      // We do not dispose() of the Surface here, as the client will typically
      // call requestAnimationFrame again from within the supplied callback.
      // For drawing a single frame, prefer drawOnce().
      this.flush(dirtyRect);
    }.bind(this));
  };

  // drawOnce will dispose of the surface after drawing the frame using the provided
  // callback.
  CanvasKit.Surface.prototype.drawOnce = function(callback, dirtyRect) {
    if (!this._cached_canvas) {
      this._cached_canvas = this.getCanvas();
    }
    requestAnimationFrame(function() {
      if (this._context !== undefined) {
        CanvasKit.setCurrentContext(this._context);
      }
      callback(this._cached_canvas);

      this.flush(dirtyRect);
      this.dispose();
    }.bind(this));
  };

  CanvasKit.PathEffect.MakeDash = function(intervals, phase) {
    if (!phase) {
      phase = 0;
    }
    if (!intervals.length || intervals.length % 2 === 1) {
      throw 'Intervals array must have even length';
    }
    var ptr = copy1dArray(intervals, 'HEAPF32');
    var dpe = CanvasKit.PathEffect._MakeDash(ptr, intervals.length, phase);
    freeArraysThatAreNotMallocedByUsers(ptr, intervals);
    return dpe;
  };

  CanvasKit.Shader.MakeColor = function(color4f, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtr = copyColorToWasm(color4f);
    return CanvasKit.Shader._MakeColor(cPtr, colorSpace);
  };

  // TODO(kjlubick) remove deprecated names.
  CanvasKit.Shader.Blend = CanvasKit.Shader.MakeBlend;
  CanvasKit.Shader.Color = CanvasKit.Shader.MakeColor;
  CanvasKit.Shader.Lerp = CanvasKit.Shader.MakeLerp;

  CanvasKit.Shader.MakeLinearGradient = function(start, end, colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    // Copy start and end to _scratchFourFloatsAPtr.
    var startEndPts = _scratchFourFloatsA['toTypedArray']();
    startEndPts.set(start);
    startEndPts.set(end, 2);

    var lgs = CanvasKit.Shader._MakeLinearGradient(_scratchFourFloatsAPtr, cPtrInfo.colorPtr, cPtrInfo.colorType, posPtr,
                                                   cPtrInfo.count, mode, flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return lgs;
  };

  CanvasKit.Shader.MakeRadialGradient = function(center, radius, colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var rgs = CanvasKit.Shader._MakeRadialGradient(center[0], center[1], radius, cPtrInfo.colorPtr,
                                                   cPtrInfo.colorType, posPtr, cPtrInfo.count, mode,
                                                   flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return rgs;
  };

  CanvasKit.Shader.MakeSweepGradient = function(cx, cy, colors, pos, mode, localMatrix, flags, startAngle, endAngle, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr = copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    startAngle = startAngle || 0;
    endAngle = endAngle || 360;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    var sgs = CanvasKit.Shader._MakeSweepGradient(cx, cy, cPtrInfo.colorPtr, cPtrInfo.colorType, posPtr,
                                                  cPtrInfo.count, mode,
                                                  startAngle, endAngle, flags,
                                                  localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return sgs;
  };

  CanvasKit.Shader.MakeTwoPointConicalGradient = function(start, startRadius, end, endRadius,
                                                          colors, pos, mode, localMatrix, flags, colorSpace) {
    colorSpace = colorSpace || null;
    var cPtrInfo = copyFlexibleColorArray(colors);
    var posPtr =   copy1dArray(pos, 'HEAPF32');
    flags = flags || 0;
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);

    // Copy start and end to _scratchFourFloatsAPtr.
    var startEndPts = _scratchFourFloatsA['toTypedArray']();
    startEndPts.set(start);
    startEndPts.set(end, 2);

    var rgs = CanvasKit.Shader._MakeTwoPointConicalGradient(_scratchFourFloatsAPtr,
                          startRadius, endRadius, cPtrInfo.colorPtr, cPtrInfo.colorType,
                          posPtr, cPtrInfo.count, mode, flags, localMatrixPtr, colorSpace);

    freeArraysThatAreNotMallocedByUsers(cPtrInfo.colorPtr, colors);
    pos && freeArraysThatAreNotMallocedByUsers(posPtr, pos);
    return rgs;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.Vertices.prototype.bounds = function(optionalOutputArray) {
    this._bounds(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  // Run through the JS files that are added at compile time.
  if (CanvasKit._extraInitializations) {
    CanvasKit._extraInitializations.forEach(function(init) {
      init();
    });
  }
}; // end CanvasKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.

// Accepts an object holding two canvaskit colors.
// {
//    ambient: [r, g, b, a],
//    spot: [r, g, b, a],
// }
// Returns the same format. Note, if malloced colors are passed in, the memory
// housing the passed in colors passed in will be overwritten with the computed
// tonal colors.
CanvasKit.computeTonalColors = function(tonalColors) {
    // copy the colors into WASM
    var cPtrAmbi = copyColorToWasmNoScratch(tonalColors['ambient']);
    var cPtrSpot = copyColorToWasmNoScratch(tonalColors['spot']);
    // The output of this function will be the same pointers we passed in.
    this._computeTonalColors(cPtrAmbi, cPtrSpot);
    // Read the results out.
    var result =  {
      'ambient': copyColorFromWasm(cPtrAmbi),
      'spot': copyColorFromWasm(cPtrSpot),
    };
    // If the user passed us malloced colors in here, we don't want to clean them up.
    freeArraysThatAreNotMallocedByUsers(cPtrAmbi, tonalColors['ambient']);
    freeArraysThatAreNotMallocedByUsers(cPtrSpot, tonalColors['spot']);
    return result;
};

CanvasKit.LTRBRect = function(l, t, r, b) {
  return Float32Array.of(l, t, r, b);
};

CanvasKit.XYWHRect = function(x, y, w, h) {
  return Float32Array.of(x, y, x+w, y+h);
};

CanvasKit.LTRBiRect = function(l, t, r, b) {
  return Int32Array.of(l, t, r, b);
};

CanvasKit.XYWHiRect = function(x, y, w, h) {
  return Int32Array.of(x, y, x+w, y+h);
};

// RRectXY returns a TypedArray representing an RRect with the given rect and a radiusX and
// radiusY for all 4 corners.
CanvasKit.RRectXY = function(rect, rx, ry) {
  return Float32Array.of(
    rect[0], rect[1], rect[2], rect[3],
    rx, ry,
    rx, ry,
    rx, ry,
    rx, ry,
  );
};

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeAnimatedImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeAnimatedImage(iptr, data.byteLength);
  if (!img) {
    Debug('Could not decode animated image');
    return null;
  }
  return img;
};

// data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
CanvasKit.MakeImageFromEncoded = function(data) {
  data = new Uint8Array(data);

  var iptr = CanvasKit._malloc(data.byteLength);
  CanvasKit.HEAPU8.set(data, iptr);
  var img = CanvasKit._decodeImage(iptr, data.byteLength);
  if (!img) {
    Debug('Could not decode image');
    return null;
  }
  return img;
};

// A variable to hold a canvasElement which can be reused once created the first time.
var memoizedCanvas2dElement = null;

// Alternative to CanvasKit.MakeImageFromEncoded. Allows for CanvasKit users to take advantage of
// browser APIs to decode images instead of using codecs included in the CanvasKit wasm binary.
// Expects that the canvasImageSource has already loaded/decoded.
// CanvasImageSource reference: https://developer.mozilla.org/en-US/docs/Web/API/CanvasImageSource
CanvasKit.MakeImageFromCanvasImageSource = function(canvasImageSource) {
  var width = canvasImageSource.width;
  var height = canvasImageSource.height;

  if (!memoizedCanvas2dElement) {
    memoizedCanvas2dElement = document.createElement('canvas');
  }
  memoizedCanvas2dElement.width = width;
  memoizedCanvas2dElement.height = height;

  var ctx2d = memoizedCanvas2dElement.getContext('2d');
  ctx2d.drawImage(canvasImageSource, 0, 0);

  var imageData = ctx2d.getImageData(0, 0, width, height);

  return CanvasKit.MakeImage({
      'width': width,
      'height': height,
      'alphaType': CanvasKit.AlphaType.Unpremul,
      'colorType': CanvasKit.ColorType.RGBA_8888,
      'colorSpace': CanvasKit.ColorSpace.SRGB
    }, imageData.data, 4 * width);
};

// pixels may be an array but Uint8Array or Uint8ClampedArray is recommended,
// with the bytes representing the pixel values.
// (e.g. each set of 4 bytes could represent RGBA values for a single pixel).
CanvasKit.MakeImage = function(info, pixels, bytesPerRow) {
  var pptr = CanvasKit._malloc(pixels.length);
  CanvasKit.HEAPU8.set(pixels, pptr); // We always want to copy the bytes into the WASM heap.
  // No need to _free pptr, Image takes it with SkData::MakeFromMalloc
  return CanvasKit._MakeImage(info, pptr, pixels.length, bytesPerRow);
};

// Colors may be a Uint32Array of int colors, a Flat Float32Array of float colors
// or a 2d Array of Float32Array(4) (deprecated)
// the underlying Skia function accepts only int colors so it is recommended
// to pass an array of int colors to avoid an extra conversion.
// ColorBuilder is not accepted.
CanvasKit.MakeVertices = function(mode, positions, textureCoordinates, colors,
                                  indices, isVolatile) {
  // Default isVolatile to true if not set
  isVolatile = isVolatile === undefined ? true : isVolatile;
  var idxCount = (indices && indices.length) || 0;

  var flags = 0;
  // These flags are from SkVertices.h and should be kept in sync with those.
  if (textureCoordinates && textureCoordinates.length) {
    flags |= (1 << 0);
  }
  if (colors && colors.length) {
    flags |= (1 << 1);
  }
  if (!isVolatile) {
    flags |= (1 << 2);
  }

  var builder = new CanvasKit._VerticesBuilder(mode, positions.length / 2, idxCount, flags);

  copy1dArray(positions, 'HEAPF32', builder.positions());
  if (builder.texCoords()) {
    copy1dArray(textureCoordinates, 'HEAPF32', builder.texCoords());
  }
  if (builder.colors()) {
    if (colors.build) {
      throw('Color builder not accepted by MakeVertices, use array of ints');
    } else {
      copy1dArray(assureIntColors(colors), 'HEAPU32', builder.colors());
    }
  }
  if (builder.indices()) {
    copy1dArray(indices, 'HEAPU16', builder.indices());
  }

  // Create the vertices, which owns the memory that the builder had allocated.
  return builder.detach();
};

/*
 * Add some helpers for matrices. This is ported from SkMatrix.cpp and others
 * to save complexity and overhead of going back and forth between C++ and JS layers.
 * I would have liked to use something like DOMMatrix, except it
 * isn't widely supported (would need polyfills) and it doesn't
 * have a mapPoints() function (which could maybe be tacked on here).
 * If DOMMatrix catches on, it would be worth re-considering this usage.
 */

CanvasKit.Matrix = {};
function sdot() { // to be called with an even number of scalar args
  var acc = 0;
  for (var i=0; i < arguments.length-1; i+=2) {
    acc += arguments[i] * arguments[i+1];
  }
  return acc;
}

// Private general matrix functions used in both 3x3s and 4x4s.
// Return a square identity matrix of size n.
var identityN = function(n) {
  var size = n*n;
  var m = new Array(size);
  while(size--) {
    m[size] = size%(n+1) === 0 ? 1.0 : 0.0;
  }
  return m;
};

// Stride, a function for compactly representing several ways of copying an array into another.
// Write vector `v` into matrix `m`. `m` is a matrix encoded as an array in row-major
// order. Its width is passed as `width`. `v` is an array with length < (m.length/width).
// An element of `v` is copied into `m` starting at `offset` and moving `colStride` cols right
// each row.
//
// For example, a width of 4, offset of 3, and stride of -1 would put the vector here.
// _ _ 0 _
// _ 1 _ _
// 2 _ _ _
// _ _ _ 3
//
var stride = function(v, m, width, offset, colStride) {
  for (var i=0; i<v.length; i++) {
    m[i * width + // column
      (i * colStride + offset + width) % width // row
    ] = v[i];
  }
  return m;
};

CanvasKit.Matrix.identity = function() {
  return identityN(3);
};

// Return the inverse (if it exists) of this matrix.
// Otherwise, return null.
CanvasKit.Matrix.invert = function(m) {
  // Find the determinant by the sarrus rule. https://en.wikipedia.org/wiki/Rule_of_Sarrus
  var det = m[0]*m[4]*m[8] + m[1]*m[5]*m[6] + m[2]*m[3]*m[7]
          - m[2]*m[4]*m[6] - m[1]*m[3]*m[8] - m[0]*m[5]*m[7];
  if (!det) {
    Debug('Warning, uninvertible matrix');
    return null;
  }
  // Return the inverse by the formula adj(m)/det.
  // adj (adjugate) of a 3x3 is the transpose of it's cofactor matrix.
  // a cofactor matrix is a matrix where each term is +-det(N) where matrix N is the 2x2 formed
  // by removing the row and column we're currently setting from the source.
  // the sign alternates in a checkerboard pattern with a `+` at the top left.
  // that's all been combined here into one expression.
  return [
    (m[4]*m[8] - m[5]*m[7])/det, (m[2]*m[7] - m[1]*m[8])/det, (m[1]*m[5] - m[2]*m[4])/det,
    (m[5]*m[6] - m[3]*m[8])/det, (m[0]*m[8] - m[2]*m[6])/det, (m[2]*m[3] - m[0]*m[5])/det,
    (m[3]*m[7] - m[4]*m[6])/det, (m[1]*m[6] - m[0]*m[7])/det, (m[0]*m[4] - m[1]*m[3])/det,
  ];
};

// Maps the given points according to the passed in matrix.
// Results are done in place.
// See SkMatrix.h::mapPoints for the docs on the math.
CanvasKit.Matrix.mapPoints = function(matrix, ptArr) {
  if (IsDebug && (ptArr.length % 2)) {
    throw 'mapPoints requires an even length arr';
  }
  for (var i = 0; i < ptArr.length; i+=2) {
    var x = ptArr[i], y = ptArr[i+1];
    // Gx+Hy+I
    var denom  = matrix[6]*x + matrix[7]*y + matrix[8];
    // Ax+By+C
    var xTrans = matrix[0]*x + matrix[1]*y + matrix[2];
    // Dx+Ey+F
    var yTrans = matrix[3]*x + matrix[4]*y + matrix[5];
    ptArr[i]   = xTrans/denom;
    ptArr[i+1] = yTrans/denom;
  }
  return ptArr;
};

function isnumber(val) { return !isNaN(val); }

// generalized iterative algorithm for multiplying two matrices.
function multiply(m1, m2, size) {

  if (IsDebug && (!m1.every(isnumber) || !m2.every(isnumber))) {
    throw 'Some members of matrices are NaN m1='+m1+', m2='+m2+'';
  }
  if (IsDebug && (m1.length !== m2.length)) {
    throw 'Undefined for matrices of different sizes. m1.length='+m1.length+', m2.length='+m2.length;
  }
  if (IsDebug && (size*size !== m1.length)) {
    throw 'Undefined for non-square matrices. array size was '+size;
  }

  var result = Array(m1.length);
  for (var r = 0; r < size; r++) {
    for (var c = 0; c < size; c++) {
      // accumulate a sum of m1[r,k]*m2[k, c]
      var acc = 0;
      for (var k = 0; k < size; k++) {
        acc += m1[size * r + k] * m2[size * k + c];
      }
      result[r * size + c] = acc;
    }
  }
  return result;
}

// Accept an integer indicating the size of the matrices being multiplied (3 for 3x3), and any
// number of matrices following it.
function multiplyMany(size, listOfMatrices) {
  if (IsDebug && (listOfMatrices.length < 2)) {
    throw 'multiplication expected two or more matrices';
  }
  var result = multiply(listOfMatrices[0], listOfMatrices[1], size);
  var next = 2;
  while (next < listOfMatrices.length) {
    result = multiply(result, listOfMatrices[next], size);
    next++;
  }
  return result;
}

// Accept any number 3x3 of matrices as arguments, multiply them together.
// Matrix multiplication is associative but not commutative. the order of the arguments
// matters, but it does not matter that this implementation multiplies them left to right.
CanvasKit.Matrix.multiply = function() {
  return multiplyMany(3, arguments);
};

// Return a matrix representing a rotation by n radians.
// px, py optionally say which point the rotation should be around
// with the default being (0, 0);
CanvasKit.Matrix.rotated = function(radians, px, py) {
  px = px || 0;
  py = py || 0;
  var sinV = Math.sin(radians);
  var cosV = Math.cos(radians);
  return [
    cosV, -sinV, sdot( sinV, py, 1 - cosV, px),
    sinV,  cosV, sdot(-sinV, px, 1 - cosV, py),
    0,        0,                             1,
  ];
};

CanvasKit.Matrix.scaled = function(sx, sy, px, py) {
  px = px || 0;
  py = py || 0;
  var m = stride([sx, sy], identityN(3), 3, 0, 1);
  return stride([px-sx*px, py-sy*py], m, 3, 2, 0);
};

CanvasKit.Matrix.skewed = function(kx, ky, px, py) {
  px = px || 0;
  py = py || 0;
  var m = stride([kx, ky], identityN(3), 3, 1, -1);
  return stride([-kx*px, -ky*py], m, 3, 2, 0);
};

CanvasKit.Matrix.translated = function(dx, dy) {
  return stride(arguments, identityN(3), 3, 2, 0);
};

// Functions for manipulating vectors.
// Loosely based off of SkV3 in SkM44.h but skia also has SkVec2 and Skv4. This combines them and
// works on vectors of any length.
CanvasKit.Vector = {};
CanvasKit.Vector.dot = function(a, b) {
  if (IsDebug && (a.length !== b.length)) {
    throw 'Cannot perform dot product on arrays of different length ('+a.length+' vs '+b.length+')';
  }
  return a.map(function(v, i) { return v*b[i] }).reduce(function(acc, cur) { return acc + cur; });
};
CanvasKit.Vector.lengthSquared = function(v) {
  return CanvasKit.Vector.dot(v, v);
};
CanvasKit.Vector.length = function(v) {
  return Math.sqrt(CanvasKit.Vector.lengthSquared(v));
};
CanvasKit.Vector.mulScalar = function(v, s) {
  return v.map(function(i) { return i*s });
};
CanvasKit.Vector.add = function(a, b) {
  return a.map(function(v, i) { return v+b[i] });
};
CanvasKit.Vector.sub = function(a, b) {
  return a.map(function(v, i) { return v-b[i]; });
};
CanvasKit.Vector.dist = function(a, b) {
  return CanvasKit.Vector.length(CanvasKit.Vector.sub(a, b));
};
CanvasKit.Vector.normalize = function(v) {
  return CanvasKit.Vector.mulScalar(v, 1/CanvasKit.Vector.length(v));
};
CanvasKit.Vector.cross = function(a, b) {
  if (IsDebug && (a.length !== 3 || a.length !== 3)) {
    throw 'Cross product is only defined for 3-dimensional vectors (a.length='+a.length+', b.length='+b.length+')';
  }
  return [
    a[1]*b[2] - a[2]*b[1],
    a[2]*b[0] - a[0]*b[2],
    a[0]*b[1] - a[1]*b[0],
  ];
};

// Functions for creating and manipulating (row-major) 4x4 matrices. Accepted in place of
// SkM44 in canvas methods, for the same reasons as the 3x3 matrices above.
// ported from C++ code in SkM44.cpp
CanvasKit.M44 = {};
// Create a 4x4 identity matrix
CanvasKit.M44.identity = function() {
  return identityN(4);
};

// Anything named vec below is an array of length 3 representing a vector/point in 3D space.
// Create a 4x4 matrix representing a translate by the provided 3-vec
CanvasKit.M44.translated = function(vec) {
  return stride(vec, identityN(4), 4, 3, 0);
};
// Create a 4x4 matrix representing a scaling by the provided 3-vec
CanvasKit.M44.scaled = function(vec) {
  return stride(vec, identityN(4), 4, 0, 1);
};
// Create a 4x4 matrix representing a rotation about the provided axis 3-vec.
// axis does not need to be normalized.
CanvasKit.M44.rotated = function(axisVec, radians) {
  return CanvasKit.M44.rotatedUnitSinCos(
    CanvasKit.Vector.normalize(axisVec), Math.sin(radians), Math.cos(radians));
};
// Create a 4x4 matrix representing a rotation about the provided normalized axis 3-vec.
// Rotation is provided redundantly as both sin and cos values.
// This rotate can be used when you already have the cosAngle and sinAngle values
// so you don't have to atan(cos/sin) to call roatated() which expects an angle in radians.
// this does no checking! Behavior for invalid sin or cos values or non-normalized axis vectors
// is incorrect. Prefer rotated().
CanvasKit.M44.rotatedUnitSinCos = function(axisVec, sinAngle, cosAngle) {
  var x = axisVec[0];
  var y = axisVec[1];
  var z = axisVec[2];
  var c = cosAngle;
  var s = sinAngle;
  var t = 1 - c;
  return [
    t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
    t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
    t*x*z - s*y, t*y*z + s*x, t*z*z + c,   0,
    0,           0,           0,           1
  ];
};
// Create a 4x4 matrix representing a camera at eyeVec, pointed at centerVec.
CanvasKit.M44.lookat = function(eyeVec, centerVec, upVec) {
  var f = CanvasKit.Vector.normalize(CanvasKit.Vector.sub(centerVec, eyeVec));
  var u = CanvasKit.Vector.normalize(upVec);
  var s = CanvasKit.Vector.normalize(CanvasKit.Vector.cross(f, u));

  var m = CanvasKit.M44.identity();
  // set each column's top three numbers
  stride(s,                                   m, 4, 0, 0);
  stride(CanvasKit.Vector.cross(s, f),      m, 4, 1, 0);
  stride(CanvasKit.Vector.mulScalar(f, -1), m, 4, 2, 0);
  stride(eyeVec,                              m, 4, 3, 0);

  var m2 = CanvasKit.M44.invert(m);
  if (m2 === null) {
    return CanvasKit.M44.identity();
  }
  return m2;
};
// Create a 4x4 matrix representing a perspective. All arguments are scalars.
// angle is in radians.
CanvasKit.M44.perspective = function(near, far, angle) {
  if (IsDebug && (far <= near)) {
    throw 'far must be greater than near when constructing M44 using perspective.';
  }
  var dInv = 1 / (far - near);
  var halfAngle = angle / 2;
  var cot = Math.cos(halfAngle) / Math.sin(halfAngle);
  return [
    cot, 0,   0,               0,
    0,   cot, 0,               0,
    0,   0,   (far+near)*dInv, 2*far*near*dInv,
    0,   0,   -1,              1,
  ];
};
// Returns the number at the given row and column in matrix m.
CanvasKit.M44.rc = function(m, r, c) {
  return m[r*4+c];
};
// Accepts any number of 4x4 matrix arguments, multiplies them left to right.
CanvasKit.M44.multiply = function() {
  return multiplyMany(4, arguments);
};

// Invert the 4x4 matrix if it is invertible and return it. if not, return null.
// taken from SkM44.cpp (altered to use row-major order)
// m is not altered.
CanvasKit.M44.invert = function(m) {
  if (IsDebug && !m.every(isnumber)) {
    throw 'some members of matrix are NaN m='+m;
  }

  var a00 = m[0];
  var a01 = m[4];
  var a02 = m[8];
  var a03 = m[12];
  var a10 = m[1];
  var a11 = m[5];
  var a12 = m[9];
  var a13 = m[13];
  var a20 = m[2];
  var a21 = m[6];
  var a22 = m[10];
  var a23 = m[14];
  var a30 = m[3];
  var a31 = m[7];
  var a32 = m[11];
  var a33 = m[15];

  var b00 = a00 * a11 - a01 * a10;
  var b01 = a00 * a12 - a02 * a10;
  var b02 = a00 * a13 - a03 * a10;
  var b03 = a01 * a12 - a02 * a11;
  var b04 = a01 * a13 - a03 * a11;
  var b05 = a02 * a13 - a03 * a12;
  var b06 = a20 * a31 - a21 * a30;
  var b07 = a20 * a32 - a22 * a30;
  var b08 = a20 * a33 - a23 * a30;
  var b09 = a21 * a32 - a22 * a31;
  var b10 = a21 * a33 - a23 * a31;
  var b11 = a22 * a33 - a23 * a32;

  // calculate determinate
  var det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
  var invdet = 1.0 / det;

  // bail out if the matrix is not invertible
  if (det === 0 || invdet === Infinity) {
    Debug('Warning, uninvertible matrix');
    return null;
  }

  b00 *= invdet;
  b01 *= invdet;
  b02 *= invdet;
  b03 *= invdet;
  b04 *= invdet;
  b05 *= invdet;
  b06 *= invdet;
  b07 *= invdet;
  b08 *= invdet;
  b09 *= invdet;
  b10 *= invdet;
  b11 *= invdet;

  // store result in row major order
  var tmp = [
    a11 * b11 - a12 * b10 + a13 * b09,
    a12 * b08 - a10 * b11 - a13 * b07,
    a10 * b10 - a11 * b08 + a13 * b06,
    a11 * b07 - a10 * b09 - a12 * b06,

    a02 * b10 - a01 * b11 - a03 * b09,
    a00 * b11 - a02 * b08 + a03 * b07,
    a01 * b08 - a00 * b10 - a03 * b06,
    a00 * b09 - a01 * b07 + a02 * b06,

    a31 * b05 - a32 * b04 + a33 * b03,
    a32 * b02 - a30 * b05 - a33 * b01,
    a30 * b04 - a31 * b02 + a33 * b00,
    a31 * b01 - a30 * b03 - a32 * b00,

    a22 * b04 - a21 * b05 - a23 * b03,
    a20 * b05 - a22 * b02 + a23 * b01,
    a21 * b02 - a20 * b04 - a23 * b00,
    a20 * b03 - a21 * b01 + a22 * b00,
  ];


  if (!tmp.every(function(val) { return !isNaN(val) && val !== Infinity && val !== -Infinity; })) {
    Debug('inverted matrix contains infinities or NaN '+tmp);
    return null;
  }
  return tmp;
};

CanvasKit.M44.transpose = function(m) {
  return [
    m[0], m[4], m[8], m[12],
    m[1], m[5], m[9], m[13],
    m[2], m[6], m[10], m[14],
    m[3], m[7], m[11], m[15],
  ];
};

// Return the inverse of an SkM44. throw an error if it's not invertible
CanvasKit.M44.mustInvert = function(m) {
  var m2 = CanvasKit.M44.invert(m);
  if (m2 === null) {
    throw 'Matrix not invertible';
  }
  return m2;
};

// returns a matrix that sets up a 3D perspective view from a given camera.
//
// area - a rect describing the viewport. (0, 0, canvas_width, canvas_height) suggested
// zscale - a scalar describing the scale of the z axis. min(width, height)/2 suggested
// cam - an object with the following attributes
// const cam = {
//   'eye'  : [0, 0, 1 / Math.tan(Math.PI / 24) - 1], // a 3D point locating the camera
//   'coa'  : [0, 0, 0], // center of attention - the 3D point the camera is looking at.
//   'up'   : [0, 1, 0], // a unit vector pointing in the camera's up direction, because eye and
//                       // coa alone leave roll unspecified.
//   'near' : 0.02,      // near clipping plane
//   'far'  : 4,         // far clipping plane
//   'angle': Math.PI / 12, // field of view in radians
// };
CanvasKit.M44.setupCamera = function(area, zscale, cam) {
  var camera = CanvasKit.M44.lookat(cam['eye'], cam['coa'], cam['up']);
  var perspective = CanvasKit.M44.perspective(cam['near'], cam['far'], cam['angle']);
  var center = [(area[0] + area[2])/2, (area[1] + area[3])/2, 0];
  var viewScale = [(area[2] - area[0])/2, (area[3] - area[1])/2, zscale];
  var viewport = CanvasKit.M44.multiply(
    CanvasKit.M44.translated(center),
    CanvasKit.M44.scaled(viewScale));
  return CanvasKit.M44.multiply(
    viewport, perspective, camera, CanvasKit.M44.mustInvert(viewport));
};

// An ColorMatrix is a 4x4 color matrix that transforms the 4 color channels
//  with a 1x4 matrix that post-translates those 4 channels.
// For example, the following is the layout with the scale (S) and post-transform
// (PT) items indicated.
// RS,  0,  0,  0 | RPT
//  0, GS,  0,  0 | GPT
//  0,  0, BS,  0 | BPT
//  0,  0,  0, AS | APT
//
// Much of this was hand-transcribed from SkColorMatrix.cpp, because it's easier to
// deal with a Float32Array of length 20 than to try to expose the SkColorMatrix object.

var rScale = 0;
var gScale = 6;
var bScale = 12;
var aScale = 18;

var rPostTrans = 4;
var gPostTrans = 9;
var bPostTrans = 14;
var aPostTrans = 19;

CanvasKit.ColorMatrix = {};
CanvasKit.ColorMatrix.identity = function() {
  var m = new Float32Array(20);
  m[rScale] = 1;
  m[gScale] = 1;
  m[bScale] = 1;
  m[aScale] = 1;
  return m;
};

CanvasKit.ColorMatrix.scaled = function(rs, gs, bs, as) {
  var m = new Float32Array(20);
  m[rScale] = rs;
  m[gScale] = gs;
  m[bScale] = bs;
  m[aScale] = as;
  return m;
};

var rotateIndices = [
  [6, 7, 11, 12],
  [0, 10, 2, 12],
  [0, 1,  5,  6],
];
// axis should be 0, 1, 2 for r, g, b
CanvasKit.ColorMatrix.rotated = function(axis, sine, cosine) {
  var m = CanvasKit.ColorMatrix.identity();
  var indices = rotateIndices[axis];
  m[indices[0]] = cosine;
  m[indices[1]] = sine;
  m[indices[2]] = -sine;
  m[indices[3]] = cosine;
  return m;
};

// m is a ColorMatrix (i.e. a Float32Array), and this sets the 4 "special"
// params that will translate the colors after they are multiplied by the 4x4 matrix.
CanvasKit.ColorMatrix.postTranslate = function(m, dr, dg, db, da) {
  m[rPostTrans] += dr;
  m[gPostTrans] += dg;
  m[bPostTrans] += db;
  m[aPostTrans] += da;
  return m;
};

// concat returns a new ColorMatrix that is the result of multiplying outer*inner
CanvasKit.ColorMatrix.concat = function(outer, inner) {
  var m = new Float32Array(20);
  var index = 0;
  for (var j = 0; j < 20; j += 5) {
      for (var i = 0; i < 4; i++) {
          m[index++] =  outer[j + 0] * inner[i + 0] +
                        outer[j + 1] * inner[i + 5] +
                        outer[j + 2] * inner[i + 10] +
                        outer[j + 3] * inner[i + 15];
      }
      m[index++] =  outer[j + 0] * inner[4] +
                    outer[j + 1] * inner[9] +
                    outer[j + 2] * inner[14] +
                    outer[j + 3] * inner[19] +
                    outer[j + 4];
  }

  return m;
};
(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

    CanvasKit.Paragraph.prototype.getRectsForRange = function(start, end, hStyle, wStyle) {
    /**
     * This is bytes, but we'll want to think about them as float32s
     * @type {Float32Array}
     */
      var floatArray = this._getRectsForRange(start, end, hStyle, wStyle);
      return floatArrayToRects(floatArray);
    }

    CanvasKit.Paragraph.prototype.getRectsForPlaceholders = function() {
        /**
        * This is bytes, but we'll want to think about them as float32s
        * @type {Float32Array}
        */
        var floatArray = this._getRectsForPlaceholders();
        return floatArrayToRects(floatArray);
    }

    function floatArrayToRects(floatArray) {
        if (!floatArray || !floatArray.length) {
            return [];
        }
        var ret = [];
        for (var i = 0; i < floatArray.length; i+=5) {
            var r = CanvasKit.LTRBRect(floatArray[i], floatArray[i+1], floatArray[i+2], floatArray[i+3]);
            if (floatArray[i+4] === 0) {
                r['direction'] = CanvasKit.TextDirection.RTL;
            } else {
                r['direction'] = CanvasKit.TextDirection.LTR;
            }
            ret.push(r);
        }
        CanvasKit._free(floatArray.byteOffset);
        return ret;
    }

    // Registers the font (provided as an arrayBuffer) with the alias `family`.
    CanvasKit.TypefaceFontProvider.prototype.registerFont = function(font, family) {
      var typeface = CanvasKit.FontMgr.RefDefault().MakeTypefaceFromData(font);
      if (!typeface) {
          Debug('Could not decode font data');
          // We do not need to free the data since the C++ will do that for us
          // when the font is deleted (or fails to decode);
          return null;
      }
      var familyPtr = cacheOrCopyString(family);
      this._registerFont(typeface, familyPtr);
    }

    // These helpers fill out all fields, because emscripten complains if we
    // have undefined and it expects, for example, a float.
    // TODO(kjlubick) For efficiency, we should probably just return opaque WASM objects so we do
    //   not have to keep copying them across the wire.
    CanvasKit.ParagraphStyle = function(s) {
      // Use [''] to tell closure not to minify the names
      s['disableHinting'] = s['disableHinting'] || false;
      if (s['ellipsis']) {
        var str = s['ellipsis'];
        s['_ellipsisPtr'] = cacheOrCopyString(str);
        s['_ellipsisLen'] = lengthBytesUTF8(str) + 1; // add 1 for the null terminator.
      } else {
        s['_ellipsisPtr'] = nullptr;
        s['_ellipsisLen'] = 0;
      }

      s['heightMultiplier'] = s['heightMultiplier'] || 0;
      s['maxLines'] = s['maxLines'] || 0;
      s['textAlign'] = s['textAlign'] || CanvasKit.TextAlign.Start;
      s['textDirection'] = s['textDirection'] || CanvasKit.TextDirection.LTR;
      s['textStyle'] = CanvasKit.TextStyle(s['textStyle']);
      s['strutStyle'] = strutStyle(s['strutStyle']);
      return s;
    };

    function fontStyle(s) {
      s = s || {};
      // Can't check for falsey as 0 width means "invisible".
      if (s['weight'] === undefined) {
        s['weight'] = CanvasKit.FontWeight.Normal;
      }
      s['width'] = s['width'] || CanvasKit.FontWidth.Normal;
      s['slant'] = s['slant'] || CanvasKit.FontSlant.Upright;
      return s;
    }

    function strutStyle(s) {
        s = s || {};
        s['strutEnabled'] = s['strutEnabled'] || false;

        if (s['strutEnabled'] && Array.isArray(s['fontFamilies']) && s['fontFamilies'].length) {
            s['_fontFamiliesPtr'] = naiveCopyStrArray(s['fontFamilies']);
            s['_fontFamiliesLen'] = s['fontFamilies'].length;
        } else {
            s['_fontFamiliesPtr'] = nullptr;
            s['_fontFamiliesLen'] = 0;
        }
        s['fontStyle'] = fontStyle(s['fontStyle']);
        s['fontSize'] = s['fontSize'] || 0;
        s['heightMultiplier'] = s['heightMultiplier'] || 0;
        s['leading'] = s['leading'] || 0;
        s['forceStrutHeight'] = s['forceStrutHeight'] || false;
        return s;
    }

    CanvasKit.TextStyle = function(s) {
       // Use [''] to tell closure not to minify the names
      if (!s['color']) {
        s['color'] = CanvasKit.BLACK;
      }

      s['decoration'] = s['decoration'] || 0;
      s['decorationThickness'] = s['decorationThickness'] || 0;
      s['decorationStyle'] = s['decorationStyle'] || CanvasKit.DecorationStyle.Solid;
      s['textBaseline'] = s['textBaseline'] || CanvasKit.TextBaseline.Alphabetic;
      s['fontSize'] = s['fontSize'] || 0;
      s['letterSpacing'] = s['letterSpacing'] || 0;
      s['wordSpacing'] = s['wordSpacing'] || 0;
      s['heightMultiplier'] = s['heightMultiplier'] || 0;
      if (s['locale']) {
        var str = s['locale'];
        s['_localePtr'] = cacheOrCopyString(str);
        s['_localeLen'] = lengthBytesUTF8(str) + 1; // add 1 for the null terminator.
      } else {
        s['_localePtr'] = nullptr;
        s['_localeLen'] = 0;
      }
      s['fontStyle'] = fontStyle(s['fontStyle']);
      if (s['shadows']) {
        var shadows = s['shadows'];
        var shadowColors = shadows.map(function(s) { return s['color'] || CanvasKit.BLACK; });
        var shadowBlurRadii = shadows.map(function(s) { return s['blurRadius'] || 0.0; });
        s['_shadowLen'] = shadows.length;
        var ptr = CanvasKit._malloc(shadows.length * 2, 'HEAPF32');
        var adjustedPtr = ptr / 4;  // 4 bytes per float
        for (var i = 0; i < shadows.length; i++) {
          var offset = shadows[i]['offset'] || [0, 0];
          CanvasKit.HEAPF32[adjustedPtr] = offset[0];
          CanvasKit.HEAPF32[adjustedPtr + 1] = offset[1];
          adjustedPtr += 2;
        }
        s['_shadowColorsPtr'] = copyFlexibleColorArray(shadowColors).colorPtr;
        s['_shadowOffsetsPtr'] = ptr;
        s['_shadowBlurRadiiPtr'] = copy1dArray(shadowBlurRadii, 'HEAPF32');
      } else {
        s['_shadowLen'] = 0;
        s['_shadowColorsPtr'] = nullptr;
        s['_shadowOffsetsPtr'] = nullptr;
        s['_shadowBlurRadiiPtr'] = nullptr;
      }
      if (s['fontFeatures']) {
        var fontFeatures = s['fontFeatures'];
        var fontFeatureNames = fontFeatures.map(function(s) { return s['name']; });
        var fontFeatureValues = fontFeatures.map(function(s) { return s['value']; });
        s['_fontFeatureLen'] = fontFeatures.length;
        s['_fontFeatureNamesPtr'] = naiveCopyStrArray(fontFeatureNames);
        s['_fontFeatureValuesPtr'] = copy1dArray(fontFeatureValues, 'HEAPU32');
      } else {
        s['_fontFeatureLen'] = 0;
        s['_fontFeatureNamesPtr'] = nullptr;
        s['_fontFeatureValuesPtr'] = nullptr;
      }

      return s;
    };

    // returns a pointer to a place on the heap that has an array
    // of char* (effectively a char**). For now, this does the naive thing
    // and depends on the string being null-terminated. This should be used
    // for simple, well-formed things (e.g. font-families), not arbitrary
    // text that should be drawn. If we need this to handle more complex
    // strings, it should return two pointers, a pointer of the
    // string array and a pointer to an array of the strings byte lengths.
    function naiveCopyStrArray(strings) {
      if (!strings || !strings.length) {
        return nullptr;
      }
      var sPtrs = [];
      for (var i = 0; i < strings.length; i++) {
        var strPtr = cacheOrCopyString(strings[i]);
        sPtrs.push(strPtr);
      }
      return copy1dArray(sPtrs, 'HEAPU32');
    }

    // maps string -> malloc'd pointer
    var stringCache = {};

    // cacheOrCopyString copies a string from JS into WASM on the heap and returns the pointer
    // to the memory of the string. It is expected that a caller to this helper will *not* free
    // that memory, so it is cached. Thus, if a future call to this function with the same string
    // will return the cached pointer, preventing the memory usage from growing unbounded (in
    // a normal use case).
    function cacheOrCopyString(str) {
      if (stringCache[str]) {
        return stringCache[str];
      }
      // Add 1 for null terminator, which we need when copying/converting
      var strLen = lengthBytesUTF8(str) + 1;
      var strPtr = CanvasKit._malloc(strLen);
      stringToUTF8(str, strPtr, strLen);
      stringCache[str] = strPtr;
      return strPtr;
    }

    // These scratch arrays are allocated once to copy the color data into, which saves us
    // having to free them after every invocation.
    var scratchForegroundColorPtr = CanvasKit._malloc(4 * 4); // room for 4 32bit floats
    var scratchBackgroundColorPtr = CanvasKit._malloc(4 * 4); // room for 4 32bit floats
    var scratchDecorationColorPtr = CanvasKit._malloc(4 * 4); // room for 4 32bit floats

    function copyArrays(textStyle) {
      // These color fields were arrays, but will set to WASM pointers before we pass this
      // object over the WASM interface.
      textStyle['_colorPtr'] = copyColorToWasm(textStyle['color']);
      textStyle['_foregroundColorPtr'] = nullptr; // nullptr is 0, from helper.js
      textStyle['_backgroundColorPtr'] = nullptr;
      textStyle['_decorationColorPtr'] = nullptr;
      if (textStyle['foregroundColor']) {
        textStyle['_foregroundColorPtr'] = copyColorToWasm(textStyle['foregroundColor'], scratchForegroundColorPtr);
      }
      if (textStyle['backgroundColor']) {
        textStyle['_backgroundColorPtr'] = copyColorToWasm(textStyle['backgroundColor'], scratchBackgroundColorPtr);
      }
      if (textStyle['decorationColor']) {
        textStyle['_decorationColorPtr'] = copyColorToWasm(textStyle['decorationColor'], scratchDecorationColorPtr);
      }

      if (Array.isArray(textStyle['fontFamilies']) && textStyle['fontFamilies'].length) {
        textStyle['_fontFamiliesPtr'] = naiveCopyStrArray(textStyle['fontFamilies']);
        textStyle['_fontFamiliesLen'] = textStyle['fontFamilies'].length;
      } else {
        textStyle['_fontFamiliesPtr'] = nullptr;
        textStyle['_fontFamiliesLen'] = 0;
        Debug('no font families provided, text may draw wrong or not at all');
      }
    }

    function freeArrays(textStyle) {
      // The font family strings will get copied to a vector on the C++ side, which is owned by
      // the text style.
      CanvasKit._free(textStyle['_fontFamiliesPtr']);
    }

    CanvasKit.ParagraphBuilder.Make = function(paragraphStyle, fontManager) {
      copyArrays(paragraphStyle['textStyle']);

      var result =  CanvasKit.ParagraphBuilder._Make(paragraphStyle, fontManager);
      freeArrays(paragraphStyle['textStyle']);
      return result;
    };

    CanvasKit.ParagraphBuilder.MakeFromFontProvider = function(paragraphStyle, fontProvider) {
        copyArrays(paragraphStyle['textStyle']);

        var result =  CanvasKit.ParagraphBuilder._MakeFromFontProvider(paragraphStyle, fontProvider);
        freeArrays(paragraphStyle['textStyle']);
        return result;
    };

    CanvasKit.ParagraphBuilder.prototype.pushStyle = function(textStyle) {
      copyArrays(textStyle);
      this._pushStyle(textStyle);
      freeArrays(textStyle);
    };

    CanvasKit.ParagraphBuilder.prototype.pushPaintStyle = function(textStyle, fg, bg) {
      copyArrays(textStyle);
      this._pushPaintStyle(textStyle, fg, bg);
      freeArrays(textStyle);
    };

    CanvasKit.ParagraphBuilder.prototype.addPlaceholder =
          function(width, height, alignment, baseline, offset) {
      width = width || 0;
      height = height || 0;
      alignment = alignment || CanvasKit.PlaceholderAlignment.Baseline;
      baseline = baseline || CanvasKit.TextBaseline.Alphabetic;
      offset = offset || 0;
      this._addPlaceholder(width, height, alignment, baseline, offset);
    };
});
}(Module)); // When this file is loaded in, the high level object is "Module";

// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the Skottie builds of canvaskit.

// assets is a dictionary of named blobs: { key: ArrayBuffer, ... }
// The keys should be well-behaved strings - they're turned into null-terminated
// strings for the native side.

// prop_filter_prefix is an optional string acting as a name filter for selecting
// "interesting" Lottie properties (surfaced in the embedded player controls)
CanvasKit.MakeManagedAnimation = function(json, assets, prop_filter_prefix) {
  if (!CanvasKit._MakeManagedAnimation) {
    throw 'Not compiled with MakeManagedAnimation';
  }
  if (!prop_filter_prefix) {
    prop_filter_prefix = '';
  }
  if (!assets) {
    return CanvasKit._MakeManagedAnimation(json, 0, nullptr, nullptr, nullptr, prop_filter_prefix);
  }
  var assetNamePtrs = [];
  var assetDataPtrs = [];
  var assetSizes    = [];

  var assetKeys = Object.keys(assets || {});
  for (var i = 0; i < assetKeys.length; i++) {
    var key = assetKeys[i];
    var buffer = assets[key];
    var data = new Uint8Array(buffer);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    assetDataPtrs.push(iptr);
    assetSizes.push(data.byteLength);

    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(key) + 1;
    var strPtr = CanvasKit._malloc(strLen);

    stringToUTF8(key, strPtr, strLen);
    assetNamePtrs.push(strPtr);
  }

  // Not entirely sure if it matters, but the uintptr_t are 32 bits
  // we want to copy our array of uintptr_t into the right size memory.
  var namesPtr      = copy1dArray(assetNamePtrs, "HEAPU32");
  var assetsPtr     = copy1dArray(assetDataPtrs, "HEAPU32");
  var assetSizesPtr = copy1dArray(assetSizes,    "HEAPU32");

  var anim = CanvasKit._MakeManagedAnimation(json, assetKeys.length, namesPtr,
                                             assetsPtr, assetSizesPtr, prop_filter_prefix);

  // The C++ code has made copies of the asset and string data, so free our copies.
  CanvasKit._free(namesPtr);
  CanvasKit._free(assetsPtr);
  CanvasKit._free(assetSizesPtr);

  return anim;
};

(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

  CanvasKit.Animation.prototype.render = function(canvas, dstRect) {
    copyRectToWasm(dstRect, _scratchFourFloatsAPtr);
    this._render(canvas, _scratchFourFloatsAPtr);
  };

  CanvasKit.Animation.prototype.size = function(optSize) {
    // This will copy 2 floats into a space for 4 floats
    this._size(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optSize) {
      // We cannot call optSize.set() because it is an error to call .set() with
      // a source bigger than the destination.
      optSize[0] = ta[0];
      optSize[1] = ta[1];
      return optSize;
    }
    // Be sure to return a copy of just the first 2 values.
    return ta.slice(0, 2);
  };

  if (CanvasKit.ManagedAnimation) {
    CanvasKit.ManagedAnimation.prototype.render = function(canvas, dstRect) {
    copyRectToWasm(dstRect, _scratchFourFloatsAPtr);
    this._render(canvas, _scratchFourFloatsAPtr);
    };

    CanvasKit.ManagedAnimation.prototype.seek = function(t, optDamageRect) {
      this._seek(t, _scratchFourFloatsAPtr);
      var ta = _scratchFourFloatsA['toTypedArray']();
      if (optDamageRect) {
        optDamageRect.set(ta);
        return optDamageRect;
      }
      return ta.slice();
    };

    CanvasKit.ManagedAnimation.prototype.seekFrame = function(frame, optDamageRect) {
      this._seekFrame(frame, _scratchFourFloatsAPtr);
      var ta = _scratchFourFloatsA['toTypedArray']();
      if (optDamageRect) {
        optDamageRect.set(ta);
        return optDamageRect;
      }
      return ta.slice();
    };

    CanvasKit.ManagedAnimation.prototype.setColor = function(key, color) {
      var cPtr = copyColorToWasm(color);
      this._setColor(key, cPtr);
    };

    CanvasKit.ManagedAnimation.prototype.size = function(optSize) {
      // This will copy 2 floats into a space for 4 floats
      this._size(_scratchFourFloatsAPtr);
      var ta = _scratchFourFloatsA['toTypedArray']();
      if (optSize) {
        // We cannot call optSize.set() because it is an error to call .set() with
        // a source bigger than the destination.
        optSize[0] = ta[0];
        optSize[1] = ta[1];
        return optSize;
      }
      // Be sure to return a copy of just the first 2 values.
      return ta.slice(0, 2);
    };
  }


});
}(Module)); // When this file is loaded in, the high level object is "Module";

// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the Particle builds of canvaskit.

// assets is a dictionary of named blobs: { key: ArrayBuffer, ... }
// The keys should be well-behaved strings - they're turned into null-terminated
// strings for the native side.
CanvasKit.MakeParticles = function(json, assets) {
  if (!CanvasKit._MakeParticles) {
    throw 'Not compiled with MakeParticles';
  }
  if (!assets) {
    return CanvasKit._MakeParticles(json, 0, nullptr, nullptr, nullptr);
  }
  var assetNamePtrs = [];
  var assetDataPtrs = [];
  var assetSizes    = [];

  var assetKeys = Object.keys(assets || {});
  for (var i = 0; i < assetKeys.length; i++) {
    var key = assetKeys[i];
    var buffer = assets[key];
    var data = new Uint8Array(buffer);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    assetDataPtrs.push(iptr);
    assetSizes.push(data.byteLength);

    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(key) + 1;
    var strPtr = CanvasKit._malloc(strLen);

    stringToUTF8(key, strPtr, strLen);
    assetNamePtrs.push(strPtr);
  }

  // Not entirely sure if it matters, but the uintptr_t are 32 bits
  // we want to copy our array of uintptr_t into the right size memory.
  var namesPtr      = copy1dArray(assetNamePtrs, "HEAPU32");
  var assetsPtr     = copy1dArray(assetDataPtrs, "HEAPU32");
  var assetSizesPtr = copy1dArray(assetSizes,    "HEAPU32");

  var particles = CanvasKit._MakeParticles(json, assetKeys.length,
                                           namesPtr, assetsPtr, assetSizesPtr);

  // The C++ code has made copies of the asset and string data, so free our copies.
  CanvasKit._free(namesPtr);
  CanvasKit._free(assetsPtr);
  CanvasKit._free(assetSizesPtr);

  return particles;
};

CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {

  CanvasKit.ParticleEffect.prototype.uniforms = function() {
    var fptr = this._uniformPtr();
    var numFloats = this.getUniformFloatCount();
    if (!fptr || numFloats <= 0) {
      return new Float32Array();
    }
    return new Float32Array(CanvasKit.HEAPU8.buffer, fptr, numFloats);
  };

  CanvasKit.ParticleEffect.setPosition = function(pos) {
    this._setPosition(pos[0], pos[1]);
  };
});

// Adds in the code to use pathops with Path
CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  CanvasKit.Path.prototype.op = function(otherPath, op) {
    if (this._op(otherPath, op)) {
      return this;
    }
    return null;
  };

  CanvasKit.Path.prototype.simplify = function() {
    if (this._simplify()) {
      return this;
    }
    return null;
  };
});

CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {

  // str can be either a text string or a ShapedText object
  CanvasKit.Canvas.prototype.drawText = function(str, x, y, paint, font) {
    if (typeof str === 'string') {
      // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
      // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
      var strLen = lengthBytesUTF8(str);
      // Add 1 for null terminator, which we need when copying/converting, but can ignore
      // when we call into Skia.
      var strPtr = CanvasKit._malloc(strLen + 1);
      stringToUTF8(str, strPtr, strLen + 1);
      this._drawSimpleText(strPtr, strLen, x, y, font, paint);
      CanvasKit._free(strPtr);
    } else {
      this._drawShapedText(str, x, y, paint);
    }
  };

  // Glyphs should be a Uint32Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  // The return value will be a Float32Array that is 4 times as long as the input array. For each
  // glyph, there will be 4 floats for left, top, right, bottom (relative to 0, 0) for that glyph.
  CanvasKit.Font.prototype.getGlyphBounds = function(glyphs, paint, optionalOutputArray) {
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerRect = 4 * 4;
    var rectPtr = CanvasKit._malloc(glyphs.length * bytesPerRect);
    this._getGlyphWidthBounds(glyphPtr, glyphs.length, nullptr, rectPtr, paint || null);

    var rects = new Float32Array(CanvasKit.HEAPU8.buffer, rectPtr, glyphs.length * 4);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (optionalOutputArray) {
      optionalOutputArray.set(rects);
      CanvasKit._free(rectPtr);
      return optionalOutputArray;
    }
    var rv = Float32Array.from(rects);
    CanvasKit._free(rectPtr);
    return rv;
  };

  CanvasKit.Font.prototype.getGlyphIDs = function(str, numGlyphIDs, optionalOutputArray) {
    if (!numGlyphIDs) {
      numGlyphIDs = str.length;
    }
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strBytes = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strBytes);
    stringToUTF8(str, strPtr, strBytes); // This includes the null terminator

    var bytesPerGlyph = 2;
    var glyphPtr = CanvasKit._malloc(numGlyphIDs * bytesPerGlyph);
    // We don't need to compute the id for the null terminator, so subtract 1.
    var actualIDs = this._getGlyphIDs(strPtr, strBytes - 1, numGlyphIDs, glyphPtr);
    CanvasKit._free(strPtr);
    if (actualIDs < 0) {
      Debug('Could not get glyphIDs');
      CanvasKit._free(glyphPtr);
      return null;
    }
    var glyphs = new Uint16Array(CanvasKit.HEAPU8.buffer, glyphPtr, actualIDs);
    if (optionalOutputArray) {
      optionalOutputArray.set(glyphs);
      CanvasKit._free(glyphPtr);
      return optionalOutputArray;
    }
    var rv = Uint32Array.from(glyphs);
    CanvasKit._free(glyphPtr);
    return rv;
  };

  // Glyphs should be a Uint32Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  // The return value will be a Float32Array that has one width per input glyph.
  CanvasKit.Font.prototype.getGlyphWidths = function(glyphs, paint, optionalOutputArray) {
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerWidth = 4;
    var widthPtr = CanvasKit._malloc(glyphs.length * bytesPerWidth);
    this._getGlyphWidthBounds(glyphPtr, glyphs.length, widthPtr, nullptr, paint || null);

    var widths = new Float32Array(CanvasKit.HEAPU8.buffer, widthPtr, glyphs.length);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (optionalOutputArray) {
      optionalOutputArray.set(widths);
      CanvasKit._free(widthPtr);
      return optionalOutputArray;
    }
    var rv = Float32Array.from(widths);
    CanvasKit._free(widthPtr);
    return rv;
  };

  // Returns an array of the widths of the glyphs in this string.
  // TODO(kjlubick) Remove this API - getGlyphWidths is the better API.
  CanvasKit.Font.prototype.getWidths = function(str) {
    // add 1 for null terminator
    var codePoints = str.length + 1;
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strBytes = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strBytes);
    stringToUTF8(str, strPtr, strBytes);

    var bytesPerFloat = 4;
    // allocate widths == numCodePoints
    var widthPtr = CanvasKit._malloc(codePoints * bytesPerFloat);
    if (!this._getWidths(strPtr, strBytes, codePoints, widthPtr)) {
      Debug('Could not compute widths');
      CanvasKit._free(strPtr);
      CanvasKit._free(widthPtr);
      return null;
    }
    // reminder, this shouldn't copy the data, just is a nice way to
    // wrap 4 bytes together into a float.
    var widths = new Float32Array(CanvasKit.HEAPU8.buffer, widthPtr, codePoints);
    // This copies the data so we can free the CanvasKit memory
    var retVal = Array.from(widths);
    CanvasKit._free(strPtr);
    CanvasKit._free(widthPtr);
    return retVal;
  };

  // arguments should all be arrayBuffers or be an array of arrayBuffers.
  CanvasKit.FontMgr.FromData = function() {
    if (!arguments.length) {
      Debug('Could not make FontMgr from no font sources');
      return null;
    }
    var fonts = arguments;
    if (fonts.length === 1 && Array.isArray(fonts[0])) {
      fonts = arguments[0];
    }
    if (!fonts.length) {
      Debug('Could not make FontMgr from no font sources');
      return null;
    }
    var dPtrs = [];
    var sizes = [];
    for (var i = 0; i < fonts.length; i++) {
      var data = new Uint8Array(fonts[i]);
      var dptr = copy1dArray(data, 'HEAPU8');
      dPtrs.push(dptr);
      sizes.push(data.byteLength);
    }
    // Pointers are 32 bit unsigned ints
    var datasPtr = copy1dArray(dPtrs, 'HEAPU32');
    var sizesPtr = copy1dArray(sizes, 'HEAPU32');
    var fm = CanvasKit.FontMgr._fromData(datasPtr, sizesPtr, fonts.length);
    // The FontMgr has taken ownership of the bytes we allocated in the for loop.
    CanvasKit._free(datasPtr);
    CanvasKit._free(sizesPtr);
    return fm;
  };

  // fontData should be an arrayBuffer
  CanvasKit.FontMgr.prototype.MakeTypefaceFromData = function(fontData) {
    var data = new Uint8Array(fontData);

    var fptr = copy1dArray(data, 'HEAPU8');
    var font = this._makeTypefaceFromData(fptr, data.byteLength);
    if (!font) {
      Debug('Could not decode font data');
      // We do not need to free the data since the C++ will do that for us
      // when the font is deleted (or fails to decode);
      return null;
    }
    return font;
  };

  // Clients can pass in a Float32Array with length 4 to this and the results
  // will be copied into that array. Otherwise, a new TypedArray will be allocated
  // and returned.
  CanvasKit.ShapedText.prototype.getBounds = function(optionalOutputArray) {
    this._getBounds(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optionalOutputArray) {
      optionalOutputArray.set(ta);
      return optionalOutputArray;
    }
    return ta.slice();
  };

  CanvasKit.TextBlob.MakeOnPath = function(str, path, font, initialOffset) {
    if (!str || !str.length) {
      Debug('ignoring 0 length string');
      return;
    }
    if (!path || !path.countPoints()) {
      Debug('ignoring empty path');
      return;
    }
    if (path.countPoints() === 1) {
      Debug('path has 1 point, returning normal textblob');
      return this.MakeFromText(str, font);
    }

    if (!initialOffset) {
      initialOffset = 0;
    }

    var widths = font.getWidths(str);

    var rsx = new CanvasKit.RSXFormBuilder();
    var meas = new CanvasKit.ContourMeasureIter(path, false, 1);
    var cont = meas.next();
    var dist = initialOffset;
    var xycs = new Float32Array(4);
    for (var i = 0; i < str.length && cont; i++) {
      var width = widths[i];
      dist += width/2;
      if (dist > cont.length()) {
        // jump to next contour
        cont.delete();
        cont = meas.next();
        if (!cont) {
          // We have come to the end of the path - terminate the string
          // right here.
          str = str.substring(0, i);
          break;
        }
        dist = width/2;
      }

      // Gives us the (x, y) coordinates as well as the cos/sin of the tangent
      // line at that position.
      cont.getPosTan(dist, xycs);
      var cx = xycs[0];
      var cy = xycs[1];
      var cosT = xycs[2];
      var sinT = xycs[3];

      var adjustedX = cx - (width/2 * cosT);
      var adjustedY = cy - (width/2 * sinT);

      rsx.push(cosT, sinT, adjustedX, adjustedY);
      dist += width/2;
    }
    var retVal = this.MakeFromRSXform(str, rsx, font);
    rsx.delete();
    cont && cont.delete();
    meas.delete();
    return retVal;
  };

  CanvasKit.TextBlob.MakeFromRSXform = function(str, rsxBuilderOrArray, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);

    var rPtr = nullptr;
    if (rsxBuilderOrArray.build) {
      rPtr = rsxBuilderOrArray.build();
    } else {
      rPtr = copy1dArray(rsxBuilderOrArray, 'HEAPF32');
    }

    var blob = CanvasKit.TextBlob._MakeFromRSXform(strPtr, strLen - 1, rPtr, font);
    CanvasKit._free(strPtr);
    if (!blob) {
      Debug('Could not make textblob from string "' + str + '"');
      return null;
    }
    return blob;
  };

  // Glyphs should be a Uint32Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  CanvasKit.TextBlob.MakeFromRSXformGlyphs = function(glyphs, rsxBuilderOrArray, font) {
    // Currently on the C++ side, glyph ids are 16bit, but there is an effort to change that.
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerGlyph = 2;

    var rPtr = nullptr;
    if (rsxBuilderOrArray.build) {
      rPtr = rsxBuilderOrArray.build();
    } else {
      rPtr = copy1dArray(rsxBuilderOrArray, 'HEAPF32');
    }

    var blob = CanvasKit.TextBlob._MakeFromRSXformGlyphs(glyphPtr, glyphs.length * bytesPerGlyph, rPtr, font);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (!blob) {
      Debug('Could not make textblob from glyphs "' + glyphs + '"');
      return null;
    }
    return blob;
  };

  // Glyphs should be a Uint32Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  CanvasKit.TextBlob.MakeFromGlyphs = function(glyphs, font) {
    // Currently on the C++ side, glyph ids are 16bit, but there is an effort to change that.
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerGlyph = 2;
    var blob = CanvasKit.TextBlob._MakeFromGlyphs(glyphPtr, glyphs.length * bytesPerGlyph, font);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (!blob) {
      Debug('Could not make textblob from glyphs "' + glyphs + '"');
      return null;
    }
    return blob;
  };

  CanvasKit.TextBlob.MakeFromText = function(str, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);

    var blob = CanvasKit.TextBlob._MakeFromText(strPtr, strLen - 1, font);
    CanvasKit._free(strPtr);
    if (!blob) {
      Debug('Could not make textblob from string "' + str + '"');
      return null;
    }
    return blob;
  };

  // A helper to return the right type for GlyphIDs stored internally. When that changes, this
  // will also be changed, which will help avoid future breakages.
  CanvasKit.MallocGlyphIDs = function(numGlyphIDs) {
    return CanvasKit.Malloc(Uint16Array, numGlyphIDs);
  }
});

CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  // data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
  CanvasKit.MakePicture = function(data) {
    data = new Uint8Array(data);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    var pic = CanvasKit._MakePicture(iptr, data.byteLength);
    if (!pic) {
      Debug('Could not decode picture');
      return null;
    }
    return pic;
  }

  // The serialized format of an Picture (informally called an "skp"), is not something
  // that clients should ever rely on. The format may change at anytime and no promises
  // are made for backwards or forward compatibility.
  CanvasKit.Picture.prototype.saveAsFile = function(skpName) {
    var data = this.serialize();
    if (!data) {
      Debug('Could not serialize to skpicture.');
      return;
    }
    var bytes = CanvasKit.getDataBytes(data);
    saveBytesToFile(bytes, skpName);
    data.delete();
  }
});

CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  CanvasKit.RuntimeEffect.prototype.makeShader = function(floats, isOpaque, localMatrix) {
    // We don't need to free these floats because they will become owned by the shader.
    var fptr = copy1dArray(floats, "HEAPF32");
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    // Our array has 4 bytes per float, so be sure to account for that before
    // sending it over the wire.
    return this._makeShader(fptr, floats.length * 4, !!isOpaque, localMatrixPtr);
  }

  // childrenWithShaders is an array of other shaders (e.g. Image.makeShader())
  CanvasKit.RuntimeEffect.prototype.makeShaderWithChildren = function(floats, isOpaque, childrenShaders, localMatrix) {
    // We don't need to free these floats because they will become owned by the shader.
    var fptr = copy1dArray(floats, "HEAPF32");
    var localMatrixPtr = copy3x3MatrixToWasm(localMatrix);
    var barePointers = [];
    for (var i = 0; i < childrenShaders.length; i++) {
      // childrenShaders are emscriptens smart pointer type. We want to get the bare pointer
      // and send that over the wire, so it can be re-wrapped as an sk_sp.
      barePointers.push(childrenShaders[i].$$.ptr);
    }
    var childrenPointers = copy1dArray(barePointers, "HEAPU32");
    // Our array has 4 bytes per float, so be sure to account for that before
    // sending it over the wire.
    return this._makeShaderWithChildren(fptr, floats.length * 4, !!isOpaque, childrenPointers,
                                        barePointers.length, localMatrixPtr);
  }
});

// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which is called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function() {

  // This allows us to expose internal functions (e.g. color
  // parsing) for unit-testing, even in the minified version.
  // Our tests are not minified like CanvasKit is, so the names
  // would get lost otherwise.
  CanvasKit._testing = {};

// This intentionally dangles because we want all the htmlcanvas
// JS code to be in the same scope, but JS doesn't support
// namespaces like C++ does. Thus, we simply include this
// preamble.js file, all the source .js files and then postamble.js
// to bundle everything in the same scope.

// General purpose utility functions go in this file.


function allAreFinite(args) {
  for (var i = 0; i < args.length; i++) {
    if (args[i] !== undefined && !Number.isFinite(args[i])) {
      return false;
    }
  }
  return true;
}

function toBase64String(bytes) {
  if (isNode) {
    return Buffer.from(bytes).toString('base64');
  } else {
    // From https://stackoverflow.com/a/25644409
    // because the naive solution of
    //     btoa(String.fromCharCode.apply(null, bytes));
    // would occasionally throw "Maximum call stack size exceeded"
    var CHUNK_SIZE = 0x8000; //arbitrary number
    var index = 0;
    var length = bytes.length;
    var result = '';
    var slice;
    while (index < length) {
      slice = bytes.slice(index, Math.min(index + CHUNK_SIZE, length));
      result += String.fromCharCode.apply(null, slice);
      index += CHUNK_SIZE;
    }
    return btoa(result);
  }
}


// Functions dealing with parsing/stringifying color go here.

// Create the following with
// node ./htmlcanvas/_namedcolors.js --expose-wasm
// JS/closure doesn't have a constexpr like thing which
// would really help here. Since we don't, we pre-compute
// the map, which saves (a tiny amount of) startup time
// and (a small amount of) code size.
/* @dict */
var colorMap = {
  'aliceblue':            Float32Array.of(0.941, 0.973, 1.000, 1.000),
  'antiquewhite':         Float32Array.of(0.980, 0.922, 0.843, 1.000),
  'aqua':                 Float32Array.of(0.000, 1.000, 1.000, 1.000),
  'aquamarine':           Float32Array.of(0.498, 1.000, 0.831, 1.000),
  'azure':                Float32Array.of(0.941, 1.000, 1.000, 1.000),
  'beige':                Float32Array.of(0.961, 0.961, 0.863, 1.000),
  'bisque':               Float32Array.of(1.000, 0.894, 0.769, 1.000),
  'black':                Float32Array.of(0.000, 0.000, 0.000, 1.000),
  'blanchedalmond':       Float32Array.of(1.000, 0.922, 0.804, 1.000),
  'blue':                 Float32Array.of(0.000, 0.000, 1.000, 1.000),
  'blueviolet':           Float32Array.of(0.541, 0.169, 0.886, 1.000),
  'brown':                Float32Array.of(0.647, 0.165, 0.165, 1.000),
  'burlywood':            Float32Array.of(0.871, 0.722, 0.529, 1.000),
  'cadetblue':            Float32Array.of(0.373, 0.620, 0.627, 1.000),
  'chartreuse':           Float32Array.of(0.498, 1.000, 0.000, 1.000),
  'chocolate':            Float32Array.of(0.824, 0.412, 0.118, 1.000),
  'coral':                Float32Array.of(1.000, 0.498, 0.314, 1.000),
  'cornflowerblue':       Float32Array.of(0.392, 0.584, 0.929, 1.000),
  'cornsilk':             Float32Array.of(1.000, 0.973, 0.863, 1.000),
  'crimson':              Float32Array.of(0.863, 0.078, 0.235, 1.000),
  'cyan':                 Float32Array.of(0.000, 1.000, 1.000, 1.000),
  'darkblue':             Float32Array.of(0.000, 0.000, 0.545, 1.000),
  'darkcyan':             Float32Array.of(0.000, 0.545, 0.545, 1.000),
  'darkgoldenrod':        Float32Array.of(0.722, 0.525, 0.043, 1.000),
  'darkgray':             Float32Array.of(0.663, 0.663, 0.663, 1.000),
  'darkgreen':            Float32Array.of(0.000, 0.392, 0.000, 1.000),
  'darkgrey':             Float32Array.of(0.663, 0.663, 0.663, 1.000),
  'darkkhaki':            Float32Array.of(0.741, 0.718, 0.420, 1.000),
  'darkmagenta':          Float32Array.of(0.545, 0.000, 0.545, 1.000),
  'darkolivegreen':       Float32Array.of(0.333, 0.420, 0.184, 1.000),
  'darkorange':           Float32Array.of(1.000, 0.549, 0.000, 1.000),
  'darkorchid':           Float32Array.of(0.600, 0.196, 0.800, 1.000),
  'darkred':              Float32Array.of(0.545, 0.000, 0.000, 1.000),
  'darksalmon':           Float32Array.of(0.914, 0.588, 0.478, 1.000),
  'darkseagreen':         Float32Array.of(0.561, 0.737, 0.561, 1.000),
  'darkslateblue':        Float32Array.of(0.282, 0.239, 0.545, 1.000),
  'darkslategray':        Float32Array.of(0.184, 0.310, 0.310, 1.000),
  'darkslategrey':        Float32Array.of(0.184, 0.310, 0.310, 1.000),
  'darkturquoise':        Float32Array.of(0.000, 0.808, 0.820, 1.000),
  'darkviolet':           Float32Array.of(0.580, 0.000, 0.827, 1.000),
  'deeppink':             Float32Array.of(1.000, 0.078, 0.576, 1.000),
  'deepskyblue':          Float32Array.of(0.000, 0.749, 1.000, 1.000),
  'dimgray':              Float32Array.of(0.412, 0.412, 0.412, 1.000),
  'dimgrey':              Float32Array.of(0.412, 0.412, 0.412, 1.000),
  'dodgerblue':           Float32Array.of(0.118, 0.565, 1.000, 1.000),
  'firebrick':            Float32Array.of(0.698, 0.133, 0.133, 1.000),
  'floralwhite':          Float32Array.of(1.000, 0.980, 0.941, 1.000),
  'forestgreen':          Float32Array.of(0.133, 0.545, 0.133, 1.000),
  'fuchsia':              Float32Array.of(1.000, 0.000, 1.000, 1.000),
  'gainsboro':            Float32Array.of(0.863, 0.863, 0.863, 1.000),
  'ghostwhite':           Float32Array.of(0.973, 0.973, 1.000, 1.000),
  'gold':                 Float32Array.of(1.000, 0.843, 0.000, 1.000),
  'goldenrod':            Float32Array.of(0.855, 0.647, 0.125, 1.000),
  'gray':                 Float32Array.of(0.502, 0.502, 0.502, 1.000),
  'green':                Float32Array.of(0.000, 0.502, 0.000, 1.000),
  'greenyellow':          Float32Array.of(0.678, 1.000, 0.184, 1.000),
  'grey':                 Float32Array.of(0.502, 0.502, 0.502, 1.000),
  'honeydew':             Float32Array.of(0.941, 1.000, 0.941, 1.000),
  'hotpink':              Float32Array.of(1.000, 0.412, 0.706, 1.000),
  'indianred':            Float32Array.of(0.804, 0.361, 0.361, 1.000),
  'indigo':               Float32Array.of(0.294, 0.000, 0.510, 1.000),
  'ivory':                Float32Array.of(1.000, 1.000, 0.941, 1.000),
  'khaki':                Float32Array.of(0.941, 0.902, 0.549, 1.000),
  'lavender':             Float32Array.of(0.902, 0.902, 0.980, 1.000),
  'lavenderblush':        Float32Array.of(1.000, 0.941, 0.961, 1.000),
  'lawngreen':            Float32Array.of(0.486, 0.988, 0.000, 1.000),
  'lemonchiffon':         Float32Array.of(1.000, 0.980, 0.804, 1.000),
  'lightblue':            Float32Array.of(0.678, 0.847, 0.902, 1.000),
  'lightcoral':           Float32Array.of(0.941, 0.502, 0.502, 1.000),
  'lightcyan':            Float32Array.of(0.878, 1.000, 1.000, 1.000),
  'lightgoldenrodyellow': Float32Array.of(0.980, 0.980, 0.824, 1.000),
  'lightgray':            Float32Array.of(0.827, 0.827, 0.827, 1.000),
  'lightgreen':           Float32Array.of(0.565, 0.933, 0.565, 1.000),
  'lightgrey':            Float32Array.of(0.827, 0.827, 0.827, 1.000),
  'lightpink':            Float32Array.of(1.000, 0.714, 0.757, 1.000),
  'lightsalmon':          Float32Array.of(1.000, 0.627, 0.478, 1.000),
  'lightseagreen':        Float32Array.of(0.125, 0.698, 0.667, 1.000),
  'lightskyblue':         Float32Array.of(0.529, 0.808, 0.980, 1.000),
  'lightslategray':       Float32Array.of(0.467, 0.533, 0.600, 1.000),
  'lightslategrey':       Float32Array.of(0.467, 0.533, 0.600, 1.000),
  'lightsteelblue':       Float32Array.of(0.690, 0.769, 0.871, 1.000),
  'lightyellow':          Float32Array.of(1.000, 1.000, 0.878, 1.000),
  'lime':                 Float32Array.of(0.000, 1.000, 0.000, 1.000),
  'limegreen':            Float32Array.of(0.196, 0.804, 0.196, 1.000),
  'linen':                Float32Array.of(0.980, 0.941, 0.902, 1.000),
  'magenta':              Float32Array.of(1.000, 0.000, 1.000, 1.000),
  'maroon':               Float32Array.of(0.502, 0.000, 0.000, 1.000),
  'mediumaquamarine':     Float32Array.of(0.400, 0.804, 0.667, 1.000),
  'mediumblue':           Float32Array.of(0.000, 0.000, 0.804, 1.000),
  'mediumorchid':         Float32Array.of(0.729, 0.333, 0.827, 1.000),
  'mediumpurple':         Float32Array.of(0.576, 0.439, 0.859, 1.000),
  'mediumseagreen':       Float32Array.of(0.235, 0.702, 0.443, 1.000),
  'mediumslateblue':      Float32Array.of(0.482, 0.408, 0.933, 1.000),
  'mediumspringgreen':    Float32Array.of(0.000, 0.980, 0.604, 1.000),
  'mediumturquoise':      Float32Array.of(0.282, 0.820, 0.800, 1.000),
  'mediumvioletred':      Float32Array.of(0.780, 0.082, 0.522, 1.000),
  'midnightblue':         Float32Array.of(0.098, 0.098, 0.439, 1.000),
  'mintcream':            Float32Array.of(0.961, 1.000, 0.980, 1.000),
  'mistyrose':            Float32Array.of(1.000, 0.894, 0.882, 1.000),
  'moccasin':             Float32Array.of(1.000, 0.894, 0.710, 1.000),
  'navajowhite':          Float32Array.of(1.000, 0.871, 0.678, 1.000),
  'navy':                 Float32Array.of(0.000, 0.000, 0.502, 1.000),
  'oldlace':              Float32Array.of(0.992, 0.961, 0.902, 1.000),
  'olive':                Float32Array.of(0.502, 0.502, 0.000, 1.000),
  'olivedrab':            Float32Array.of(0.420, 0.557, 0.137, 1.000),
  'orange':               Float32Array.of(1.000, 0.647, 0.000, 1.000),
  'orangered':            Float32Array.of(1.000, 0.271, 0.000, 1.000),
  'orchid':               Float32Array.of(0.855, 0.439, 0.839, 1.000),
  'palegoldenrod':        Float32Array.of(0.933, 0.910, 0.667, 1.000),
  'palegreen':            Float32Array.of(0.596, 0.984, 0.596, 1.000),
  'paleturquoise':        Float32Array.of(0.686, 0.933, 0.933, 1.000),
  'palevioletred':        Float32Array.of(0.859, 0.439, 0.576, 1.000),
  'papayawhip':           Float32Array.of(1.000, 0.937, 0.835, 1.000),
  'peachpuff':            Float32Array.of(1.000, 0.855, 0.725, 1.000),
  'peru':                 Float32Array.of(0.804, 0.522, 0.247, 1.000),
  'pink':                 Float32Array.of(1.000, 0.753, 0.796, 1.000),
  'plum':                 Float32Array.of(0.867, 0.627, 0.867, 1.000),
  'powderblue':           Float32Array.of(0.690, 0.878, 0.902, 1.000),
  'purple':               Float32Array.of(0.502, 0.000, 0.502, 1.000),
  'rebeccapurple':        Float32Array.of(0.400, 0.200, 0.600, 1.000),
  'red':                  Float32Array.of(1.000, 0.000, 0.000, 1.000),
  'rosybrown':            Float32Array.of(0.737, 0.561, 0.561, 1.000),
  'royalblue':            Float32Array.of(0.255, 0.412, 0.882, 1.000),
  'saddlebrown':          Float32Array.of(0.545, 0.271, 0.075, 1.000),
  'salmon':               Float32Array.of(0.980, 0.502, 0.447, 1.000),
  'sandybrown':           Float32Array.of(0.957, 0.643, 0.376, 1.000),
  'seagreen':             Float32Array.of(0.180, 0.545, 0.341, 1.000),
  'seashell':             Float32Array.of(1.000, 0.961, 0.933, 1.000),
  'sienna':               Float32Array.of(0.627, 0.322, 0.176, 1.000),
  'silver':               Float32Array.of(0.753, 0.753, 0.753, 1.000),
  'skyblue':              Float32Array.of(0.529, 0.808, 0.922, 1.000),
  'slateblue':            Float32Array.of(0.416, 0.353, 0.804, 1.000),
  'slategray':            Float32Array.of(0.439, 0.502, 0.565, 1.000),
  'slategrey':            Float32Array.of(0.439, 0.502, 0.565, 1.000),
  'snow':                 Float32Array.of(1.000, 0.980, 0.980, 1.000),
  'springgreen':          Float32Array.of(0.000, 1.000, 0.498, 1.000),
  'steelblue':            Float32Array.of(0.275, 0.510, 0.706, 1.000),
  'tan':                  Float32Array.of(0.824, 0.706, 0.549, 1.000),
  'teal':                 Float32Array.of(0.000, 0.502, 0.502, 1.000),
  'thistle':              Float32Array.of(0.847, 0.749, 0.847, 1.000),
  'tomato':               Float32Array.of(1.000, 0.388, 0.278, 1.000),
  'transparent':          Float32Array.of(0.000, 0.000, 0.000, 0.000),
  'turquoise':            Float32Array.of(0.251, 0.878, 0.816, 1.000),
  'violet':               Float32Array.of(0.933, 0.510, 0.933, 1.000),
  'wheat':                Float32Array.of(0.961, 0.871, 0.702, 1.000),
  'white':                Float32Array.of(1.000, 1.000, 1.000, 1.000),
  'whitesmoke':           Float32Array.of(0.961, 0.961, 0.961, 1.000),
  'yellow':               Float32Array.of(1.000, 1.000, 0.000, 1.000),
  'yellowgreen':          Float32Array.of(0.604, 0.804, 0.196, 1.000),
}

function colorToString(skcolor) {
  // https://html.spec.whatwg.org/multipage/canvas.html#serialisation-of-a-color
  var components = CanvasKit.getColorComponents(skcolor);
  var r = components[0];
  var g = components[1];
  var b = components[2];
  var a = components[3];
  if (a === 1.0) {
    // hex
    r = r.toString(16).toLowerCase();
    g = g.toString(16).toLowerCase();
    b = b.toString(16).toLowerCase();
    r = (r.length === 1 ? '0'+r: r);
    g = (g.length === 1 ? '0'+g: g);
    b = (b.length === 1 ? '0'+b: b);
    return '#'+r+g+b;
  } else {
    a = (a === 0 || a === 1) ? a : a.toFixed(8);
    return 'rgba('+r+', '+g+', '+b+', '+a+')';
  }
}

function parseColor(colorStr) {
  return CanvasKit.parseColorString(colorStr, colorMap);
}

CanvasKit._testing['parseColor'] = parseColor;
CanvasKit._testing['colorToString'] = colorToString;

// Functions dealing with parsing/stringifying fonts go here.
var fontStringRegex = new RegExp(
  '(italic|oblique|normal|)\\s*' +              // style
  '(small-caps|normal|)\\s*' +                  // variant
  '(bold|bolder|lighter|[1-9]00|normal|)\\s*' + // weight
  '([\\d\\.]+)' +                               // size
  '(px|pt|pc|in|cm|mm|%|em|ex|ch|rem|q)' +      // unit
  // line-height is ignored here, as per the spec
  '(.+)'                                        // family
  );

function stripWhitespace(str) {
  return str.replace(/^\s+|\s+$/, '');
}

var defaultHeight = 16;
// Based off of node-canvas's parseFont
// returns font size in px, which represents the em width.
function parseFontString(fontStr) {

  var font = fontStringRegex.exec(fontStr);
  if (!font) {
    Debug('Invalid font string ' + fontStr);
    return null;
  }

  var size = parseFloat(font[4]);
  var sizePx = defaultHeight;
  var unit = font[5];
  switch (unit) {
    case 'em':
    case 'rem':
      sizePx = size * defaultHeight;
      break;
    case 'pt':
      sizePx = size * 4/3;
      break;
    case 'px':
      sizePx = size;
      break;
    case 'pc':
      sizePx = size * defaultHeight;
      break;
    case 'in':
      sizePx = size * 96;
      break;
    case 'cm':
      sizePx = size * 96.0 / 2.54;
      break;
    case 'mm':
      sizePx = size * (96.0 / 25.4);
      break;
    case 'q': // quarter millimeters
      sizePx = size * (96.0 / 25.4 / 4);
      break;
    case '%':
      sizePx = size * (defaultHeight / 75);
      break;
  }
  return {
    'style':   font[1],
    'variant': font[2],
    'weight':  font[3],
    'sizePx':  sizePx,
    'family':  font[6].trim()
  };
}

function getTypeface(fontstr) {
  var descriptors = parseFontString(fontstr);
  var typeface = getFromFontCache(descriptors);
  descriptors['typeface'] = typeface;
  return descriptors;
}

// null means use the default typeface (which is currently NotoMono)
var fontCache = {
  'Noto Mono': {
    '*': null, // is used if we have this font family, but not the right style/variant/weight
  },
  'monospace': {
    '*': null,
  }
};

// descriptors is like https://developer.mozilla.org/en-US/docs/Web/API/FontFace/FontFace
// The ones currently supported are family, style, variant, weight.
function addToFontCache(typeface, descriptors) {
  var key = (descriptors['style']   || 'normal') + '|' +
            (descriptors['variant'] || 'normal') + '|' +
            (descriptors['weight']  || 'normal');
  var fam = descriptors['family'];
  if (!fontCache[fam]) {
    // preload with a fallback to this typeface
    fontCache[fam] = {
      '*': typeface,
    };
  }
  fontCache[fam][key] = typeface;
}

function getFromFontCache(descriptors) {
  var key = (descriptors['style']   || 'normal') + '|' +
            (descriptors['variant'] || 'normal') + '|' +
            (descriptors['weight']  || 'normal');
  var fam = descriptors['family'];
  if (!fontCache[fam]) {
    return null;
  }
  return fontCache[fam][key] || fontCache[fam]['*'];
}

CanvasKit._testing['parseFontString'] = parseFontString;

function CanvasRenderingContext2D(skcanvas) {
  this._canvas = skcanvas;
  this._paint = new CanvasKit.Paint();
  this._paint.setAntiAlias(true);

  this._paint.setStrokeMiter(10);
  this._paint.setStrokeCap(CanvasKit.StrokeCap.Butt);
  this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Miter);
  this._fontString = '10px monospace';

  this._font = new CanvasKit.Font(null, 10);
  this._font.setSubpixel(true);

  this._strokeStyle    = CanvasKit.BLACK;
  this._fillStyle      = CanvasKit.BLACK;
  this._shadowBlur     = 0;
  this._shadowColor    = CanvasKit.TRANSPARENT;
  this._shadowOffsetX  = 0;
  this._shadowOffsetY  = 0;
  this._globalAlpha    = 1;
  this._strokeWidth    = 1;
  this._lineDashOffset = 0;
  this._lineDashList   = [];
  // aka BlendMode
  this._globalCompositeOperation = CanvasKit.BlendMode.SrcOver;

  this._paint.setStrokeWidth(this._strokeWidth);
  this._paint.setBlendMode(this._globalCompositeOperation);

  this._currentPath = new CanvasKit.Path();
  this._currentTransform = CanvasKit.Matrix.identity();

  // Use this for save/restore
  this._canvasStateStack = [];
  // Keep a reference to all the effects (e.g. gradients, patterns)
  // that were allocated for cleanup in _dispose.
  this._toCleanUp = [];

  this._dispose = function() {
    this._currentPath.delete();
    this._paint.delete();
    this._font.delete();
    this._toCleanUp.forEach(function(c) {
      c._dispose();
    });
    // Don't delete this._canvas as it will be disposed
    // by the surface of which it is based.
  };

  // This always accepts DOMMatrix/SVGMatrix or any other
  // object that has properties a,b,c,d,e,f defined.
  // Returns a DOM-Matrix like dictionary
  Object.defineProperty(this, 'currentTransform', {
    enumerable: true,
    get: function() {
      return {
        'a' : this._currentTransform[0],
        'c' : this._currentTransform[1],
        'e' : this._currentTransform[2],
        'b' : this._currentTransform[3],
        'd' : this._currentTransform[4],
        'f' : this._currentTransform[5],
      };
    },
    // @param {DOMMatrix} matrix
    set: function(matrix) {
      if (matrix.a) {
        // if we see a property named 'a', guess that b-f will
        // also be there.
        this.setTransform(matrix.a, matrix.b, matrix.c,
                          matrix.d, matrix.e, matrix.f);
      }
    }
  });

  Object.defineProperty(this, 'fillStyle', {
    enumerable: true,
    get: function() {
      if (isCanvasKitColor(this._fillStyle)) {
        return colorToString(this._fillStyle);
      }
      return this._fillStyle;
    },
    set: function(newStyle) {
      if (typeof newStyle === 'string') {
        this._fillStyle = parseColor(newStyle);
      } else if (newStyle._getShader) {
        // It's an effect that has a shader.
        this._fillStyle = newStyle
      }
    }
  });

  Object.defineProperty(this, 'font', {
    enumerable: true,
    get: function() {
      return this._fontString;
    },
    set: function(newFont) {
      var tf = getTypeface(newFont);
      if (tf) {
        // tf is a "dict" according to closure, that is, the field
        // names are not minified. Thus, we need to access it via
        // bracket notation to tell closure not to minify these names.
        this._font.setSize(tf['sizePx']);
        this._font.setTypeface(tf['typeface']);
        this._fontString = newFont;
      }
    }
  });

  Object.defineProperty(this, 'globalAlpha', {
    enumerable: true,
    get: function() {
      return this._globalAlpha;
    },
    set: function(newAlpha) {
      // ignore invalid values, as per the spec
      if (!isFinite(newAlpha) || newAlpha < 0 || newAlpha > 1) {
        return;
      }
      this._globalAlpha = newAlpha;
    }
  });

  Object.defineProperty(this, 'globalCompositeOperation', {
    enumerable: true,
    get: function() {
      switch (this._globalCompositeOperation) {
        // composite-mode
        case CanvasKit.BlendMode.SrcOver:
          return 'source-over';
        case CanvasKit.BlendMode.DstOver:
          return 'destination-over';
        case CanvasKit.BlendMode.Src:
          return 'copy';
        case CanvasKit.BlendMode.Dst:
          return 'destination';
        case CanvasKit.BlendMode.Clear:
          return 'clear';
        case CanvasKit.BlendMode.SrcIn:
          return 'source-in';
        case CanvasKit.BlendMode.DstIn:
          return 'destination-in';
        case CanvasKit.BlendMode.SrcOut:
          return 'source-out';
        case CanvasKit.BlendMode.DstOut:
          return 'destination-out';
        case CanvasKit.BlendMode.SrcATop:
          return 'source-atop';
        case CanvasKit.BlendMode.DstATop:
          return 'destination-atop';
        case CanvasKit.BlendMode.Xor:
          return 'xor';
        case CanvasKit.BlendMode.Plus:
          return 'lighter';

        case CanvasKit.BlendMode.Multiply:
          return 'multiply';
        case CanvasKit.BlendMode.Screen:
          return 'screen';
        case CanvasKit.BlendMode.Overlay:
          return 'overlay';
        case CanvasKit.BlendMode.Darken:
          return 'darken';
        case CanvasKit.BlendMode.Lighten:
          return 'lighten';
        case CanvasKit.BlendMode.ColorDodge:
          return 'color-dodge';
        case CanvasKit.BlendMode.ColorBurn:
          return 'color-burn';
        case CanvasKit.BlendMode.HardLight:
          return 'hard-light';
        case CanvasKit.BlendMode.SoftLight:
          return 'soft-light';
        case CanvasKit.BlendMode.Difference:
          return 'difference';
        case CanvasKit.BlendMode.Exclusion:
          return 'exclusion';
        case CanvasKit.BlendMode.Hue:
          return 'hue';
        case CanvasKit.BlendMode.Saturation:
          return 'saturation';
        case CanvasKit.BlendMode.Color:
          return 'color';
        case CanvasKit.BlendMode.Luminosity:
          return 'luminosity';
      }
    },
    set: function(newMode) {
      switch (newMode) {
        // composite-mode
        case 'source-over':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcOver;
          break;
        case 'destination-over':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstOver;
          break;
        case 'copy':
          this._globalCompositeOperation = CanvasKit.BlendMode.Src;
          break;
        case 'destination':
          this._globalCompositeOperation = CanvasKit.BlendMode.Dst;
          break;
        case 'clear':
          this._globalCompositeOperation = CanvasKit.BlendMode.Clear;
          break;
        case 'source-in':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcIn;
          break;
        case 'destination-in':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstIn;
          break;
        case 'source-out':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcOut;
          break;
        case 'destination-out':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstOut;
          break;
        case 'source-atop':
          this._globalCompositeOperation = CanvasKit.BlendMode.SrcATop;
          break;
        case 'destination-atop':
          this._globalCompositeOperation = CanvasKit.BlendMode.DstATop;
          break;
        case 'xor':
          this._globalCompositeOperation = CanvasKit.BlendMode.Xor;
          break;
        case 'lighter':
          this._globalCompositeOperation = CanvasKit.BlendMode.Plus;
          break;
        case 'plus-lighter':
          this._globalCompositeOperation = CanvasKit.BlendMode.Plus;
          break;
        case 'plus-darker':
          throw 'plus-darker is not supported';

        // blend-mode
        case 'multiply':
          this._globalCompositeOperation = CanvasKit.BlendMode.Multiply;
          break;
        case 'screen':
          this._globalCompositeOperation = CanvasKit.BlendMode.Screen;
          break;
        case 'overlay':
          this._globalCompositeOperation = CanvasKit.BlendMode.Overlay;
          break;
        case 'darken':
          this._globalCompositeOperation = CanvasKit.BlendMode.Darken;
          break;
        case 'lighten':
          this._globalCompositeOperation = CanvasKit.BlendMode.Lighten;
          break;
        case 'color-dodge':
          this._globalCompositeOperation = CanvasKit.BlendMode.ColorDodge;
          break;
        case 'color-burn':
          this._globalCompositeOperation = CanvasKit.BlendMode.ColorBurn;
          break;
        case 'hard-light':
          this._globalCompositeOperation = CanvasKit.BlendMode.HardLight;
          break;
        case 'soft-light':
          this._globalCompositeOperation = CanvasKit.BlendMode.SoftLight;
          break;
        case 'difference':
          this._globalCompositeOperation = CanvasKit.BlendMode.Difference;
          break;
        case 'exclusion':
          this._globalCompositeOperation = CanvasKit.BlendMode.Exclusion;
          break;
        case 'hue':
          this._globalCompositeOperation = CanvasKit.BlendMode.Hue;
          break;
        case 'saturation':
          this._globalCompositeOperation = CanvasKit.BlendMode.Saturation;
          break;
        case 'color':
          this._globalCompositeOperation = CanvasKit.BlendMode.Color;
          break;
        case 'luminosity':
          this._globalCompositeOperation = CanvasKit.BlendMode.Luminosity;
          break;
        default:
          return;
      }
      this._paint.setBlendMode(this._globalCompositeOperation);
    }
  });

  Object.defineProperty(this, 'imageSmoothingEnabled', {
    enumerable: true,
    get: function() {
      return true;
    },
    set: function(a) {
      // ignored, we always use high quality image smoothing.
    }
  });

  Object.defineProperty(this, 'imageSmoothingQuality', {
    enumerable: true,
    get: function() {
          return 'high';
    },
    set: function(a) {
      // ignored, we always use high quality image smoothing.
    }
  });

  Object.defineProperty(this, 'lineCap', {
    enumerable: true,
    get: function() {
      switch (this._paint.getStrokeCap()) {
        case CanvasKit.StrokeCap.Butt:
          return 'butt';
        case CanvasKit.StrokeCap.Round:
          return 'round';
        case CanvasKit.StrokeCap.Square:
          return 'square';
      }
    },
    set: function(newCap) {
      switch (newCap) {
        case 'butt':
          this._paint.setStrokeCap(CanvasKit.StrokeCap.Butt);
          return;
        case 'round':
          this._paint.setStrokeCap(CanvasKit.StrokeCap.Round);
          return;
        case 'square':
          this._paint.setStrokeCap(CanvasKit.StrokeCap.Square);
          return;
      }
    }
  });

  Object.defineProperty(this, 'lineDashOffset', {
    enumerable: true,
    get: function() {
      return this._lineDashOffset;
    },
    set: function(newOffset) {
      if (!isFinite(newOffset)) {
        return;
      }
      this._lineDashOffset = newOffset;
    }
  });

  Object.defineProperty(this, 'lineJoin', {
    enumerable: true,
    get: function() {
      switch (this._paint.getStrokeJoin()) {
        case CanvasKit.StrokeJoin.Miter:
          return 'miter';
        case CanvasKit.StrokeJoin.Round:
          return 'round';
        case CanvasKit.StrokeJoin.Bevel:
          return 'bevel';
      }
    },
    set: function(newJoin) {
      switch (newJoin) {
        case 'miter':
          this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Miter);
          return;
        case 'round':
          this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Round);
          return;
        case 'bevel':
          this._paint.setStrokeJoin(CanvasKit.StrokeJoin.Bevel);
          return;
      }
    }
  });

  Object.defineProperty(this, 'lineWidth', {
    enumerable: true,
    get: function() {
      return this._paint.getStrokeWidth();
    },
    set: function(newWidth) {
      if (newWidth <= 0 || !newWidth) {
        // Spec says to ignore NaN/Inf/0/negative values
        return;
      }
      this._strokeWidth = newWidth;
      this._paint.setStrokeWidth(newWidth);
    }
  });

  Object.defineProperty(this, 'miterLimit', {
    enumerable: true,
    get: function() {
      return this._paint.getStrokeMiter();
    },
    set: function(newLimit) {
      if (newLimit <= 0 || !newLimit) {
        // Spec says to ignore NaN/Inf/0/negative values
        return;
      }
      this._paint.setStrokeMiter(newLimit);
    }
  });

  Object.defineProperty(this, 'shadowBlur', {
    enumerable: true,
    get: function() {
      return this._shadowBlur;
    },
    set: function(newBlur) {
      // ignore negative, inf and NAN (but not 0) as per the spec.
      if (newBlur < 0 || !isFinite(newBlur)) {
        return;
      }
      this._shadowBlur = newBlur;
    }
  });

  Object.defineProperty(this, 'shadowColor', {
    enumerable: true,
    get: function() {
      return colorToString(this._shadowColor);
    },
    set: function(newColor) {
      this._shadowColor = parseColor(newColor);
    }
  });

  Object.defineProperty(this, 'shadowOffsetX', {
    enumerable: true,
    get: function() {
      return this._shadowOffsetX;
    },
    set: function(newOffset) {
      if (!isFinite(newOffset)) {
        return;
      }
      this._shadowOffsetX = newOffset;
    }
  });

  Object.defineProperty(this, 'shadowOffsetY', {
    enumerable: true,
    get: function() {
      return this._shadowOffsetY;
    },
    set: function(newOffset) {
      if (!isFinite(newOffset)) {
        return;
      }
      this._shadowOffsetY = newOffset;
    }
  });

  Object.defineProperty(this, 'strokeStyle', {
    enumerable: true,
    get: function() {
      return colorToString(this._strokeStyle);
    },
    set: function(newStyle) {
      if (typeof newStyle === 'string') {
        this._strokeStyle = parseColor(newStyle);
      } else if (newStyle._getShader) {
        // It's probably an effect.
        this._strokeStyle = newStyle
      }
    }
  });

  this.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    arc(this._currentPath, x, y, radius, startAngle, endAngle, ccw);
  };

  this.arcTo = function(x1, y1, x2, y2, radius) {
    arcTo(this._currentPath, x1, y1, x2, y2, radius);
  };

  // As per the spec this doesn't begin any paths, it only
  // clears out any previous paths.
  this.beginPath = function() {
    this._currentPath.delete();
    this._currentPath = new CanvasKit.Path();
  };

  this.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    bezierCurveTo(this._currentPath, cp1x, cp1y, cp2x, cp2y, x, y);
  };

  this.clearRect = function(x, y, width, height) {
    this._paint.setStyle(CanvasKit.PaintStyle.Fill);
    this._paint.setBlendMode(CanvasKit.BlendMode.Clear);
    this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), this._paint);
    this._paint.setBlendMode(this._globalCompositeOperation);
  };

  this.clip = function(path, fillRule) {
    if (typeof path === 'string') {
      // shift the args if a Path2D is supplied
      fillRule = path;
      path = this._currentPath;
    } else if (path && path._getPath) {
      path = path._getPath();
    }
    if (!path) {
      path = this._currentPath;
    }

    var clip = path.copy();
    if (fillRule && fillRule.toLowerCase() === 'evenodd') {
      clip.setFillType(CanvasKit.FillType.EvenOdd);
    } else {
      clip.setFillType(CanvasKit.FillType.Winding);
    }
    this._canvas.clipPath(clip, CanvasKit.ClipOp.Intersect, true);
    clip.delete();
  };

  this.closePath = function() {
    closePath(this._currentPath);
  };

  this.createImageData = function() {
    // either takes in 1 or 2 arguments:
    //  - imagedata on which to copy *width* and *height* only
    //  - width, height
    if (arguments.length === 1) {
      var oldData = arguments[0];
      var byteLength = 4 * oldData.width * oldData.height;
      return new ImageData(new Uint8ClampedArray(byteLength),
                           oldData.width, oldData.height);
    } else if (arguments.length === 2) {
      var width = arguments[0];
      var height = arguments[1];
      var byteLength = 4 * width * height;
      return new ImageData(new Uint8ClampedArray(byteLength),
                           width, height);
    } else {
      throw 'createImageData expects 1 or 2 arguments, got '+arguments.length;
    }
  };

  this.createLinearGradient = function(x1, y1, x2, y2) {
    if (!allAreFinite(arguments)) {
      return;
    }
    var lcg = new LinearCanvasGradient(x1, y1, x2, y2);
    this._toCleanUp.push(lcg);
    return lcg;
  };

  this.createPattern = function(image, repetition) {
    var cp = new CanvasPattern(image, repetition);
    this._toCleanUp.push(cp);
    return cp;
  };

  this.createRadialGradient = function(x1, y1, r1, x2, y2, r2) {
    if (!allAreFinite(arguments)) {
      return;
    }
    var rcg = new RadialCanvasGradient(x1, y1, r1, x2, y2, r2);
    this._toCleanUp.push(rcg);
    return rcg;
  };

  this.drawImage = function(img) {
    // 3 potential sets of arguments
    // - image, dx, dy
    // - image, dx, dy, dWidth, dHeight
    // - image, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight
    // use the fillPaint, which has the globalAlpha in it
    // which drawImageRect will use.
    var iPaint = this._fillPaint();
    if (arguments.length === 3 || arguments.length === 5) {
      var destRect = CanvasKit.XYWHRect(arguments[1], arguments[2],
                        arguments[3] || img.width(), arguments[4] || img.height());
      var srcRect = CanvasKit.XYWHRect(0, 0, img.width(), img.height());
    } else if (arguments.length === 9){
      var destRect = CanvasKit.XYWHRect(arguments[5], arguments[6],
                                        arguments[7], arguments[8]);
      var srcRect = CanvasKit.XYWHRect(arguments[1], arguments[2],
                                       arguments[3], arguments[4]);
    } else {
      throw 'invalid number of args for drawImage, need 3, 5, or 9; got '+ arguments.length;
    }
    this._canvas.drawImageRect(img, srcRect, destRect, iPaint, false);

    iPaint.dispose();
  };

  this.ellipse = function(x, y, radiusX, radiusY, rotation,
                          startAngle, endAngle, ccw) {
    ellipse(this._currentPath, x, y, radiusX, radiusY, rotation,
            startAngle, endAngle, ccw);
  };

  // A helper to copy the current paint, ready for filling
  // This applies the global alpha.
  // Call dispose() after to clean up.
  this._fillPaint = function() {
    var paint = this._paint.copy();
    paint.setStyle(CanvasKit.PaintStyle.Fill);
    if (isCanvasKitColor(this._fillStyle)) {
      var alphaColor = CanvasKit.multiplyByAlpha(this._fillStyle, this._globalAlpha);
      paint.setColor(alphaColor);
    } else {
      var shader = this._fillStyle._getShader(this._currentTransform);
      paint.setColor(CanvasKit.Color(0,0,0, this._globalAlpha));
      paint.setShader(shader);
    }

    paint.dispose = function() {
      // If there are some helper effects in the future, clean them up
      // here. In any case, we have .dispose() to make _fillPaint behave
      // like _strokePaint and _shadowPaint.
      this.delete();
    };
    return paint;
  };

  this.fill = function(path, fillRule) {
    if (typeof path === 'string') {
      // shift the args if a Path2D is supplied
      fillRule = path;
      path = this._currentPath;
    } else if (path && path._getPath) {
      path = path._getPath();
    }
    if (fillRule === 'evenodd') {
      this._currentPath.setFillType(CanvasKit.FillType.EvenOdd);
    } else if (fillRule === 'nonzero' || !fillRule) {
      this._currentPath.setFillType(CanvasKit.FillType.Winding);
    } else {
      throw 'invalid fill rule';
    }
    if (!path) {
      path = this._currentPath;
    }

    var fillPaint = this._fillPaint();

    var shadowPaint = this._shadowPaint(fillPaint);
    if (shadowPaint) {
      this._canvas.save();
      this._applyShadowOffsetMatrix();
      this._canvas.drawPath(path, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }
    this._canvas.drawPath(path, fillPaint);
    fillPaint.dispose();
  };

  this.fillRect = function(x, y, width, height) {
    var fillPaint = this._fillPaint();

    var shadowPaint = this._shadowPaint(fillPaint);
    if (shadowPaint) {
      this._canvas.save();
      this._applyShadowOffsetMatrix();
      this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }

    this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), fillPaint);
    fillPaint.dispose();
  };

  this.fillText = function(text, x, y, maxWidth) {
    // TODO do something with maxWidth, probably involving measure
    var fillPaint = this._fillPaint();
    var blob = CanvasKit.TextBlob.MakeFromText(text, this._font);

    var shadowPaint = this._shadowPaint(fillPaint);
    if (shadowPaint) {
      this._canvas.save();
      this._applyShadowOffsetMatrix();
      this._canvas.drawTextBlob(blob, x, y, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }
    this._canvas.drawTextBlob(blob, x, y, fillPaint);
    blob.delete();
    fillPaint.dispose();
  };

  this.getImageData = function(x, y, w, h) {
    var pixels = this._canvas.readPixels(x, y, {
        'width': w,
        'height': h,
        'colorType': CanvasKit.ColorType.RGBA_8888,
        'alphaType': CanvasKit.AlphaType.Unpremul,
        'colorSpace': CanvasKit.ColorSpace.SRGB,
    });
    if (!pixels) {
      return null;
    }
    // This essentially re-wraps the pixels from a Uint8Array to
    // a Uint8ClampedArray (without making a copy of pixels).
    return new ImageData(
      new Uint8ClampedArray(pixels.buffer),
      w, h);
  };

  this.getLineDash = function() {
    return this._lineDashList.slice();
  };

  this._mapToLocalCoordinates = function(pts) {
    var inverted = CanvasKit.Matrix.invert(this._currentTransform);
    CanvasKit.Matrix.mapPoints(inverted, pts);
    return pts;
  };

  this.isPointInPath = function(x, y, fillmode) {
    var args = arguments;
    if (args.length === 3) {
      var path = this._currentPath;
    } else if (args.length === 4) {
      var path = args[0];
      x = args[1];
      y = args[2];
      fillmode = args[3];
    } else {
      throw 'invalid arg count, need 3 or 4, got ' + args.length;
    }
    if (!isFinite(x) || !isFinite(y)) {
      return false;
    }
    fillmode = fillmode || 'nonzero';
    if (!(fillmode === 'nonzero' || fillmode === 'evenodd')) {
      return false;
    }
    // x and y are in canvas coordinates (i.e. unaffected by CTM)
    var pts = this._mapToLocalCoordinates([x, y]);
    x = pts[0];
    y = pts[1];
    path.setFillType(fillmode === 'nonzero' ?
                                  CanvasKit.FillType.Winding :
                                  CanvasKit.FillType.EvenOdd);
    return path.contains(x, y);
  };

  this.isPointInStroke = function(x, y) {
    var args = arguments;
    if (args.length === 2) {
      var path = this._currentPath;
    } else if (args.length === 3) {
      var path = args[0];
      x = args[1];
      y = args[2];
    } else {
      throw 'invalid arg count, need 2 or 3, got ' + args.length;
    }
    if (!isFinite(x) || !isFinite(y)) {
      return false;
    }
    var pts = this._mapToLocalCoordinates([x, y]);
    x = pts[0];
    y = pts[1];
    var temp = path.copy();
    // fillmode is always nonzero
    temp.setFillType(CanvasKit.FillType.Winding);
    temp.stroke({'width': this.lineWidth, 'miter_limit': this.miterLimit,
                 'cap': this._paint.getStrokeCap(), 'join': this._paint.getStrokeJoin(),
                 'precision': 0.3, // this is what Chrome uses to compute this
                });
    var retVal = temp.contains(x, y);
    temp.delete();
    return retVal;
  };

  this.lineTo = function(x, y) {
    lineTo(this._currentPath, x, y);
  };

  this.measureText = function(text) {
    throw new Error('Clients wishing to properly measure text should use the Paragraph API');
  };

  this.moveTo = function(x, y) {
    moveTo(this._currentPath, x, y);
  };

  this.putImageData = function(imageData, x, y, dirtyX, dirtyY, dirtyWidth, dirtyHeight) {
    if (!allAreFinite([x, y, dirtyX, dirtyY, dirtyWidth, dirtyHeight])) {
      return;
    }
    if (dirtyX === undefined) {
      // fast, simple path for basic call
      this._canvas.writePixels(imageData.data, imageData.width, imageData.height, x, y);
      return;
    }
    dirtyX = dirtyX || 0;
    dirtyY = dirtyY || 0;
    dirtyWidth = dirtyWidth || imageData.width;
    dirtyHeight = dirtyHeight || imageData.height;

    // as per https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-putimagedata
    if (dirtyWidth < 0) {
      dirtyX = dirtyX+dirtyWidth;
      dirtyWidth = Math.abs(dirtyWidth);
    }
    if (dirtyHeight < 0) {
      dirtyY = dirtyY+dirtyHeight;
      dirtyHeight = Math.abs(dirtyHeight);
    }
    if (dirtyX < 0) {
      dirtyWidth = dirtyWidth + dirtyX;
      dirtyX = 0;
    }
    if (dirtyY < 0) {
      dirtyHeight = dirtyHeight + dirtyY;
      dirtyY = 0;
    }
    if (dirtyWidth <= 0 || dirtyHeight <= 0) {
      return;
    }
    var img = CanvasKit.MakeImage({
      'width': imageData.width,
      'height': imageData.height,
      'alphaType': CanvasKit.AlphaType.Unpremul,
      'colorType': CanvasKit.ColorType.RGBA_8888,
      'colorSpace': CanvasKit.ColorSpace.SRGB
    }, imageData.data, 4 * imageData.width);
    var src = CanvasKit.XYWHRect(dirtyX, dirtyY, dirtyWidth, dirtyHeight);
    var dst = CanvasKit.XYWHRect(x+dirtyX, y+dirtyY, dirtyWidth, dirtyHeight);
    var inverted = CanvasKit.Matrix.invert(this._currentTransform);
    this._canvas.save();
    // putImageData() operates in device space.
    this._canvas.concat(inverted);
    this._canvas.drawImageRect(img, src, dst, null, false);
    this._canvas.restore();
    img.delete();
  };

  this.quadraticCurveTo = function(cpx, cpy, x, y) {
    quadraticCurveTo(this._currentPath, cpx, cpy, x, y);
  };

  this.rect = function(x, y, width, height) {
    rect(this._currentPath, x, y, width, height);
  };

  this.resetTransform = function() {
    // Apply the current transform to the path and then reset
    // to the identity. Essentially "commit" the transform.
    this._currentPath.transform(this._currentTransform);
    var inverted = CanvasKit.Matrix.invert(this._currentTransform);
    this._canvas.concat(inverted);
    // This should be identity, modulo floating point drift.
    this._currentTransform = this._canvas.getTotalMatrix();
  };

  this.restore = function() {
    var newState = this._canvasStateStack.pop();
    if (!newState) {
      return;
    }
    // "commit" the current transform. We pop, then apply the inverse of the
    // popped state, which has the effect of applying just the delta of
    // transforms between old and new.
    var combined = CanvasKit.Matrix.multiply(
      this._currentTransform,
      CanvasKit.Matrix.invert(newState.ctm)
    );
    this._currentPath.transform(combined);
    this._paint.delete();
    this._paint = newState.paint;

    this._lineDashList = newState.ldl;
    this._strokeWidth = newState.sw;
    this._strokeStyle = newState.ss;
    this._fillStyle = newState.fs;
    this._shadowOffsetX = newState.sox;
    this._shadowOffsetY = newState.soy;
    this._shadowBlur = newState.sb;
    this._shadowColor = newState.shc;
    this._globalAlpha = newState.ga;
    this._globalCompositeOperation = newState.gco;
    this._lineDashOffset = newState.ldo;
    this._fontString = newState.fontstr;

    //TODO: textAlign, textBaseline

    // restores the clip and ctm
    this._canvas.restore();
    this._currentTransform = this._canvas.getTotalMatrix();
  };

  this.rotate = function(radians) {
    if (!isFinite(radians)) {
      return;
    }
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.Matrix.rotated(-radians);
    this._currentPath.transform(inverted);
    this._canvas.rotate(radiansToDegrees(radians), 0, 0);
    this._currentTransform = this._canvas.getTotalMatrix();
  };

  this.save = function() {
    if (this._fillStyle._copy) {
      var fs = this._fillStyle._copy();
      this._toCleanUp.push(fs);
    } else {
      var fs = this._fillStyle;
    }

    if (this._strokeStyle._copy) {
      var ss = this._strokeStyle._copy();
      this._toCleanUp.push(ss);
    } else {
      var ss = this._strokeStyle;
    }

    this._canvasStateStack.push({
      ctm:     this._currentTransform.slice(),
      ldl:     this._lineDashList.slice(),
      sw:      this._strokeWidth,
      ss:      ss,
      fs:      fs,
      sox:     this._shadowOffsetX,
      soy:     this._shadowOffsetY,
      sb:      this._shadowBlur,
      shc:     this._shadowColor,
      ga:      this._globalAlpha,
      ldo:     this._lineDashOffset,
      gco:     this._globalCompositeOperation,
      paint:   this._paint.copy(),
      fontstr: this._fontString,
      //TODO: textAlign, textBaseline
    });
    // Saves the clip
    this._canvas.save();
  };

  this.scale = function(sx, sy) {
    if (!allAreFinite(arguments)) {
      return;
    }
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.Matrix.scaled(1/sx, 1/sy);
    this._currentPath.transform(inverted);
    this._canvas.scale(sx, sy);
    this._currentTransform = this._canvas.getTotalMatrix();
  };

  this.setLineDash = function(dashes) {
    for (var i = 0; i < dashes.length; i++) {
      if (!isFinite(dashes[i]) || dashes[i] < 0) {
        Debug('dash list must have positive, finite values');
        return;
      }
    }
    if (dashes.length % 2 === 1) {
      // as per the spec, concatenate 2 copies of dashes
      // to give it an even number of elements.
      Array.prototype.push.apply(dashes, dashes);
    }
    this._lineDashList = dashes;
  };

  this.setTransform = function(a, b, c, d, e, f) {
    if (!(allAreFinite(arguments))) {
      return;
    }
    this.resetTransform();
    this.transform(a, b, c, d, e, f);
  };

  // We need to apply the shadowOffsets on the device coordinates, so we undo
  // the CTM, apply the offsets, then re-apply the CTM.
  this._applyShadowOffsetMatrix = function() {
    var inverted = CanvasKit.Matrix.invert(this._currentTransform);
    this._canvas.concat(inverted);
    this._canvas.concat(CanvasKit.Matrix.translated(this._shadowOffsetX, this._shadowOffsetY));
    this._canvas.concat(this._currentTransform);
  };

  // Returns the shadow paint for the current settings or null if there
  // should be no shadow. This ends up being a copy of the given
  // paint with a blur maskfilter and the correct color.
  this._shadowPaint = function(basePaint) {
    // multiply first to see if the alpha channel goes to 0 after multiplication.
    var alphaColor = CanvasKit.multiplyByAlpha(this._shadowColor, this._globalAlpha);
    // if alpha is zero, no shadows
    if (!CanvasKit.getColorComponents(alphaColor)[3]) {
      return null;
    }
    // one of these must also be non-zero (otherwise the shadow is
    // completely hidden.  And the spec says so).
    if (!(this._shadowBlur || this._shadowOffsetY || this._shadowOffsetX)) {
      return null;
    }
    var shadowPaint = basePaint.copy();
    shadowPaint.setColor(alphaColor);
    var blurEffect = CanvasKit.MaskFilter.MakeBlur(CanvasKit.BlurStyle.Normal,
      BlurRadiusToSigma(this._shadowBlur),
      false);
    shadowPaint.setMaskFilter(blurEffect);

    // hack up a "destructor" which also cleans up the blurEffect. Otherwise,
    // we leak the blurEffect (since smart pointers don't help us in JS land).
    shadowPaint.dispose = function() {
      blurEffect.delete();
      this.delete();
    };
    return shadowPaint;
  };

  // A helper to get a copy of the current paint, ready for stroking.
  // This applies the global alpha and the dashedness.
  // Call dispose() after to clean up.
  this._strokePaint = function() {
    var paint = this._paint.copy();
    paint.setStyle(CanvasKit.PaintStyle.Stroke);
    if (isCanvasKitColor(this._strokeStyle)) {
      var alphaColor = CanvasKit.multiplyByAlpha(this._strokeStyle, this._globalAlpha);
      paint.setColor(alphaColor);
    } else {
      var shader = this._strokeStyle._getShader(this._currentTransform);
      paint.setColor(CanvasKit.Color(0,0,0, this._globalAlpha));
      paint.setShader(shader);
    }

    paint.setStrokeWidth(this._strokeWidth);

    if (this._lineDashList.length) {
      var dashedEffect = CanvasKit.PathEffect.MakeDash(this._lineDashList, this._lineDashOffset);
      paint.setPathEffect(dashedEffect);
    }

    paint.dispose = function() {
      dashedEffect && dashedEffect.delete();
      this.delete();
    };
    return paint;
  };

  this.stroke = function(path) {
    path = path ? path._getPath() : this._currentPath;
    var strokePaint = this._strokePaint();

    var shadowPaint = this._shadowPaint(strokePaint);
    if (shadowPaint) {
      this._canvas.save();
      this._applyShadowOffsetMatrix();
      this._canvas.drawPath(path, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }

    this._canvas.drawPath(path, strokePaint);
    strokePaint.dispose();
  };

  this.strokeRect = function(x, y, width, height) {
    var strokePaint = this._strokePaint();

    var shadowPaint = this._shadowPaint(strokePaint);
    if (shadowPaint) {
      this._canvas.save();
      this._applyShadowOffsetMatrix();
      this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }
    this._canvas.drawRect(CanvasKit.XYWHRect(x, y, width, height), strokePaint);
    strokePaint.dispose();
  };

  this.strokeText = function(text, x, y, maxWidth) {
    // TODO do something with maxWidth, probably involving measure
    var strokePaint = this._strokePaint();
    var blob = CanvasKit.TextBlob.MakeFromText(text, this._font);

    var shadowPaint = this._shadowPaint(strokePaint);
    if (shadowPaint) {
      this._canvas.save();
      this._applyShadowOffsetMatrix();
      this._canvas.drawTextBlob(blob, x, y, shadowPaint);
      this._canvas.restore();
      shadowPaint.dispose();
    }
    this._canvas.drawTextBlob(blob, x, y, strokePaint);
    blob.delete();
    strokePaint.dispose();
  };

  this.translate = function(dx, dy) {
    if (!allAreFinite(arguments)) {
      return;
    }
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.Matrix.translated(-dx, -dy);
    this._currentPath.transform(inverted);
    this._canvas.translate(dx, dy);
    this._currentTransform = this._canvas.getTotalMatrix();
  };

  this.transform = function(a, b, c, d, e, f) {
    var newTransform = [a, c, e,
                        b, d, f,
                        0, 0, 1];
    // retroactively apply the inverse of this transform to the previous
    // path so it cancels out when we apply the transform at draw time.
    var inverted = CanvasKit.Matrix.invert(newTransform);
    this._currentPath.transform(inverted);
    this._canvas.concat(newTransform);
    this._currentTransform = this._canvas.getTotalMatrix();
  };

  // Not supported operations (e.g. for Web only)
  this.addHitRegion = function() {};
  this.clearHitRegions = function() {};
  this.drawFocusIfNeeded = function() {};
  this.removeHitRegion = function() {};
  this.scrollPathIntoView = function() {};

  Object.defineProperty(this, 'canvas', {
    value: null,
    writable: false
  });
}

function BlurRadiusToSigma(radius) {
  // Blink (Chrome) does the following, for legacy reasons, even though it
  // is against the spec. https://bugs.chromium.org/p/chromium/issues/detail?id=179006
  // This may change in future releases.
  // This code is staying here in case any clients are interested in using it
  // to match Blink "exactly".
  // if (radius <= 0)
  //   return 0;
  // return 0.288675 * radius + 0.5;
  //
  // This is what the spec says, which is how Firefox and others operate.
  return radius/2;
}

CanvasKit.MakeCanvas = function(width, height) {
  var surf = CanvasKit.MakeSurface(width, height);
  if (surf) {
    return new HTMLCanvas(surf);
  }
  return null;
}

function HTMLCanvas(skSurface) {
  this._surface = skSurface;
  this._context = new CanvasRenderingContext2D(skSurface.getCanvas());
  this._toCleanup = [];
  this._fontmgr = CanvasKit.FontMgr.RefDefault();

  // Data is either an ArrayBuffer, a TypedArray, or a Node Buffer
  this.decodeImage = function(data) {
    var img = CanvasKit.MakeImageFromEncoded(data);
    if (!img) {
      throw 'Invalid input';
    }
    this._toCleanup.push(img);
    return img;
  }

  this.loadFont = function(buffer, descriptors) {
    var newFont = this._fontmgr.MakeTypefaceFromData(buffer);
    if (!newFont) {
      Debug('font could not be processed', descriptors);
      return null;
    }
    this._toCleanup.push(newFont);
    addToFontCache(newFont, descriptors);
  }

  this.makePath2D = function(path) {
    var p2d = new Path2D(path);
    this._toCleanup.push(p2d._getPath());
    return p2d;
  }

  // A normal <canvas> requires that clients call getContext
  this.getContext = function(type) {
    if (type === '2d') {
      return this._context;
    }
    return null;
  }

  this.toDataURL = function(codec, quality) {
    // TODO(kjlubick): maybe support other codecs (webp?)
    // For now, just to png and jpeg
    this._surface.flush();

    var img = this._surface.makeImageSnapshot();
    if (!img) {
      Debug('no snapshot');
      return;
    }
    var codec = codec || 'image/png';
    var format = CanvasKit.ImageFormat.PNG;
    if (codec === 'image/jpeg') {
      format = CanvasKit.ImageFormat.JPEG;
    }
    var quality = quality || 0.92;
    var skimg = img.encodeToData(format, quality);
    if (!skimg) {
      Debug('encoding failure');
      return
    }
    var imgBytes = CanvasKit.getDataBytes(skimg);
    return 'data:' + codec + ';base64,' + toBase64String(imgBytes);
  }

  this.dispose = function() {
    this._context._dispose();
    this._toCleanup.forEach(function(i) {
      i.delete();
    });
    this._surface.dispose();
  }
}

function ImageData(arr, width, height) {
  if (!width || height === 0) {
    throw 'invalid dimensions, width and height must be non-zero';
  }
  if (arr.length % 4) {
    throw 'arr must be a multiple of 4';
  }
  height = height || arr.length/(4*width);

  Object.defineProperty(this, 'data', {
    value: arr,
    writable: false
  });
  Object.defineProperty(this, 'height', {
    value: height,
    writable: false
  });
  Object.defineProperty(this, 'width', {
    value: width,
    writable: false
  });
}

CanvasKit.ImageData = function() {
  if (arguments.length === 2) {
    var width = arguments[0];
    var height = arguments[1];
    var byteLength = 4 * width * height;
    return new ImageData(new Uint8ClampedArray(byteLength),
                         width, height);
  } else if (arguments.length === 3) {
    var arr = arguments[0];
    if (arr.prototype.constructor !== Uint8ClampedArray ) {
      throw 'bytes must be given as a Uint8ClampedArray';
    }
    var width = arguments[1];
    var height = arguments[2];
    if (arr % 4) {
      throw 'bytes must be given in a multiple of 4';
    }
    if (arr % width) {
      throw 'bytes must divide evenly by width';
    }
    if (height && (height !== (arr / (width * 4)))) {
      throw 'invalid height given';
    }
    height = arr / (width * 4);
    return new ImageData(arr, width, height);
  } else {
    throw 'invalid number of arguments - takes 2 or 3, saw ' + arguments.length;
  }
}

function LinearCanvasGradient(x1, y1, x2, y2) {
  this._shader = null;
  this._colors = [];
  this._pos = [];

  this.addColorStop = function(offset, color) {
    if (offset < 0 || offset > 1 || !isFinite(offset)) {
      throw 'offset must be between 0 and 1 inclusively';
    }

    color = parseColor(color);
    // From the spec: If multiple stops are added at the same offset on a
    // gradient, then they must be placed in the order added, with the first
    // one closest to the start of the gradient, and each subsequent one
    // infinitesimally further along towards the end point (in effect
    // causing all but the first and last stop added at each point to be
    // ignored).
    // To implement that, if an offset is already in the list,
    // we just overwrite its color (since the user can't remove Color stops
    // after the fact).
    var idx = this._pos.indexOf(offset);
    if (idx !== -1) {
      this._colors[idx] = color;
    } else {
      // insert it in sorted order
      for (idx = 0; idx < this._pos.length; idx++) {
        if (this._pos[idx] > offset) {
          break;
        }
      }
      this._pos   .splice(idx, 0, offset);
      this._colors.splice(idx, 0, color);
    }
  }

  this._copy = function() {
    var lcg = new LinearCanvasGradient(x1, y1, x2, y2);
    lcg._colors = this._colors.slice();
    lcg._pos    = this._pos.slice();
    return lcg;
  }

  this._dispose = function() {
    if (this._shader) {
      this._shader.delete();
      this._shader = null;
    }
  }

  this._getShader = function(currentTransform) {
    // From the spec: "The points in the linear gradient must be transformed
    // as described by the current transformation matrix when rendering."
    var pts = [x1, y1, x2, y2];
    CanvasKit.Matrix.mapPoints(currentTransform, pts);
    var sx1 = pts[0];
    var sy1 = pts[1];
    var sx2 = pts[2];
    var sy2 = pts[3];

    this._dispose();
    this._shader = CanvasKit.Shader.MakeLinearGradient([sx1, sy1], [sx2, sy2],
      this._colors, this._pos, CanvasKit.TileMode.Clamp);
    return this._shader;
  }
}

// CanvasPath methods, which all take an Path object as the first param

function arc(skpath, x, y, radius, startAngle, endAngle, ccw) {
  // As per  https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-arc
  // arc is essentially a simpler version of ellipse.
  ellipse(skpath, x, y, radius, radius, 0, startAngle, endAngle, ccw);
}

function arcTo(skpath, x1, y1, x2, y2, radius) {
  if (!allAreFinite([x1, y1, x2, y2, radius])) {
    return;
  }
  if (radius < 0) {
    throw 'radii cannot be negative';
  }
  if (skpath.isEmpty()) {
    skpath.moveTo(x1, y1);
  }
  skpath.arcToTangent(x1, y1, x2, y2, radius);
}

function bezierCurveTo(skpath, cp1x, cp1y, cp2x, cp2y, x, y) {
  if (!allAreFinite([cp1x, cp1y, cp2x, cp2y, x, y])) {
    return;
  }
  if (skpath.isEmpty()) {
    skpath.moveTo(cp1x, cp1y);
  }
  skpath.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
}

function closePath(skpath) {
  if (skpath.isEmpty()) {
    return;
  }
  // Check to see if we are not just a single point
  var bounds = skpath.getBounds();
  if ((bounds[3] - bounds[1]) || (bounds[2] - bounds[0])) {
    skpath.close();
  }
}

function _ellipseHelper(skpath, x, y, radiusX, radiusY, startAngle, endAngle) {
  var sweepDegrees = radiansToDegrees(endAngle - startAngle);
  var startDegrees = radiansToDegrees(startAngle);

  var oval = CanvasKit.LTRBRect(x - radiusX, y - radiusY, x + radiusX, y + radiusY);

  // draw in 2 180 degree segments because trying to draw all 360 degrees at once
  // draws nothing.
  if (almostEqual(Math.abs(sweepDegrees), 360)) {
    var halfSweep = sweepDegrees/2;
    skpath.arcToOval(oval, startDegrees, halfSweep, false);
    skpath.arcToOval(oval, startDegrees + halfSweep, halfSweep, false);
    return;
  }
  skpath.arcToOval(oval, startDegrees, sweepDegrees, false);
}

function ellipse(skpath, x, y, radiusX, radiusY, rotation,
                 startAngle, endAngle, ccw) {
  if (!allAreFinite([x, y, radiusX, radiusY, rotation, startAngle, endAngle])) {
    return;
  }
  if (radiusX < 0 || radiusY < 0) {
    throw 'radii cannot be negative';
  }

  // based off of CanonicalizeAngle in Chrome
  var tao = 2 * Math.PI;
  var newStartAngle = startAngle % tao;
  if (newStartAngle < 0) {
    newStartAngle += tao;
  }
  var delta = newStartAngle - startAngle;
  startAngle = newStartAngle;
  endAngle += delta;

  // Based off of AdjustEndAngle in Chrome.
  if (!ccw && (endAngle - startAngle) >= tao) {
    // Draw complete ellipse
    endAngle = startAngle + tao;
  } else if (ccw && (startAngle - endAngle) >= tao) {
    // Draw complete ellipse
    endAngle = startAngle - tao;
  } else if (!ccw && startAngle > endAngle) {
    endAngle = startAngle + (tao - (startAngle - endAngle) % tao);
  } else if (ccw && startAngle < endAngle) {
    endAngle = startAngle - (tao - (endAngle - startAngle) % tao);
  }

  // Based off of Chrome's implementation in
  // https://cs.chromium.org/chromium/src/third_party/blink/renderer/platform/graphics/path.cc
  // of note, can't use addArc or addOval because they close the arc, which
  // the spec says not to do (unless the user explicitly calls closePath).
  // This throws off points being in/out of the arc.
  if (!rotation) {
    _ellipseHelper(skpath, x, y, radiusX, radiusY, startAngle, endAngle);
    return;
  }
  var rotated = CanvasKit.Matrix.rotated(rotation, x, y);
  var rotatedInvert = CanvasKit.Matrix.rotated(-rotation, x, y);
  skpath.transform(rotatedInvert);
  _ellipseHelper(skpath, x, y, radiusX, radiusY, startAngle, endAngle);
  skpath.transform(rotated);
}

function lineTo(skpath, x, y) {
  if (!allAreFinite([x, y])) {
    return;
  }
  // A lineTo without a previous point has a moveTo inserted before it
  if (skpath.isEmpty()) {
    skpath.moveTo(x, y);
  }
  skpath.lineTo(x, y);
}

function moveTo(skpath, x, y) {
  if (!allAreFinite([x, y])) {
    return;
  }
  skpath.moveTo(x, y);
}

function quadraticCurveTo(skpath, cpx, cpy, x, y) {
  if (!allAreFinite([cpx, cpy, x, y])) {
    return;
  }
  if (skpath.isEmpty()) {
    skpath.moveTo(cpx, cpy);
  }
  skpath.quadTo(cpx, cpy, x, y);
}

function rect(skpath, x, y, width, height) {
  var rect = CanvasKit.XYWHRect(x, y, width, height);
  if (!allAreFinite(rect)) {
    return;
  }
  // https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-rect
  skpath.addRect(rect);
}

function Path2D(path) {
  this._path = null;
  if (typeof path === 'string') {
      this._path = CanvasKit.Path.MakeFromSVGString(path);
  } else if (path && path._getPath) {
      this._path = path._getPath().copy();
  } else {
    this._path = new CanvasKit.Path();
  }

  this._getPath = function() {
      return this._path;
  }

  this.addPath = function(path2d, transform) {
    if (!transform) {
      transform = {
        'a': 1, 'c': 0, 'e': 0,
        'b': 0, 'd': 1, 'f': 0,
      };
    }
    this._path.addPath(path2d._getPath(), [transform.a, transform.c, transform.e,
                                           transform.b, transform.d, transform.f]);
  }

  this.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    arc(this._path, x, y, radius, startAngle, endAngle, ccw);
  }

  this.arcTo = function(x1, y1, x2, y2, radius) {
    arcTo(this._path, x1, y1, x2, y2, radius);
  }

  this.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    bezierCurveTo(this._path, cp1x, cp1y, cp2x, cp2y, x, y);
  }

  this.closePath = function() {
    closePath(this._path);
  }

  this.ellipse = function(x, y, radiusX, radiusY, rotation,
                          startAngle, endAngle, ccw) {
    ellipse(this._path, x, y, radiusX, radiusY, rotation,
            startAngle, endAngle, ccw);
  }

  this.lineTo = function(x, y) {
    lineTo(this._path, x, y);
  }

  this.moveTo = function(x, y) {
    moveTo(this._path, x, y);
  }

  this.quadraticCurveTo = function(cpx, cpy, x, y) {
    quadraticCurveTo(this._path, cpx, cpy, x, y);
  }

  this.rect = function(x, y, width, height) {
    rect(this._path, x, y, width, height);
  }
}

function CanvasPattern(image, repetition) {
  this._shader = null;
  // image should be an Image returned from HTMLCanvas.decodeImage()
  this._image = image;
  this._transform = CanvasKit.Matrix.identity();

  if (repetition === '') {
    repetition = 'repeat';
  }
  switch(repetition) {
    case 'repeat-x':
      this._tileX = CanvasKit.TileMode.Repeat;
      // Skia's 'clamp' mode repeats the last row/column
      // which looks very very strange.
      // Decal mode does just transparent copying, which
      // is exactly what the spec wants.
      this._tileY = CanvasKit.TileMode.Decal;
      break;
    case 'repeat-y':
      this._tileX = CanvasKit.TileMode.Decal;
      this._tileY = CanvasKit.TileMode.Repeat;
      break;
    case 'repeat':
      this._tileX = CanvasKit.TileMode.Repeat;
      this._tileY = CanvasKit.TileMode.Repeat;
      break;
    case 'no-repeat':
      this._tileX = CanvasKit.TileMode.Decal;
      this._tileY = CanvasKit.TileMode.Decal;
      break;
    default:
      throw 'invalid repetition mode ' + repetition;
  }

  // Takes a DOMMatrix like object. e.g. the identity would be:
  // {a:1, b: 0, c: 0, d: 1, e: 0, f: 0}
  // @param {DOMMatrix} m
  this.setTransform = function(m) {
    var t = [m.a, m.c, m.e,
             m.b, m.d, m.f,
               0,   0,   1];
    if (allAreFinite(t)) {
      this._transform = t;
    }
  };

  this._copy = function() {
    var cp = new CanvasPattern();
    cp._tileX = this._tileX;
    cp._tileY = this._tileY;
    return cp;
  };

  this._dispose = function() {
    if (this._shader) {
      this._shader.delete();
      this._shader = null;
    }
  };

  this._getShader = function(currentTransform) {
    // Ignore currentTransform since it will be applied later
    this._dispose();
    // A shader with cubic sampling options is high quality.
    this._shader = this._image.makeShaderCubic(this._tileX, this._tileY, 1/3, 1/3, this._transform);
    return this._shader;
  }

}

// Note, Skia has a different notion of a "radial" gradient.
// Skia has a twoPointConical gradient that is the same as the
// canvas's RadialGradient.

function RadialCanvasGradient(x1, y1, r1, x2, y2, r2) {
  this._shader = null;
  this._colors = [];
  this._pos = [];

  this.addColorStop = function(offset, color) {
    if (offset < 0 || offset > 1 || !isFinite(offset)) {
      throw 'offset must be between 0 and 1 inclusively';
    }

    color = parseColor(color);
    // From the spec: If multiple stops are added at the same offset on a
    // gradient, then they must be placed in the order added, with the first
    // one closest to the start of the gradient, and each subsequent one
    // infinitesimally further along towards the end point (in effect
    // causing all but the first and last stop added at each point to be
    // ignored).
    // To implement that, if an offset is already in the list,
    // we just overwrite its color (since the user can't remove Color stops
    // after the fact).
    var idx = this._pos.indexOf(offset);
    if (idx !== -1) {
      this._colors[idx] = color;
    } else {
      // insert it in sorted order
      for (idx = 0; idx < this._pos.length; idx++) {
        if (this._pos[idx] > offset) {
          break;
        }
      }
      this._pos   .splice(idx, 0, offset);
      this._colors.splice(idx, 0, color);
    }
  }

  this._copy = function() {
    var rcg = new RadialCanvasGradient(x1, y1, r1, x2, y2, r2);
    rcg._colors = this._colors.slice();
    rcg._pos    = this._pos.slice();
    return rcg;
  }

  this._dispose = function() {
    if (this._shader) {
      this._shader.delete();
      this._shader = null;
    }
  }

  this._getShader = function(currentTransform) {
    // From the spec: "The points in the linear gradient must be transformed
    // as described by the current transformation matrix when rendering."
    var pts = [x1, y1, x2, y2];
    CanvasKit.Matrix.mapPoints(currentTransform, pts);
    var sx1 = pts[0];
    var sy1 = pts[1];
    var sx2 = pts[2];
    var sy2 = pts[3];

    var sx = currentTransform[0];
    var sy = currentTransform[4];
    var scaleFactor = (Math.abs(sx) + Math.abs(sy))/2;

    var sr1 = r1 * scaleFactor;
    var sr2 = r2 * scaleFactor;

    this._dispose();
    this._shader = CanvasKit.Shader.MakeTwoPointConicalGradient(
        [sx1, sy1], sr1, [sx2, sy2], sr2, this._colors, this._pos,
        CanvasKit.TileMode.Clamp);
    return this._shader;
  }
}

// This closes the scope started in preamble.js
}());

// This closes the scope started in preamble.js
}(Module)); // When this file is loaded in, the high level object is "Module";



// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = {};
var key;
for (key in Module) {
  if (Module.hasOwnProperty(key)) {
    moduleOverrides[key] = Module[key];
  }
}

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = function(status, toThrow) {
  throw toThrow;
};

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

var ENVIRONMENT_IS_WEB = false;
var ENVIRONMENT_IS_WORKER = false;
var ENVIRONMENT_IS_NODE = false;
var ENVIRONMENT_IS_SHELL = false;
ENVIRONMENT_IS_WEB = typeof window === 'object';
ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string';
ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (Module['ENVIRONMENT']) {
  throw new Error('Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -s ENVIRONMENT=web or -s ENVIRONMENT=node)');
}

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var read_,
    readAsync,
    readBinary,
    setWindowTitle;

var nodeFS;
var nodePath;

if (ENVIRONMENT_IS_NODE) {
  if (ENVIRONMENT_IS_WORKER) {
    scriptDirectory = require('path').dirname(scriptDirectory) + '/';
  } else {
    scriptDirectory = __dirname + '/';
  }

// include: node_shell_read.js


read_ = function shell_read(filename, binary) {
  if (!nodeFS) nodeFS = require('fs');
  if (!nodePath) nodePath = require('path');
  filename = nodePath['normalize'](filename);
  return nodeFS['readFileSync'](filename, binary ? null : 'utf8');
};

readBinary = function readBinary(filename) {
  var ret = read_(filename, true);
  if (!ret.buffer) {
    ret = new Uint8Array(ret);
  }
  assert(ret.buffer);
  return ret;
};

// end include: node_shell_read.js
  if (process['argv'].length > 1) {
    thisProgram = process['argv'][1].replace(/\\/g, '/');
  }

  arguments_ = process['argv'].slice(2);

  // MODULARIZE will export the module in the proper place outside, we don't need to export here

  process['on']('uncaughtException', function(ex) {
    // suppress ExitStatus exceptions from showing an error
    if (!(ex instanceof ExitStatus)) {
      throw ex;
    }
  });

  process['on']('unhandledRejection', abort);

  quit_ = function(status) {
    process['exit'](status);
  };

  Module['inspect'] = function () { return '[Emscripten Module object]'; };

} else
if (ENVIRONMENT_IS_SHELL) {

  if (typeof read != 'undefined') {
    read_ = function shell_read(f) {
      return read(f);
    };
  }

  readBinary = function readBinary(f) {
    var data;
    if (typeof readbuffer === 'function') {
      return new Uint8Array(readbuffer(f));
    }
    data = read(f, 'binary');
    assert(typeof data === 'object');
    return data;
  };

  if (typeof scriptArgs != 'undefined') {
    arguments_ = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    arguments_ = arguments;
  }

  if (typeof quit === 'function') {
    quit_ = function(status) {
      quit(status);
    };
  }

  if (typeof print !== 'undefined') {
    // Prefer to use print/printErr where they exist, as they usually work better.
    if (typeof console === 'undefined') console = /** @type{!Console} */({});
    console.log = /** @type{!function(this:Console, ...*): undefined} */ (print);
    console.warn = console.error = /** @type{!function(this:Console, ...*): undefined} */ (typeof printErr !== 'undefined' ? printErr : print);
  }

} else

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  if (ENVIRONMENT_IS_WORKER) { // Check worker, not web, since window could be polyfilled
    scriptDirectory = self.location.href;
  } else if (typeof document !== 'undefined' && document.currentScript) { // web
    scriptDirectory = document.currentScript.src;
  }
  // When MODULARIZE, this JS may be executed later, after document.currentScript
  // is gone, so we saved it, and we use it here instead of any other info.
  if (_scriptDir) {
    scriptDirectory = _scriptDir;
  }
  // blob urls look like blob:http://site.com/etc/etc and we cannot infer anything from them.
  // otherwise, slice off the final part of the url to find the script directory.
  // if scriptDirectory does not contain a slash, lastIndexOf will return -1,
  // and scriptDirectory will correctly be replaced with an empty string.
  if (scriptDirectory.indexOf('blob:') !== 0) {
    scriptDirectory = scriptDirectory.substr(0, scriptDirectory.lastIndexOf('/')+1);
  } else {
    scriptDirectory = '';
  }

  // Differentiate the Web Worker from the Node Worker case, as reading must
  // be done differently.
  {

// include: web_or_worker_shell_read.js


  read_ = function(url) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.send(null);
      return xhr.responseText;
  };

  if (ENVIRONMENT_IS_WORKER) {
    readBinary = function(url) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, false);
        xhr.responseType = 'arraybuffer';
        xhr.send(null);
        return new Uint8Array(/** @type{!ArrayBuffer} */(xhr.response));
    };
  }

  readAsync = function(url, onload, onerror) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer';
    xhr.onload = function() {
      if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
        onload(xhr.response);
        return;
      }
      onerror();
    };
    xhr.onerror = onerror;
    xhr.send(null);
  };

// end include: web_or_worker_shell_read.js
  }

  setWindowTitle = function(title) { document.title = title };
} else
{
  throw new Error('environment detection error');
}

// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
var out = Module['print'] || console.log.bind(console);
var err = Module['printErr'] || console.warn.bind(console);

// Merge back in the overrides
for (key in moduleOverrides) {
  if (moduleOverrides.hasOwnProperty(key)) {
    Module[key] = moduleOverrides[key];
  }
}
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used e.g. in memoryInitializerRequest, which is a large typed array.
moduleOverrides = null;

// Emit code to handle expected values on the Module object. This applies Module.x
// to the proper local x. This has two benefits: first, we only emit it if it is
// expected to arrive, and second, by using a local everywhere else that can be
// minified.
if (Module['arguments']) arguments_ = Module['arguments'];if (!Object.getOwnPropertyDescriptor(Module, 'arguments')) {
  Object.defineProperty(Module, 'arguments', {
    configurable: true,
    get: function() {
      abort('Module.arguments has been replaced with plain arguments_ (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}
if (Module['thisProgram']) thisProgram = Module['thisProgram'];if (!Object.getOwnPropertyDescriptor(Module, 'thisProgram')) {
  Object.defineProperty(Module, 'thisProgram', {
    configurable: true,
    get: function() {
      abort('Module.thisProgram has been replaced with plain thisProgram (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}
if (Module['quit']) quit_ = Module['quit'];if (!Object.getOwnPropertyDescriptor(Module, 'quit')) {
  Object.defineProperty(Module, 'quit', {
    configurable: true,
    get: function() {
      abort('Module.quit has been replaced with plain quit_ (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}

// perform assertions in shell.js after we set up out() and err(), as otherwise if an assertion fails it cannot print the message
// Assertions on removed incoming Module JS APIs.
assert(typeof Module['memoryInitializerPrefixURL'] === 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['pthreadMainPrefixURL'] === 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['cdInitializerPrefixURL'] === 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['filePackagePrefixURL'] === 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['read'] === 'undefined', 'Module.read option was removed (modify read_ in JS)');
assert(typeof Module['readAsync'] === 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
assert(typeof Module['readBinary'] === 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
assert(typeof Module['setWindowTitle'] === 'undefined', 'Module.setWindowTitle option was removed (modify setWindowTitle in JS)');
assert(typeof Module['TOTAL_MEMORY'] === 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
if (!Object.getOwnPropertyDescriptor(Module, 'read')) {
  Object.defineProperty(Module, 'read', {
    configurable: true,
    get: function() {
      abort('Module.read has been replaced with plain read_ (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}
if (!Object.getOwnPropertyDescriptor(Module, 'readAsync')) {
  Object.defineProperty(Module, 'readAsync', {
    configurable: true,
    get: function() {
      abort('Module.readAsync has been replaced with plain readAsync (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}
if (!Object.getOwnPropertyDescriptor(Module, 'readBinary')) {
  Object.defineProperty(Module, 'readBinary', {
    configurable: true,
    get: function() {
      abort('Module.readBinary has been replaced with plain readBinary (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}
if (!Object.getOwnPropertyDescriptor(Module, 'setWindowTitle')) {
  Object.defineProperty(Module, 'setWindowTitle', {
    configurable: true,
    get: function() {
      abort('Module.setWindowTitle has been replaced with plain setWindowTitle (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}
var IDBFS = 'IDBFS is no longer included by default; build with -lidbfs.js';
var PROXYFS = 'PROXYFS is no longer included by default; build with -lproxyfs.js';
var WORKERFS = 'WORKERFS is no longer included by default; build with -lworkerfs.js';
var NODEFS = 'NODEFS is no longer included by default; build with -lnodefs.js';




var STACK_ALIGN = 16;

function alignMemory(size, factor) {
  if (!factor) factor = STACK_ALIGN; // stack alignment (16-byte) by default
  return Math.ceil(size / factor) * factor;
}

function getNativeTypeSize(type) {
  switch (type) {
    case 'i1': case 'i8': return 1;
    case 'i16': return 2;
    case 'i32': return 4;
    case 'i64': return 8;
    case 'float': return 4;
    case 'double': return 8;
    default: {
      if (type[type.length-1] === '*') {
        return 4; // A pointer
      } else if (type[0] === 'i') {
        var bits = Number(type.substr(1));
        assert(bits % 8 === 0, 'getNativeTypeSize invalid bits ' + bits + ', type ' + type);
        return bits / 8;
      } else {
        return 0;
      }
    }
  }
}

function warnOnce(text) {
  if (!warnOnce.shown) warnOnce.shown = {};
  if (!warnOnce.shown[text]) {
    warnOnce.shown[text] = 1;
    err(text);
  }
}

// include: runtime_functions.js


// Wraps a JS function as a wasm function with a given signature.
function convertJsFunctionToWasm(func, sig) {

  // If the type reflection proposal is available, use the new
  // "WebAssembly.Function" constructor.
  // Otherwise, construct a minimal wasm module importing the JS function and
  // re-exporting it.
  if (typeof WebAssembly.Function === "function") {
    var typeNames = {
      'i': 'i32',
      'j': 'i64',
      'f': 'f32',
      'd': 'f64'
    };
    var type = {
      parameters: [],
      results: sig[0] == 'v' ? [] : [typeNames[sig[0]]]
    };
    for (var i = 1; i < sig.length; ++i) {
      type.parameters.push(typeNames[sig[i]]);
    }
    return new WebAssembly.Function(type, func);
  }

  // The module is static, with the exception of the type section, which is
  // generated based on the signature passed in.
  var typeSection = [
    0x01, // id: section,
    0x00, // length: 0 (placeholder)
    0x01, // count: 1
    0x60, // form: func
  ];
  var sigRet = sig.slice(0, 1);
  var sigParam = sig.slice(1);
  var typeCodes = {
    'i': 0x7f, // i32
    'j': 0x7e, // i64
    'f': 0x7d, // f32
    'd': 0x7c, // f64
  };

  // Parameters, length + signatures
  typeSection.push(sigParam.length);
  for (var i = 0; i < sigParam.length; ++i) {
    typeSection.push(typeCodes[sigParam[i]]);
  }

  // Return values, length + signatures
  // With no multi-return in MVP, either 0 (void) or 1 (anything else)
  if (sigRet == 'v') {
    typeSection.push(0x00);
  } else {
    typeSection = typeSection.concat([0x01, typeCodes[sigRet]]);
  }

  // Write the overall length of the type section back into the section header
  // (excepting the 2 bytes for the section id and length)
  typeSection[1] = typeSection.length - 2;

  // Rest of the module is static
  var bytes = new Uint8Array([
    0x00, 0x61, 0x73, 0x6d, // magic ("\0asm")
    0x01, 0x00, 0x00, 0x00, // version: 1
  ].concat(typeSection, [
    0x02, 0x07, // import section
      // (import "e" "f" (func 0 (type 0)))
      0x01, 0x01, 0x65, 0x01, 0x66, 0x00, 0x00,
    0x07, 0x05, // export section
      // (export "f" (func 0 (type 0)))
      0x01, 0x01, 0x66, 0x00, 0x00,
  ]));

   // We can compile this wasm module synchronously because it is very small.
  // This accepts an import (at "e.f"), that it reroutes to an export (at "f")
  var module = new WebAssembly.Module(bytes);
  var instance = new WebAssembly.Instance(module, {
    'e': {
      'f': func
    }
  });
  var wrappedFunc = instance.exports['f'];
  return wrappedFunc;
}

var freeTableIndexes = [];

// Weak map of functions in the table to their indexes, created on first use.
var functionsInTableMap;

function getEmptyTableSlot() {
  // Reuse a free index if there is one, otherwise grow.
  if (freeTableIndexes.length) {
    return freeTableIndexes.pop();
  }
  // Grow the table
  try {
    wasmTable.grow(1);
  } catch (err) {
    if (!(err instanceof RangeError)) {
      throw err;
    }
    throw 'Unable to grow wasm table. Set ALLOW_TABLE_GROWTH.';
  }
  return wasmTable.length - 1;
}

// Add a wasm function to the table.
function addFunctionWasm(func, sig) {
  // Check if the function is already in the table, to ensure each function
  // gets a unique index. First, create the map if this is the first use.
  if (!functionsInTableMap) {
    functionsInTableMap = new WeakMap();
    for (var i = 0; i < wasmTable.length; i++) {
      var item = wasmTable.get(i);
      // Ignore null values.
      if (item) {
        functionsInTableMap.set(item, i);
      }
    }
  }
  if (functionsInTableMap.has(func)) {
    return functionsInTableMap.get(func);
  }

  // It's not in the table, add it now.

  var ret = getEmptyTableSlot();

  // Set the new value.
  try {
    // Attempting to call this with JS function will cause of table.set() to fail
    wasmTable.set(ret, func);
  } catch (err) {
    if (!(err instanceof TypeError)) {
      throw err;
    }
    assert(typeof sig !== 'undefined', 'Missing signature argument to addFunction: ' + func);
    var wrapped = convertJsFunctionToWasm(func, sig);
    wasmTable.set(ret, wrapped);
  }

  functionsInTableMap.set(func, ret);

  return ret;
}

function removeFunction(index) {
  functionsInTableMap.delete(wasmTable.get(index));
  freeTableIndexes.push(index);
}

// 'sig' parameter is required for the llvm backend but only when func is not
// already a WebAssembly function.
function addFunction(func, sig) {
  assert(typeof func !== 'undefined');

  return addFunctionWasm(func, sig);
}

// end include: runtime_functions.js
// include: runtime_debug.js


// end include: runtime_debug.js
function makeBigInt(low, high, unsigned) {
  return unsigned ? ((+((low>>>0)))+((+((high>>>0)))*4294967296.0)) : ((+((low>>>0)))+((+((high|0)))*4294967296.0));
}

var tempRet0 = 0;

var setTempRet0 = function(value) {
  tempRet0 = value;
};

var getTempRet0 = function() {
  return tempRet0;
};

function getCompilerSetting(name) {
  throw 'You must build with -s RETAIN_COMPILER_SETTINGS=1 for getCompilerSetting or emscripten_get_compiler_setting to work';
}



// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

var wasmBinary;if (Module['wasmBinary']) wasmBinary = Module['wasmBinary'];if (!Object.getOwnPropertyDescriptor(Module, 'wasmBinary')) {
  Object.defineProperty(Module, 'wasmBinary', {
    configurable: true,
    get: function() {
      abort('Module.wasmBinary has been replaced with plain wasmBinary (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}
var noExitRuntime;if (Module['noExitRuntime']) noExitRuntime = Module['noExitRuntime'];if (!Object.getOwnPropertyDescriptor(Module, 'noExitRuntime')) {
  Object.defineProperty(Module, 'noExitRuntime', {
    configurable: true,
    get: function() {
      abort('Module.noExitRuntime has been replaced with plain noExitRuntime (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}

if (typeof WebAssembly !== 'object') {
  abort('no native wasm support detected');
}

// include: runtime_safe_heap.js


// In MINIMAL_RUNTIME, setValue() and getValue() are only available when building with safe heap enabled, for heap safety checking.
// In traditional runtime, setValue() and getValue() are always available (although their use is highly discouraged due to perf penalties)

/** @param {number} ptr
    @param {number} value
    @param {string} type
    @param {number|boolean=} noSafe */
function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': HEAP8[((ptr)>>0)] = value; break;
      case 'i8': HEAP8[((ptr)>>0)] = value; break;
      case 'i16': HEAP16[((ptr)>>1)] = value; break;
      case 'i32': HEAP32[((ptr)>>2)] = value; break;
      case 'i64': (tempI64 = [value>>>0,(tempDouble=value,(+(Math.abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math.min((+(Math.floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math.ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],HEAP32[((ptr)>>2)] = tempI64[0],HEAP32[(((ptr)+(4))>>2)] = tempI64[1]); break;
      case 'float': HEAPF32[((ptr)>>2)] = value; break;
      case 'double': HEAPF64[((ptr)>>3)] = value; break;
      default: abort('invalid type for setValue: ' + type);
    }
}

/** @param {number} ptr
    @param {string} type
    @param {number|boolean=} noSafe */
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': return HEAP8[((ptr)>>0)];
      case 'i8': return HEAP8[((ptr)>>0)];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP32[((ptr)>>2)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      default: abort('invalid type for getValue: ' + type);
    }
  return null;
}

// end include: runtime_safe_heap.js
// Wasm globals

var wasmMemory;

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  var func = Module['_' + ident]; // closure exported function
  assert(func, 'Cannot call unknown function ' + ident + ', make sure it is exported');
  return func;
}

// C calling interface.
/** @param {string|null=} returnType
    @param {Array=} argTypes
    @param {Arguments|Array=} args
    @param {Object=} opts */
function ccall(ident, returnType, argTypes, args, opts) {
  // For fast lookup of conversion functions
  var toC = {
    'string': function(str) {
      var ret = 0;
      if (str !== null && str !== undefined && str !== 0) { // null string
        // at most 4 bytes per UTF-8 code point, +1 for the trailing '\0'
        var len = (str.length << 2) + 1;
        ret = stackAlloc(len);
        stringToUTF8(str, ret, len);
      }
      return ret;
    },
    'array': function(arr) {
      var ret = stackAlloc(arr.length);
      writeArrayToMemory(arr, ret);
      return ret;
    }
  };

  function convertReturnValue(ret) {
    if (returnType === 'string') return UTF8ToString(ret);
    if (returnType === 'boolean') return Boolean(ret);
    return ret;
  }

  var func = getCFunc(ident);
  var cArgs = [];
  var stack = 0;
  assert(returnType !== 'array', 'Return type should not be "array".');
  if (args) {
    for (var i = 0; i < args.length; i++) {
      var converter = toC[argTypes[i]];
      if (converter) {
        if (stack === 0) stack = stackSave();
        cArgs[i] = converter(args[i]);
      } else {
        cArgs[i] = args[i];
      }
    }
  }
  var ret = func.apply(null, cArgs);

  ret = convertReturnValue(ret);
  if (stack !== 0) stackRestore(stack);
  return ret;
}

/** @param {string=} returnType
    @param {Array=} argTypes
    @param {Object=} opts */
function cwrap(ident, returnType, argTypes, opts) {
  return function() {
    return ccall(ident, returnType, argTypes, arguments, opts);
  }
}

// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call

// allocate(): This is for internal use. You can use it yourself as well, but the interface
//             is a little tricky (see docs right below). The reason is that it is optimized
//             for multiple syntaxes to save space in generated code. So you should
//             normally not use allocate(), and instead allocate memory using _malloc(),
//             initialize it with setValue(), and so forth.
// @slab: An array of data.
// @allocator: How to allocate memory, see ALLOC_*
/** @type {function((Uint8Array|Array<number>), number)} */
function allocate(slab, allocator) {
  var ret;
  assert(typeof allocator === 'number', 'allocate no longer takes a type argument')
  assert(typeof slab !== 'number', 'allocate no longer takes a number as arg0')

  if (allocator == ALLOC_STACK) {
    ret = stackAlloc(slab.length);
  } else {
    ret = _malloc(slab.length);
  }

  if (slab.subarray || slab.slice) {
    HEAPU8.set(/** @type {!Uint8Array} */(slab), ret);
  } else {
    HEAPU8.set(new Uint8Array(slab), ret);
  }
  return ret;
}

// include: runtime_strings.js


// runtime_strings.js: Strings related runtime functions that are part of both MINIMAL_RUNTIME and regular runtime.

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the given array that contains uint8 values, returns
// a copy of that string as a Javascript String object.

var UTF8Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf8') : undefined;

/**
 * @param {number} idx
 * @param {number=} maxBytesToRead
 * @return {string}
 */
function UTF8ArrayToString(heap, idx, maxBytesToRead) {
  var endIdx = idx + maxBytesToRead;
  var endPtr = idx;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  // (As a tiny code save trick, compare endPtr against endIdx using a negation, so that undefined means Infinity)
  while (heap[endPtr] && !(endPtr >= endIdx)) ++endPtr;

  if (endPtr - idx > 16 && heap.subarray && UTF8Decoder) {
    return UTF8Decoder.decode(heap.subarray(idx, endPtr));
  } else {
    var str = '';
    // If building with TextDecoder, we have already computed the string length above, so test loop end condition against that
    while (idx < endPtr) {
      // For UTF8 byte structure, see:
      // http://en.wikipedia.org/wiki/UTF-8#Description
      // https://www.ietf.org/rfc/rfc2279.txt
      // https://tools.ietf.org/html/rfc3629
      var u0 = heap[idx++];
      if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
      var u1 = heap[idx++] & 63;
      if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
      var u2 = heap[idx++] & 63;
      if ((u0 & 0xF0) == 0xE0) {
        u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
      } else {
        if ((u0 & 0xF8) != 0xF0) warnOnce('Invalid UTF-8 leading byte 0x' + u0.toString(16) + ' encountered when deserializing a UTF-8 string on the asm.js/wasm heap to a JS string!');
        u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heap[idx++] & 63);
      }

      if (u0 < 0x10000) {
        str += String.fromCharCode(u0);
      } else {
        var ch = u0 - 0x10000;
        str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
      }
    }
  }
  return str;
}

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the emscripten HEAP, returns a
// copy of that string as a Javascript String object.
// maxBytesToRead: an optional length that specifies the maximum number of bytes to read. You can omit
//                 this parameter to scan the string until the first \0 byte. If maxBytesToRead is
//                 passed, and the string at [ptr, ptr+maxBytesToReadr[ contains a null byte in the
//                 middle, then the string will cut short at that byte index (i.e. maxBytesToRead will
//                 not produce a string of exact length [ptr, ptr+maxBytesToRead[)
//                 N.B. mixing frequent uses of UTF8ToString() with and without maxBytesToRead may
//                 throw JS JIT optimizations off, so it is worth to consider consistently using one
//                 style or the other.
/**
 * @param {number} ptr
 * @param {number=} maxBytesToRead
 * @return {string}
 */
function UTF8ToString(ptr, maxBytesToRead) {
  return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : '';
}

// Copies the given Javascript String object 'str' to the given byte array at address 'outIdx',
// encoded in UTF8 form and null-terminated. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   heap: the array to copy to. Each index in this array is assumed to be one 8-byte element.
//   outIdx: The starting offset in the array to begin the copying.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array.
//                    This count should include the null terminator,
//                    i.e. if maxBytesToWrite=1, only the null terminator will be written and nothing else.
//                    maxBytesToWrite=0 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8Array(str, heap, outIdx, maxBytesToWrite) {
  if (!(maxBytesToWrite > 0)) // Parameter maxBytesToWrite is not optional. Negative values, 0, null, undefined and false each don't write out any bytes.
    return 0;

  var startIdx = outIdx;
  var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description and https://www.ietf.org/rfc/rfc2279.txt and https://tools.ietf.org/html/rfc3629
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) {
      var u1 = str.charCodeAt(++i);
      u = 0x10000 + ((u & 0x3FF) << 10) | (u1 & 0x3FF);
    }
    if (u <= 0x7F) {
      if (outIdx >= endIdx) break;
      heap[outIdx++] = u;
    } else if (u <= 0x7FF) {
      if (outIdx + 1 >= endIdx) break;
      heap[outIdx++] = 0xC0 | (u >> 6);
      heap[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0xFFFF) {
      if (outIdx + 2 >= endIdx) break;
      heap[outIdx++] = 0xE0 | (u >> 12);
      heap[outIdx++] = 0x80 | ((u >> 6) & 63);
      heap[outIdx++] = 0x80 | (u & 63);
    } else {
      if (outIdx + 3 >= endIdx) break;
      if (u >= 0x200000) warnOnce('Invalid Unicode code point 0x' + u.toString(16) + ' encountered when serializing a JS string to an UTF-8 string on the asm.js/wasm heap! (Valid unicode code points should be in range 0-0x1FFFFF).');
      heap[outIdx++] = 0xF0 | (u >> 18);
      heap[outIdx++] = 0x80 | ((u >> 12) & 63);
      heap[outIdx++] = 0x80 | ((u >> 6) & 63);
      heap[outIdx++] = 0x80 | (u & 63);
    }
  }
  // Null-terminate the pointer to the buffer.
  heap[outIdx] = 0;
  return outIdx - startIdx;
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF8 form. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8(str, outPtr, maxBytesToWrite) {
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
  return stringToUTF8Array(str, HEAPU8,outPtr, maxBytesToWrite);
}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF8 byte array, EXCLUDING the null terminator byte.
function lengthBytesUTF8(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) u = 0x10000 + ((u & 0x3FF) << 10) | (str.charCodeAt(++i) & 0x3FF);
    if (u <= 0x7F) ++len;
    else if (u <= 0x7FF) len += 2;
    else if (u <= 0xFFFF) len += 3;
    else len += 4;
  }
  return len;
}

// end include: runtime_strings.js
// include: runtime_strings_extra.js


// runtime_strings_extra.js: Strings related runtime functions that are available only in regular runtime.

// Given a pointer 'ptr' to a null-terminated ASCII-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

function AsciiToString(ptr) {
  var str = '';
  while (1) {
    var ch = HEAPU8[((ptr++)>>0)];
    if (!ch) return str;
    str += String.fromCharCode(ch);
  }
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in ASCII form. The copy will require at most str.length+1 bytes of space in the HEAP.

function stringToAscii(str, outPtr) {
  return writeAsciiToMemory(str, outPtr, false);
}

// Given a pointer 'ptr' to a null-terminated UTF16LE-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

var UTF16Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf-16le') : undefined;

function UTF16ToString(ptr, maxBytesToRead) {
  assert(ptr % 2 == 0, 'Pointer passed to UTF16ToString must be aligned to two bytes!');
  var endPtr = ptr;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  var idx = endPtr >> 1;
  var maxIdx = idx + maxBytesToRead / 2;
  // If maxBytesToRead is not passed explicitly, it will be undefined, and this
  // will always evaluate to true. This saves on code size.
  while (!(idx >= maxIdx) && HEAPU16[idx]) ++idx;
  endPtr = idx << 1;

  if (endPtr - ptr > 32 && UTF16Decoder) {
    return UTF16Decoder.decode(HEAPU8.subarray(ptr, endPtr));
  } else {
    var str = '';

    // If maxBytesToRead is not passed explicitly, it will be undefined, and the for-loop's condition
    // will always evaluate to true. The loop is then terminated on the first null char.
    for (var i = 0; !(i >= maxBytesToRead / 2); ++i) {
      var codeUnit = HEAP16[(((ptr)+(i*2))>>1)];
      if (codeUnit == 0) break;
      // fromCharCode constructs a character from a UTF-16 code unit, so we can pass the UTF16 string right through.
      str += String.fromCharCode(codeUnit);
    }

    return str;
  }
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF16 form. The copy will require at most str.length*4+2 bytes of space in the HEAP.
// Use the function lengthBytesUTF16() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=2, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<2 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF16(str, outPtr, maxBytesToWrite) {
  assert(outPtr % 2 == 0, 'Pointer passed to stringToUTF16 must be aligned to two bytes!');
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF16(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 2) return 0;
  maxBytesToWrite -= 2; // Null terminator.
  var startPtr = outPtr;
  var numCharsToWrite = (maxBytesToWrite < str.length*2) ? (maxBytesToWrite / 2) : str.length;
  for (var i = 0; i < numCharsToWrite; ++i) {
    // charCodeAt returns a UTF-16 encoded code unit, so it can be directly written to the HEAP.
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    HEAP16[((outPtr)>>1)] = codeUnit;
    outPtr += 2;
  }
  // Null-terminate the pointer to the HEAP.
  HEAP16[((outPtr)>>1)] = 0;
  return outPtr - startPtr;
}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF16(str) {
  return str.length*2;
}

function UTF32ToString(ptr, maxBytesToRead) {
  assert(ptr % 4 == 0, 'Pointer passed to UTF32ToString must be aligned to four bytes!');
  var i = 0;

  var str = '';
  // If maxBytesToRead is not passed explicitly, it will be undefined, and this
  // will always evaluate to true. This saves on code size.
  while (!(i >= maxBytesToRead / 4)) {
    var utf32 = HEAP32[(((ptr)+(i*4))>>2)];
    if (utf32 == 0) break;
    ++i;
    // Gotcha: fromCharCode constructs a character from a UTF-16 encoded code (pair), not from a Unicode code point! So encode the code point to UTF-16 for constructing.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    if (utf32 >= 0x10000) {
      var ch = utf32 - 0x10000;
      str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
    } else {
      str += String.fromCharCode(utf32);
    }
  }
  return str;
}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF32 form. The copy will require at most str.length*4+4 bytes of space in the HEAP.
// Use the function lengthBytesUTF32() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=4, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<4 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF32(str, outPtr, maxBytesToWrite) {
  assert(outPtr % 4 == 0, 'Pointer passed to stringToUTF32 must be aligned to four bytes!');
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF32(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 4) return 0;
  var startPtr = outPtr;
  var endPtr = startPtr + maxBytesToWrite - 4;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) {
      var trailSurrogate = str.charCodeAt(++i);
      codeUnit = 0x10000 + ((codeUnit & 0x3FF) << 10) | (trailSurrogate & 0x3FF);
    }
    HEAP32[((outPtr)>>2)] = codeUnit;
    outPtr += 4;
    if (outPtr + 4 > endPtr) break;
  }
  // Null-terminate the pointer to the HEAP.
  HEAP32[((outPtr)>>2)] = 0;
  return outPtr - startPtr;
}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF32(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i);
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) ++i; // possibly a lead surrogate, so skip over the tail surrogate.
    len += 4;
  }

  return len;
}

// Allocate heap space for a JS string, and write it there.
// It is the responsibility of the caller to free() that memory.
function allocateUTF8(str) {
  var size = lengthBytesUTF8(str) + 1;
  var ret = _malloc(size);
  if (ret) stringToUTF8Array(str, HEAP8, ret, size);
  return ret;
}

// Allocate stack space for a JS string, and write it there.
function allocateUTF8OnStack(str) {
  var size = lengthBytesUTF8(str) + 1;
  var ret = stackAlloc(size);
  stringToUTF8Array(str, HEAP8, ret, size);
  return ret;
}

// Deprecated: This function should not be called because it is unsafe and does not provide
// a maximum length limit of how many bytes it is allowed to write. Prefer calling the
// function stringToUTF8Array() instead, which takes in a maximum length that can be used
// to be secure from out of bounds writes.
/** @deprecated
    @param {boolean=} dontAddNull */
function writeStringToMemory(string, buffer, dontAddNull) {
  warnOnce('writeStringToMemory is deprecated and should not be called! Use stringToUTF8() instead!');

  var /** @type {number} */ lastChar, /** @type {number} */ end;
  if (dontAddNull) {
    // stringToUTF8Array always appends null. If we don't want to do that, remember the
    // character that existed at the location where the null will be placed, and restore
    // that after the write (below).
    end = buffer + lengthBytesUTF8(string);
    lastChar = HEAP8[end];
  }
  stringToUTF8(string, buffer, Infinity);
  if (dontAddNull) HEAP8[end] = lastChar; // Restore the value under the null character.
}

function writeArrayToMemory(array, buffer) {
  assert(array.length >= 0, 'writeArrayToMemory array must have a length (should be an array or typed array)')
  HEAP8.set(array, buffer);
}

/** @param {boolean=} dontAddNull */
function writeAsciiToMemory(str, buffer, dontAddNull) {
  for (var i = 0; i < str.length; ++i) {
    assert(str.charCodeAt(i) === str.charCodeAt(i)&0xff);
    HEAP8[((buffer++)>>0)] = str.charCodeAt(i);
  }
  // Null-terminate the pointer to the HEAP.
  if (!dontAddNull) HEAP8[((buffer)>>0)] = 0;
}

// end include: runtime_strings_extra.js
// Memory management

function alignUp(x, multiple) {
  if (x % multiple > 0) {
    x += multiple - (x % multiple);
  }
  return x;
}

var HEAP,
/** @type {ArrayBuffer} */
  buffer,
/** @type {Int8Array} */
  HEAP8,
/** @type {Uint8Array} */
  HEAPU8,
/** @type {Int16Array} */
  HEAP16,
/** @type {Uint16Array} */
  HEAPU16,
/** @type {Int32Array} */
  HEAP32,
/** @type {Uint32Array} */
  HEAPU32,
/** @type {Float32Array} */
  HEAPF32,
/** @type {Float64Array} */
  HEAPF64;

function updateGlobalBufferAndViews(buf) {
  buffer = buf;
  Module['HEAP8'] = HEAP8 = new Int8Array(buf);
  Module['HEAP16'] = HEAP16 = new Int16Array(buf);
  Module['HEAP32'] = HEAP32 = new Int32Array(buf);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buf);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buf);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buf);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buf);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buf);
}

var TOTAL_STACK = 5242880;
if (Module['TOTAL_STACK']) assert(TOTAL_STACK === Module['TOTAL_STACK'], 'the stack size can no longer be determined at runtime')

var INITIAL_MEMORY = Module['INITIAL_MEMORY'] || 134217728;if (!Object.getOwnPropertyDescriptor(Module, 'INITIAL_MEMORY')) {
  Object.defineProperty(Module, 'INITIAL_MEMORY', {
    configurable: true,
    get: function() {
      abort('Module.INITIAL_MEMORY has been replaced with plain INITIAL_MEMORY (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)')
    }
  });
}

assert(INITIAL_MEMORY >= TOTAL_STACK, 'INITIAL_MEMORY should be larger than TOTAL_STACK, was ' + INITIAL_MEMORY + '! (TOTAL_STACK=' + TOTAL_STACK + ')');

// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(typeof Int32Array !== 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray !== undefined && Int32Array.prototype.set !== undefined,
       'JS engine does not provide full typed array support');

// If memory is defined in wasm, the user can't provide it.
assert(!Module['wasmMemory'], 'Use of `wasmMemory` detected.  Use -s IMPORTED_MEMORY to define wasmMemory externally');
assert(INITIAL_MEMORY == 134217728, 'Detected runtime INITIAL_MEMORY setting.  Use -s IMPORTED_MEMORY to define wasmMemory dynamically');

// include: runtime_init_table.js
// In regular non-RELOCATABLE mode the table is exported
// from the wasm module and this will be assigned once
// the exports are available.
var wasmTable;

// end include: runtime_init_table.js
// include: runtime_stack_check.js


// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
  assert((max & 3) == 0);
  // The stack grows downwards
  HEAPU32[(max >> 2)+1] = 0x2135467;
  HEAPU32[(max >> 2)+2] = 0x89BACDFE;
  // Also test the global address 0 for integrity.
  HEAP32[0] = 0x63736d65; /* 'emsc' */
}

function checkStackCookie() {
  if (ABORT) return;
  var max = _emscripten_stack_get_end();
  var cookie1 = HEAPU32[(max >> 2)+1];
  var cookie2 = HEAPU32[(max >> 2)+2];
  if (cookie1 != 0x2135467 || cookie2 != 0x89BACDFE) {
    abort('Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x2135467, but received 0x' + cookie2.toString(16) + ' ' + cookie1.toString(16));
  }
  // Also test the global address 0 for integrity.
  if (HEAP32[0] !== 0x63736d65 /* 'emsc' */) abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
}

// end include: runtime_stack_check.js
// include: runtime_assertions.js


// Endianness check (note: assumes compiler arch was little-endian)
(function() {
  var h16 = new Int16Array(1);
  var h8 = new Int8Array(h16.buffer);
  h16[0] = 0x6373;
  if (h8[0] !== 0x73 || h8[1] !== 0x63) throw 'Runtime error: expected the system to be little-endian!';
})();

function abortFnPtrError(ptr, sig) {
	abort("Invalid function pointer " + ptr + " called with signature '" + sig + "'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this). Build with ASSERTIONS=2 for more info.");
}

// end include: runtime_assertions.js
var __ATPRERUN__  = []; // functions called before the runtime is initialized
var __ATINIT__    = []; // functions called during startup
var __ATMAIN__    = []; // functions called when main() is to be run
var __ATEXIT__    = []; // functions called during shutdown
var __ATPOSTRUN__ = []; // functions called after the main() is called

var runtimeInitialized = false;
var runtimeExited = false;

__ATINIT__.push({ func: function() { ___wasm_call_ctors() } });

function preRun() {

  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }

  callRuntimeCallbacks(__ATPRERUN__);
}

function initRuntime() {
  checkStackCookie();
  assert(!runtimeInitialized);
  runtimeInitialized = true;
  
  callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
  checkStackCookie();
  
  callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
  checkStackCookie();
  runtimeExited = true;
}

function postRun() {
  checkStackCookie();

  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }

  callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
  __ATPRERUN__.unshift(cb);
}

function addOnInit(cb) {
  __ATINIT__.unshift(cb);
}

function addOnPreMain(cb) {
  __ATMAIN__.unshift(cb);
}

function addOnExit(cb) {
}

function addOnPostRun(cb) {
  __ATPOSTRUN__.unshift(cb);
}

// include: runtime_math.js


// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/imul

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/fround

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/clz32

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/trunc

assert(Math.imul, 'This browser does not support Math.imul(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');
assert(Math.fround, 'This browser does not support Math.fround(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');
assert(Math.clz32, 'This browser does not support Math.clz32(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');
assert(Math.trunc, 'This browser does not support Math.trunc(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');

// end include: runtime_math.js
// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// Module.preRun (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
var runDependencyTracking = {};

function getUniqueRunDependency(id) {
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
}

function addRunDependency(id) {
  runDependencies++;

  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }

  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval !== 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(function() {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            err('still waiting on run dependencies:');
          }
          err('dependency: ' + dep);
        }
        if (shown) {
          err('(end of list)');
        }
      }, 10000);
    }
  } else {
    err('warning: run dependency added without ID');
  }
}

function removeRunDependency(id) {
  runDependencies--;

  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }

  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    err('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data

/** @param {string|number=} what */
function abort(what) {
  if (Module['onAbort']) {
    Module['onAbort'](what);
  }

  what += '';
  err(what);

  ABORT = true;
  EXITSTATUS = 1;

  var output = 'abort(' + what + ') at ' + stackTrace();
  what = output;

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  var e = new WebAssembly.RuntimeError(what);

  readyPromiseReject(e);
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

// {{MEM_INITIALIZER}}

// include: memoryprofiler.js


// end include: memoryprofiler.js
// show errors on likely calls to FS when it was not included
var FS = {
  error: function() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1');
  },
  init: function() { FS.error() },
  createDataFile: function() { FS.error() },
  createPreloadedFile: function() { FS.error() },
  createLazyFile: function() { FS.error() },
  open: function() { FS.error() },
  mkdev: function() { FS.error() },
  registerDevice: function() { FS.error() },
  analyzePath: function() { FS.error() },
  loadFilesFromDB: function() { FS.error() },

  ErrnoError: function ErrnoError() { FS.error() },
};
Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;

// include: URIUtils.js


function hasPrefix(str, prefix) {
  return String.prototype.startsWith ?
      str.startsWith(prefix) :
      str.indexOf(prefix) === 0;
}

// Prefix of data URIs emitted by SINGLE_FILE and related options.
var dataURIPrefix = 'data:application/octet-stream;base64,';

// Indicates whether filename is a base64 data URI.
function isDataURI(filename) {
  return hasPrefix(filename, dataURIPrefix);
}

var fileURIPrefix = "file://";

// Indicates whether filename is delivered via file protocol (as opposed to http/https)
function isFileURI(filename) {
  return hasPrefix(filename, fileURIPrefix);
}

// end include: URIUtils.js
function createExportWrapper(name, fixedasm) {
  return function() {
    var displayName = name;
    var asm = fixedasm;
    if (!fixedasm) {
      asm = Module['asm'];
    }
    assert(runtimeInitialized, 'native function `' + displayName + '` called before runtime initialization');
    assert(!runtimeExited, 'native function `' + displayName + '` called after runtime exit (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
    if (!asm[name]) {
      assert(asm[name], 'exported native function `' + displayName + '` not found');
    }
    return asm[name].apply(null, arguments);
  };
}

var wasmBinaryFile = 'canvaskit.wasm';
if (!isDataURI(wasmBinaryFile)) {
  wasmBinaryFile = locateFile(wasmBinaryFile);
}

function getBinary(file) {
  try {
    if (file == wasmBinaryFile && wasmBinary) {
      return new Uint8Array(wasmBinary);
    }
    if (readBinary) {
      return readBinary(file);
    } else {
      throw "both async and sync fetching of the wasm failed";
    }
  }
  catch (err) {
    abort(err);
  }
}

function getBinaryPromise() {
  // If we don't have the binary yet, try to to load it asynchronously.
  // Fetch has some additional restrictions over XHR, like it can't be used on a file:// url.
  // See https://github.com/github/fetch/pull/92#issuecomment-140665932
  // Cordova or Electron apps are typically loaded from a file:// url.
  // So use fetch if it is available and the url is not a file, otherwise fall back to XHR.
  if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
    if (typeof fetch === 'function'
      && !isFileURI(wasmBinaryFile)
    ) {
      return fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function(response) {
        if (!response['ok']) {
          throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
        }
        return response['arrayBuffer']();
      }).catch(function () {
          return getBinary(wasmBinaryFile);
      });
    }
    else {
      if (readAsync) {
        // fetch is not available or url is file => try XHR (readAsync uses XHR internally)
        return new Promise(function(resolve, reject) {
          readAsync(wasmBinaryFile, function(response) { resolve(new Uint8Array(/** @type{!ArrayBuffer} */(response))) }, reject)
        });
      }
    }
  }
    
  // Otherwise, getBinary should be able to get it synchronously
  return Promise.resolve().then(function() { return getBinary(wasmBinaryFile); });
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
function createWasm() {
  // prepare imports
  var info = {
    'env': asmLibraryArg,
    'wasi_snapshot_preview1': asmLibraryArg,
  };
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    var exports = instance.exports;

    Module['asm'] = exports;

    wasmMemory = Module['asm']['memory'];
    assert(wasmMemory, "memory not found in wasm exports");
    // This assertion doesn't hold when emscripten is run in --post-link
    // mode.
    // TODO(sbc): Read INITIAL_MEMORY out of the wasm file in post-link mode.
    //assert(wasmMemory.buffer.byteLength === 134217728);
    updateGlobalBufferAndViews(wasmMemory.buffer);

    wasmTable = Module['asm']['__indirect_function_table'];
    assert(wasmTable, "table not found in wasm exports");

    removeRunDependency('wasm-instantiate');
  }
  // we can't run yet (except in a pthread, where we have a custom sync instantiator)
  addRunDependency('wasm-instantiate');

  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
  function receiveInstantiatedSource(output) {
    // 'output' is a WebAssemblyInstantiatedSource object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above USE_PTHREADS-enabled path.
    receiveInstance(output['instance']);
  }

  function instantiateArrayBuffer(receiver) {
    return getBinaryPromise().then(function(binary) {
      return WebAssembly.instantiate(binary, info);
    }).then(receiver, function(reason) {
      err('failed to asynchronously prepare wasm: ' + reason);

      // Warn on some common problems.
      if (isFileURI(wasmBinaryFile)) {
        err('warning: Loading from a file URI (' + wasmBinaryFile + ') is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing');
      }
      abort(reason);
    });
  }

  // Prefer streaming instantiation if available.
  function instantiateAsync() {
    if (!wasmBinary &&
        typeof WebAssembly.instantiateStreaming === 'function' &&
        !isDataURI(wasmBinaryFile) &&
        // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
        !isFileURI(wasmBinaryFile) &&
        typeof fetch === 'function') {
      return fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function (response) {
        var result = WebAssembly.instantiateStreaming(response, info);
        return result.then(receiveInstantiatedSource, function(reason) {
            // We expect the most common failure cause to be a bad MIME type for the binary,
            // in which case falling back to ArrayBuffer instantiation should work.
            err('wasm streaming compile failed: ' + reason);
            err('falling back to ArrayBuffer instantiation');
            return instantiateArrayBuffer(receiveInstantiatedSource);
          });
      });
    } else {
      return instantiateArrayBuffer(receiveInstantiatedSource);
    }
  }

  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to run the instantiation parallel
  // to any other async startup actions they are performing.
  if (Module['instantiateWasm']) {
    try {
      var exports = Module['instantiateWasm'](info, receiveInstance);
      return exports;
    } catch(e) {
      err('Module.instantiateWasm callback failed with error: ' + e);
      return false;
    }
  }

  // If instantiation fails, reject the module ready promise.
  instantiateAsync().catch(readyPromiseReject);
  return {}; // no exports yet; we'll fill them in later
}

// Globals used by JS i64 conversions (see makeSetValue)
var tempDouble;
var tempI64;

// === Body ===

var ASM_CONSTS = {
  
};






  function abortStackOverflow(allocSize) {
      abort('Stack overflow! Attempted to allocate ' + allocSize + ' bytes on the stack, but stack has only ' + (_emscripten_stack_get_free() + allocSize) + ' bytes available!');
    }

  function callRuntimeCallbacks(callbacks) {
      while(callbacks.length > 0) {
        var callback = callbacks.shift();
        if (typeof callback == 'function') {
          callback(Module); // Pass the module as the first argument.
          continue;
        }
        var func = callback.func;
        if (typeof func === 'number') {
          if (callback.arg === undefined) {
            wasmTable.get(func)();
          } else {
            wasmTable.get(func)(callback.arg);
          }
        } else {
          func(callback.arg === undefined ? null : callback.arg);
        }
      }
    }

  function demangle(func) {
      // If demangle has failed before, stop demangling any further function names
      // This avoids an infinite recursion with malloc()->abort()->stackTrace()->demangle()->malloc()->...
      demangle.recursionGuard = (demangle.recursionGuard|0)+1;
      if (demangle.recursionGuard > 1) return func;
      var __cxa_demangle_func = Module['___cxa_demangle'] || Module['__cxa_demangle'];
      assert(__cxa_demangle_func);
      var stackTop = stackSave();
      try {
        var s = func;
        if (s.startsWith('__Z'))
          s = s.substr(1);
        var len = lengthBytesUTF8(s)+1;
        var buf = stackAlloc(len);
        stringToUTF8(s, buf, len);
        var status = stackAlloc(4);
        var ret = __cxa_demangle_func(buf, 0, 0, status);
        if (HEAP32[((status)>>2)] === 0 && ret) {
          return UTF8ToString(ret);
        }
        // otherwise, libcxxabi failed
      } catch(e) {
      } finally {
        _free(ret);
        stackRestore(stackTop);
        if (demangle.recursionGuard < 2) --demangle.recursionGuard;
      }
      // failure when using libcxxabi, don't demangle
      return func;
    }

  function demangleAll(text) {
      var regex =
        /\b_Z[\w\d_]+/g;
      return text.replace(regex,
        function(x) {
          var y = demangle(x);
          return x === y ? x : (y + ' [' + x + ']');
        });
    }

  function jsStackTrace() {
      var error = new Error();
      if (!error.stack) {
        // IE10+ special cases: It does have callstack info, but it is only populated if an Error object is thrown,
        // so try that as a special-case.
        try {
          throw new Error();
        } catch(e) {
          error = e;
        }
        if (!error.stack) {
          return '(no stack trace available)';
        }
      }
      return error.stack.toString();
    }

  function stackTrace() {
      var js = jsStackTrace();
      if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
      return demangleAll(js);
    }

  var ExceptionInfoAttrs={DESTRUCTOR_OFFSET:0,REFCOUNT_OFFSET:4,TYPE_OFFSET:8,CAUGHT_OFFSET:12,RETHROWN_OFFSET:13,SIZE:16};
  function ___cxa_allocate_exception(size) {
      // Thrown object is prepended by exception metadata block
      return _malloc(size + ExceptionInfoAttrs.SIZE) + ExceptionInfoAttrs.SIZE;
    }

  function _atexit(func, arg) {
    }
  function ___cxa_atexit(a0,a1
  ) {
  return _atexit(a0,a1);
  }

  function ExceptionInfo(excPtr) {
      this.excPtr = excPtr;
      this.ptr = excPtr - ExceptionInfoAttrs.SIZE;
  
      this.set_type = function(type) {
        HEAP32[(((this.ptr)+(ExceptionInfoAttrs.TYPE_OFFSET))>>2)] = type;
      };
  
      this.get_type = function() {
        return HEAP32[(((this.ptr)+(ExceptionInfoAttrs.TYPE_OFFSET))>>2)];
      };
  
      this.set_destructor = function(destructor) {
        HEAP32[(((this.ptr)+(ExceptionInfoAttrs.DESTRUCTOR_OFFSET))>>2)] = destructor;
      };
  
      this.get_destructor = function() {
        return HEAP32[(((this.ptr)+(ExceptionInfoAttrs.DESTRUCTOR_OFFSET))>>2)];
      };
  
      this.set_refcount = function(refcount) {
        HEAP32[(((this.ptr)+(ExceptionInfoAttrs.REFCOUNT_OFFSET))>>2)] = refcount;
      };
  
      this.set_caught = function (caught) {
        caught = caught ? 1 : 0;
        HEAP8[(((this.ptr)+(ExceptionInfoAttrs.CAUGHT_OFFSET))>>0)] = caught;
      };
  
      this.get_caught = function () {
        return HEAP8[(((this.ptr)+(ExceptionInfoAttrs.CAUGHT_OFFSET))>>0)] != 0;
      };
  
      this.set_rethrown = function (rethrown) {
        rethrown = rethrown ? 1 : 0;
        HEAP8[(((this.ptr)+(ExceptionInfoAttrs.RETHROWN_OFFSET))>>0)] = rethrown;
      };
  
      this.get_rethrown = function () {
        return HEAP8[(((this.ptr)+(ExceptionInfoAttrs.RETHROWN_OFFSET))>>0)] != 0;
      };
  
      // Initialize native structure fields. Should be called once after allocated.
      this.init = function(type, destructor) {
        this.set_type(type);
        this.set_destructor(destructor);
        this.set_refcount(0);
        this.set_caught(false);
        this.set_rethrown(false);
      }
  
      this.add_ref = function() {
        var value = HEAP32[(((this.ptr)+(ExceptionInfoAttrs.REFCOUNT_OFFSET))>>2)];
        HEAP32[(((this.ptr)+(ExceptionInfoAttrs.REFCOUNT_OFFSET))>>2)] = value + 1;
      };
  
      // Returns true if last reference released.
      this.release_ref = function() {
        var prev = HEAP32[(((this.ptr)+(ExceptionInfoAttrs.REFCOUNT_OFFSET))>>2)];
        HEAP32[(((this.ptr)+(ExceptionInfoAttrs.REFCOUNT_OFFSET))>>2)] = prev - 1;
        assert(prev > 0);
        return prev === 1;
      };
    }
  
  var exceptionLast=0;
  
  var uncaughtExceptionCount=0;
  function ___cxa_throw(ptr, type, destructor) {
      var info = new ExceptionInfo(ptr);
      // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
      info.init(type, destructor);
      exceptionLast = ptr;
      uncaughtExceptionCount++;
      throw ptr + " - Exception catching is disabled, this exception cannot be caught. Compile with -s DISABLE_EXCEPTION_CATCHING=0 or DISABLE_EXCEPTION_CATCHING=2 to catch.";
    }

  function setErrNo(value) {
      HEAP32[((___errno_location())>>2)] = value;
      return value;
    }
  
  var SYSCALLS={mappings:{},buffers:[null,[],[]],printChar:function(stream, curr) {
        var buffer = SYSCALLS.buffers[stream];
        assert(buffer);
        if (curr === 0 || curr === 10) {
          (stream === 1 ? out : err)(UTF8ArrayToString(buffer, 0));
          buffer.length = 0;
        } else {
          buffer.push(curr);
        }
      },varargs:undefined,get:function() {
        assert(SYSCALLS.varargs != undefined);
        SYSCALLS.varargs += 4;
        var ret = HEAP32[(((SYSCALLS.varargs)-(4))>>2)];
        return ret;
      },getStr:function(ptr) {
        var ret = UTF8ToString(ptr);
        return ret;
      },get64:function(low, high) {
        if (low >= 0) assert(high === 0);
        else assert(high === -1);
        return low;
      }};
  function ___sys_fcntl64(fd, cmd, varargs) {SYSCALLS.varargs = varargs;
  
      return 0;
    }

  function ___sys_fstat64(fd, buf) {
  abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');
  }

  function ___sys_ioctl(fd, op, varargs) {SYSCALLS.varargs = varargs;
  
      return 0;
    }

  function syscallMmap2(addr, len, prot, flags, fd, off) {
      off <<= 12; // undo pgoffset
      var ptr;
      var allocated = false;
  
      // addr argument must be page aligned if MAP_FIXED flag is set.
      if ((flags & 16) !== 0 && (addr % 16384) !== 0) {
        return -28;
      }
  
      // MAP_ANONYMOUS (aka MAP_ANON) isn't actually defined by POSIX spec,
      // but it is widely used way to allocate memory pages on Linux, BSD and Mac.
      // In this case fd argument is ignored.
      if ((flags & 32) !== 0) {
        ptr = _memalign(16384, len);
        if (!ptr) return -48;
        _memset(ptr, 0, len);
        allocated = true;
      } else {
        return -52;
      }
      SYSCALLS.mappings[ptr] = { malloc: ptr, len: len, allocated: allocated, fd: fd, prot: prot, flags: flags, offset: off };
      return ptr;
    }
  function ___sys_mmap2(addr, len, prot, flags, fd, off) {
      return syscallMmap2(addr, len, prot, flags, fd, off);
    }

  function syscallMunmap(addr, len) {
      if ((addr | 0) === -1 || len === 0) {
        return -28;
      }
      // TODO: support unmmap'ing parts of allocations
      var info = SYSCALLS.mappings[addr];
      if (!info) return 0;
      if (len === info.len) {
        // Without FS support, only anonymous mappings are supported.
        assert(SYSCALLS.mappings[addr].flags & 32);
        SYSCALLS.mappings[addr] = null;
        if (info.allocated) {
          _free(info.malloc);
        }
      }
      return 0;
    }
  function ___sys_munmap(addr, len) {
      return syscallMunmap(addr, len);
    }

  function ___sys_open(path, flags, varargs) {SYSCALLS.varargs = varargs;
  
  abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');
  }

  function ___sys_stat64(path, buf) {
  abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');
  }

  var structRegistrations={};
  
  function runDestructors(destructors) {
      while (destructors.length) {
          var ptr = destructors.pop();
          var del = destructors.pop();
          del(ptr);
      }
    }
  
  function simpleReadValueFromPointer(pointer) {
      return this['fromWireType'](HEAPU32[pointer >> 2]);
    }
  
  var awaitingDependencies={};
  
  var registeredTypes={};
  
  var typeDependencies={};
  
  var char_0=48;
  
  var char_9=57;
  function makeLegalFunctionName(name) {
      if (undefined === name) {
          return '_unknown';
      }
      name = name.replace(/[^a-zA-Z0-9_]/g, '$');
      var f = name.charCodeAt(0);
      if (f >= char_0 && f <= char_9) {
          return '_' + name;
      } else {
          return name;
      }
    }
  function createNamedFunction(name, body) {
      name = makeLegalFunctionName(name);
      /*jshint evil:true*/
      return new Function(
          "body",
          "return function " + name + "() {\n" +
          "    \"use strict\";" +
          "    return body.apply(this, arguments);\n" +
          "};\n"
      )(body);
    }
  function extendError(baseErrorType, errorName) {
      var errorClass = createNamedFunction(errorName, function(message) {
          this.name = errorName;
          this.message = message;
  
          var stack = (new Error(message)).stack;
          if (stack !== undefined) {
              this.stack = this.toString() + '\n' +
                  stack.replace(/^Error(:[^\n]*)?\n/, '');
          }
      });
      errorClass.prototype = Object.create(baseErrorType.prototype);
      errorClass.prototype.constructor = errorClass;
      errorClass.prototype.toString = function() {
          if (this.message === undefined) {
              return this.name;
          } else {
              return this.name + ': ' + this.message;
          }
      };
  
      return errorClass;
    }
  var InternalError=undefined;
  function throwInternalError(message) {
      throw new InternalError(message);
    }
  function whenDependentTypesAreResolved(myTypes, dependentTypes, getTypeConverters) {
      myTypes.forEach(function(type) {
          typeDependencies[type] = dependentTypes;
      });
  
      function onComplete(typeConverters) {
          var myTypeConverters = getTypeConverters(typeConverters);
          if (myTypeConverters.length !== myTypes.length) {
              throwInternalError('Mismatched type converter count');
          }
          for (var i = 0; i < myTypes.length; ++i) {
              registerType(myTypes[i], myTypeConverters[i]);
          }
      }
  
      var typeConverters = new Array(dependentTypes.length);
      var unregisteredTypes = [];
      var registered = 0;
      dependentTypes.forEach(function(dt, i) {
          if (registeredTypes.hasOwnProperty(dt)) {
              typeConverters[i] = registeredTypes[dt];
          } else {
              unregisteredTypes.push(dt);
              if (!awaitingDependencies.hasOwnProperty(dt)) {
                  awaitingDependencies[dt] = [];
              }
              awaitingDependencies[dt].push(function() {
                  typeConverters[i] = registeredTypes[dt];
                  ++registered;
                  if (registered === unregisteredTypes.length) {
                      onComplete(typeConverters);
                  }
              });
          }
      });
      if (0 === unregisteredTypes.length) {
          onComplete(typeConverters);
      }
    }
  function __embind_finalize_value_object(structType) {
      var reg = structRegistrations[structType];
      delete structRegistrations[structType];
  
      var rawConstructor = reg.rawConstructor;
      var rawDestructor = reg.rawDestructor;
      var fieldRecords = reg.fields;
      var fieldTypes = fieldRecords.map(function(field) { return field.getterReturnType; }).
                concat(fieldRecords.map(function(field) { return field.setterArgumentType; }));
      whenDependentTypesAreResolved([structType], fieldTypes, function(fieldTypes) {
          var fields = {};
          fieldRecords.forEach(function(field, i) {
              var fieldName = field.fieldName;
              var getterReturnType = fieldTypes[i];
              var getter = field.getter;
              var getterContext = field.getterContext;
              var setterArgumentType = fieldTypes[i + fieldRecords.length];
              var setter = field.setter;
              var setterContext = field.setterContext;
              fields[fieldName] = {
                  read: function(ptr) {
                      return getterReturnType['fromWireType'](
                          getter(getterContext, ptr));
                  },
                  write: function(ptr, o) {
                      var destructors = [];
                      setter(setterContext, ptr, setterArgumentType['toWireType'](destructors, o));
                      runDestructors(destructors);
                  }
              };
          });
  
          return [{
              name: reg.name,
              'fromWireType': function(ptr) {
                  var rv = {};
                  for (var i in fields) {
                      rv[i] = fields[i].read(ptr);
                  }
                  rawDestructor(ptr);
                  return rv;
              },
              'toWireType': function(destructors, o) {
                  // todo: Here we have an opportunity for -O3 level "unsafe" optimizations:
                  // assume all fields are present without checking.
                  for (var fieldName in fields) {
                      if (!(fieldName in o)) {
                          throw new TypeError('Missing field:  "' + fieldName + '"');
                      }
                  }
                  var ptr = rawConstructor();
                  for (fieldName in fields) {
                      fields[fieldName].write(ptr, o[fieldName]);
                  }
                  if (destructors !== null) {
                      destructors.push(rawDestructor, ptr);
                  }
                  return ptr;
              },
              'argPackAdvance': 8,
              'readValueFromPointer': simpleReadValueFromPointer,
              destructorFunction: rawDestructor,
          }];
      });
    }

  function getShiftFromSize(size) {
      switch (size) {
          case 1: return 0;
          case 2: return 1;
          case 4: return 2;
          case 8: return 3;
          default:
              throw new TypeError('Unknown type size: ' + size);
      }
    }
  
  function embind_init_charCodes() {
      var codes = new Array(256);
      for (var i = 0; i < 256; ++i) {
          codes[i] = String.fromCharCode(i);
      }
      embind_charCodes = codes;
    }
  var embind_charCodes=undefined;
  function readLatin1String(ptr) {
      var ret = "";
      var c = ptr;
      while (HEAPU8[c]) {
          ret += embind_charCodes[HEAPU8[c++]];
      }
      return ret;
    }
  
  var BindingError=undefined;
  function throwBindingError(message) {
      throw new BindingError(message);
    }
  /** @param {Object=} options */
  function registerType(rawType, registeredInstance, options) {
      options = options || {};
  
      if (!('argPackAdvance' in registeredInstance)) {
          throw new TypeError('registerType registeredInstance requires argPackAdvance');
      }
  
      var name = registeredInstance.name;
      if (!rawType) {
          throwBindingError('type "' + name + '" must have a positive integer typeid pointer');
      }
      if (registeredTypes.hasOwnProperty(rawType)) {
          if (options.ignoreDuplicateRegistrations) {
              return;
          } else {
              throwBindingError("Cannot register type '" + name + "' twice");
          }
      }
  
      registeredTypes[rawType] = registeredInstance;
      delete typeDependencies[rawType];
  
      if (awaitingDependencies.hasOwnProperty(rawType)) {
          var callbacks = awaitingDependencies[rawType];
          delete awaitingDependencies[rawType];
          callbacks.forEach(function(cb) {
              cb();
          });
      }
    }
  function __embind_register_bool(rawType, name, size, trueValue, falseValue) {
      var shift = getShiftFromSize(size);
  
      name = readLatin1String(name);
      registerType(rawType, {
          name: name,
          'fromWireType': function(wt) {
              // ambiguous emscripten ABI: sometimes return values are
              // true or false, and sometimes integers (0 or 1)
              return !!wt;
          },
          'toWireType': function(destructors, o) {
              return o ? trueValue : falseValue;
          },
          'argPackAdvance': 8,
          'readValueFromPointer': function(pointer) {
              // TODO: if heap is fixed (like in asm.js) this could be executed outside
              var heap;
              if (size === 1) {
                  heap = HEAP8;
              } else if (size === 2) {
                  heap = HEAP16;
              } else if (size === 4) {
                  heap = HEAP32;
              } else {
                  throw new TypeError("Unknown boolean type size: " + name);
              }
              return this['fromWireType'](heap[pointer >> shift]);
          },
          destructorFunction: null, // This type does not need a destructor
      });
    }

  function ClassHandle_isAliasOf(other) {
      if (!(this instanceof ClassHandle)) {
          return false;
      }
      if (!(other instanceof ClassHandle)) {
          return false;
      }
  
      var leftClass = this.$$.ptrType.registeredClass;
      var left = this.$$.ptr;
      var rightClass = other.$$.ptrType.registeredClass;
      var right = other.$$.ptr;
  
      while (leftClass.baseClass) {
          left = leftClass.upcast(left);
          leftClass = leftClass.baseClass;
      }
  
      while (rightClass.baseClass) {
          right = rightClass.upcast(right);
          rightClass = rightClass.baseClass;
      }
  
      return leftClass === rightClass && left === right;
    }
  
  function shallowCopyInternalPointer(o) {
      return {
          count: o.count,
          deleteScheduled: o.deleteScheduled,
          preservePointerOnDelete: o.preservePointerOnDelete,
          ptr: o.ptr,
          ptrType: o.ptrType,
          smartPtr: o.smartPtr,
          smartPtrType: o.smartPtrType,
      };
    }
  
  function throwInstanceAlreadyDeleted(obj) {
      function getInstanceTypeName(handle) {
        return handle.$$.ptrType.registeredClass.name;
      }
      throwBindingError(getInstanceTypeName(obj) + ' instance already deleted');
    }
  
  var finalizationGroup=false;
  
  function detachFinalizer(handle) {}
  
  function runDestructor($$) {
      if ($$.smartPtr) {
          $$.smartPtrType.rawDestructor($$.smartPtr);
      } else {
          $$.ptrType.registeredClass.rawDestructor($$.ptr);
      }
    }
  function releaseClassHandle($$) {
      $$.count.value -= 1;
      var toDelete = 0 === $$.count.value;
      if (toDelete) {
          runDestructor($$);
      }
    }
  function attachFinalizer(handle) {
      if ('undefined' === typeof FinalizationGroup) {
          attachFinalizer = function (handle) { return handle; };
          return handle;
      }
      // If the running environment has a FinalizationGroup (see
      // https://github.com/tc39/proposal-weakrefs), then attach finalizers
      // for class handles.  We check for the presence of FinalizationGroup
      // at run-time, not build-time.
      finalizationGroup = new FinalizationGroup(function (iter) {
          for (var result = iter.next(); !result.done; result = iter.next()) {
              var $$ = result.value;
              if (!$$.ptr) {
                  console.warn('object already deleted: ' + $$.ptr);
              } else {
                  releaseClassHandle($$);
              }
          }
      });
      attachFinalizer = function(handle) {
          finalizationGroup.register(handle, handle.$$, handle.$$);
          return handle;
      };
      detachFinalizer = function(handle) {
          finalizationGroup.unregister(handle.$$);
      };
      return attachFinalizer(handle);
    }
  function ClassHandle_clone() {
      if (!this.$$.ptr) {
          throwInstanceAlreadyDeleted(this);
      }
  
      if (this.$$.preservePointerOnDelete) {
          this.$$.count.value += 1;
          return this;
      } else {
          var clone = attachFinalizer(Object.create(Object.getPrototypeOf(this), {
              $$: {
                  value: shallowCopyInternalPointer(this.$$),
              }
          }));
  
          clone.$$.count.value += 1;
          clone.$$.deleteScheduled = false;
          return clone;
      }
    }
  
  function ClassHandle_delete() {
      if (!this.$$.ptr) {
          throwInstanceAlreadyDeleted(this);
      }
  
      if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
          throwBindingError('Object already scheduled for deletion');
      }
  
      detachFinalizer(this);
      releaseClassHandle(this.$$);
  
      if (!this.$$.preservePointerOnDelete) {
          this.$$.smartPtr = undefined;
          this.$$.ptr = undefined;
      }
    }
  
  function ClassHandle_isDeleted() {
      return !this.$$.ptr;
    }
  
  var delayFunction=undefined;
  
  var deletionQueue=[];
  
  function flushPendingDeletes() {
      while (deletionQueue.length) {
          var obj = deletionQueue.pop();
          obj.$$.deleteScheduled = false;
          obj['delete']();
      }
    }
  function ClassHandle_deleteLater() {
      if (!this.$$.ptr) {
          throwInstanceAlreadyDeleted(this);
      }
      if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
          throwBindingError('Object already scheduled for deletion');
      }
      deletionQueue.push(this);
      if (deletionQueue.length === 1 && delayFunction) {
          delayFunction(flushPendingDeletes);
      }
      this.$$.deleteScheduled = true;
      return this;
    }
  function init_ClassHandle() {
      ClassHandle.prototype['isAliasOf'] = ClassHandle_isAliasOf;
      ClassHandle.prototype['clone'] = ClassHandle_clone;
      ClassHandle.prototype['delete'] = ClassHandle_delete;
      ClassHandle.prototype['isDeleted'] = ClassHandle_isDeleted;
      ClassHandle.prototype['deleteLater'] = ClassHandle_deleteLater;
    }
  function ClassHandle() {
    }
  
  var registeredPointers={};
  
  function ensureOverloadTable(proto, methodName, humanName) {
      if (undefined === proto[methodName].overloadTable) {
          var prevFunc = proto[methodName];
          // Inject an overload resolver function that routes to the appropriate overload based on the number of arguments.
          proto[methodName] = function() {
              // TODO This check can be removed in -O3 level "unsafe" optimizations.
              if (!proto[methodName].overloadTable.hasOwnProperty(arguments.length)) {
                  throwBindingError("Function '" + humanName + "' called with an invalid number of arguments (" + arguments.length + ") - expects one of (" + proto[methodName].overloadTable + ")!");
              }
              return proto[methodName].overloadTable[arguments.length].apply(this, arguments);
          };
          // Move the previous function into the overload table.
          proto[methodName].overloadTable = [];
          proto[methodName].overloadTable[prevFunc.argCount] = prevFunc;
      }
    }
  /** @param {number=} numArguments */
  function exposePublicSymbol(name, value, numArguments) {
      if (Module.hasOwnProperty(name)) {
          if (undefined === numArguments || (undefined !== Module[name].overloadTable && undefined !== Module[name].overloadTable[numArguments])) {
              throwBindingError("Cannot register public name '" + name + "' twice");
          }
  
          // We are exposing a function with the same name as an existing function. Create an overload table and a function selector
          // that routes between the two.
          ensureOverloadTable(Module, name, name);
          if (Module.hasOwnProperty(numArguments)) {
              throwBindingError("Cannot register multiple overloads of a function with the same number of arguments (" + numArguments + ")!");
          }
          // Add the new function into the overload table.
          Module[name].overloadTable[numArguments] = value;
      }
      else {
          Module[name] = value;
          if (undefined !== numArguments) {
              Module[name].numArguments = numArguments;
          }
      }
    }
  
  /** @constructor */
  function RegisteredClass(
      name,
      constructor,
      instancePrototype,
      rawDestructor,
      baseClass,
      getActualType,
      upcast,
      downcast
    ) {
      this.name = name;
      this.constructor = constructor;
      this.instancePrototype = instancePrototype;
      this.rawDestructor = rawDestructor;
      this.baseClass = baseClass;
      this.getActualType = getActualType;
      this.upcast = upcast;
      this.downcast = downcast;
      this.pureVirtualFunctions = [];
    }
  
  function upcastPointer(ptr, ptrClass, desiredClass) {
      while (ptrClass !== desiredClass) {
          if (!ptrClass.upcast) {
              throwBindingError("Expected null or instance of " + desiredClass.name + ", got an instance of " + ptrClass.name);
          }
          ptr = ptrClass.upcast(ptr);
          ptrClass = ptrClass.baseClass;
      }
      return ptr;
    }
  function constNoSmartPtrRawPointerToWireType(destructors, handle) {
      if (handle === null) {
          if (this.isReference) {
              throwBindingError('null is not a valid ' + this.name);
          }
          return 0;
      }
  
      if (!handle.$$) {
          throwBindingError('Cannot pass "' + _embind_repr(handle) + '" as a ' + this.name);
      }
      if (!handle.$$.ptr) {
          throwBindingError('Cannot pass deleted object as a pointer of type ' + this.name);
      }
      var handleClass = handle.$$.ptrType.registeredClass;
      var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
      return ptr;
    }
  
  function genericPointerToWireType(destructors, handle) {
      var ptr;
      if (handle === null) {
          if (this.isReference) {
              throwBindingError('null is not a valid ' + this.name);
          }
  
          if (this.isSmartPointer) {
              ptr = this.rawConstructor();
              if (destructors !== null) {
                  destructors.push(this.rawDestructor, ptr);
              }
              return ptr;
          } else {
              return 0;
          }
      }
  
      if (!handle.$$) {
          throwBindingError('Cannot pass "' + _embind_repr(handle) + '" as a ' + this.name);
      }
      if (!handle.$$.ptr) {
          throwBindingError('Cannot pass deleted object as a pointer of type ' + this.name);
      }
      if (!this.isConst && handle.$$.ptrType.isConst) {
          throwBindingError('Cannot convert argument of type ' + (handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name) + ' to parameter type ' + this.name);
      }
      var handleClass = handle.$$.ptrType.registeredClass;
      ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
  
      if (this.isSmartPointer) {
          // TODO: this is not strictly true
          // We could support BY_EMVAL conversions from raw pointers to smart pointers
          // because the smart pointer can hold a reference to the handle
          if (undefined === handle.$$.smartPtr) {
              throwBindingError('Passing raw pointer to smart pointer is illegal');
          }
  
          switch (this.sharingPolicy) {
              case 0: // NONE
                  // no upcasting
                  if (handle.$$.smartPtrType === this) {
                      ptr = handle.$$.smartPtr;
                  } else {
                      throwBindingError('Cannot convert argument of type ' + (handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name) + ' to parameter type ' + this.name);
                  }
                  break;
  
              case 1: // INTRUSIVE
                  ptr = handle.$$.smartPtr;
                  break;
  
              case 2: // BY_EMVAL
                  if (handle.$$.smartPtrType === this) {
                      ptr = handle.$$.smartPtr;
                  } else {
                      var clonedHandle = handle['clone']();
                      ptr = this.rawShare(
                          ptr,
                          __emval_register(function() {
                              clonedHandle['delete']();
                          })
                      );
                      if (destructors !== null) {
                          destructors.push(this.rawDestructor, ptr);
                      }
                  }
                  break;
  
              default:
                  throwBindingError('Unsupporting sharing policy');
          }
      }
      return ptr;
    }
  
  function nonConstNoSmartPtrRawPointerToWireType(destructors, handle) {
      if (handle === null) {
          if (this.isReference) {
              throwBindingError('null is not a valid ' + this.name);
          }
          return 0;
      }
  
      if (!handle.$$) {
          throwBindingError('Cannot pass "' + _embind_repr(handle) + '" as a ' + this.name);
      }
      if (!handle.$$.ptr) {
          throwBindingError('Cannot pass deleted object as a pointer of type ' + this.name);
      }
      if (handle.$$.ptrType.isConst) {
          throwBindingError('Cannot convert argument of type ' + handle.$$.ptrType.name + ' to parameter type ' + this.name);
      }
      var handleClass = handle.$$.ptrType.registeredClass;
      var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
      return ptr;
    }
  
  function RegisteredPointer_getPointee(ptr) {
      if (this.rawGetPointee) {
          ptr = this.rawGetPointee(ptr);
      }
      return ptr;
    }
  
  function RegisteredPointer_destructor(ptr) {
      if (this.rawDestructor) {
          this.rawDestructor(ptr);
      }
    }
  
  function RegisteredPointer_deleteObject(handle) {
      if (handle !== null) {
          handle['delete']();
      }
    }
  
  function downcastPointer(ptr, ptrClass, desiredClass) {
      if (ptrClass === desiredClass) {
          return ptr;
      }
      if (undefined === desiredClass.baseClass) {
          return null; // no conversion
      }
  
      var rv = downcastPointer(ptr, ptrClass, desiredClass.baseClass);
      if (rv === null) {
          return null;
      }
      return desiredClass.downcast(rv);
    }
  
  function getInheritedInstanceCount() {
      return Object.keys(registeredInstances).length;
    }
  
  function getLiveInheritedInstances() {
      var rv = [];
      for (var k in registeredInstances) {
          if (registeredInstances.hasOwnProperty(k)) {
              rv.push(registeredInstances[k]);
          }
      }
      return rv;
    }
  
  function setDelayFunction(fn) {
      delayFunction = fn;
      if (deletionQueue.length && delayFunction) {
          delayFunction(flushPendingDeletes);
      }
    }
  function init_embind() {
      Module['getInheritedInstanceCount'] = getInheritedInstanceCount;
      Module['getLiveInheritedInstances'] = getLiveInheritedInstances;
      Module['flushPendingDeletes'] = flushPendingDeletes;
      Module['setDelayFunction'] = setDelayFunction;
    }
  var registeredInstances={};
  
  function getBasestPointer(class_, ptr) {
      if (ptr === undefined) {
          throwBindingError('ptr should not be undefined');
      }
      while (class_.baseClass) {
          ptr = class_.upcast(ptr);
          class_ = class_.baseClass;
      }
      return ptr;
    }
  function getInheritedInstance(class_, ptr) {
      ptr = getBasestPointer(class_, ptr);
      return registeredInstances[ptr];
    }
  
  function makeClassHandle(prototype, record) {
      if (!record.ptrType || !record.ptr) {
          throwInternalError('makeClassHandle requires ptr and ptrType');
      }
      var hasSmartPtrType = !!record.smartPtrType;
      var hasSmartPtr = !!record.smartPtr;
      if (hasSmartPtrType !== hasSmartPtr) {
          throwInternalError('Both smartPtrType and smartPtr must be specified');
      }
      record.count = { value: 1 };
      return attachFinalizer(Object.create(prototype, {
          $$: {
              value: record,
          },
      }));
    }
  function RegisteredPointer_fromWireType(ptr) {
      // ptr is a raw pointer (or a raw smartpointer)
  
      // rawPointer is a maybe-null raw pointer
      var rawPointer = this.getPointee(ptr);
      if (!rawPointer) {
          this.destructor(ptr);
          return null;
      }
  
      var registeredInstance = getInheritedInstance(this.registeredClass, rawPointer);
      if (undefined !== registeredInstance) {
          // JS object has been neutered, time to repopulate it
          if (0 === registeredInstance.$$.count.value) {
              registeredInstance.$$.ptr = rawPointer;
              registeredInstance.$$.smartPtr = ptr;
              return registeredInstance['clone']();
          } else {
              // else, just increment reference count on existing object
              // it already has a reference to the smart pointer
              var rv = registeredInstance['clone']();
              this.destructor(ptr);
              return rv;
          }
      }
  
      function makeDefaultHandle() {
          if (this.isSmartPointer) {
              return makeClassHandle(this.registeredClass.instancePrototype, {
                  ptrType: this.pointeeType,
                  ptr: rawPointer,
                  smartPtrType: this,
                  smartPtr: ptr,
              });
          } else {
              return makeClassHandle(this.registeredClass.instancePrototype, {
                  ptrType: this,
                  ptr: ptr,
              });
          }
      }
  
      var actualType = this.registeredClass.getActualType(rawPointer);
      var registeredPointerRecord = registeredPointers[actualType];
      if (!registeredPointerRecord) {
          return makeDefaultHandle.call(this);
      }
  
      var toType;
      if (this.isConst) {
          toType = registeredPointerRecord.constPointerType;
      } else {
          toType = registeredPointerRecord.pointerType;
      }
      var dp = downcastPointer(
          rawPointer,
          this.registeredClass,
          toType.registeredClass);
      if (dp === null) {
          return makeDefaultHandle.call(this);
      }
      if (this.isSmartPointer) {
          return makeClassHandle(toType.registeredClass.instancePrototype, {
              ptrType: toType,
              ptr: dp,
              smartPtrType: this,
              smartPtr: ptr,
          });
      } else {
          return makeClassHandle(toType.registeredClass.instancePrototype, {
              ptrType: toType,
              ptr: dp,
          });
      }
    }
  function init_RegisteredPointer() {
      RegisteredPointer.prototype.getPointee = RegisteredPointer_getPointee;
      RegisteredPointer.prototype.destructor = RegisteredPointer_destructor;
      RegisteredPointer.prototype['argPackAdvance'] = 8;
      RegisteredPointer.prototype['readValueFromPointer'] = simpleReadValueFromPointer;
      RegisteredPointer.prototype['deleteObject'] = RegisteredPointer_deleteObject;
      RegisteredPointer.prototype['fromWireType'] = RegisteredPointer_fromWireType;
    }
  /** @constructor
      @param {*=} pointeeType,
      @param {*=} sharingPolicy,
      @param {*=} rawGetPointee,
      @param {*=} rawConstructor,
      @param {*=} rawShare,
      @param {*=} rawDestructor,
       */
  function RegisteredPointer(
      name,
      registeredClass,
      isReference,
      isConst,
  
      // smart pointer properties
      isSmartPointer,
      pointeeType,
      sharingPolicy,
      rawGetPointee,
      rawConstructor,
      rawShare,
      rawDestructor
    ) {
      this.name = name;
      this.registeredClass = registeredClass;
      this.isReference = isReference;
      this.isConst = isConst;
  
      // smart pointer properties
      this.isSmartPointer = isSmartPointer;
      this.pointeeType = pointeeType;
      this.sharingPolicy = sharingPolicy;
      this.rawGetPointee = rawGetPointee;
      this.rawConstructor = rawConstructor;
      this.rawShare = rawShare;
      this.rawDestructor = rawDestructor;
  
      if (!isSmartPointer && registeredClass.baseClass === undefined) {
          if (isConst) {
              this['toWireType'] = constNoSmartPtrRawPointerToWireType;
              this.destructorFunction = null;
          } else {
              this['toWireType'] = nonConstNoSmartPtrRawPointerToWireType;
              this.destructorFunction = null;
          }
      } else {
          this['toWireType'] = genericPointerToWireType;
          // Here we must leave this.destructorFunction undefined, since whether genericPointerToWireType returns
          // a pointer that needs to be freed up is runtime-dependent, and cannot be evaluated at registration time.
          // TODO: Create an alternative mechanism that allows removing the use of var destructors = []; array in
          //       craftInvokerFunction altogether.
      }
    }
  
  /** @param {number=} numArguments */
  function replacePublicSymbol(name, value, numArguments) {
      if (!Module.hasOwnProperty(name)) {
          throwInternalError('Replacing nonexistant public symbol');
      }
      // If there's an overload table for this symbol, replace the symbol in the overload table instead.
      if (undefined !== Module[name].overloadTable && undefined !== numArguments) {
          Module[name].overloadTable[numArguments] = value;
      }
      else {
          Module[name] = value;
          Module[name].argCount = numArguments;
      }
    }
  
  function dynCallLegacy(sig, ptr, args) {
      assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
      if (args && args.length) {
        // j (64-bit integer) must be passed in as two numbers [low 32, high 32].
        assert(args.length === sig.substring(1).replace(/j/g, '--').length);
      } else {
        assert(sig.length == 1);
      }
      var f = Module["dynCall_" + sig];
      return args && args.length ? f.apply(null, [ptr].concat(args)) : f.call(null, ptr);
    }
  function dynCall(sig, ptr, args) {
      // Without WASM_BIGINT support we cannot directly call function with i64 as
      // part of thier signature, so we rely the dynCall functions generated by
      // wasm-emscripten-finalize
      if (sig.indexOf('j') != -1) {
        return dynCallLegacy(sig, ptr, args);
      }
      assert(wasmTable.get(ptr), 'missing table entry in dynCall: ' + ptr);
      return wasmTable.get(ptr).apply(null, args)
    }
  function getDynCaller(sig, ptr) {
      assert(sig.indexOf('j') >= 0, 'getDynCaller should only be called with i64 sigs')
      var argCache = [];
      return function() {
        argCache.length = arguments.length;
        for (var i = 0; i < arguments.length; i++) {
          argCache[i] = arguments[i];
        }
        return dynCall(sig, ptr, argCache);
      };
    }
  function embind__requireFunction(signature, rawFunction) {
      signature = readLatin1String(signature);
  
      function makeDynCaller() {
        if (signature.indexOf('j') != -1) {
          return getDynCaller(signature, rawFunction);
        }
        return wasmTable.get(rawFunction);
      }
  
      var fp = makeDynCaller();
      if (typeof fp !== "function") {
          throwBindingError("unknown function pointer with signature " + signature + ": " + rawFunction);
      }
      return fp;
    }
  
  var UnboundTypeError=undefined;
  
  function getTypeName(type) {
      var ptr = ___getTypeName(type);
      var rv = readLatin1String(ptr);
      _free(ptr);
      return rv;
    }
  function throwUnboundTypeError(message, types) {
      var unboundTypes = [];
      var seen = {};
      function visit(type) {
          if (seen[type]) {
              return;
          }
          if (registeredTypes[type]) {
              return;
          }
          if (typeDependencies[type]) {
              typeDependencies[type].forEach(visit);
              return;
          }
          unboundTypes.push(type);
          seen[type] = true;
      }
      types.forEach(visit);
  
      throw new UnboundTypeError(message + ': ' + unboundTypes.map(getTypeName).join([', ']));
    }
  function __embind_register_class(
      rawType,
      rawPointerType,
      rawConstPointerType,
      baseClassRawType,
      getActualTypeSignature,
      getActualType,
      upcastSignature,
      upcast,
      downcastSignature,
      downcast,
      name,
      destructorSignature,
      rawDestructor
    ) {
      name = readLatin1String(name);
      getActualType = embind__requireFunction(getActualTypeSignature, getActualType);
      if (upcast) {
          upcast = embind__requireFunction(upcastSignature, upcast);
      }
      if (downcast) {
          downcast = embind__requireFunction(downcastSignature, downcast);
      }
      rawDestructor = embind__requireFunction(destructorSignature, rawDestructor);
      var legalFunctionName = makeLegalFunctionName(name);
  
      exposePublicSymbol(legalFunctionName, function() {
          // this code cannot run if baseClassRawType is zero
          throwUnboundTypeError('Cannot construct ' + name + ' due to unbound types', [baseClassRawType]);
      });
  
      whenDependentTypesAreResolved(
          [rawType, rawPointerType, rawConstPointerType],
          baseClassRawType ? [baseClassRawType] : [],
          function(base) {
              base = base[0];
  
              var baseClass;
              var basePrototype;
              if (baseClassRawType) {
                  baseClass = base.registeredClass;
                  basePrototype = baseClass.instancePrototype;
              } else {
                  basePrototype = ClassHandle.prototype;
              }
  
              var constructor = createNamedFunction(legalFunctionName, function() {
                  if (Object.getPrototypeOf(this) !== instancePrototype) {
                      throw new BindingError("Use 'new' to construct " + name);
                  }
                  if (undefined === registeredClass.constructor_body) {
                      throw new BindingError(name + " has no accessible constructor");
                  }
                  var body = registeredClass.constructor_body[arguments.length];
                  if (undefined === body) {
                      throw new BindingError("Tried to invoke ctor of " + name + " with invalid number of parameters (" + arguments.length + ") - expected (" + Object.keys(registeredClass.constructor_body).toString() + ") parameters instead!");
                  }
                  return body.apply(this, arguments);
              });
  
              var instancePrototype = Object.create(basePrototype, {
                  constructor: { value: constructor },
              });
  
              constructor.prototype = instancePrototype;
  
              var registeredClass = new RegisteredClass(
                  name,
                  constructor,
                  instancePrototype,
                  rawDestructor,
                  baseClass,
                  getActualType,
                  upcast,
                  downcast);
  
              var referenceConverter = new RegisteredPointer(
                  name,
                  registeredClass,
                  true,
                  false,
                  false);
  
              var pointerConverter = new RegisteredPointer(
                  name + '*',
                  registeredClass,
                  false,
                  false,
                  false);
  
              var constPointerConverter = new RegisteredPointer(
                  name + ' const*',
                  registeredClass,
                  false,
                  true,
                  false);
  
              registeredPointers[rawType] = {
                  pointerType: pointerConverter,
                  constPointerType: constPointerConverter
              };
  
              replacePublicSymbol(legalFunctionName, constructor);
  
              return [referenceConverter, pointerConverter, constPointerConverter];
          }
      );
    }

  function new_(constructor, argumentList) {
      if (!(constructor instanceof Function)) {
          throw new TypeError('new_ called with constructor type ' + typeof(constructor) + " which is not a function");
      }
  
      /*
       * Previously, the following line was just:
  
       function dummy() {};
  
       * Unfortunately, Chrome was preserving 'dummy' as the object's name, even though at creation, the 'dummy' has the
       * correct constructor name.  Thus, objects created with IMVU.new would show up in the debugger as 'dummy', which
       * isn't very helpful.  Using IMVU.createNamedFunction addresses the issue.  Doublely-unfortunately, there's no way
       * to write a test for this behavior.  -NRD 2013.02.22
       */
      var dummy = createNamedFunction(constructor.name || 'unknownFunctionName', function(){});
      dummy.prototype = constructor.prototype;
      var obj = new dummy;
  
      var r = constructor.apply(obj, argumentList);
      return (r instanceof Object) ? r : obj;
    }
  function craftInvokerFunction(humanName, argTypes, classType, cppInvokerFunc, cppTargetFunc) {
      // humanName: a human-readable string name for the function to be generated.
      // argTypes: An array that contains the embind type objects for all types in the function signature.
      //    argTypes[0] is the type object for the function return value.
      //    argTypes[1] is the type object for function this object/class type, or null if not crafting an invoker for a class method.
      //    argTypes[2...] are the actual function parameters.
      // classType: The embind type object for the class to be bound, or null if this is not a method of a class.
      // cppInvokerFunc: JS Function object to the C++-side function that interops into C++ code.
      // cppTargetFunc: Function pointer (an integer to FUNCTION_TABLE) to the target C++ function the cppInvokerFunc will end up calling.
      var argCount = argTypes.length;
  
      if (argCount < 2) {
          throwBindingError("argTypes array size mismatch! Must at least get return value and 'this' types!");
      }
  
      var isClassMethodFunc = (argTypes[1] !== null && classType !== null);
  
      // Free functions with signature "void function()" do not need an invoker that marshalls between wire types.
  // TODO: This omits argument count check - enable only at -O3 or similar.
  //    if (ENABLE_UNSAFE_OPTS && argCount == 2 && argTypes[0].name == "void" && !isClassMethodFunc) {
  //       return FUNCTION_TABLE[fn];
  //    }
  
      // Determine if we need to use a dynamic stack to store the destructors for the function parameters.
      // TODO: Remove this completely once all function invokers are being dynamically generated.
      var needsDestructorStack = false;
  
      for(var i = 1; i < argTypes.length; ++i) { // Skip return value at index 0 - it's not deleted here.
          if (argTypes[i] !== null && argTypes[i].destructorFunction === undefined) { // The type does not define a destructor function - must use dynamic stack
              needsDestructorStack = true;
              break;
          }
      }
  
      var returns = (argTypes[0].name !== "void");
  
      var argsList = "";
      var argsListWired = "";
      for(var i = 0; i < argCount - 2; ++i) {
          argsList += (i!==0?", ":"")+"arg"+i;
          argsListWired += (i!==0?", ":"")+"arg"+i+"Wired";
      }
  
      var invokerFnBody =
          "return function "+makeLegalFunctionName(humanName)+"("+argsList+") {\n" +
          "if (arguments.length !== "+(argCount - 2)+") {\n" +
              "throwBindingError('function "+humanName+" called with ' + arguments.length + ' arguments, expected "+(argCount - 2)+" args!');\n" +
          "}\n";
  
      if (needsDestructorStack) {
          invokerFnBody +=
              "var destructors = [];\n";
      }
  
      var dtorStack = needsDestructorStack ? "destructors" : "null";
      var args1 = ["throwBindingError", "invoker", "fn", "runDestructors", "retType", "classParam"];
      var args2 = [throwBindingError, cppInvokerFunc, cppTargetFunc, runDestructors, argTypes[0], argTypes[1]];
  
      if (isClassMethodFunc) {
          invokerFnBody += "var thisWired = classParam.toWireType("+dtorStack+", this);\n";
      }
  
      for(var i = 0; i < argCount - 2; ++i) {
          invokerFnBody += "var arg"+i+"Wired = argType"+i+".toWireType("+dtorStack+", arg"+i+"); // "+argTypes[i+2].name+"\n";
          args1.push("argType"+i);
          args2.push(argTypes[i+2]);
      }
  
      if (isClassMethodFunc) {
          argsListWired = "thisWired" + (argsListWired.length > 0 ? ", " : "") + argsListWired;
      }
  
      invokerFnBody +=
          (returns?"var rv = ":"") + "invoker(fn"+(argsListWired.length>0?", ":"")+argsListWired+");\n";
  
      if (needsDestructorStack) {
          invokerFnBody += "runDestructors(destructors);\n";
      } else {
          for(var i = isClassMethodFunc?1:2; i < argTypes.length; ++i) { // Skip return value at index 0 - it's not deleted here. Also skip class type if not a method.
              var paramName = (i === 1 ? "thisWired" : ("arg"+(i - 2)+"Wired"));
              if (argTypes[i].destructorFunction !== null) {
                  invokerFnBody += paramName+"_dtor("+paramName+"); // "+argTypes[i].name+"\n";
                  args1.push(paramName+"_dtor");
                  args2.push(argTypes[i].destructorFunction);
              }
          }
      }
  
      if (returns) {
          invokerFnBody += "var ret = retType.fromWireType(rv);\n" +
                           "return ret;\n";
      } else {
      }
      invokerFnBody += "}\n";
  
      args1.push(invokerFnBody);
  
      var invokerFunction = new_(Function, args1).apply(null, args2);
      return invokerFunction;
    }
  
  function heap32VectorToArray(count, firstElement) {
      var array = [];
      for (var i = 0; i < count; i++) {
          array.push(HEAP32[(firstElement >> 2) + i]);
      }
      return array;
    }
  function __embind_register_class_class_function(
      rawClassType,
      methodName,
      argCount,
      rawArgTypesAddr,
      invokerSignature,
      rawInvoker,
      fn
    ) {
      var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
      methodName = readLatin1String(methodName);
      rawInvoker = embind__requireFunction(invokerSignature, rawInvoker);
      whenDependentTypesAreResolved([], [rawClassType], function(classType) {
          classType = classType[0];
          var humanName = classType.name + '.' + methodName;
  
          function unboundTypesHandler() {
              throwUnboundTypeError('Cannot call ' + humanName + ' due to unbound types', rawArgTypes);
          }
  
          var proto = classType.registeredClass.constructor;
          if (undefined === proto[methodName]) {
              // This is the first function to be registered with this name.
              unboundTypesHandler.argCount = argCount-1;
              proto[methodName] = unboundTypesHandler;
          } else {
              // There was an existing function with the same name registered. Set up a function overload routing table.
              ensureOverloadTable(proto, methodName, humanName);
              proto[methodName].overloadTable[argCount-1] = unboundTypesHandler;
          }
  
          whenDependentTypesAreResolved([], rawArgTypes, function(argTypes) {
              // Replace the initial unbound-types-handler stub with the proper function. If multiple overloads are registered,
              // the function handlers go into an overload table.
              var invokerArgsArray = [argTypes[0] /* return value */, null /* no class 'this'*/].concat(argTypes.slice(1) /* actual params */);
              var func = craftInvokerFunction(humanName, invokerArgsArray, null /* no class 'this'*/, rawInvoker, fn);
              if (undefined === proto[methodName].overloadTable) {
                  func.argCount = argCount-1;
                  proto[methodName] = func;
              } else {
                  proto[methodName].overloadTable[argCount-1] = func;
              }
              return [];
          });
          return [];
      });
    }

  function __embind_register_class_constructor(
      rawClassType,
      argCount,
      rawArgTypesAddr,
      invokerSignature,
      invoker,
      rawConstructor
    ) {
      assert(argCount > 0);
      var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
      invoker = embind__requireFunction(invokerSignature, invoker);
      var args = [rawConstructor];
      var destructors = [];
  
      whenDependentTypesAreResolved([], [rawClassType], function(classType) {
          classType = classType[0];
          var humanName = 'constructor ' + classType.name;
  
          if (undefined === classType.registeredClass.constructor_body) {
              classType.registeredClass.constructor_body = [];
          }
          if (undefined !== classType.registeredClass.constructor_body[argCount - 1]) {
              throw new BindingError("Cannot register multiple constructors with identical number of parameters (" + (argCount-1) + ") for class '" + classType.name + "'! Overload resolution is currently only performed using the parameter count, not actual type info!");
          }
          classType.registeredClass.constructor_body[argCount - 1] = function unboundTypeHandler() {
              throwUnboundTypeError('Cannot construct ' + classType.name + ' due to unbound types', rawArgTypes);
          };
  
          whenDependentTypesAreResolved([], rawArgTypes, function(argTypes) {
              classType.registeredClass.constructor_body[argCount - 1] = function constructor_body() {
                  if (arguments.length !== argCount - 1) {
                      throwBindingError(humanName + ' called with ' + arguments.length + ' arguments, expected ' + (argCount-1));
                  }
                  destructors.length = 0;
                  args.length = argCount;
                  for (var i = 1; i < argCount; ++i) {
                      args[i] = argTypes[i]['toWireType'](destructors, arguments[i - 1]);
                  }
  
                  var ptr = invoker.apply(null, args);
                  runDestructors(destructors);
  
                  return argTypes[0]['fromWireType'](ptr);
              };
              return [];
          });
          return [];
      });
    }

  function __embind_register_class_function(
      rawClassType,
      methodName,
      argCount,
      rawArgTypesAddr, // [ReturnType, ThisType, Args...]
      invokerSignature,
      rawInvoker,
      context,
      isPureVirtual
    ) {
      var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
      methodName = readLatin1String(methodName);
      rawInvoker = embind__requireFunction(invokerSignature, rawInvoker);
  
      whenDependentTypesAreResolved([], [rawClassType], function(classType) {
          classType = classType[0];
          var humanName = classType.name + '.' + methodName;
  
          if (isPureVirtual) {
              classType.registeredClass.pureVirtualFunctions.push(methodName);
          }
  
          function unboundTypesHandler() {
              throwUnboundTypeError('Cannot call ' + humanName + ' due to unbound types', rawArgTypes);
          }
  
          var proto = classType.registeredClass.instancePrototype;
          var method = proto[methodName];
          if (undefined === method || (undefined === method.overloadTable && method.className !== classType.name && method.argCount === argCount - 2)) {
              // This is the first overload to be registered, OR we are replacing a function in the base class with a function in the derived class.
              unboundTypesHandler.argCount = argCount - 2;
              unboundTypesHandler.className = classType.name;
              proto[methodName] = unboundTypesHandler;
          } else {
              // There was an existing function with the same name registered. Set up a function overload routing table.
              ensureOverloadTable(proto, methodName, humanName);
              proto[methodName].overloadTable[argCount - 2] = unboundTypesHandler;
          }
  
          whenDependentTypesAreResolved([], rawArgTypes, function(argTypes) {
  
              var memberFunction = craftInvokerFunction(humanName, argTypes, classType, rawInvoker, context);
  
              // Replace the initial unbound-handler-stub function with the appropriate member function, now that all types
              // are resolved. If multiple overloads are registered for this function, the function goes into an overload table.
              if (undefined === proto[methodName].overloadTable) {
                  // Set argCount in case an overload is registered later
                  memberFunction.argCount = argCount - 2;
                  proto[methodName] = memberFunction;
              } else {
                  proto[methodName].overloadTable[argCount - 2] = memberFunction;
              }
  
              return [];
          });
          return [];
      });
    }

  function __embind_register_constant(name, type, value) {
      name = readLatin1String(name);
      whenDependentTypesAreResolved([], [type], function(type) {
          type = type[0];
          Module[name] = type['fromWireType'](value);
          return [];
      });
    }

  var emval_free_list=[];
  
  var emval_handle_array=[{},{value:undefined},{value:null},{value:true},{value:false}];
  function __emval_decref(handle) {
      if (handle > 4 && 0 === --emval_handle_array[handle].refcount) {
          emval_handle_array[handle] = undefined;
          emval_free_list.push(handle);
      }
    }
  
  function count_emval_handles() {
      var count = 0;
      for (var i = 5; i < emval_handle_array.length; ++i) {
          if (emval_handle_array[i] !== undefined) {
              ++count;
          }
      }
      return count;
    }
  
  function get_first_emval() {
      for (var i = 5; i < emval_handle_array.length; ++i) {
          if (emval_handle_array[i] !== undefined) {
              return emval_handle_array[i];
          }
      }
      return null;
    }
  function init_emval() {
      Module['count_emval_handles'] = count_emval_handles;
      Module['get_first_emval'] = get_first_emval;
    }
  function __emval_register(value) {
  
      switch(value){
        case undefined :{ return 1; }
        case null :{ return 2; }
        case true :{ return 3; }
        case false :{ return 4; }
        default:{
          var handle = emval_free_list.length ?
              emval_free_list.pop() :
              emval_handle_array.length;
  
          emval_handle_array[handle] = {refcount: 1, value: value};
          return handle;
          }
        }
    }
  function __embind_register_emval(rawType, name) {
      name = readLatin1String(name);
      registerType(rawType, {
          name: name,
          'fromWireType': function(handle) {
              var rv = emval_handle_array[handle].value;
              __emval_decref(handle);
              return rv;
          },
          'toWireType': function(destructors, value) {
              return __emval_register(value);
          },
          'argPackAdvance': 8,
          'readValueFromPointer': simpleReadValueFromPointer,
          destructorFunction: null, // This type does not need a destructor
  
          // TODO: do we need a deleteObject here?  write a test where
          // emval is passed into JS via an interface
      });
    }

  function enumReadValueFromPointer(name, shift, signed) {
      switch (shift) {
          case 0: return function(pointer) {
              var heap = signed ? HEAP8 : HEAPU8;
              return this['fromWireType'](heap[pointer]);
          };
          case 1: return function(pointer) {
              var heap = signed ? HEAP16 : HEAPU16;
              return this['fromWireType'](heap[pointer >> 1]);
          };
          case 2: return function(pointer) {
              var heap = signed ? HEAP32 : HEAPU32;
              return this['fromWireType'](heap[pointer >> 2]);
          };
          default:
              throw new TypeError("Unknown integer type: " + name);
      }
    }
  function __embind_register_enum(
      rawType,
      name,
      size,
      isSigned
    ) {
      var shift = getShiftFromSize(size);
      name = readLatin1String(name);
  
      function ctor() {
      }
      ctor.values = {};
  
      registerType(rawType, {
          name: name,
          constructor: ctor,
          'fromWireType': function(c) {
              return this.constructor.values[c];
          },
          'toWireType': function(destructors, c) {
              return c.value;
          },
          'argPackAdvance': 8,
          'readValueFromPointer': enumReadValueFromPointer(name, shift, isSigned),
          destructorFunction: null,
      });
      exposePublicSymbol(name, ctor);
    }

  function requireRegisteredType(rawType, humanName) {
      var impl = registeredTypes[rawType];
      if (undefined === impl) {
          throwBindingError(humanName + " has unknown type " + getTypeName(rawType));
      }
      return impl;
    }
  function __embind_register_enum_value(
      rawEnumType,
      name,
      enumValue
    ) {
      var enumType = requireRegisteredType(rawEnumType, 'enum');
      name = readLatin1String(name);
  
      var Enum = enumType.constructor;
  
      var Value = Object.create(enumType.constructor.prototype, {
          value: {value: enumValue},
          constructor: {value: createNamedFunction(enumType.name + '_' + name, function() {})},
      });
      Enum.values[enumValue] = Value;
      Enum[name] = Value;
    }

  function _embind_repr(v) {
      if (v === null) {
          return 'null';
      }
      var t = typeof v;
      if (t === 'object' || t === 'array' || t === 'function') {
          return v.toString();
      } else {
          return '' + v;
      }
    }
  
  function floatReadValueFromPointer(name, shift) {
      switch (shift) {
          case 2: return function(pointer) {
              return this['fromWireType'](HEAPF32[pointer >> 2]);
          };
          case 3: return function(pointer) {
              return this['fromWireType'](HEAPF64[pointer >> 3]);
          };
          default:
              throw new TypeError("Unknown float type: " + name);
      }
    }
  function __embind_register_float(rawType, name, size) {
      var shift = getShiftFromSize(size);
      name = readLatin1String(name);
      registerType(rawType, {
          name: name,
          'fromWireType': function(value) {
              return value;
          },
          'toWireType': function(destructors, value) {
              // todo: Here we have an opportunity for -O3 level "unsafe" optimizations: we could
              // avoid the following if() and assume value is of proper type.
              if (typeof value !== "number" && typeof value !== "boolean") {
                  throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' + this.name);
              }
              return value;
          },
          'argPackAdvance': 8,
          'readValueFromPointer': floatReadValueFromPointer(name, shift),
          destructorFunction: null, // This type does not need a destructor
      });
    }

  function __embind_register_function(name, argCount, rawArgTypesAddr, signature, rawInvoker, fn) {
      var argTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
      name = readLatin1String(name);
  
      rawInvoker = embind__requireFunction(signature, rawInvoker);
  
      exposePublicSymbol(name, function() {
          throwUnboundTypeError('Cannot call ' + name + ' due to unbound types', argTypes);
      }, argCount - 1);
  
      whenDependentTypesAreResolved([], argTypes, function(argTypes) {
          var invokerArgsArray = [argTypes[0] /* return value */, null /* no class 'this'*/].concat(argTypes.slice(1) /* actual params */);
          replacePublicSymbol(name, craftInvokerFunction(name, invokerArgsArray, null /* no class 'this'*/, rawInvoker, fn), argCount - 1);
          return [];
      });
    }

  function integerReadValueFromPointer(name, shift, signed) {
      // integers are quite common, so generate very specialized functions
      switch (shift) {
          case 0: return signed ?
              function readS8FromPointer(pointer) { return HEAP8[pointer]; } :
              function readU8FromPointer(pointer) { return HEAPU8[pointer]; };
          case 1: return signed ?
              function readS16FromPointer(pointer) { return HEAP16[pointer >> 1]; } :
              function readU16FromPointer(pointer) { return HEAPU16[pointer >> 1]; };
          case 2: return signed ?
              function readS32FromPointer(pointer) { return HEAP32[pointer >> 2]; } :
              function readU32FromPointer(pointer) { return HEAPU32[pointer >> 2]; };
          default:
              throw new TypeError("Unknown integer type: " + name);
      }
    }
  function __embind_register_integer(primitiveType, name, size, minRange, maxRange) {
      name = readLatin1String(name);
      if (maxRange === -1) { // LLVM doesn't have signed and unsigned 32-bit types, so u32 literals come out as 'i32 -1'. Always treat those as max u32.
          maxRange = 4294967295;
      }
  
      var shift = getShiftFromSize(size);
  
      var fromWireType = function(value) {
          return value;
      };
  
      if (minRange === 0) {
          var bitshift = 32 - 8*size;
          fromWireType = function(value) {
              return (value << bitshift) >>> bitshift;
          };
      }
  
      var isUnsignedType = (name.indexOf('unsigned') != -1);
  
      registerType(primitiveType, {
          name: name,
          'fromWireType': fromWireType,
          'toWireType': function(destructors, value) {
              // todo: Here we have an opportunity for -O3 level "unsafe" optimizations: we could
              // avoid the following two if()s and assume value is of proper type.
              if (typeof value !== "number" && typeof value !== "boolean") {
                  throw new TypeError('Cannot convert "' + _embind_repr(value) + '" to ' + this.name);
              }
              if (value < minRange || value > maxRange) {
                  throw new TypeError('Passing a number "' + _embind_repr(value) + '" from JS side to C/C++ side to an argument of type "' + name + '", which is outside the valid range [' + minRange + ', ' + maxRange + ']!');
              }
              return isUnsignedType ? (value >>> 0) : (value | 0);
          },
          'argPackAdvance': 8,
          'readValueFromPointer': integerReadValueFromPointer(name, shift, minRange !== 0),
          destructorFunction: null, // This type does not need a destructor
      });
    }

  function __embind_register_memory_view(rawType, dataTypeIndex, name) {
      var typeMapping = [
          Int8Array,
          Uint8Array,
          Int16Array,
          Uint16Array,
          Int32Array,
          Uint32Array,
          Float32Array,
          Float64Array,
      ];
  
      var TA = typeMapping[dataTypeIndex];
  
      function decodeMemoryView(handle) {
          handle = handle >> 2;
          var heap = HEAPU32;
          var size = heap[handle]; // in elements
          var data = heap[handle + 1]; // byte offset into emscripten heap
          return new TA(buffer, data, size);
      }
  
      name = readLatin1String(name);
      registerType(rawType, {
          name: name,
          'fromWireType': decodeMemoryView,
          'argPackAdvance': 8,
          'readValueFromPointer': decodeMemoryView,
      }, {
          ignoreDuplicateRegistrations: true,
      });
    }

  function __embind_register_smart_ptr(
      rawType,
      rawPointeeType,
      name,
      sharingPolicy,
      getPointeeSignature,
      rawGetPointee,
      constructorSignature,
      rawConstructor,
      shareSignature,
      rawShare,
      destructorSignature,
      rawDestructor
    ) {
      name = readLatin1String(name);
      rawGetPointee = embind__requireFunction(getPointeeSignature, rawGetPointee);
      rawConstructor = embind__requireFunction(constructorSignature, rawConstructor);
      rawShare = embind__requireFunction(shareSignature, rawShare);
      rawDestructor = embind__requireFunction(destructorSignature, rawDestructor);
  
      whenDependentTypesAreResolved([rawType], [rawPointeeType], function(pointeeType) {
          pointeeType = pointeeType[0];
  
          var registeredPointer = new RegisteredPointer(
              name,
              pointeeType.registeredClass,
              false,
              false,
              // smart pointer properties
              true,
              pointeeType,
              sharingPolicy,
              rawGetPointee,
              rawConstructor,
              rawShare,
              rawDestructor);
          return [registeredPointer];
      });
    }

  function __embind_register_std_string(rawType, name) {
      name = readLatin1String(name);
      var stdStringIsUTF8
      //process only std::string bindings with UTF8 support, in contrast to e.g. std::basic_string<unsigned char>
      = (name === "std::string");
  
      registerType(rawType, {
          name: name,
          'fromWireType': function(value) {
              var length = HEAPU32[value >> 2];
  
              var str;
              if (stdStringIsUTF8) {
                  var decodeStartPtr = value + 4;
                  // Looping here to support possible embedded '0' bytes
                  for (var i = 0; i <= length; ++i) {
                      var currentBytePtr = value + 4 + i;
                      if (i == length || HEAPU8[currentBytePtr] == 0) {
                          var maxRead = currentBytePtr - decodeStartPtr;
                          var stringSegment = UTF8ToString(decodeStartPtr, maxRead);
                          if (str === undefined) {
                              str = stringSegment;
                          } else {
                              str += String.fromCharCode(0);
                              str += stringSegment;
                          }
                          decodeStartPtr = currentBytePtr + 1;
                      }
                  }
              } else {
                  var a = new Array(length);
                  for (var i = 0; i < length; ++i) {
                      a[i] = String.fromCharCode(HEAPU8[value + 4 + i]);
                  }
                  str = a.join('');
              }
  
              _free(value);
  
              return str;
          },
          'toWireType': function(destructors, value) {
              if (value instanceof ArrayBuffer) {
                  value = new Uint8Array(value);
              }
  
              var getLength;
              var valueIsOfTypeString = (typeof value === 'string');
  
              if (!(valueIsOfTypeString || value instanceof Uint8Array || value instanceof Uint8ClampedArray || value instanceof Int8Array)) {
                  throwBindingError('Cannot pass non-string to std::string');
              }
              if (stdStringIsUTF8 && valueIsOfTypeString) {
                  getLength = function() {return lengthBytesUTF8(value);};
              } else {
                  getLength = function() {return value.length;};
              }
  
              // assumes 4-byte alignment
              var length = getLength();
              var ptr = _malloc(4 + length + 1);
              HEAPU32[ptr >> 2] = length;
              if (stdStringIsUTF8 && valueIsOfTypeString) {
                  stringToUTF8(value, ptr + 4, length + 1);
              } else {
                  if (valueIsOfTypeString) {
                      for (var i = 0; i < length; ++i) {
                          var charCode = value.charCodeAt(i);
                          if (charCode > 255) {
                              _free(ptr);
                              throwBindingError('String has UTF-16 code units that do not fit in 8 bits');
                          }
                          HEAPU8[ptr + 4 + i] = charCode;
                      }
                  } else {
                      for (var i = 0; i < length; ++i) {
                          HEAPU8[ptr + 4 + i] = value[i];
                      }
                  }
              }
  
              if (destructors !== null) {
                  destructors.push(_free, ptr);
              }
              return ptr;
          },
          'argPackAdvance': 8,
          'readValueFromPointer': simpleReadValueFromPointer,
          destructorFunction: function(ptr) { _free(ptr); },
      });
    }

  function __embind_register_std_wstring(rawType, charSize, name) {
      name = readLatin1String(name);
      var decodeString, encodeString, getHeap, lengthBytesUTF, shift;
      if (charSize === 2) {
          decodeString = UTF16ToString;
          encodeString = stringToUTF16;
          lengthBytesUTF = lengthBytesUTF16;
          getHeap = function() { return HEAPU16; };
          shift = 1;
      } else if (charSize === 4) {
          decodeString = UTF32ToString;
          encodeString = stringToUTF32;
          lengthBytesUTF = lengthBytesUTF32;
          getHeap = function() { return HEAPU32; };
          shift = 2;
      }
      registerType(rawType, {
          name: name,
          'fromWireType': function(value) {
              // Code mostly taken from _embind_register_std_string fromWireType
              var length = HEAPU32[value >> 2];
              var HEAP = getHeap();
              var str;
  
              var decodeStartPtr = value + 4;
              // Looping here to support possible embedded '0' bytes
              for (var i = 0; i <= length; ++i) {
                  var currentBytePtr = value + 4 + i * charSize;
                  if (i == length || HEAP[currentBytePtr >> shift] == 0) {
                      var maxReadBytes = currentBytePtr - decodeStartPtr;
                      var stringSegment = decodeString(decodeStartPtr, maxReadBytes);
                      if (str === undefined) {
                          str = stringSegment;
                      } else {
                          str += String.fromCharCode(0);
                          str += stringSegment;
                      }
                      decodeStartPtr = currentBytePtr + charSize;
                  }
              }
  
              _free(value);
  
              return str;
          },
          'toWireType': function(destructors, value) {
              if (!(typeof value === 'string')) {
                  throwBindingError('Cannot pass non-string to C++ string type ' + name);
              }
  
              // assumes 4-byte alignment
              var length = lengthBytesUTF(value);
              var ptr = _malloc(4 + length + charSize);
              HEAPU32[ptr >> 2] = length >> shift;
  
              encodeString(value, ptr + 4, length + charSize);
  
              if (destructors !== null) {
                  destructors.push(_free, ptr);
              }
              return ptr;
          },
          'argPackAdvance': 8,
          'readValueFromPointer': simpleReadValueFromPointer,
          destructorFunction: function(ptr) { _free(ptr); },
      });
    }

  function __embind_register_value_object(
      rawType,
      name,
      constructorSignature,
      rawConstructor,
      destructorSignature,
      rawDestructor
    ) {
      structRegistrations[rawType] = {
          name: readLatin1String(name),
          rawConstructor: embind__requireFunction(constructorSignature, rawConstructor),
          rawDestructor: embind__requireFunction(destructorSignature, rawDestructor),
          fields: [],
      };
    }

  function __embind_register_value_object_field(
      structType,
      fieldName,
      getterReturnType,
      getterSignature,
      getter,
      getterContext,
      setterArgumentType,
      setterSignature,
      setter,
      setterContext
    ) {
      structRegistrations[structType].fields.push({
          fieldName: readLatin1String(fieldName),
          getterReturnType: getterReturnType,
          getter: embind__requireFunction(getterSignature, getter),
          getterContext: getterContext,
          setterArgumentType: setterArgumentType,
          setter: embind__requireFunction(setterSignature, setter),
          setterContext: setterContext,
      });
    }

  function __embind_register_void(rawType, name) {
      name = readLatin1String(name);
      registerType(rawType, {
          isVoid: true, // void return values can be optimized out sometimes
          name: name,
          'argPackAdvance': 0,
          'fromWireType': function() {
              return undefined;
          },
          'toWireType': function(destructors, o) {
              // TODO: assert if anything else is given?
              return undefined;
          },
      });
    }

  function __emval_allocateDestructors(destructorsRef) {
      var destructors = [];
      HEAP32[destructorsRef >> 2] = __emval_register(destructors);
      return destructors;
    }
  
  var emval_symbols={};
  function getStringOrSymbol(address) {
      var symbol = emval_symbols[address];
      if (symbol === undefined) {
          return readLatin1String(address);
      } else {
          return symbol;
      }
    }
  
  var emval_methodCallers=[];
  
  function requireHandle(handle) {
      if (!handle) {
          throwBindingError('Cannot use deleted val. handle = ' + handle);
      }
      return emval_handle_array[handle].value;
    }
  function __emval_call_void_method(caller, handle, methodName, args) {
      caller = emval_methodCallers[caller];
      handle = requireHandle(handle);
      methodName = getStringOrSymbol(methodName);
      caller(handle, methodName, null, args);
    }


  function __emval_addMethodCaller(caller) {
      var id = emval_methodCallers.length;
      emval_methodCallers.push(caller);
      return id;
    }
  
  function __emval_lookupTypes(argCount, argTypes) {
      var a = new Array(argCount);
      for (var i = 0; i < argCount; ++i) {
          a[i] = requireRegisteredType(
              HEAP32[(argTypes >> 2) + i],
              "parameter " + i);
      }
      return a;
    }
  function __emval_get_method_caller(argCount, argTypes) {
      var types = __emval_lookupTypes(argCount, argTypes);
  
      var retType = types[0];
      var signatureName = retType.name + "_$" + types.slice(1).map(function (t) { return t.name; }).join("_") + "$";
  
      var params = ["retType"];
      var args = [retType];
  
      var argsList = ""; // 'arg0, arg1, arg2, ... , argN'
      for (var i = 0; i < argCount - 1; ++i) {
          argsList += (i !== 0 ? ", " : "") + "arg" + i;
          params.push("argType" + i);
          args.push(types[1 + i]);
      }
  
      var functionName = makeLegalFunctionName("methodCaller_" + signatureName);
      var functionBody =
          "return function " + functionName + "(handle, name, destructors, args) {\n";
  
      var offset = 0;
      for (var i = 0; i < argCount - 1; ++i) {
          functionBody +=
          "    var arg" + i + " = argType" + i + ".readValueFromPointer(args" + (offset ? ("+"+offset) : "") + ");\n";
          offset += types[i + 1]['argPackAdvance'];
      }
      functionBody +=
          "    var rv = handle[name](" + argsList + ");\n";
      for (var i = 0; i < argCount - 1; ++i) {
          if (types[i + 1]['deleteObject']) {
              functionBody +=
              "    argType" + i + ".deleteObject(arg" + i + ");\n";
          }
      }
      if (!retType.isVoid) {
          functionBody +=
          "    return retType.toWireType(destructors, rv);\n";
      }
      functionBody +=
          "};\n";
  
      params.push(functionBody);
      var invokerFunction = new_(Function, params).apply(null, args);
      return __emval_addMethodCaller(invokerFunction);
    }

  function __emval_incref(handle) {
      if (handle > 4) {
          emval_handle_array[handle].refcount += 1;
      }
    }

  function __emval_new_array() {
      return __emval_register([]);
    }

  function __emval_new_cstring(v) {
      return __emval_register(getStringOrSymbol(v));
    }

  function __emval_new_object() {
      return __emval_register({});
    }

  function __emval_set_property(handle, key, value) {
      handle = requireHandle(handle);
      key = requireHandle(key);
      value = requireHandle(value);
      handle[key] = value;
    }

  function __emval_take_value(type, argv) {
      type = requireRegisteredType(type, '_emval_take_value');
      var v = type['readValueFromPointer'](argv);
      return __emval_register(v);
    }

  function _abort() {
      abort();
    }

  var _emscripten_get_now;if (ENVIRONMENT_IS_NODE) {
    _emscripten_get_now = function() {
      var t = process['hrtime']();
      return t[0] * 1e3 + t[1] / 1e6;
    };
  } else if (typeof dateNow !== 'undefined') {
    _emscripten_get_now = dateNow;
  } else _emscripten_get_now = function() { return performance.now(); }
  ;
  
  var _emscripten_get_now_is_monotonic=true;;
  function _clock_gettime(clk_id, tp) {
      // int clock_gettime(clockid_t clk_id, struct timespec *tp);
      var now;
      if (clk_id === 0) {
        now = Date.now();
      } else if ((clk_id === 1 || clk_id === 4) && _emscripten_get_now_is_monotonic) {
        now = _emscripten_get_now();
      } else {
        setErrNo(28);
        return -1;
      }
      HEAP32[((tp)>>2)] = (now/1000)|0; // seconds
      HEAP32[(((tp)+(4))>>2)] = ((now % 1000)*1000*1000)|0; // nanoseconds
      return 0;
    }

  function __webgl_enable_ANGLE_instanced_arrays(ctx) {
      // Extension available in WebGL 1 from Firefox 26 and Google Chrome 30 onwards. Core feature in WebGL 2.
      var ext = ctx.getExtension('ANGLE_instanced_arrays');
      if (ext) {
        ctx['vertexAttribDivisor'] = function(index, divisor) { ext['vertexAttribDivisorANGLE'](index, divisor); };
        ctx['drawArraysInstanced'] = function(mode, first, count, primcount) { ext['drawArraysInstancedANGLE'](mode, first, count, primcount); };
        ctx['drawElementsInstanced'] = function(mode, count, type, indices, primcount) { ext['drawElementsInstancedANGLE'](mode, count, type, indices, primcount); };
        return 1;
      }
    }
  
  function __webgl_enable_OES_vertex_array_object(ctx) {
      // Extension available in WebGL 1 from Firefox 25 and WebKit 536.28/desktop Safari 6.0.3 onwards. Core feature in WebGL 2.
      var ext = ctx.getExtension('OES_vertex_array_object');
      if (ext) {
        ctx['createVertexArray'] = function() { return ext['createVertexArrayOES'](); };
        ctx['deleteVertexArray'] = function(vao) { ext['deleteVertexArrayOES'](vao); };
        ctx['bindVertexArray'] = function(vao) { ext['bindVertexArrayOES'](vao); };
        ctx['isVertexArray'] = function(vao) { return ext['isVertexArrayOES'](vao); };
        return 1;
      }
    }
  
  function __webgl_enable_WEBGL_draw_buffers(ctx) {
      // Extension available in WebGL 1 from Firefox 28 onwards. Core feature in WebGL 2.
      var ext = ctx.getExtension('WEBGL_draw_buffers');
      if (ext) {
        ctx['drawBuffers'] = function(n, bufs) { ext['drawBuffersWEBGL'](n, bufs); };
        return 1;
      }
    }
  
  function __webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(ctx) {
      // Closure is expected to be allowed to minify the '.dibvbi' property, so not accessing it quoted.
      return !!(ctx.dibvbi = ctx.getExtension('WEBGL_draw_instanced_base_vertex_base_instance'));
    }
  
  function __webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(ctx) {
      // Closure is expected to be allowed to minify the '.mdibvbi' property, so not accessing it quoted.
      return !!(ctx.mdibvbi = ctx.getExtension('WEBGL_multi_draw_instanced_base_vertex_base_instance'));
    }
  
  function __webgl_enable_WEBGL_multi_draw(ctx) {
      // Closure is expected to be allowed to minify the '.multiDrawWebgl' property, so not accessing it quoted.
      return !!(ctx.multiDrawWebgl = ctx.getExtension('WEBGL_multi_draw'));
    }
  var GL={counter:1,buffers:[],programs:[],framebuffers:[],renderbuffers:[],textures:[],uniforms:[],shaders:[],vaos:[],contexts:[],offscreenCanvases:{},timerQueriesEXT:[],queries:[],samplers:[],transformFeedbacks:[],syncs:[],programInfos:{},stringCache:{},stringiCache:{},unpackAlignment:4,recordError:function recordError(errorCode) {
        if (!GL.lastError) {
          GL.lastError = errorCode;
        }
      },getNewId:function(table) {
        var ret = GL.counter++;
        for (var i = table.length; i < ret; i++) {
          table[i] = null;
        }
        return ret;
      },getSource:function(shader, count, string, length) {
        var source = '';
        for (var i = 0; i < count; ++i) {
          var len = length ? HEAP32[(((length)+(i*4))>>2)] : -1;
          source += UTF8ToString(HEAP32[(((string)+(i*4))>>2)], len < 0 ? undefined : len);
        }
        return source;
      },validateGLObjectID:function(objectHandleArray, objectID, callerFunctionName, objectReadableType) {
        if (objectID != 0) {
          if (objectHandleArray[objectID] === null) {
            console.error(callerFunctionName + ' called with an already deleted ' + objectReadableType + ' ID ' + objectID + '!');
          } else if (!objectHandleArray[objectID]) {
            console.error(callerFunctionName + ' called with an invalid ' + objectReadableType + ' ID ' + objectID + '!');
          }
        }
      },validateVertexAttribPointer:function(dimension, dataType, stride, offset) {
        var sizeBytes = 1;
        switch(dataType) {
          case 0x1400 /* GL_BYTE */:
          case 0x1401 /* GL_UNSIGNED_BYTE */:
            sizeBytes = 1;
            break;
          case 0x1402 /* GL_SHORT */:
          case 0x1403 /* GL_UNSIGNED_SHORT */:
            sizeBytes = 2;
            break;
          case 0x1404 /* GL_INT */:
          case 0x1405 /* GL_UNSIGNED_INT */:
          case 0x1406 /* GL_FLOAT */:
            sizeBytes = 4;
            break;
          case 0x140A /* GL_DOUBLE */:
            sizeBytes = 8;
            break;
          default:
            if (GL.currentContext.version >= 2) {
              if (dataType == 0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */ || dataType == 0x8D9F /* GL_INT_2_10_10_10_REV */) {
                sizeBytes = 4;
                break;
              } else if (dataType == 0x140B /* GL_HALF_FLOAT */) {
                sizeBytes = 2;
                break;
              } else {
                // else fall through
              }
            }
            console.error('Invalid vertex attribute data type GLenum ' + dataType + ' passed to GL function!');
        }
        if (dimension == 0x80E1 /* GL_BGRA */) {
          console.error('WebGL does not support size=GL_BGRA in a call to glVertexAttribPointer! Please use size=4 and type=GL_UNSIGNED_BYTE instead!');
        } else if (dimension < 1 || dimension > 4) {
          console.error('Invalid dimension='+dimension+' in call to glVertexAttribPointer, must be 1,2,3 or 4.');
        }
        if (stride < 0 || stride > 255) {
          console.error('Invalid stride='+stride+' in call to glVertexAttribPointer. Note that maximum supported stride in WebGL is 255!');
        }
        if (offset % sizeBytes != 0) {
          console.error('GL spec section 6.4 error: vertex attribute data offset of ' + offset + ' bytes should have been a multiple of the data type size that was used: GLenum ' + dataType + ' has size of ' + sizeBytes + ' bytes!');
        }
        if (stride % sizeBytes != 0) {
          console.error('GL spec section 6.4 error: vertex attribute data stride of ' + stride + ' bytes should have been a multiple of the data type size that was used: GLenum ' + dataType + ' has size of ' + sizeBytes + ' bytes!');
        }
      },createContext:function(canvas, webGLContextAttributes) {
  
        var ctx = 
          (webGLContextAttributes.majorVersion > 1)
          ?
            canvas.getContext("webgl2", webGLContextAttributes)
          :
          (canvas.getContext("webgl", webGLContextAttributes)
            // https://caniuse.com/#feat=webgl
            );
  
        if (!ctx) return 0;
  
        var handle = GL.registerContext(ctx, webGLContextAttributes);
  
        return handle;
      },registerContext:function(ctx, webGLContextAttributes) {
        // without pthreads a context is just an integer ID
        var handle = GL.getNewId(GL.contexts);
  
        var context = {
          handle: handle,
          attributes: webGLContextAttributes,
          version: webGLContextAttributes.majorVersion,
          GLctx: ctx
        };
  
        // Store the created context object so that we can access the context given a canvas without having to pass the parameters again.
        if (ctx.canvas) ctx.canvas.GLctxObject = context;
        GL.contexts[handle] = context;
        if (typeof webGLContextAttributes.enableExtensionsByDefault === 'undefined' || webGLContextAttributes.enableExtensionsByDefault) {
          GL.initExtensions(context);
        }
  
        return handle;
      },makeContextCurrent:function(contextHandle) {
  
        GL.currentContext = GL.contexts[contextHandle]; // Active Emscripten GL layer context object.
        Module.ctx = GLctx = GL.currentContext && GL.currentContext.GLctx; // Active WebGL context object.
        return !(contextHandle && !GLctx);
      },getContext:function(contextHandle) {
        return GL.contexts[contextHandle];
      },deleteContext:function(contextHandle) {
        if (GL.currentContext === GL.contexts[contextHandle]) GL.currentContext = null;
        if (typeof JSEvents === 'object') JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas); // Release all JS event handlers on the DOM element that the GL context is associated with since the context is now deleted.
        if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined; // Make sure the canvas object no longer refers to the context object so there are no GC surprises.
        GL.contexts[contextHandle] = null;
      },initExtensions:function(context) {
        // If this function is called without a specific context object, init the extensions of the currently active context.
        if (!context) context = GL.currentContext;
  
        if (context.initExtensionsDone) return;
        context.initExtensionsDone = true;
  
        var GLctx = context.GLctx;
  
        // Detect the presence of a few extensions manually, this GL interop layer itself will need to know if they exist.
  
        // Extensions that are only available in WebGL 1 (the calls will be no-ops if called on a WebGL 2 context active)
        __webgl_enable_ANGLE_instanced_arrays(GLctx);
        __webgl_enable_OES_vertex_array_object(GLctx);
        __webgl_enable_WEBGL_draw_buffers(GLctx);
        // Extensions that are available from WebGL >= 2 (no-op if called on a WebGL 1 context active)
        __webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
        __webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);
  
        GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
        __webgl_enable_WEBGL_multi_draw(GLctx);
  
        // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
        var exts = GLctx.getSupportedExtensions() || [];
        exts.forEach(function(ext) {
          // WEBGL_lose_context, WEBGL_debug_renderer_info and WEBGL_debug_shaders are not enabled by default.
          if (ext.indexOf('lose_context') < 0 && ext.indexOf('debug') < 0) {
            // Call .getExtension() to enable that extension permanently.
            GLctx.getExtension(ext);
          }
        });
      },populateUniformTable:function(program) {
        GL.validateGLObjectID(GL.programs, program, 'populateUniformTable', 'program');
        var p = GL.programs[program];
        var ptable = GL.programInfos[program] = {
          uniforms: {},
          maxUniformLength: 0, // This is eagerly computed below, since we already enumerate all uniforms anyway.
          maxAttributeLength: -1, // This is lazily computed and cached, computed when/if first asked, "-1" meaning not computed yet.
          maxUniformBlockNameLength: -1 // Lazily computed as well
        };
  
        var utable = ptable.uniforms;
        // A program's uniform table maps the string name of an uniform to an integer location of that uniform.
        // The global GL.uniforms map maps integer locations to WebGLUniformLocations.
        var numUniforms = GLctx.getProgramParameter(p, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
        for (var i = 0; i < numUniforms; ++i) {
          var u = GLctx.getActiveUniform(p, i);
  
          var name = u.name;
          ptable.maxUniformLength = Math.max(ptable.maxUniformLength, name.length+1);
  
          // If we are dealing with an array, e.g. vec4 foo[3], strip off the array index part to canonicalize that "foo", "foo[]",
          // and "foo[0]" will mean the same. Loop below will populate foo[1] and foo[2].
          if (name.slice(-1) == ']') {
            name = name.slice(0, name.lastIndexOf('['));
          }
  
          // Optimize memory usage slightly: If we have an array of uniforms, e.g. 'vec3 colors[3];', then
          // only store the string 'colors' in utable, and 'colors[0]', 'colors[1]' and 'colors[2]' will be parsed as 'colors'+i.
          // Note that for the GL.uniforms table, we still need to fetch the all WebGLUniformLocations for all the indices.
          var loc = GLctx.getUniformLocation(p, name);
          if (loc) {
            var id = GL.getNewId(GL.uniforms);
            utable[name] = [u.size, id];
            GL.uniforms[id] = loc;
  
            for (var j = 1; j < u.size; ++j) {
              var n = name + '['+j+']';
              loc = GLctx.getUniformLocation(p, n);
              id = GL.getNewId(GL.uniforms);
  
              GL.uniforms[id] = loc;
            }
          }
        }
      }};
  function _emscripten_glActiveTexture(x0) { GLctx['activeTexture'](x0) }

  function _emscripten_glAttachShader(program, shader) {
      GL.validateGLObjectID(GL.programs, program, 'glAttachShader', 'program');
      GL.validateGLObjectID(GL.shaders, shader, 'glAttachShader', 'shader');
      GLctx.attachShader(GL.programs[program],
                              GL.shaders[shader]);
    }

  function _emscripten_glBindAttribLocation(program, index, name) {
      GL.validateGLObjectID(GL.programs, program, 'glBindAttribLocation', 'program');
      GLctx.bindAttribLocation(GL.programs[program], index, UTF8ToString(name));
    }

  function _emscripten_glBindBuffer(target, buffer) {
      GL.validateGLObjectID(GL.buffers, buffer, 'glBindBuffer', 'buffer');
  
      if (target == 0x88EB /*GL_PIXEL_PACK_BUFFER*/) {
        // In WebGL 2 glReadPixels entry point, we need to use a different WebGL 2 API function call when a buffer is bound to
        // GL_PIXEL_PACK_BUFFER_BINDING point, so must keep track whether that binding point is non-null to know what is
        // the proper API function to call.
        GLctx.currentPixelPackBufferBinding = buffer;
      } else if (target == 0x88EC /*GL_PIXEL_UNPACK_BUFFER*/) {
        // In WebGL 2 gl(Compressed)Tex(Sub)Image[23]D entry points, we need to
        // use a different WebGL 2 API function call when a buffer is bound to
        // GL_PIXEL_UNPACK_BUFFER_BINDING point, so must keep track whether that
        // binding point is non-null to know what is the proper API function to
        // call.
        GLctx.currentPixelUnpackBufferBinding = buffer;
      }
      GLctx.bindBuffer(target, GL.buffers[buffer]);
    }

  function _emscripten_glBindFramebuffer(target, framebuffer) {
      GL.validateGLObjectID(GL.framebuffers, framebuffer, 'glBindFramebuffer', 'framebuffer');
  
      GLctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
  
    }

  function _emscripten_glBindRenderbuffer(target, renderbuffer) {
      GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glBindRenderbuffer', 'renderbuffer');
      GLctx.bindRenderbuffer(target, GL.renderbuffers[renderbuffer]);
    }

  function _emscripten_glBindSampler(unit, sampler) {
      GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
      GLctx['bindSampler'](unit, GL.samplers[sampler]);
    }

  function _emscripten_glBindTexture(target, texture) {
      GL.validateGLObjectID(GL.textures, texture, 'glBindTexture', 'texture');
      GLctx.bindTexture(target, GL.textures[texture]);
    }

  function _emscripten_glBindVertexArray(vao) {
      assert(GLctx['bindVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
      GLctx['bindVertexArray'](GL.vaos[vao]);
    }

  function _emscripten_glBindVertexArrayOES(vao) {
      assert(GLctx['bindVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
      GLctx['bindVertexArray'](GL.vaos[vao]);
    }

  function _emscripten_glBlendColor(x0, x1, x2, x3) { GLctx['blendColor'](x0, x1, x2, x3) }

  function _emscripten_glBlendEquation(x0) { GLctx['blendEquation'](x0) }

  function _emscripten_glBlendFunc(x0, x1) { GLctx['blendFunc'](x0, x1) }

  function _emscripten_glBlitFramebuffer(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) { GLctx['blitFramebuffer'](x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) }

  function _emscripten_glBufferData(target, size, data, usage) {
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        if (data) {
          GLctx.bufferData(target, HEAPU8, usage, data, size);
        } else {
          GLctx.bufferData(target, size, usage);
        }
      } else {
        // N.b. here first form specifies a heap subarray, second form an integer size, so the ?: code here is polymorphic. It is advised to avoid
        // randomly mixing both uses in calling code, to avoid any potential JS engine JIT issues.
        GLctx.bufferData(target, data ? HEAPU8.subarray(data, data+size) : size, usage);
      }
    }

  function _emscripten_glBufferSubData(target, offset, size, data) {
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.bufferSubData(target, offset, HEAPU8, data, size);
        return;
      }
      GLctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
    }

  function _emscripten_glCheckFramebufferStatus(x0) { return GLctx['checkFramebufferStatus'](x0) }

  function _emscripten_glClear(x0) { GLctx['clear'](x0) }

  function _emscripten_glClearColor(x0, x1, x2, x3) { GLctx['clearColor'](x0, x1, x2, x3) }

  function _emscripten_glClearStencil(x0) { GLctx['clearStencil'](x0) }

  function convertI32PairToI53(lo, hi) {
      // This function should not be getting called with too large unsigned numbers
      // in high part (if hi >= 0x7FFFFFFFF, one should have been calling
      // convertU32PairToI53())
      assert(hi === (hi|0));
      return (lo >>> 0) + hi * 4294967296;
    }
  function _emscripten_glClientWaitSync(sync, flags, timeoutLo, timeoutHi) {
      // WebGL2 vs GLES3 differences: in GLES3, the timeout parameter is a uint64, where 0xFFFFFFFFFFFFFFFFULL means GL_TIMEOUT_IGNORED.
      // In JS, there's no 64-bit value types, so instead timeout is taken to be signed, and GL_TIMEOUT_IGNORED is given value -1.
      // Inherently the value accepted in the timeout is lossy, and can't take in arbitrary u64 bit pattern (but most likely doesn't matter)
      // See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15
      return GLctx.clientWaitSync(GL.syncs[sync], flags, convertI32PairToI53(timeoutLo, timeoutHi));
    }

  function _emscripten_glColorMask(red, green, blue, alpha) {
      GLctx.colorMask(!!red, !!green, !!blue, !!alpha);
    }

  function _emscripten_glCompileShader(shader) {
      GL.validateGLObjectID(GL.shaders, shader, 'glCompileShader', 'shader');
      GLctx.compileShader(GL.shaders[shader]);
    }

  function _emscripten_glCompressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data) {
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, imageSize, data);
        } else {
          GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, HEAPU8, data, imageSize);
        }
        return;
      }
      GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, data ? HEAPU8.subarray((data), (data+imageSize)) : null);
    }

  function _emscripten_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, imageSize, data);
        } else {
          GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, HEAPU8, data, imageSize);
        }
        return;
      }
      GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, data ? HEAPU8.subarray((data), (data+imageSize)) : null);
    }

  function _emscripten_glCopyTexSubImage2D(x0, x1, x2, x3, x4, x5, x6, x7) { GLctx['copyTexSubImage2D'](x0, x1, x2, x3, x4, x5, x6, x7) }

  function _emscripten_glCreateProgram() {
      var id = GL.getNewId(GL.programs);
      var program = GLctx.createProgram();
      program.name = id;
      GL.programs[id] = program;
      return id;
    }

  function _emscripten_glCreateShader(shaderType) {
      var id = GL.getNewId(GL.shaders);
      GL.shaders[id] = GLctx.createShader(shaderType);
      return id;
    }

  function _emscripten_glCullFace(x0) { GLctx['cullFace'](x0) }

  function _emscripten_glDeleteBuffers(n, buffers) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((buffers)+(i*4))>>2)];
        var buffer = GL.buffers[id];
  
        // From spec: "glDeleteBuffers silently ignores 0's and names that do not
        // correspond to existing buffer objects."
        if (!buffer) continue;
  
        GLctx.deleteBuffer(buffer);
        buffer.name = 0;
        GL.buffers[id] = null;
  
        if (id == GLctx.currentPixelPackBufferBinding) GLctx.currentPixelPackBufferBinding = 0;
        if (id == GLctx.currentPixelUnpackBufferBinding) GLctx.currentPixelUnpackBufferBinding = 0;
      }
    }

  function _emscripten_glDeleteFramebuffers(n, framebuffers) {
      for (var i = 0; i < n; ++i) {
        var id = HEAP32[(((framebuffers)+(i*4))>>2)];
        var framebuffer = GL.framebuffers[id];
        if (!framebuffer) continue; // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
        GLctx.deleteFramebuffer(framebuffer);
        framebuffer.name = 0;
        GL.framebuffers[id] = null;
      }
    }

  function _emscripten_glDeleteProgram(id) {
      if (!id) return;
      var program = GL.programs[id];
      if (!program) { // glDeleteProgram actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteProgram(program);
      program.name = 0;
      GL.programs[id] = null;
      GL.programInfos[id] = null;
    }

  function _emscripten_glDeleteRenderbuffers(n, renderbuffers) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((renderbuffers)+(i*4))>>2)];
        var renderbuffer = GL.renderbuffers[id];
        if (!renderbuffer) continue; // GL spec: "glDeleteRenderbuffers silently ignores 0s and names that do not correspond to existing renderbuffer objects".
        GLctx.deleteRenderbuffer(renderbuffer);
        renderbuffer.name = 0;
        GL.renderbuffers[id] = null;
      }
    }

  function _emscripten_glDeleteSamplers(n, samplers) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((samplers)+(i*4))>>2)];
        var sampler = GL.samplers[id];
        if (!sampler) continue;
        GLctx['deleteSampler'](sampler);
        sampler.name = 0;
        GL.samplers[id] = null;
      }
    }

  function _emscripten_glDeleteShader(id) {
      if (!id) return;
      var shader = GL.shaders[id];
      if (!shader) { // glDeleteShader actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteShader(shader);
      GL.shaders[id] = null;
    }

  function _emscripten_glDeleteSync(id) {
      if (!id) return;
      var sync = GL.syncs[id];
      if (!sync) { // glDeleteSync signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteSync(sync);
      sync.name = 0;
      GL.syncs[id] = null;
    }

  function _emscripten_glDeleteTextures(n, textures) {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((textures)+(i*4))>>2)];
        var texture = GL.textures[id];
        if (!texture) continue; // GL spec: "glDeleteTextures silently ignores 0s and names that do not correspond to existing textures".
        GLctx.deleteTexture(texture);
        texture.name = 0;
        GL.textures[id] = null;
      }
    }

  function _emscripten_glDeleteVertexArrays(n, vaos) {
      assert(GLctx['deleteVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((vaos)+(i*4))>>2)];
        GLctx['deleteVertexArray'](GL.vaos[id]);
        GL.vaos[id] = null;
      }
    }

  function _emscripten_glDeleteVertexArraysOES(n, vaos) {
      assert(GLctx['deleteVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((vaos)+(i*4))>>2)];
        GLctx['deleteVertexArray'](GL.vaos[id]);
        GL.vaos[id] = null;
      }
    }

  function _emscripten_glDepthMask(flag) {
      GLctx.depthMask(!!flag);
    }

  function _emscripten_glDisable(x0) { GLctx['disable'](x0) }

  function _emscripten_glDisableVertexAttribArray(index) {
      GLctx.disableVertexAttribArray(index);
    }

  function _emscripten_glDrawArrays(mode, first, count) {
  
      GLctx.drawArrays(mode, first, count);
  
    }

  function _emscripten_glDrawArraysInstanced(mode, first, count, primcount) {
      assert(GLctx['drawArraysInstanced'], 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
      GLctx['drawArraysInstanced'](mode, first, count, primcount);
    }

  function _emscripten_glDrawArraysInstancedBaseInstanceWEBGL(mode, first, count, instanceCount, baseInstance) {
      GLctx.dibvbi['drawArraysInstancedBaseInstanceWEBGL'](mode, first, count, instanceCount, baseInstance);
    }

  var tempFixedLengthArray=[];
  function _emscripten_glDrawBuffers(n, bufs) {
      assert(GLctx['drawBuffers'], 'Must have WebGL2 or WEBGL_draw_buffers extension to use drawBuffers');
      assert(n < tempFixedLengthArray.length, 'Invalid count of numBuffers=' + n + ' passed to glDrawBuffers (that many draw buffer points do not exist in GL)');
  
      var bufArray = tempFixedLengthArray[n];
      for (var i = 0; i < n; i++) {
        bufArray[i] = HEAP32[(((bufs)+(i*4))>>2)];
      }
  
      GLctx['drawBuffers'](bufArray);
    }

  function _emscripten_glDrawElements(mode, count, type, indices) {
  
      GLctx.drawElements(mode, count, type, indices);
  
    }

  function _emscripten_glDrawElementsInstanced(mode, count, type, indices, primcount) {
      assert(GLctx['drawElementsInstanced'], 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
      GLctx['drawElementsInstanced'](mode, count, type, indices, primcount);
    }

  function _emscripten_glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(mode, count, type, offset, instanceCount, baseVertex, baseinstance) {
      GLctx.dibvbi['drawElementsInstancedBaseVertexBaseInstanceWEBGL'](mode, count, type, offset, instanceCount, baseVertex, baseinstance);
    }

  function _glDrawElements(mode, count, type, indices) {
  
      GLctx.drawElements(mode, count, type, indices);
  
    }
  function _emscripten_glDrawRangeElements(mode, start, end, count, type, indices) {
      // TODO: This should be a trivial pass-though function registered at the bottom of this page as
      // glFuncs[6][1] += ' drawRangeElements';
      // but due to https://bugzilla.mozilla.org/show_bug.cgi?id=1202427,
      // we work around by ignoring the range.
      _glDrawElements(mode, count, type, indices);
    }

  function _emscripten_glEnable(x0) { GLctx['enable'](x0) }

  function _emscripten_glEnableVertexAttribArray(index) {
      GLctx.enableVertexAttribArray(index);
    }

  function _emscripten_glFenceSync(condition, flags) {
      var sync = GLctx.fenceSync(condition, flags);
      if (sync) {
        var id = GL.getNewId(GL.syncs);
        sync.name = id;
        GL.syncs[id] = sync;
        return id;
      } else {
        return 0; // Failed to create a sync object
      }
    }

  function _emscripten_glFinish() { GLctx['finish']() }

  function _emscripten_glFlush() { GLctx['flush']() }

  function _emscripten_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer) {
      GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glFramebufferRenderbuffer', 'renderbuffer');
      GLctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                         GL.renderbuffers[renderbuffer]);
    }

  function _emscripten_glFramebufferTexture2D(target, attachment, textarget, texture, level) {
      GL.validateGLObjectID(GL.textures, texture, 'glFramebufferTexture2D', 'texture');
      GLctx.framebufferTexture2D(target, attachment, textarget,
                                      GL.textures[texture], level);
    }

  function _emscripten_glFrontFace(x0) { GLctx['frontFace'](x0) }

  function __glGenObject(n, buffers, createFunction, objectTable
      , functionName
      ) {
      for (var i = 0; i < n; i++) {
        var buffer = GLctx[createFunction]();
        var id = buffer && GL.getNewId(objectTable);
        if (buffer) {
          buffer.name = id;
          objectTable[id] = buffer;
        } else {
          GL.recordError(0x502 /* GL_INVALID_OPERATION */);
          err('GL_INVALID_OPERATION in ' + functionName + ': GLctx.' + createFunction + ' returned null - most likely GL context is lost!');
        }
        HEAP32[(((buffers)+(i*4))>>2)] = id;
      }
    }
  function _emscripten_glGenBuffers(n, buffers) {
      __glGenObject(n, buffers, 'createBuffer', GL.buffers
      , 'glGenBuffers'
        );
    }

  function _emscripten_glGenFramebuffers(n, ids) {
      __glGenObject(n, ids, 'createFramebuffer', GL.framebuffers
      , 'glGenFramebuffers'
        );
    }

  function _emscripten_glGenRenderbuffers(n, renderbuffers) {
      __glGenObject(n, renderbuffers, 'createRenderbuffer', GL.renderbuffers
      , 'glGenRenderbuffers'
        );
    }

  function _emscripten_glGenSamplers(n, samplers) {
      __glGenObject(n, samplers, 'createSampler', GL.samplers
      , 'glGenSamplers'
        );
    }

  function _emscripten_glGenTextures(n, textures) {
      __glGenObject(n, textures, 'createTexture', GL.textures
      , 'glGenTextures'
        );
    }

  function _emscripten_glGenVertexArrays(n, arrays) {
      assert(GLctx['createVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
      __glGenObject(n, arrays, 'createVertexArray', GL.vaos
      , 'glGenVertexArrays'
        );
    }

  function _emscripten_glGenVertexArraysOES(n, arrays) {
      assert(GLctx['createVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
      __glGenObject(n, arrays, 'createVertexArray', GL.vaos
      , 'glGenVertexArrays'
        );
    }

  function _emscripten_glGenerateMipmap(x0) { GLctx['generateMipmap'](x0) }

  function _emscripten_glGetBufferParameteriv(target, value, data) {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
        // if data == null, issue a GL error to notify user about it.
        err('GL_INVALID_VALUE in glGetBufferParameteriv(target=' + target + ', value=' + value + ', data=0): Function called with null out pointer!');
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((data)>>2)] = GLctx.getBufferParameter(target, value);
    }

  function _emscripten_glGetError() {
      var error = GLctx.getError() || GL.lastError;
      GL.lastError = 0/*GL_NO_ERROR*/;
      return error;
    }

  function _emscripten_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params) {
      var result = GLctx.getFramebufferAttachmentParameter(target, attachment, pname);
      if (result instanceof WebGLRenderbuffer ||
          result instanceof WebGLTexture) {
        result = result.name | 0;
      }
      HEAP32[((params)>>2)] = result;
    }

  function readI53FromI64(ptr) {
      return HEAPU32[ptr>>2] + HEAP32[ptr+4>>2] * 4294967296;
    }
  
  function readI53FromU64(ptr) {
      return HEAPU32[ptr>>2] + HEAPU32[ptr+4>>2] * 4294967296;
    }
  function writeI53ToI64(ptr, num) {
      HEAPU32[ptr>>2] = num;
      HEAPU32[ptr+4>>2] = (num - HEAPU32[ptr>>2])/4294967296;
      var deserialized = (num >= 0) ? readI53FromU64(ptr) : readI53FromI64(ptr);
      if (deserialized != num) warnOnce('writeI53ToI64() out of range: serialized JS Number ' + num + ' to Wasm heap as bytes lo=0x' + HEAPU32[ptr>>2].toString(16) + ', hi=0x' + HEAPU32[ptr+4>>2].toString(16) + ', which deserializes back to ' + deserialized + ' instead!');
    }
  function emscriptenWebGLGet(name_, p, type) {
      // Guard against user passing a null pointer.
      // Note that GLES2 spec does not say anything about how passing a null pointer should be treated.
      // Testing on desktop core GL 3, the application crashes on glGetIntegerv to a null pointer, but
      // better to report an error instead of doing anything random.
      if (!p) {
        err('GL_INVALID_VALUE in glGet' + type + 'v(name=' + name_ + ': Function called with null out pointer!');
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = undefined;
      switch(name_) { // Handle a few trivial GLES values
        case 0x8DFA: // GL_SHADER_COMPILER
          ret = 1;
          break;
        case 0x8DF8: // GL_SHADER_BINARY_FORMATS
          if (type != 0 && type != 1) {
            GL.recordError(0x500); // GL_INVALID_ENUM
            err('GL_INVALID_ENUM in glGet' + type + 'v(GL_SHADER_BINARY_FORMATS): Invalid parameter type!');
          }
          return; // Do not write anything to the out pointer, since no binary formats are supported.
        case 0x87FE: // GL_NUM_PROGRAM_BINARY_FORMATS
        case 0x8DF9: // GL_NUM_SHADER_BINARY_FORMATS
          ret = 0;
          break;
        case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
          // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be queried for length),
          // so implement it ourselves to allow C++ GLES2 code get the length.
          var formats = GLctx.getParameter(0x86A3 /*GL_COMPRESSED_TEXTURE_FORMATS*/);
          ret = formats ? formats.length : 0;
          break;
        case 0x821D: // GL_NUM_EXTENSIONS
          if (GL.currentContext.version < 2) {
            GL.recordError(0x502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
            return;
          }
          // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
          var exts = GLctx.getSupportedExtensions() || [];
          ret = 2 * exts.length; // each extension is duplicated, first in unprefixed WebGL form, and then a second time with "GL_" prefix.
          break;
        case 0x821B: // GL_MAJOR_VERSION
        case 0x821C: // GL_MINOR_VERSION
          if (GL.currentContext.version < 2) {
            GL.recordError(0x500); // GL_INVALID_ENUM
            return;
          }
          ret = name_ == 0x821B ? 3 : 0; // return version 3.0
          break;
      }
  
      if (ret === undefined) {
        var result = GLctx.getParameter(name_);
        switch (typeof(result)) {
          case "number":
            ret = result;
            break;
          case "boolean":
            ret = result ? 1 : 0;
            break;
          case "string":
            GL.recordError(0x500); // GL_INVALID_ENUM
            err('GL_INVALID_ENUM in glGet' + type + 'v(' + name_ + ') on a name which returns a string!');
            return;
          case "object":
            if (result === null) {
              // null is a valid result for some (e.g., which buffer is bound - perhaps nothing is bound), but otherwise
              // can mean an invalid name_, which we need to report as an error
              switch(name_) {
                case 0x8894: // ARRAY_BUFFER_BINDING
                case 0x8B8D: // CURRENT_PROGRAM
                case 0x8895: // ELEMENT_ARRAY_BUFFER_BINDING
                case 0x8CA6: // FRAMEBUFFER_BINDING or DRAW_FRAMEBUFFER_BINDING
                case 0x8CA7: // RENDERBUFFER_BINDING
                case 0x8069: // TEXTURE_BINDING_2D
                case 0x85B5: // WebGL 2 GL_VERTEX_ARRAY_BINDING, or WebGL 1 extension OES_vertex_array_object GL_VERTEX_ARRAY_BINDING_OES
                case 0x8F36: // COPY_READ_BUFFER_BINDING or COPY_READ_BUFFER
                case 0x8F37: // COPY_WRITE_BUFFER_BINDING or COPY_WRITE_BUFFER
                case 0x88ED: // PIXEL_PACK_BUFFER_BINDING
                case 0x88EF: // PIXEL_UNPACK_BUFFER_BINDING
                case 0x8CAA: // READ_FRAMEBUFFER_BINDING
                case 0x8919: // SAMPLER_BINDING
                case 0x8C1D: // TEXTURE_BINDING_2D_ARRAY
                case 0x806A: // TEXTURE_BINDING_3D
                case 0x8E25: // TRANSFORM_FEEDBACK_BINDING
                case 0x8C8F: // TRANSFORM_FEEDBACK_BUFFER_BINDING
                case 0x8A28: // UNIFORM_BUFFER_BINDING
                case 0x8514: { // TEXTURE_BINDING_CUBE_MAP
                  ret = 0;
                  break;
                }
                default: {
                  GL.recordError(0x500); // GL_INVALID_ENUM
                  err('GL_INVALID_ENUM in glGet' + type + 'v(' + name_ + ') and it returns null!');
                  return;
                }
              }
            } else if (result instanceof Float32Array ||
                       result instanceof Uint32Array ||
                       result instanceof Int32Array ||
                       result instanceof Array) {
              for (var i = 0; i < result.length; ++i) {
                switch (type) {
                  case 0: HEAP32[(((p)+(i*4))>>2)] = result[i]; break;
                  case 2: HEAPF32[(((p)+(i*4))>>2)] = result[i]; break;
                  case 4: HEAP8[(((p)+(i))>>0)] = result[i] ? 1 : 0; break;
                  default: throw 'internal glGet error, bad type: ' + type;
                }
              }
              return;
            } else {
              try {
                ret = result.name | 0;
              } catch(e) {
                GL.recordError(0x500); // GL_INVALID_ENUM
                err('GL_INVALID_ENUM in glGet' + type + 'v: Unknown object returned from WebGL getParameter(' + name_ + ')! (error: ' + e + ')');
                return;
              }
            }
            break;
          default:
            GL.recordError(0x500); // GL_INVALID_ENUM
            err('GL_INVALID_ENUM in glGet' + type + 'v: Native code calling glGet' + type + 'v(' + name_ + ') and it returns ' + result + ' of type ' + typeof(result) + '!');
            return;
        }
      }
  
      switch (type) {
        case 1: writeI53ToI64(p, ret); break;
        case 0: HEAP32[((p)>>2)] = ret; break;
        case 2:   HEAPF32[((p)>>2)] = ret; break;
        case 4: HEAP8[((p)>>0)] = ret ? 1 : 0; break;
        default: throw 'internal glGet error, bad type: ' + type;
      }
    }
  function _emscripten_glGetIntegerv(name_, p) {
      emscriptenWebGLGet(name_, p, 0);
    }

  function _emscripten_glGetProgramInfoLog(program, maxLength, length, infoLog) {
      GL.validateGLObjectID(GL.programs, program, 'glGetProgramInfoLog', 'program');
      var log = GLctx.getProgramInfoLog(GL.programs[program]);
      if (log === null) log = '(unknown error)';
      var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    }

  function _emscripten_glGetProgramiv(program, pname, p) {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        err('GL_INVALID_VALUE in glGetProgramiv(program=' + program + ', pname=' + pname + ', p=0): Function called with null out pointer!');
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GL.validateGLObjectID(GL.programs, program, 'glGetProgramiv', 'program');
  
      if (program >= GL.counter) {
        err('GL_INVALID_VALUE in glGetProgramiv(program=' + program + ', pname=' + pname + ', p=0x' + p.toString(16) + '): The specified program object name was not generated by GL!');
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
  
      var ptable = GL.programInfos[program];
      if (!ptable) {
        err('GL_INVALID_OPERATION in glGetProgramiv(program=' + program + ', pname=' + pname + ', p=0x' + p.toString(16) + '): The specified GL object name does not refer to a program object!');
        GL.recordError(0x502 /* GL_INVALID_OPERATION */);
        return;
      }
  
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getProgramInfoLog(GL.programs[program]);
        if (log === null) log = '(unknown error)';
        HEAP32[((p)>>2)] = log.length + 1;
      } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
        HEAP32[((p)>>2)] = ptable.maxUniformLength;
      } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
        if (ptable.maxAttributeLength == -1) {
          program = GL.programs[program];
          var numAttribs = GLctx.getProgramParameter(program, 0x8B89/*GL_ACTIVE_ATTRIBUTES*/);
          ptable.maxAttributeLength = 0; // Spec says if there are no active attribs, 0 must be returned.
          for (var i = 0; i < numAttribs; ++i) {
            var activeAttrib = GLctx.getActiveAttrib(program, i);
            ptable.maxAttributeLength = Math.max(ptable.maxAttributeLength, activeAttrib.name.length+1);
          }
        }
        HEAP32[((p)>>2)] = ptable.maxAttributeLength;
      } else if (pname == 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */) {
        if (ptable.maxUniformBlockNameLength == -1) {
          program = GL.programs[program];
          var numBlocks = GLctx.getProgramParameter(program, 0x8A36/*GL_ACTIVE_UNIFORM_BLOCKS*/);
          ptable.maxUniformBlockNameLength = 0;
          for (var i = 0; i < numBlocks; ++i) {
            var activeBlockName = GLctx.getActiveUniformBlockName(program, i);
            ptable.maxUniformBlockNameLength = Math.max(ptable.maxUniformBlockNameLength, activeBlockName.length+1);
          }
        }
        HEAP32[((p)>>2)] = ptable.maxUniformBlockNameLength;
      } else {
        HEAP32[((p)>>2)] = GLctx.getProgramParameter(GL.programs[program], pname);
      }
    }

  function _emscripten_glGetRenderbufferParameteriv(target, pname, params) {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it.
        err('GL_INVALID_VALUE in glGetRenderbufferParameteriv(target=' + target + ', pname=' + pname + ', params=0): Function called with null out pointer!');
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getRenderbufferParameter(target, pname);
    }

  function _emscripten_glGetShaderInfoLog(shader, maxLength, length, infoLog) {
      GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderInfoLog', 'shader');
      var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
      if (log === null) log = '(unknown error)';
      var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    }

  function _emscripten_glGetShaderPrecisionFormat(shaderType, precisionType, range, precision) {
      var result = GLctx.getShaderPrecisionFormat(shaderType, precisionType);
      HEAP32[((range)>>2)] = result.rangeMin;
      HEAP32[(((range)+(4))>>2)] = result.rangeMax;
      HEAP32[((precision)>>2)] = result.precision;
    }

  function _emscripten_glGetShaderiv(shader, pname, p) {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        err('GL_INVALID_VALUE in glGetShaderiv(shader=' + shader + ', pname=' + pname + ', p=0): Function called with null out pointer!');
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderiv', 'shader');
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
        if (log === null) log = '(unknown error)';
        // The GLES2 specification says that if the shader has an empty info log,
        // a value of 0 is returned. Otherwise the log has a null char appended.
        // (An empty string is falsey, so we can just check that instead of
        // looking at log.length.)
        var logLength = log ? log.length + 1 : 0;
        HEAP32[((p)>>2)] = logLength;
      } else if (pname == 0x8B88) { // GL_SHADER_SOURCE_LENGTH
        var source = GLctx.getShaderSource(GL.shaders[shader]);
        // source may be a null, or the empty string, both of which are falsey
        // values that we report a 0 length for.
        var sourceLength = source ? source.length + 1 : 0;
        HEAP32[((p)>>2)] = sourceLength;
      } else {
        HEAP32[((p)>>2)] = GLctx.getShaderParameter(GL.shaders[shader], pname);
      }
    }

  function stringToNewUTF8(jsString) {
      var length = lengthBytesUTF8(jsString)+1;
      var cString = _malloc(length);
      stringToUTF8(jsString, cString, length);
      return cString;
    }
  function _emscripten_glGetString(name_) {
      if (GL.stringCache[name_]) return GL.stringCache[name_];
      var ret;
      switch(name_) {
        case 0x1F03 /* GL_EXTENSIONS */:
          var exts = GLctx.getSupportedExtensions() || []; // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
          exts = exts.concat(exts.map(function(e) { return "GL_" + e; }));
          ret = stringToNewUTF8(exts.join(' '));
          break;
        case 0x1F00 /* GL_VENDOR */:
        case 0x1F01 /* GL_RENDERER */:
        case 0x9245 /* UNMASKED_VENDOR_WEBGL */:
        case 0x9246 /* UNMASKED_RENDERER_WEBGL */:
          var s = GLctx.getParameter(name_);
          if (!s) {
            GL.recordError(0x500/*GL_INVALID_ENUM*/);
            err('GL_INVALID_ENUM in glGetString: Received empty parameter for query name ' + name_ + '!'); // This occurs e.g. if one attempts GL_UNMASKED_VENDOR_WEBGL when it is not supported.
          }
          ret = stringToNewUTF8(s);
          break;
  
        case 0x1F02 /* GL_VERSION */:
          var glVersion = GLctx.getParameter(0x1F02 /*GL_VERSION*/);
          // return GLES version string corresponding to the version of the WebGL context
          if (GL.currentContext.version >= 2) glVersion = 'OpenGL ES 3.0 (' + glVersion + ')';
          else
          {
            glVersion = 'OpenGL ES 2.0 (' + glVersion + ')';
          }
          ret = stringToNewUTF8(glVersion);
          break;
        case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
          var glslVersion = GLctx.getParameter(0x8B8C /*GL_SHADING_LANGUAGE_VERSION*/);
          // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
          var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
          var ver_num = glslVersion.match(ver_re);
          if (ver_num !== null) {
            if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + '0'; // ensure minor version has 2 digits
            glslVersion = 'OpenGL ES GLSL ES ' + ver_num[1] + ' (' + glslVersion + ')';
          }
          ret = stringToNewUTF8(glslVersion);
          break;
        default:
          GL.recordError(0x500/*GL_INVALID_ENUM*/);
          err('GL_INVALID_ENUM in glGetString: Unknown parameter ' + name_ + '!');
          return 0;
      }
      GL.stringCache[name_] = ret;
      return ret;
    }

  function _emscripten_glGetStringi(name, index) {
      if (GL.currentContext.version < 2) {
        GL.recordError(0x502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
        return 0;
      }
      var stringiCache = GL.stringiCache[name];
      if (stringiCache) {
        if (index < 0 || index >= stringiCache.length) {
          GL.recordError(0x501/*GL_INVALID_VALUE*/);
          err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ')!');
          return 0;
        }
        return stringiCache[index];
      }
      switch(name) {
        case 0x1F03 /* GL_EXTENSIONS */:
          var exts = GLctx.getSupportedExtensions() || []; // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
          exts = exts.concat(exts.map(function(e) { return "GL_" + e; }));
          exts = exts.map(function(e) { return stringToNewUTF8(e); });
  
          stringiCache = GL.stringiCache[name] = exts;
          if (index < 0 || index >= stringiCache.length) {
            GL.recordError(0x501/*GL_INVALID_VALUE*/);
            err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ') in a call to GL_EXTENSIONS!');
            return 0;
          }
          return stringiCache[index];
        default:
          GL.recordError(0x500/*GL_INVALID_ENUM*/);
          err('GL_INVALID_ENUM in glGetStringi: Unknown parameter ' + name + '!');
          return 0;
      }
    }

  /** @suppress {checkTypes} */
  function jstoi_q(str) {
      return parseInt(str);
    }
  function _emscripten_glGetUniformLocation(program, name) {
      GL.validateGLObjectID(GL.programs, program, 'glGetUniformLocation', 'program');
      name = UTF8ToString(name);
  
      var arrayIndex = 0;
      // If user passed an array accessor "[index]", parse the array index off the accessor.
      if (name[name.length - 1] == ']') {
        var leftBrace = name.lastIndexOf('[');
        arrayIndex = name[leftBrace+1] != ']' ? jstoi_q(name.slice(leftBrace + 1)) : 0; // "index]", parseInt will ignore the ']' at the end; but treat "foo[]" as "foo[0]"
        name = name.slice(0, leftBrace);
      }
  
      var uniformInfo = GL.programInfos[program] && GL.programInfos[program].uniforms[name]; // returns pair [ dimension_of_uniform_array, uniform_location ]
      if (uniformInfo && arrayIndex >= 0 && arrayIndex < uniformInfo[0]) { // Check if user asked for an out-of-bounds element, i.e. for 'vec4 colors[3];' user could ask for 'colors[10]' which should return -1.
        return uniformInfo[1] + arrayIndex;
      } else {
        return -1;
      }
    }

  function _emscripten_glInvalidateFramebuffer(target, numAttachments, attachments) {
      assert(numAttachments < tempFixedLengthArray.length, 'Invalid count of numAttachments=' + numAttachments + ' passed to glInvalidateFramebuffer (that many attachment points do not exist in GL)');
      var list = tempFixedLengthArray[numAttachments];
      for (var i = 0; i < numAttachments; i++) {
        list[i] = HEAP32[(((attachments)+(i*4))>>2)];
      }
  
      GLctx['invalidateFramebuffer'](target, list);
    }

  function _emscripten_glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height) {
      assert(numAttachments < tempFixedLengthArray.length, 'Invalid count of numAttachments=' + numAttachments + ' passed to glInvalidateSubFramebuffer (that many attachment points do not exist in GL)');
      var list = tempFixedLengthArray[numAttachments];
      for (var i = 0; i < numAttachments; i++) {
        list[i] = HEAP32[(((attachments)+(i*4))>>2)];
      }
  
      GLctx['invalidateSubFramebuffer'](target, list, x, y, width, height);
    }

  function _emscripten_glIsSync(sync) {
      return GLctx.isSync(GL.syncs[sync]);
    }

  function _emscripten_glIsTexture(id) {
      var texture = GL.textures[id];
      if (!texture) return 0;
      return GLctx.isTexture(texture);
    }

  function _emscripten_glLineWidth(x0) { GLctx['lineWidth'](x0) }

  function _emscripten_glLinkProgram(program) {
      GL.validateGLObjectID(GL.programs, program, 'glLinkProgram', 'program');
      GLctx.linkProgram(GL.programs[program]);
      GL.populateUniformTable(program);
    }

  function _emscripten_glMultiDrawArraysInstancedBaseInstanceWEBGL(mode, firsts, counts, instanceCounts, baseInstances, drawCount) {
      GLctx.mdibvbi['multiDrawArraysInstancedBaseInstanceWEBGL'](
        mode,
        HEAP32,
        firsts >> 2,
        HEAP32,
        counts >> 2,
        HEAP32,
        instanceCounts >> 2,
        HEAPU32,
        baseInstances >> 2,
        drawCount);
    }

  function _emscripten_glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(mode, counts, type, offsets, instanceCounts, baseVertices, baseInstances, drawCount) {
      GLctx.mdibvbi['multiDrawElementsInstancedBaseVertexBaseInstanceWEBGL'](
        mode,
        HEAP32,
        counts >> 2,
        type,
        HEAP32,
        offsets >> 2,
        HEAP32,
        instanceCounts >> 2,
        HEAP32,
        baseVertices >> 2,
        HEAPU32,
        baseInstances >> 2,
        drawCount);
    }

  function _emscripten_glPixelStorei(pname, param) {
      if (pname == 0xCF5 /* GL_UNPACK_ALIGNMENT */) {
        GL.unpackAlignment = param;
      }
      GLctx.pixelStorei(pname, param);
    }

  function _emscripten_glReadBuffer(x0) { GLctx['readBuffer'](x0) }

  function computeUnpackAlignedImageSize(width, height, sizePerPixel, alignment) {
      function roundedToNextMultipleOf(x, y) {
        assert((y & (y-1)) === 0, 'Unpack alignment must be a power of 2! (Allowed values per WebGL spec are 1, 2, 4 or 8)');
        return (x + y - 1) & -y;
      }
      var plainRowSize = width * sizePerPixel;
      var alignedRowSize = roundedToNextMultipleOf(plainRowSize, alignment);
      return height * alignedRowSize;
    }
  
  function __colorChannelsInGlTextureFormat(format) {
      // Micro-optimizations for size: map format to size by subtracting smallest enum value (0x1902) from all values first.
      // Also omit the most common size value (1) from the list, which is assumed by formats not on the list.
      var colorChannels = {
        // 0x1902 /* GL_DEPTH_COMPONENT */ - 0x1902: 1,
        // 0x1906 /* GL_ALPHA */ - 0x1902: 1,
        5: 3,
        6: 4,
        // 0x1909 /* GL_LUMINANCE */ - 0x1902: 1,
        8: 2,
        29502: 3,
        29504: 4,
        // 0x1903 /* GL_RED */ - 0x1902: 1,
        26917: 2,
        26918: 2,
        // 0x8D94 /* GL_RED_INTEGER */ - 0x1902: 1,
        29846: 3,
        29847: 4
      };
      if (!colorChannels[format - 0x1902]
        && format != 0x1902 /* GL_DEPTH_COMPONENT */
        && format != 0x1906 /* GL_ALPHA */
        && format != 0x1909 /* GL_LUMINANCE */
        && format != 0x1903 /* GL_RED */
        && format != 0x8D94 /* GL_RED_INTEGER */) {
        err('Invalid format=0x' + format.toString(16) + ' passed to function _colorChannelsInGlTextureFormat()!');
      }
      return colorChannels[format - 0x1902]||1;
    }
  
  function heapObjectForWebGLType(type) {
      // Micro-optimization for size: Subtract lowest GL enum number (0x1400/* GL_BYTE */) from type to compare
      // smaller values for the heap, for shorter generated code size.
      // Also the type HEAPU16 is not tested for explicitly, but any unrecognized type will return out HEAPU16.
      // (since most types are HEAPU16)
      type -= 0x1400;
      if (type == 0) return HEAP8;
  
      if (type == 1) return HEAPU8;
  
      if (type == 2) return HEAP16;
  
      if (type == 4) return HEAP32;
  
      if (type == 6) return HEAPF32;
  
      if (type == 5
        || type == 28922
        || type == 28520
        || type == 30779
        || type == 30782
        )
        return HEAPU32;
  
        if (type != 3
          && type != 11
          && type != 27699
          && type != 27700
          && type != 28515
          && type != 31073) {
          err('Invalid WebGL type 0x' + (type+0x1400).toString() + ' passed to $heapObjectForWebGLType!');
        }
      return HEAPU16;
    }
  
  function heapAccessShiftForWebGLHeap(heap) {
      return 31 - Math.clz32(heap.BYTES_PER_ELEMENT);
    }
  function emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) {
      var heap = heapObjectForWebGLType(type);
      var shift = heapAccessShiftForWebGLHeap(heap);
      var byteSize = 1<<shift;
      var sizePerPixel = __colorChannelsInGlTextureFormat(format) * byteSize;
      var bytes = computeUnpackAlignedImageSize(width, height, sizePerPixel, GL.unpackAlignment);
      assert((pixels >> shift) << shift == pixels, 'Pointer to texture data passed to texture get function must be aligned to the byte size of the pixel type!');
      return heap.subarray(pixels >> shift, pixels + bytes >> shift);
    }
  function _emscripten_glReadPixels(x, y, width, height, format, type, pixels) {
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        if (GLctx.currentPixelPackBufferBinding) {
          GLctx.readPixels(x, y, width, height, format, type, pixels);
        } else {
          var heap = heapObjectForWebGLType(type);
          GLctx.readPixels(x, y, width, height, format, type, heap, pixels >> heapAccessShiftForWebGLHeap(heap));
        }
        return;
      }
      var pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, format);
      if (!pixelData) {
        GL.recordError(0x500/*GL_INVALID_ENUM*/);
        err('GL_INVALID_ENUM in glReadPixels: Unrecognized combination of type=' + type + ' and format=' + format + '!');
        return;
      }
      GLctx.readPixels(x, y, width, height, format, type, pixelData);
    }

  function _emscripten_glRenderbufferStorage(x0, x1, x2, x3) { GLctx['renderbufferStorage'](x0, x1, x2, x3) }

  function _emscripten_glRenderbufferStorageMultisample(x0, x1, x2, x3, x4) { GLctx['renderbufferStorageMultisample'](x0, x1, x2, x3, x4) }

  function _emscripten_glSamplerParameteri(sampler, pname, param) {
      GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
      GLctx['samplerParameteri'](GL.samplers[sampler], pname, param);
    }

  function _emscripten_glSamplerParameteriv(sampler, pname, params) {
      GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
      var param = HEAP32[((params)>>2)];
      GLctx['samplerParameteri'](GL.samplers[sampler], pname, param);
    }

  function _emscripten_glScissor(x0, x1, x2, x3) { GLctx['scissor'](x0, x1, x2, x3) }

  function _emscripten_glShaderSource(shader, count, string, length) {
      GL.validateGLObjectID(GL.shaders, shader, 'glShaderSource', 'shader');
      var source = GL.getSource(shader, count, string, length);
  
      GLctx.shaderSource(GL.shaders[shader], source);
    }

  function _emscripten_glStencilFunc(x0, x1, x2) { GLctx['stencilFunc'](x0, x1, x2) }

  function _emscripten_glStencilFuncSeparate(x0, x1, x2, x3) { GLctx['stencilFuncSeparate'](x0, x1, x2, x3) }

  function _emscripten_glStencilMask(x0) { GLctx['stencilMask'](x0) }

  function _emscripten_glStencilMaskSeparate(x0, x1) { GLctx['stencilMaskSeparate'](x0, x1) }

  function _emscripten_glStencilOp(x0, x1, x2) { GLctx['stencilOp'](x0, x1, x2) }

  function _emscripten_glStencilOpSeparate(x0, x1, x2, x3) { GLctx['stencilOpSeparate'](x0, x1, x2, x3) }

  function _emscripten_glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels) {
      if (GL.currentContext.version >= 2) {
        // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
        } else if (pixels) {
          var heap = heapObjectForWebGLType(type);
          GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, heap, pixels >> heapAccessShiftForWebGLHeap(heap));
        } else {
          GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, null);
        }
        return;
      }
      GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) : null);
    }

  function _emscripten_glTexParameterf(x0, x1, x2) { GLctx['texParameterf'](x0, x1, x2) }

  function _emscripten_glTexParameterfv(target, pname, params) {
      var param = HEAPF32[((params)>>2)];
      GLctx.texParameterf(target, pname, param);
    }

  function _emscripten_glTexParameteri(x0, x1, x2) { GLctx['texParameteri'](x0, x1, x2) }

  function _emscripten_glTexParameteriv(target, pname, params) {
      var param = HEAP32[((params)>>2)];
      GLctx.texParameteri(target, pname, param);
    }

  function _emscripten_glTexStorage2D(x0, x1, x2, x3, x4) { GLctx['texStorage2D'](x0, x1, x2, x3, x4) }

  function _emscripten_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) {
      if (GL.currentContext.version >= 2) {
        // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        } else if (pixels) {
          var heap = heapObjectForWebGLType(type);
          GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, heap, pixels >> heapAccessShiftForWebGLHeap(heap));
        } else {
          GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, null);
        }
        return;
      }
      var pixelData = null;
      if (pixels) pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, 0);
      GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
    }

  function _emscripten_glUniform1f(location, v0) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform1f', 'location');
      GLctx.uniform1f(GL.uniforms[location], v0);
    }

  var miniTempWebGLFloatBuffers=[];
  function _emscripten_glUniform1fv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform1fv', 'location');
      assert((value & 3) == 0, 'Pointer to float data passed to glUniform1fv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform1fv(GL.uniforms[location], HEAPF32, value>>2, count);
        return;
      }
  
      if (count <= 288) {
        // avoid allocation when uploading few enough uniforms
        var view = miniTempWebGLFloatBuffers[count-1];
        for (var i = 0; i < count; ++i) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
        }
      } else
      {
        var view = HEAPF32.subarray((value)>>2, (value+count*4)>>2);
      }
      GLctx.uniform1fv(GL.uniforms[location], view);
    }

  function _emscripten_glUniform1i(location, v0) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform1i', 'location');
      GLctx.uniform1i(GL.uniforms[location], v0);
    }

  var __miniTempWebGLIntBuffers=[];
  function _emscripten_glUniform1iv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform1iv', 'location');
      assert((value & 3) == 0, 'Pointer to integer data passed to glUniform1iv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform1iv(GL.uniforms[location], HEAP32, value>>2, count);
        return;
      }
  
      if (count <= 288) {
        // avoid allocation when uploading few enough uniforms
        var view = __miniTempWebGLIntBuffers[count-1];
        for (var i = 0; i < count; ++i) {
          view[i] = HEAP32[(((value)+(4*i))>>2)];
        }
      } else
      {
        var view = HEAP32.subarray((value)>>2, (value+count*4)>>2);
      }
      GLctx.uniform1iv(GL.uniforms[location], view);
    }

  function _emscripten_glUniform2f(location, v0, v1) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform2f', 'location');
      GLctx.uniform2f(GL.uniforms[location], v0, v1);
    }

  function _emscripten_glUniform2fv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform2fv', 'location');
      assert((value & 3) == 0, 'Pointer to float data passed to glUniform2fv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform2fv(GL.uniforms[location], HEAPF32, value>>2, count*2);
        return;
      }
  
      if (count <= 144) {
        // avoid allocation when uploading few enough uniforms
        var view = miniTempWebGLFloatBuffers[2*count-1];
        for (var i = 0; i < 2*count; i += 2) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
        }
      } else
      {
        var view = HEAPF32.subarray((value)>>2, (value+count*8)>>2);
      }
      GLctx.uniform2fv(GL.uniforms[location], view);
    }

  function _emscripten_glUniform2i(location, v0, v1) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform2i', 'location');
      GLctx.uniform2i(GL.uniforms[location], v0, v1);
    }

  function _emscripten_glUniform2iv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform2iv', 'location');
      assert((value & 3) == 0, 'Pointer to integer data passed to glUniform2iv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform2iv(GL.uniforms[location], HEAP32, value>>2, count*2);
        return;
      }
  
      if (count <= 144) {
        // avoid allocation when uploading few enough uniforms
        var view = __miniTempWebGLIntBuffers[2*count-1];
        for (var i = 0; i < 2*count; i += 2) {
          view[i] = HEAP32[(((value)+(4*i))>>2)];
          view[i+1] = HEAP32[(((value)+(4*i+4))>>2)];
        }
      } else
      {
        var view = HEAP32.subarray((value)>>2, (value+count*8)>>2);
      }
      GLctx.uniform2iv(GL.uniforms[location], view);
    }

  function _emscripten_glUniform3f(location, v0, v1, v2) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform3f', 'location');
      GLctx.uniform3f(GL.uniforms[location], v0, v1, v2);
    }

  function _emscripten_glUniform3fv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform3fv', 'location');
      assert((value & 3) == 0, 'Pointer to float data passed to glUniform3fv must be aligned to four bytes!' + value);
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform3fv(GL.uniforms[location], HEAPF32, value>>2, count*3);
        return;
      }
  
      if (count <= 96) {
        // avoid allocation when uploading few enough uniforms
        var view = miniTempWebGLFloatBuffers[3*count-1];
        for (var i = 0; i < 3*count; i += 3) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
        }
      } else
      {
        var view = HEAPF32.subarray((value)>>2, (value+count*12)>>2);
      }
      GLctx.uniform3fv(GL.uniforms[location], view);
    }

  function _emscripten_glUniform3i(location, v0, v1, v2) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform3i', 'location');
      GLctx.uniform3i(GL.uniforms[location], v0, v1, v2);
    }

  function _emscripten_glUniform3iv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform3iv', 'location');
      assert((value & 3) == 0, 'Pointer to integer data passed to glUniform3iv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform3iv(GL.uniforms[location], HEAP32, value>>2, count*3);
        return;
      }
  
      if (count <= 96) {
        // avoid allocation when uploading few enough uniforms
        var view = __miniTempWebGLIntBuffers[3*count-1];
        for (var i = 0; i < 3*count; i += 3) {
          view[i] = HEAP32[(((value)+(4*i))>>2)];
          view[i+1] = HEAP32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAP32[(((value)+(4*i+8))>>2)];
        }
      } else
      {
        var view = HEAP32.subarray((value)>>2, (value+count*12)>>2);
      }
      GLctx.uniform3iv(GL.uniforms[location], view);
    }

  function _emscripten_glUniform4f(location, v0, v1, v2, v3) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform4f', 'location');
      GLctx.uniform4f(GL.uniforms[location], v0, v1, v2, v3);
    }

  function _emscripten_glUniform4fv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform4fv', 'location');
      assert((value & 3) == 0, 'Pointer to float data passed to glUniform4fv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform4fv(GL.uniforms[location], HEAPF32, value>>2, count*4);
        return;
      }
  
      if (count <= 72) {
        // avoid allocation when uploading few enough uniforms
        var view = miniTempWebGLFloatBuffers[4*count-1];
        // hoist the heap out of the loop for size and for pthreads+growth.
        var heap = HEAPF32;
        value >>= 2;
        for (var i = 0; i < 4 * count; i += 4) {
          var dst = value + i;
          view[i] = heap[dst];
          view[i + 1] = heap[dst + 1];
          view[i + 2] = heap[dst + 2];
          view[i + 3] = heap[dst + 3];
        }
      } else
      {
        var view = HEAPF32.subarray((value)>>2, (value+count*16)>>2);
      }
      GLctx.uniform4fv(GL.uniforms[location], view);
    }

  function _emscripten_glUniform4i(location, v0, v1, v2, v3) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform4i', 'location');
      GLctx.uniform4i(GL.uniforms[location], v0, v1, v2, v3);
    }

  function _emscripten_glUniform4iv(location, count, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniform4iv', 'location');
      assert((value & 3) == 0, 'Pointer to integer data passed to glUniform4iv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniform4iv(GL.uniforms[location], HEAP32, value>>2, count*4);
        return;
      }
  
      if (count <= 72) {
        // avoid allocation when uploading few enough uniforms
        var view = __miniTempWebGLIntBuffers[4*count-1];
        for (var i = 0; i < 4*count; i += 4) {
          view[i] = HEAP32[(((value)+(4*i))>>2)];
          view[i+1] = HEAP32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAP32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAP32[(((value)+(4*i+12))>>2)];
        }
      } else
      {
        var view = HEAP32.subarray((value)>>2, (value+count*16)>>2);
      }
      GLctx.uniform4iv(GL.uniforms[location], view);
    }

  function _emscripten_glUniformMatrix2fv(location, count, transpose, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix2fv', 'location');
      assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix2fv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniformMatrix2fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*4);
        return;
      }
  
      if (count <= 72) {
        // avoid allocation when uploading few enough uniforms
        var view = miniTempWebGLFloatBuffers[4*count-1];
        for (var i = 0; i < 4*count; i += 4) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAPF32[(((value)+(4*i+12))>>2)];
        }
      } else
      {
        var view = HEAPF32.subarray((value)>>2, (value+count*16)>>2);
      }
      GLctx.uniformMatrix2fv(GL.uniforms[location], !!transpose, view);
    }

  function _emscripten_glUniformMatrix3fv(location, count, transpose, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix3fv', 'location');
      assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix3fv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniformMatrix3fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*9);
        return;
      }
  
      if (count <= 32) {
        // avoid allocation when uploading few enough uniforms
        var view = miniTempWebGLFloatBuffers[9*count-1];
        for (var i = 0; i < 9*count; i += 9) {
          view[i] = HEAPF32[(((value)+(4*i))>>2)];
          view[i+1] = HEAPF32[(((value)+(4*i+4))>>2)];
          view[i+2] = HEAPF32[(((value)+(4*i+8))>>2)];
          view[i+3] = HEAPF32[(((value)+(4*i+12))>>2)];
          view[i+4] = HEAPF32[(((value)+(4*i+16))>>2)];
          view[i+5] = HEAPF32[(((value)+(4*i+20))>>2)];
          view[i+6] = HEAPF32[(((value)+(4*i+24))>>2)];
          view[i+7] = HEAPF32[(((value)+(4*i+28))>>2)];
          view[i+8] = HEAPF32[(((value)+(4*i+32))>>2)];
        }
      } else
      {
        var view = HEAPF32.subarray((value)>>2, (value+count*36)>>2);
      }
      GLctx.uniformMatrix3fv(GL.uniforms[location], !!transpose, view);
    }

  function _emscripten_glUniformMatrix4fv(location, count, transpose, value) {
      GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix4fv', 'location');
      assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix4fv must be aligned to four bytes!');
  
      if (GL.currentContext.version >= 2) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.uniformMatrix4fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*16);
        return;
      }
  
      if (count <= 18) {
        // avoid allocation when uploading few enough uniforms
        var view = miniTempWebGLFloatBuffers[16*count-1];
        // hoist the heap out of the loop for size and for pthreads+growth.
        var heap = HEAPF32;
        value >>= 2;
        for (var i = 0; i < 16 * count; i += 16) {
          var dst = value + i;
          view[i] = heap[dst];
          view[i + 1] = heap[dst + 1];
          view[i + 2] = heap[dst + 2];
          view[i + 3] = heap[dst + 3];
          view[i + 4] = heap[dst + 4];
          view[i + 5] = heap[dst + 5];
          view[i + 6] = heap[dst + 6];
          view[i + 7] = heap[dst + 7];
          view[i + 8] = heap[dst + 8];
          view[i + 9] = heap[dst + 9];
          view[i + 10] = heap[dst + 10];
          view[i + 11] = heap[dst + 11];
          view[i + 12] = heap[dst + 12];
          view[i + 13] = heap[dst + 13];
          view[i + 14] = heap[dst + 14];
          view[i + 15] = heap[dst + 15];
        }
      } else
      {
        var view = HEAPF32.subarray((value)>>2, (value+count*64)>>2);
      }
      GLctx.uniformMatrix4fv(GL.uniforms[location], !!transpose, view);
    }

  function _emscripten_glUseProgram(program) {
      GL.validateGLObjectID(GL.programs, program, 'glUseProgram', 'program');
      GLctx.useProgram(GL.programs[program]);
    }

  function _emscripten_glVertexAttrib1f(x0, x1) { GLctx['vertexAttrib1f'](x0, x1) }

  function _emscripten_glVertexAttrib2fv(index, v) {
      assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib2fv must be aligned to four bytes!');
      assert(v != 0, 'Null pointer passed to glVertexAttrib2fv!');
  
      GLctx.vertexAttrib2f(index, HEAPF32[v>>2], HEAPF32[v+4>>2]);
    }

  function _emscripten_glVertexAttrib3fv(index, v) {
      assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib3fv must be aligned to four bytes!');
      assert(v != 0, 'Null pointer passed to glVertexAttrib3fv!');
  
      GLctx.vertexAttrib3f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2]);
    }

  function _emscripten_glVertexAttrib4fv(index, v) {
      assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib4fv must be aligned to four bytes!');
      assert(v != 0, 'Null pointer passed to glVertexAttrib4fv!');
  
      GLctx.vertexAttrib4f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2], HEAPF32[v+12>>2]);
    }

  function _emscripten_glVertexAttribDivisor(index, divisor) {
      assert(GLctx['vertexAttribDivisor'], 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
      GLctx['vertexAttribDivisor'](index, divisor);
    }

  function _emscripten_glVertexAttribIPointer(index, size, type, stride, ptr) {
      GL.validateVertexAttribPointer(size, type, stride, ptr);
      GLctx['vertexAttribIPointer'](index, size, type, stride, ptr);
    }

  function _emscripten_glVertexAttribPointer(index, size, type, normalized, stride, ptr) {
      GL.validateVertexAttribPointer(size, type, stride, ptr);
      GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
    }

  function _emscripten_glViewport(x0, x1, x2, x3) { GLctx['viewport'](x0, x1, x2, x3) }

  function _emscripten_glWaitSync(sync, flags, timeoutLo, timeoutHi) {
      // See WebGL2 vs GLES3 difference on GL_TIMEOUT_IGNORED above (https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15)
      GLctx.waitSync(GL.syncs[sync], flags, convertI32PairToI53(timeoutLo, timeoutHi));
    }

  function _longjmp(env, value) {
      _setThrew(env, value || 1);
      throw 'longjmp';
    }
  function _emscripten_longjmp(a0,a1
  ) {
  return _longjmp(a0,a1);
  }

  function _emscripten_memcpy_big(dest, src, num) {
      HEAPU8.copyWithin(dest, src, src + num);
    }

  function _emscripten_get_heap_size() {
      return HEAPU8.length;
    }
  
  function emscripten_realloc_buffer(size) {
      try {
        // round size grow request up to wasm page size (fixed 64KB per spec)
        wasmMemory.grow((size - buffer.byteLength + 65535) >>> 16); // .grow() takes a delta compared to the previous size
        updateGlobalBufferAndViews(wasmMemory.buffer);
        return 1 /*success*/;
      } catch(e) {
        console.error('emscripten_realloc_buffer: Attempted to grow heap from ' + buffer.byteLength  + ' bytes to ' + size + ' bytes, but got error: ' + e);
      }
      // implicit 0 return to save code size (caller will cast "undefined" into 0
      // anyhow)
    }
  function _emscripten_resize_heap(requestedSize) {
      requestedSize = requestedSize >>> 0;
      var oldSize = _emscripten_get_heap_size();
      // With pthreads, races can happen (another thread might increase the size in between), so return a failure, and let the caller retry.
      assert(requestedSize > oldSize);
  
      // Memory resize rules:
      // 1. When resizing, always produce a resized heap that is at least 16MB (to avoid tiny heap sizes receiving lots of repeated resizes at startup)
      // 2. Always increase heap size to at least the requested size, rounded up to next page multiple.
      // 3a. If MEMORY_GROWTH_LINEAR_STEP == -1, excessively resize the heap geometrically: increase the heap size according to 
      //                                         MEMORY_GROWTH_GEOMETRIC_STEP factor (default +20%),
      //                                         At most overreserve by MEMORY_GROWTH_GEOMETRIC_CAP bytes (default 96MB).
      // 3b. If MEMORY_GROWTH_LINEAR_STEP != -1, excessively resize the heap linearly: increase the heap size by at least MEMORY_GROWTH_LINEAR_STEP bytes.
      // 4. Max size for the heap is capped at 2048MB-WASM_PAGE_SIZE, or by MAXIMUM_MEMORY, or by ASAN limit, depending on which is smallest
      // 5. If we were unable to allocate as much memory, it may be due to over-eager decision to excessively reserve due to (3) above.
      //    Hence if an allocation fails, cut down on the amount of excess growth, in an attempt to succeed to perform a smaller allocation.
  
      // A limit was set for how much we can grow. We should not exceed that
      // (the wasm binary specifies it, so if we tried, we'd fail anyhow).
      var maxHeapSize = 2147483648;
      if (requestedSize > maxHeapSize) {
        err('Cannot enlarge memory, asked to go up to ' + requestedSize + ' bytes, but the limit is ' + maxHeapSize + ' bytes!');
        return false;
      }
  
      var minHeapSize = 16777216;
  
      // Loop through potential heap size increases. If we attempt a too eager reservation that fails, cut down on the
      // attempted size and reserve a smaller bump instead. (max 3 times, chosen somewhat arbitrarily)
      for(var cutDown = 1; cutDown <= 4; cutDown *= 2) {
        var overGrownHeapSize = oldSize * (1 + 0.2 / cutDown); // ensure geometric growth
        // but limit overreserving (default to capping at +96MB overgrowth at most)
        overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296 );
  
        var newSize = Math.min(maxHeapSize, alignUp(Math.max(minHeapSize, requestedSize, overGrownHeapSize), 65536));
  
        var replacement = emscripten_realloc_buffer(newSize);
        if (replacement) {
  
          return true;
        }
      }
      err('Failed to grow the heap from ' + oldSize + ' bytes to ' + newSize + ' bytes, not enough memory!');
      return false;
    }

  function _emscripten_webgl_do_get_current_context() {
      return GL.currentContext ? GL.currentContext.handle : 0;
    }
  function _emscripten_webgl_get_current_context(
  ) {
  return _emscripten_webgl_do_get_current_context();
  }

  function _emscripten_webgl_make_context_current(contextHandle) {
      var success = GL.makeContextCurrent(contextHandle);
      return success ? 0 : -5;
    }

  var ENV={};
  
  function getExecutableName() {
      return thisProgram || './this.program';
    }
  function getEnvStrings() {
      if (!getEnvStrings.strings) {
        // Default values.
        // Browser language detection #8751
        var lang = ((typeof navigator === 'object' && navigator.languages && navigator.languages[0]) || 'C').replace('-', '_') + '.UTF-8';
        var env = {
          'USER': 'web_user',
          'LOGNAME': 'web_user',
          'PATH': '/',
          'PWD': '/',
          'HOME': '/home/web_user',
          'LANG': lang,
          '_': getExecutableName()
        };
        // Apply the user-provided values, if any.
        for (var x in ENV) {
          env[x] = ENV[x];
        }
        var strings = [];
        for (var x in env) {
          strings.push(x + '=' + env[x]);
        }
        getEnvStrings.strings = strings;
      }
      return getEnvStrings.strings;
    }
  function _environ_get(__environ, environ_buf) {
      var bufSize = 0;
      getEnvStrings().forEach(function(string, i) {
        var ptr = environ_buf + bufSize;
        HEAP32[(((__environ)+(i * 4))>>2)] = ptr;
        writeAsciiToMemory(string, ptr);
        bufSize += string.length + 1;
      });
      return 0;
    }

  function _environ_sizes_get(penviron_count, penviron_buf_size) {
      var strings = getEnvStrings();
      HEAP32[((penviron_count)>>2)] = strings.length;
      var bufSize = 0;
      strings.forEach(function(string) {
        bufSize += string.length + 1;
      });
      HEAP32[((penviron_buf_size)>>2)] = bufSize;
      return 0;
    }

  function _exit(status) {
      // void _exit(int status);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/exit.html
      exit(status);
    }

  function _fd_close(fd) {
      abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');
      return 0;
    }

  function _fd_fdstat_get(fd, pbuf) {
      // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
      var type = fd == 1 || fd == 2 ? 2 : abort();
      HEAP8[((pbuf)>>0)] = type;
      // TODO HEAP16[(((pbuf)+(2))>>1)] = ?;
      // TODO (tempI64 = [?>>>0,(tempDouble=?,(+(Math.abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math.min((+(Math.floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math.ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],HEAP32[(((pbuf)+(8))>>2)] = tempI64[0],HEAP32[(((pbuf)+(12))>>2)] = tempI64[1]);
      // TODO (tempI64 = [?>>>0,(tempDouble=?,(+(Math.abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math.min((+(Math.floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math.ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],HEAP32[(((pbuf)+(16))>>2)] = tempI64[0],HEAP32[(((pbuf)+(20))>>2)] = tempI64[1]);
      return 0;
    }

  function _fd_pread(fd, iov, iovcnt, offset_low, offset_high, pnum) {
      
      assert(!offset_high, 'offsets over 2^32 not yet supported');
      var stream = SYSCALLS.getStreamFromFD(fd)
      var num = SYSCALLS.doReadv(stream, iov, iovcnt, offset_low);
      HEAP32[((pnum)>>2)] = num
      return 0;
    }

  function _fd_read(fd, iov, iovcnt, pnum) {
      var stream = SYSCALLS.getStreamFromFD(fd);
      var num = SYSCALLS.doReadv(stream, iov, iovcnt);
      HEAP32[((pnum)>>2)] = num
      return 0;
    }

  function _fd_seek(fd, offset_low, offset_high, whence, newOffset) {
  abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');
  }

  function flush_NO_FILESYSTEM() {
      // flush anything remaining in the buffers during shutdown
      if (typeof _fflush !== 'undefined') _fflush(0);
      var buffers = SYSCALLS.buffers;
      if (buffers[1].length) SYSCALLS.printChar(1, 10);
      if (buffers[2].length) SYSCALLS.printChar(2, 10);
    }
  function _fd_write(fd, iov, iovcnt, pnum) {
      // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
      var num = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAP32[(((iov)+(i*8))>>2)];
        var len = HEAP32[(((iov)+(i*8 + 4))>>2)];
        for (var j = 0; j < len; j++) {
          SYSCALLS.printChar(fd, HEAPU8[ptr+j]);
        }
        num += len;
      }
      HEAP32[((pnum)>>2)] = num
      return 0;
    }

  function _getTempRet0() {
      return (getTempRet0() | 0);
    }

  function _glBindFramebuffer(target, framebuffer) {
      GL.validateGLObjectID(GL.framebuffers, framebuffer, 'glBindFramebuffer', 'framebuffer');
  
      GLctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
  
    }

  function _glClear(x0) { GLctx['clear'](x0) }

  function _glClearColor(x0, x1, x2, x3) { GLctx['clearColor'](x0, x1, x2, x3) }

  function _glClearStencil(x0) { GLctx['clearStencil'](x0) }

  function _glGetIntegerv(name_, p) {
      emscriptenWebGLGet(name_, p, 0);
    }

  function _setTempRet0($i) {
      setTempRet0(($i) | 0);
    }

  function __isLeapYear(year) {
        return year%4 === 0 && (year%100 !== 0 || year%400 === 0);
    }
  
  function __arraySum(array, index) {
      var sum = 0;
      for (var i = 0; i <= index; sum += array[i++]) {
        // no-op
      }
      return sum;
    }
  
  var __MONTH_DAYS_LEAP=[31,29,31,30,31,30,31,31,30,31,30,31];
  
  var __MONTH_DAYS_REGULAR=[31,28,31,30,31,30,31,31,30,31,30,31];
  function __addDays(date, days) {
      var newDate = new Date(date.getTime());
      while(days > 0) {
        var leap = __isLeapYear(newDate.getFullYear());
        var currentMonth = newDate.getMonth();
        var daysInCurrentMonth = (leap ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR)[currentMonth];
  
        if (days > daysInCurrentMonth-newDate.getDate()) {
          // we spill over to next month
          days -= (daysInCurrentMonth-newDate.getDate()+1);
          newDate.setDate(1);
          if (currentMonth < 11) {
            newDate.setMonth(currentMonth+1)
          } else {
            newDate.setMonth(0);
            newDate.setFullYear(newDate.getFullYear()+1);
          }
        } else {
          // we stay in current month
          newDate.setDate(newDate.getDate()+days);
          return newDate;
        }
      }
  
      return newDate;
    }
  function _strftime(s, maxsize, format, tm) {
      // size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);
      // http://pubs.opengroup.org/onlinepubs/009695399/functions/strftime.html
  
      var tm_zone = HEAP32[(((tm)+(40))>>2)];
  
      var date = {
        tm_sec: HEAP32[((tm)>>2)],
        tm_min: HEAP32[(((tm)+(4))>>2)],
        tm_hour: HEAP32[(((tm)+(8))>>2)],
        tm_mday: HEAP32[(((tm)+(12))>>2)],
        tm_mon: HEAP32[(((tm)+(16))>>2)],
        tm_year: HEAP32[(((tm)+(20))>>2)],
        tm_wday: HEAP32[(((tm)+(24))>>2)],
        tm_yday: HEAP32[(((tm)+(28))>>2)],
        tm_isdst: HEAP32[(((tm)+(32))>>2)],
        tm_gmtoff: HEAP32[(((tm)+(36))>>2)],
        tm_zone: tm_zone ? UTF8ToString(tm_zone) : ''
      };
  
      var pattern = UTF8ToString(format);
  
      // expand format
      var EXPANSION_RULES_1 = {
        '%c': '%a %b %d %H:%M:%S %Y',     // Replaced by the locale's appropriate date and time representation - e.g., Mon Aug  3 14:02:01 2013
        '%D': '%m/%d/%y',                 // Equivalent to %m / %d / %y
        '%F': '%Y-%m-%d',                 // Equivalent to %Y - %m - %d
        '%h': '%b',                       // Equivalent to %b
        '%r': '%I:%M:%S %p',              // Replaced by the time in a.m. and p.m. notation
        '%R': '%H:%M',                    // Replaced by the time in 24-hour notation
        '%T': '%H:%M:%S',                 // Replaced by the time
        '%x': '%m/%d/%y',                 // Replaced by the locale's appropriate date representation
        '%X': '%H:%M:%S',                 // Replaced by the locale's appropriate time representation
        // Modified Conversion Specifiers
        '%Ec': '%c',                      // Replaced by the locale's alternative appropriate date and time representation.
        '%EC': '%C',                      // Replaced by the name of the base year (period) in the locale's alternative representation.
        '%Ex': '%m/%d/%y',                // Replaced by the locale's alternative date representation.
        '%EX': '%H:%M:%S',                // Replaced by the locale's alternative time representation.
        '%Ey': '%y',                      // Replaced by the offset from %EC (year only) in the locale's alternative representation.
        '%EY': '%Y',                      // Replaced by the full alternative year representation.
        '%Od': '%d',                      // Replaced by the day of the month, using the locale's alternative numeric symbols, filled as needed with leading zeros if there is any alternative symbol for zero; otherwise, with leading <space> characters.
        '%Oe': '%e',                      // Replaced by the day of the month, using the locale's alternative numeric symbols, filled as needed with leading <space> characters.
        '%OH': '%H',                      // Replaced by the hour (24-hour clock) using the locale's alternative numeric symbols.
        '%OI': '%I',                      // Replaced by the hour (12-hour clock) using the locale's alternative numeric symbols.
        '%Om': '%m',                      // Replaced by the month using the locale's alternative numeric symbols.
        '%OM': '%M',                      // Replaced by the minutes using the locale's alternative numeric symbols.
        '%OS': '%S',                      // Replaced by the seconds using the locale's alternative numeric symbols.
        '%Ou': '%u',                      // Replaced by the weekday as a number in the locale's alternative representation (Monday=1).
        '%OU': '%U',                      // Replaced by the week number of the year (Sunday as the first day of the week, rules corresponding to %U ) using the locale's alternative numeric symbols.
        '%OV': '%V',                      // Replaced by the week number of the year (Monday as the first day of the week, rules corresponding to %V ) using the locale's alternative numeric symbols.
        '%Ow': '%w',                      // Replaced by the number of the weekday (Sunday=0) using the locale's alternative numeric symbols.
        '%OW': '%W',                      // Replaced by the week number of the year (Monday as the first day of the week) using the locale's alternative numeric symbols.
        '%Oy': '%y',                      // Replaced by the year (offset from %C ) using the locale's alternative numeric symbols.
      };
      for (var rule in EXPANSION_RULES_1) {
        pattern = pattern.replace(new RegExp(rule, 'g'), EXPANSION_RULES_1[rule]);
      }
  
      var WEEKDAYS = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
      var MONTHS = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];
  
      function leadingSomething(value, digits, character) {
        var str = typeof value === 'number' ? value.toString() : (value || '');
        while (str.length < digits) {
          str = character[0]+str;
        }
        return str;
      }
  
      function leadingNulls(value, digits) {
        return leadingSomething(value, digits, '0');
      }
  
      function compareByDay(date1, date2) {
        function sgn(value) {
          return value < 0 ? -1 : (value > 0 ? 1 : 0);
        }
  
        var compare;
        if ((compare = sgn(date1.getFullYear()-date2.getFullYear())) === 0) {
          if ((compare = sgn(date1.getMonth()-date2.getMonth())) === 0) {
            compare = sgn(date1.getDate()-date2.getDate());
          }
        }
        return compare;
      }
  
      function getFirstWeekStartDate(janFourth) {
          switch (janFourth.getDay()) {
            case 0: // Sunday
              return new Date(janFourth.getFullYear()-1, 11, 29);
            case 1: // Monday
              return janFourth;
            case 2: // Tuesday
              return new Date(janFourth.getFullYear(), 0, 3);
            case 3: // Wednesday
              return new Date(janFourth.getFullYear(), 0, 2);
            case 4: // Thursday
              return new Date(janFourth.getFullYear(), 0, 1);
            case 5: // Friday
              return new Date(janFourth.getFullYear()-1, 11, 31);
            case 6: // Saturday
              return new Date(janFourth.getFullYear()-1, 11, 30);
          }
      }
  
      function getWeekBasedYear(date) {
          var thisDate = __addDays(new Date(date.tm_year+1900, 0, 1), date.tm_yday);
  
          var janFourthThisYear = new Date(thisDate.getFullYear(), 0, 4);
          var janFourthNextYear = new Date(thisDate.getFullYear()+1, 0, 4);
  
          var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
          var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);
  
          if (compareByDay(firstWeekStartThisYear, thisDate) <= 0) {
            // this date is after the start of the first week of this year
            if (compareByDay(firstWeekStartNextYear, thisDate) <= 0) {
              return thisDate.getFullYear()+1;
            } else {
              return thisDate.getFullYear();
            }
          } else {
            return thisDate.getFullYear()-1;
          }
      }
  
      var EXPANSION_RULES_2 = {
        '%a': function(date) {
          return WEEKDAYS[date.tm_wday].substring(0,3);
        },
        '%A': function(date) {
          return WEEKDAYS[date.tm_wday];
        },
        '%b': function(date) {
          return MONTHS[date.tm_mon].substring(0,3);
        },
        '%B': function(date) {
          return MONTHS[date.tm_mon];
        },
        '%C': function(date) {
          var year = date.tm_year+1900;
          return leadingNulls((year/100)|0,2);
        },
        '%d': function(date) {
          return leadingNulls(date.tm_mday, 2);
        },
        '%e': function(date) {
          return leadingSomething(date.tm_mday, 2, ' ');
        },
        '%g': function(date) {
          // %g, %G, and %V give values according to the ISO 8601:2000 standard week-based year.
          // In this system, weeks begin on a Monday and week 1 of the year is the week that includes
          // January 4th, which is also the week that includes the first Thursday of the year, and
          // is also the first week that contains at least four days in the year.
          // If the first Monday of January is the 2nd, 3rd, or 4th, the preceding days are part of
          // the last week of the preceding year; thus, for Saturday 2nd January 1999,
          // %G is replaced by 1998 and %V is replaced by 53. If December 29th, 30th,
          // or 31st is a Monday, it and any following days are part of week 1 of the following year.
          // Thus, for Tuesday 30th December 1997, %G is replaced by 1998 and %V is replaced by 01.
  
          return getWeekBasedYear(date).toString().substring(2);
        },
        '%G': function(date) {
          return getWeekBasedYear(date);
        },
        '%H': function(date) {
          return leadingNulls(date.tm_hour, 2);
        },
        '%I': function(date) {
          var twelveHour = date.tm_hour;
          if (twelveHour == 0) twelveHour = 12;
          else if (twelveHour > 12) twelveHour -= 12;
          return leadingNulls(twelveHour, 2);
        },
        '%j': function(date) {
          // Day of the year (001-366)
          return leadingNulls(date.tm_mday+__arraySum(__isLeapYear(date.tm_year+1900) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, date.tm_mon-1), 3);
        },
        '%m': function(date) {
          return leadingNulls(date.tm_mon+1, 2);
        },
        '%M': function(date) {
          return leadingNulls(date.tm_min, 2);
        },
        '%n': function() {
          return '\n';
        },
        '%p': function(date) {
          if (date.tm_hour >= 0 && date.tm_hour < 12) {
            return 'AM';
          } else {
            return 'PM';
          }
        },
        '%S': function(date) {
          return leadingNulls(date.tm_sec, 2);
        },
        '%t': function() {
          return '\t';
        },
        '%u': function(date) {
          return date.tm_wday || 7;
        },
        '%U': function(date) {
          // Replaced by the week number of the year as a decimal number [00,53].
          // The first Sunday of January is the first day of week 1;
          // days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
          var janFirst = new Date(date.tm_year+1900, 0, 1);
          var firstSunday = janFirst.getDay() === 0 ? janFirst : __addDays(janFirst, 7-janFirst.getDay());
          var endDate = new Date(date.tm_year+1900, date.tm_mon, date.tm_mday);
  
          // is target date after the first Sunday?
          if (compareByDay(firstSunday, endDate) < 0) {
            // calculate difference in days between first Sunday and endDate
            var februaryFirstUntilEndMonth = __arraySum(__isLeapYear(endDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, endDate.getMonth()-1)-31;
            var firstSundayUntilEndJanuary = 31-firstSunday.getDate();
            var days = firstSundayUntilEndJanuary+februaryFirstUntilEndMonth+endDate.getDate();
            return leadingNulls(Math.ceil(days/7), 2);
          }
  
          return compareByDay(firstSunday, janFirst) === 0 ? '01': '00';
        },
        '%V': function(date) {
          // Replaced by the week number of the year (Monday as the first day of the week)
          // as a decimal number [01,53]. If the week containing 1 January has four
          // or more days in the new year, then it is considered week 1.
          // Otherwise, it is the last week of the previous year, and the next week is week 1.
          // Both January 4th and the first Thursday of January are always in week 1. [ tm_year, tm_wday, tm_yday]
          var janFourthThisYear = new Date(date.tm_year+1900, 0, 4);
          var janFourthNextYear = new Date(date.tm_year+1901, 0, 4);
  
          var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
          var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);
  
          var endDate = __addDays(new Date(date.tm_year+1900, 0, 1), date.tm_yday);
  
          if (compareByDay(endDate, firstWeekStartThisYear) < 0) {
            // if given date is before this years first week, then it belongs to the 53rd week of last year
            return '53';
          }
  
          if (compareByDay(firstWeekStartNextYear, endDate) <= 0) {
            // if given date is after next years first week, then it belongs to the 01th week of next year
            return '01';
          }
  
          // given date is in between CW 01..53 of this calendar year
          var daysDifference;
          if (firstWeekStartThisYear.getFullYear() < date.tm_year+1900) {
            // first CW of this year starts last year
            daysDifference = date.tm_yday+32-firstWeekStartThisYear.getDate()
          } else {
            // first CW of this year starts this year
            daysDifference = date.tm_yday+1-firstWeekStartThisYear.getDate();
          }
          return leadingNulls(Math.ceil(daysDifference/7), 2);
        },
        '%w': function(date) {
          return date.tm_wday;
        },
        '%W': function(date) {
          // Replaced by the week number of the year as a decimal number [00,53].
          // The first Monday of January is the first day of week 1;
          // days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
          var janFirst = new Date(date.tm_year, 0, 1);
          var firstMonday = janFirst.getDay() === 1 ? janFirst : __addDays(janFirst, janFirst.getDay() === 0 ? 1 : 7-janFirst.getDay()+1);
          var endDate = new Date(date.tm_year+1900, date.tm_mon, date.tm_mday);
  
          // is target date after the first Monday?
          if (compareByDay(firstMonday, endDate) < 0) {
            var februaryFirstUntilEndMonth = __arraySum(__isLeapYear(endDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, endDate.getMonth()-1)-31;
            var firstMondayUntilEndJanuary = 31-firstMonday.getDate();
            var days = firstMondayUntilEndJanuary+februaryFirstUntilEndMonth+endDate.getDate();
            return leadingNulls(Math.ceil(days/7), 2);
          }
          return compareByDay(firstMonday, janFirst) === 0 ? '01': '00';
        },
        '%y': function(date) {
          // Replaced by the last two digits of the year as a decimal number [00,99]. [ tm_year]
          return (date.tm_year+1900).toString().substring(2);
        },
        '%Y': function(date) {
          // Replaced by the year as a decimal number (for example, 1997). [ tm_year]
          return date.tm_year+1900;
        },
        '%z': function(date) {
          // Replaced by the offset from UTC in the ISO 8601:2000 standard format ( +hhmm or -hhmm ).
          // For example, "-0430" means 4 hours 30 minutes behind UTC (west of Greenwich).
          var off = date.tm_gmtoff;
          var ahead = off >= 0;
          off = Math.abs(off) / 60;
          // convert from minutes into hhmm format (which means 60 minutes = 100 units)
          off = (off / 60)*100 + (off % 60);
          return (ahead ? '+' : '-') + String("0000" + off).slice(-4);
        },
        '%Z': function(date) {
          return date.tm_zone;
        },
        '%%': function() {
          return '%';
        }
      };
      for (var rule in EXPANSION_RULES_2) {
        if (pattern.indexOf(rule) >= 0) {
          pattern = pattern.replace(new RegExp(rule, 'g'), EXPANSION_RULES_2[rule](date));
        }
      }
  
      var bytes = intArrayFromString(pattern, false);
      if (bytes.length > maxsize) {
        return 0;
      }
  
      writeArrayToMemory(bytes, s);
      return bytes.length-1;
    }
  function _strftime_l(s, maxsize, format, tm) {
      return _strftime(s, maxsize, format, tm); // no locale support yet
    }
InternalError = Module['InternalError'] = extendError(Error, 'InternalError');;
embind_init_charCodes();
BindingError = Module['BindingError'] = extendError(Error, 'BindingError');;
init_ClassHandle();
init_RegisteredPointer();
init_embind();;
UnboundTypeError = Module['UnboundTypeError'] = extendError(Error, 'UnboundTypeError');;
init_emval();;
var GLctx;;
for (var i = 0; i < 32; ++i) tempFixedLengthArray.push(new Array(i));;
var miniTempWebGLFloatBuffersStorage = new Float32Array(288);
  for (/**@suppress{duplicate}*/var i = 0; i < 288; ++i) {
  miniTempWebGLFloatBuffers[i] = miniTempWebGLFloatBuffersStorage.subarray(0, i+1);
  }
  ;
var __miniTempWebGLIntBuffersStorage = new Int32Array(288);
  for (/**@suppress{duplicate}*/var i = 0; i < 288; ++i) {
  __miniTempWebGLIntBuffers[i] = __miniTempWebGLIntBuffersStorage.subarray(0, i+1);
  }
  ;
var ASSERTIONS = true;



/** @type {function(string, boolean=, number=)} */
function intArrayFromString(stringy, dontAddNull, length) {
  var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
  var u8array = new Array(len);
  var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
  if (dontAddNull) u8array.length = numBytesWritten;
  return u8array;
}

function intArrayToString(array) {
  var ret = [];
  for (var i = 0; i < array.length; i++) {
    var chr = array[i];
    if (chr > 0xFF) {
      if (ASSERTIONS) {
        assert(false, 'Character code ' + chr + ' (' + String.fromCharCode(chr) + ')  at offset ' + i + ' not in 0x00-0xFF.');
      }
      chr &= 0xFF;
    }
    ret.push(String.fromCharCode(chr));
  }
  return ret.join('');
}


var asmLibraryArg = {
  "__cxa_allocate_exception": ___cxa_allocate_exception,
  "__cxa_atexit": ___cxa_atexit,
  "__cxa_throw": ___cxa_throw,
  "__sys_fcntl64": ___sys_fcntl64,
  "__sys_fstat64": ___sys_fstat64,
  "__sys_ioctl": ___sys_ioctl,
  "__sys_mmap2": ___sys_mmap2,
  "__sys_munmap": ___sys_munmap,
  "__sys_open": ___sys_open,
  "__sys_stat64": ___sys_stat64,
  "_embind_finalize_value_object": __embind_finalize_value_object,
  "_embind_register_bool": __embind_register_bool,
  "_embind_register_class": __embind_register_class,
  "_embind_register_class_class_function": __embind_register_class_class_function,
  "_embind_register_class_constructor": __embind_register_class_constructor,
  "_embind_register_class_function": __embind_register_class_function,
  "_embind_register_constant": __embind_register_constant,
  "_embind_register_emval": __embind_register_emval,
  "_embind_register_enum": __embind_register_enum,
  "_embind_register_enum_value": __embind_register_enum_value,
  "_embind_register_float": __embind_register_float,
  "_embind_register_function": __embind_register_function,
  "_embind_register_integer": __embind_register_integer,
  "_embind_register_memory_view": __embind_register_memory_view,
  "_embind_register_smart_ptr": __embind_register_smart_ptr,
  "_embind_register_std_string": __embind_register_std_string,
  "_embind_register_std_wstring": __embind_register_std_wstring,
  "_embind_register_value_object": __embind_register_value_object,
  "_embind_register_value_object_field": __embind_register_value_object_field,
  "_embind_register_void": __embind_register_void,
  "_emval_call_void_method": __emval_call_void_method,
  "_emval_decref": __emval_decref,
  "_emval_get_method_caller": __emval_get_method_caller,
  "_emval_incref": __emval_incref,
  "_emval_new_array": __emval_new_array,
  "_emval_new_cstring": __emval_new_cstring,
  "_emval_new_object": __emval_new_object,
  "_emval_set_property": __emval_set_property,
  "_emval_take_value": __emval_take_value,
  "abort": _abort,
  "clock_gettime": _clock_gettime,
  "emscripten_glActiveTexture": _emscripten_glActiveTexture,
  "emscripten_glAttachShader": _emscripten_glAttachShader,
  "emscripten_glBindAttribLocation": _emscripten_glBindAttribLocation,
  "emscripten_glBindBuffer": _emscripten_glBindBuffer,
  "emscripten_glBindFramebuffer": _emscripten_glBindFramebuffer,
  "emscripten_glBindRenderbuffer": _emscripten_glBindRenderbuffer,
  "emscripten_glBindSampler": _emscripten_glBindSampler,
  "emscripten_glBindTexture": _emscripten_glBindTexture,
  "emscripten_glBindVertexArray": _emscripten_glBindVertexArray,
  "emscripten_glBindVertexArrayOES": _emscripten_glBindVertexArrayOES,
  "emscripten_glBlendColor": _emscripten_glBlendColor,
  "emscripten_glBlendEquation": _emscripten_glBlendEquation,
  "emscripten_glBlendFunc": _emscripten_glBlendFunc,
  "emscripten_glBlitFramebuffer": _emscripten_glBlitFramebuffer,
  "emscripten_glBufferData": _emscripten_glBufferData,
  "emscripten_glBufferSubData": _emscripten_glBufferSubData,
  "emscripten_glCheckFramebufferStatus": _emscripten_glCheckFramebufferStatus,
  "emscripten_glClear": _emscripten_glClear,
  "emscripten_glClearColor": _emscripten_glClearColor,
  "emscripten_glClearStencil": _emscripten_glClearStencil,
  "emscripten_glClientWaitSync": _emscripten_glClientWaitSync,
  "emscripten_glColorMask": _emscripten_glColorMask,
  "emscripten_glCompileShader": _emscripten_glCompileShader,
  "emscripten_glCompressedTexImage2D": _emscripten_glCompressedTexImage2D,
  "emscripten_glCompressedTexSubImage2D": _emscripten_glCompressedTexSubImage2D,
  "emscripten_glCopyTexSubImage2D": _emscripten_glCopyTexSubImage2D,
  "emscripten_glCreateProgram": _emscripten_glCreateProgram,
  "emscripten_glCreateShader": _emscripten_glCreateShader,
  "emscripten_glCullFace": _emscripten_glCullFace,
  "emscripten_glDeleteBuffers": _emscripten_glDeleteBuffers,
  "emscripten_glDeleteFramebuffers": _emscripten_glDeleteFramebuffers,
  "emscripten_glDeleteProgram": _emscripten_glDeleteProgram,
  "emscripten_glDeleteRenderbuffers": _emscripten_glDeleteRenderbuffers,
  "emscripten_glDeleteSamplers": _emscripten_glDeleteSamplers,
  "emscripten_glDeleteShader": _emscripten_glDeleteShader,
  "emscripten_glDeleteSync": _emscripten_glDeleteSync,
  "emscripten_glDeleteTextures": _emscripten_glDeleteTextures,
  "emscripten_glDeleteVertexArrays": _emscripten_glDeleteVertexArrays,
  "emscripten_glDeleteVertexArraysOES": _emscripten_glDeleteVertexArraysOES,
  "emscripten_glDepthMask": _emscripten_glDepthMask,
  "emscripten_glDisable": _emscripten_glDisable,
  "emscripten_glDisableVertexAttribArray": _emscripten_glDisableVertexAttribArray,
  "emscripten_glDrawArrays": _emscripten_glDrawArrays,
  "emscripten_glDrawArraysInstanced": _emscripten_glDrawArraysInstanced,
  "emscripten_glDrawArraysInstancedBaseInstanceWEBGL": _emscripten_glDrawArraysInstancedBaseInstanceWEBGL,
  "emscripten_glDrawBuffers": _emscripten_glDrawBuffers,
  "emscripten_glDrawElements": _emscripten_glDrawElements,
  "emscripten_glDrawElementsInstanced": _emscripten_glDrawElementsInstanced,
  "emscripten_glDrawElementsInstancedBaseVertexBaseInstanceWEBGL": _emscripten_glDrawElementsInstancedBaseVertexBaseInstanceWEBGL,
  "emscripten_glDrawRangeElements": _emscripten_glDrawRangeElements,
  "emscripten_glEnable": _emscripten_glEnable,
  "emscripten_glEnableVertexAttribArray": _emscripten_glEnableVertexAttribArray,
  "emscripten_glFenceSync": _emscripten_glFenceSync,
  "emscripten_glFinish": _emscripten_glFinish,
  "emscripten_glFlush": _emscripten_glFlush,
  "emscripten_glFramebufferRenderbuffer": _emscripten_glFramebufferRenderbuffer,
  "emscripten_glFramebufferTexture2D": _emscripten_glFramebufferTexture2D,
  "emscripten_glFrontFace": _emscripten_glFrontFace,
  "emscripten_glGenBuffers": _emscripten_glGenBuffers,
  "emscripten_glGenFramebuffers": _emscripten_glGenFramebuffers,
  "emscripten_glGenRenderbuffers": _emscripten_glGenRenderbuffers,
  "emscripten_glGenSamplers": _emscripten_glGenSamplers,
  "emscripten_glGenTextures": _emscripten_glGenTextures,
  "emscripten_glGenVertexArrays": _emscripten_glGenVertexArrays,
  "emscripten_glGenVertexArraysOES": _emscripten_glGenVertexArraysOES,
  "emscripten_glGenerateMipmap": _emscripten_glGenerateMipmap,
  "emscripten_glGetBufferParameteriv": _emscripten_glGetBufferParameteriv,
  "emscripten_glGetError": _emscripten_glGetError,
  "emscripten_glGetFramebufferAttachmentParameteriv": _emscripten_glGetFramebufferAttachmentParameteriv,
  "emscripten_glGetIntegerv": _emscripten_glGetIntegerv,
  "emscripten_glGetProgramInfoLog": _emscripten_glGetProgramInfoLog,
  "emscripten_glGetProgramiv": _emscripten_glGetProgramiv,
  "emscripten_glGetRenderbufferParameteriv": _emscripten_glGetRenderbufferParameteriv,
  "emscripten_glGetShaderInfoLog": _emscripten_glGetShaderInfoLog,
  "emscripten_glGetShaderPrecisionFormat": _emscripten_glGetShaderPrecisionFormat,
  "emscripten_glGetShaderiv": _emscripten_glGetShaderiv,
  "emscripten_glGetString": _emscripten_glGetString,
  "emscripten_glGetStringi": _emscripten_glGetStringi,
  "emscripten_glGetUniformLocation": _emscripten_glGetUniformLocation,
  "emscripten_glInvalidateFramebuffer": _emscripten_glInvalidateFramebuffer,
  "emscripten_glInvalidateSubFramebuffer": _emscripten_glInvalidateSubFramebuffer,
  "emscripten_glIsSync": _emscripten_glIsSync,
  "emscripten_glIsTexture": _emscripten_glIsTexture,
  "emscripten_glLineWidth": _emscripten_glLineWidth,
  "emscripten_glLinkProgram": _emscripten_glLinkProgram,
  "emscripten_glMultiDrawArraysInstancedBaseInstanceWEBGL": _emscripten_glMultiDrawArraysInstancedBaseInstanceWEBGL,
  "emscripten_glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL": _emscripten_glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL,
  "emscripten_glPixelStorei": _emscripten_glPixelStorei,
  "emscripten_glReadBuffer": _emscripten_glReadBuffer,
  "emscripten_glReadPixels": _emscripten_glReadPixels,
  "emscripten_glRenderbufferStorage": _emscripten_glRenderbufferStorage,
  "emscripten_glRenderbufferStorageMultisample": _emscripten_glRenderbufferStorageMultisample,
  "emscripten_glSamplerParameteri": _emscripten_glSamplerParameteri,
  "emscripten_glSamplerParameteriv": _emscripten_glSamplerParameteriv,
  "emscripten_glScissor": _emscripten_glScissor,
  "emscripten_glShaderSource": _emscripten_glShaderSource,
  "emscripten_glStencilFunc": _emscripten_glStencilFunc,
  "emscripten_glStencilFuncSeparate": _emscripten_glStencilFuncSeparate,
  "emscripten_glStencilMask": _emscripten_glStencilMask,
  "emscripten_glStencilMaskSeparate": _emscripten_glStencilMaskSeparate,
  "emscripten_glStencilOp": _emscripten_glStencilOp,
  "emscripten_glStencilOpSeparate": _emscripten_glStencilOpSeparate,
  "emscripten_glTexImage2D": _emscripten_glTexImage2D,
  "emscripten_glTexParameterf": _emscripten_glTexParameterf,
  "emscripten_glTexParameterfv": _emscripten_glTexParameterfv,
  "emscripten_glTexParameteri": _emscripten_glTexParameteri,
  "emscripten_glTexParameteriv": _emscripten_glTexParameteriv,
  "emscripten_glTexStorage2D": _emscripten_glTexStorage2D,
  "emscripten_glTexSubImage2D": _emscripten_glTexSubImage2D,
  "emscripten_glUniform1f": _emscripten_glUniform1f,
  "emscripten_glUniform1fv": _emscripten_glUniform1fv,
  "emscripten_glUniform1i": _emscripten_glUniform1i,
  "emscripten_glUniform1iv": _emscripten_glUniform1iv,
  "emscripten_glUniform2f": _emscripten_glUniform2f,
  "emscripten_glUniform2fv": _emscripten_glUniform2fv,
  "emscripten_glUniform2i": _emscripten_glUniform2i,
  "emscripten_glUniform2iv": _emscripten_glUniform2iv,
  "emscripten_glUniform3f": _emscripten_glUniform3f,
  "emscripten_glUniform3fv": _emscripten_glUniform3fv,
  "emscripten_glUniform3i": _emscripten_glUniform3i,
  "emscripten_glUniform3iv": _emscripten_glUniform3iv,
  "emscripten_glUniform4f": _emscripten_glUniform4f,
  "emscripten_glUniform4fv": _emscripten_glUniform4fv,
  "emscripten_glUniform4i": _emscripten_glUniform4i,
  "emscripten_glUniform4iv": _emscripten_glUniform4iv,
  "emscripten_glUniformMatrix2fv": _emscripten_glUniformMatrix2fv,
  "emscripten_glUniformMatrix3fv": _emscripten_glUniformMatrix3fv,
  "emscripten_glUniformMatrix4fv": _emscripten_glUniformMatrix4fv,
  "emscripten_glUseProgram": _emscripten_glUseProgram,
  "emscripten_glVertexAttrib1f": _emscripten_glVertexAttrib1f,
  "emscripten_glVertexAttrib2fv": _emscripten_glVertexAttrib2fv,
  "emscripten_glVertexAttrib3fv": _emscripten_glVertexAttrib3fv,
  "emscripten_glVertexAttrib4fv": _emscripten_glVertexAttrib4fv,
  "emscripten_glVertexAttribDivisor": _emscripten_glVertexAttribDivisor,
  "emscripten_glVertexAttribIPointer": _emscripten_glVertexAttribIPointer,
  "emscripten_glVertexAttribPointer": _emscripten_glVertexAttribPointer,
  "emscripten_glViewport": _emscripten_glViewport,
  "emscripten_glWaitSync": _emscripten_glWaitSync,
  "emscripten_longjmp": _emscripten_longjmp,
  "emscripten_memcpy_big": _emscripten_memcpy_big,
  "emscripten_resize_heap": _emscripten_resize_heap,
  "emscripten_webgl_get_current_context": _emscripten_webgl_get_current_context,
  "emscripten_webgl_make_context_current": _emscripten_webgl_make_context_current,
  "environ_get": _environ_get,
  "environ_sizes_get": _environ_sizes_get,
  "exit": _exit,
  "fd_close": _fd_close,
  "fd_fdstat_get": _fd_fdstat_get,
  "fd_pread": _fd_pread,
  "fd_read": _fd_read,
  "fd_seek": _fd_seek,
  "fd_write": _fd_write,
  "getTempRet0": _getTempRet0,
  "glBindFramebuffer": _glBindFramebuffer,
  "glClear": _glClear,
  "glClearColor": _glClearColor,
  "glClearStencil": _glClearStencil,
  "glGetIntegerv": _glGetIntegerv,
  "invoke_ii": invoke_ii,
  "invoke_iii": invoke_iii,
  "invoke_iiii": invoke_iiii,
  "invoke_iiiii": invoke_iiiii,
  "invoke_iiiiii": invoke_iiiiii,
  "invoke_iiiiiii": invoke_iiiiiii,
  "invoke_iiiiiiiiii": invoke_iiiiiiiiii,
  "invoke_v": invoke_v,
  "invoke_vi": invoke_vi,
  "invoke_vii": invoke_vii,
  "invoke_viii": invoke_viii,
  "invoke_viiii": invoke_viiii,
  "invoke_viiiii": invoke_viiiii,
  "invoke_viiiiii": invoke_viiiiii,
  "invoke_viiiiiii": invoke_viiiiiii,
  "invoke_viiiiiiiii": invoke_viiiiiiiii,
  "setTempRet0": _setTempRet0,
  "strftime_l": _strftime_l
};
var asm = createWasm();
/** @type {function(...*):?} */
var ___wasm_call_ctors = Module["___wasm_call_ctors"] = createExportWrapper("__wasm_call_ctors");

/** @type {function(...*):?} */
var _malloc = Module["_malloc"] = createExportWrapper("malloc");

/** @type {function(...*):?} */
var _memset = Module["_memset"] = createExportWrapper("memset");

/** @type {function(...*):?} */
var _free = Module["_free"] = createExportWrapper("free");

/** @type {function(...*):?} */
var _realloc = Module["_realloc"] = createExportWrapper("realloc");

/** @type {function(...*):?} */
var ___errno_location = Module["___errno_location"] = createExportWrapper("__errno_location");

/** @type {function(...*):?} */
var _testSetjmp = Module["_testSetjmp"] = createExportWrapper("testSetjmp");

/** @type {function(...*):?} */
var _saveSetjmp = Module["_saveSetjmp"] = createExportWrapper("saveSetjmp");

/** @type {function(...*):?} */
var ___getTypeName = Module["___getTypeName"] = createExportWrapper("__getTypeName");

/** @type {function(...*):?} */
var ___embind_register_native_and_builtin_types = Module["___embind_register_native_and_builtin_types"] = createExportWrapper("__embind_register_native_and_builtin_types");

/** @type {function(...*):?} */
var __get_tzname = Module["__get_tzname"] = createExportWrapper("_get_tzname");

/** @type {function(...*):?} */
var __get_daylight = Module["__get_daylight"] = createExportWrapper("_get_daylight");

/** @type {function(...*):?} */
var __get_timezone = Module["__get_timezone"] = createExportWrapper("_get_timezone");

/** @type {function(...*):?} */
var _emscripten_main_thread_process_queued_calls = Module["_emscripten_main_thread_process_queued_calls"] = createExportWrapper("emscripten_main_thread_process_queued_calls");

/** @type {function(...*):?} */
var _emscripten_stack_get_end = Module["_emscripten_stack_get_end"] = function() {
  return (_emscripten_stack_get_end = Module["_emscripten_stack_get_end"] = Module["asm"]["emscripten_stack_get_end"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var stackSave = Module["stackSave"] = createExportWrapper("stackSave");

/** @type {function(...*):?} */
var stackRestore = Module["stackRestore"] = createExportWrapper("stackRestore");

/** @type {function(...*):?} */
var stackAlloc = Module["stackAlloc"] = createExportWrapper("stackAlloc");

/** @type {function(...*):?} */
var _emscripten_stack_init = Module["_emscripten_stack_init"] = function() {
  return (_emscripten_stack_init = Module["_emscripten_stack_init"] = Module["asm"]["emscripten_stack_init"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _emscripten_stack_get_free = Module["_emscripten_stack_get_free"] = function() {
  return (_emscripten_stack_get_free = Module["_emscripten_stack_get_free"] = Module["asm"]["emscripten_stack_get_free"]).apply(null, arguments);
};

/** @type {function(...*):?} */
var _setThrew = Module["_setThrew"] = createExportWrapper("setThrew");

/** @type {function(...*):?} */
var ___cxa_demangle = Module["___cxa_demangle"] = createExportWrapper("__cxa_demangle");

/** @type {function(...*):?} */
var _memalign = Module["_memalign"] = createExportWrapper("memalign");

/** @type {function(...*):?} */
var dynCall_ji = Module["dynCall_ji"] = createExportWrapper("dynCall_ji");

/** @type {function(...*):?} */
var dynCall_iiji = Module["dynCall_iiji"] = createExportWrapper("dynCall_iiji");

/** @type {function(...*):?} */
var dynCall_iijjiii = Module["dynCall_iijjiii"] = createExportWrapper("dynCall_iijjiii");

/** @type {function(...*):?} */
var dynCall_iij = Module["dynCall_iij"] = createExportWrapper("dynCall_iij");

/** @type {function(...*):?} */
var dynCall_vijjjii = Module["dynCall_vijjjii"] = createExportWrapper("dynCall_vijjjii");

/** @type {function(...*):?} */
var dynCall_viji = Module["dynCall_viji"] = createExportWrapper("dynCall_viji");

/** @type {function(...*):?} */
var dynCall_vijiii = Module["dynCall_vijiii"] = createExportWrapper("dynCall_vijiii");

/** @type {function(...*):?} */
var dynCall_viiiiij = Module["dynCall_viiiiij"] = createExportWrapper("dynCall_viiiiij");

/** @type {function(...*):?} */
var dynCall_jiiiijiiiii = Module["dynCall_jiiiijiiiii"] = createExportWrapper("dynCall_jiiiijiiiii");

/** @type {function(...*):?} */
var dynCall_viiij = Module["dynCall_viiij"] = createExportWrapper("dynCall_viiij");

/** @type {function(...*):?} */
var dynCall_jii = Module["dynCall_jii"] = createExportWrapper("dynCall_jii");

/** @type {function(...*):?} */
var dynCall_viijii = Module["dynCall_viijii"] = createExportWrapper("dynCall_viijii");

/** @type {function(...*):?} */
var dynCall_iiij = Module["dynCall_iiij"] = createExportWrapper("dynCall_iiij");

/** @type {function(...*):?} */
var dynCall_iiiij = Module["dynCall_iiiij"] = createExportWrapper("dynCall_iiiij");

/** @type {function(...*):?} */
var dynCall_viij = Module["dynCall_viij"] = createExportWrapper("dynCall_viij");

/** @type {function(...*):?} */
var dynCall_vij = Module["dynCall_vij"] = createExportWrapper("dynCall_vij");

/** @type {function(...*):?} */
var dynCall_jiiii = Module["dynCall_jiiii"] = createExportWrapper("dynCall_jiiii");

/** @type {function(...*):?} */
var dynCall_jiiiiii = Module["dynCall_jiiiiii"] = createExportWrapper("dynCall_jiiiiii");

/** @type {function(...*):?} */
var dynCall_iijj = Module["dynCall_iijj"] = createExportWrapper("dynCall_iijj");

/** @type {function(...*):?} */
var dynCall_jiji = Module["dynCall_jiji"] = createExportWrapper("dynCall_jiji");

/** @type {function(...*):?} */
var dynCall_iiiiij = Module["dynCall_iiiiij"] = createExportWrapper("dynCall_iiiiij");

/** @type {function(...*):?} */
var dynCall_iiiiijj = Module["dynCall_iiiiijj"] = createExportWrapper("dynCall_iiiiijj");

/** @type {function(...*):?} */
var dynCall_iiiiiijj = Module["dynCall_iiiiiijj"] = createExportWrapper("dynCall_iiiiiijj");


function invoke_ii(index,a1) {
  var sp = stackSave();
  try {
    return wasmTable.get(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_iii(index,a1,a2) {
  var sp = stackSave();
  try {
    return wasmTable.get(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_viii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return wasmTable.get(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_vi(index,a1) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_v(index) {
  var sp = stackSave();
  try {
    wasmTable.get(index)();
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_vii(index,a1,a2) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return wasmTable.get(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiii(index,a1,a2,a3,a4,a5,a6) {
  var sp = stackSave();
  try {
    return wasmTable.get(index)(a1,a2,a3,a4,a5,a6);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    return wasmTable.get(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiii(index,a1,a2,a3,a4,a5,a6) {
  var sp = stackSave();
  try {
    wasmTable.get(index)(a1,a2,a3,a4,a5,a6);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9) {
  var sp = stackSave();
  try {
    return wasmTable.get(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0 && e !== 'longjmp') throw e;
    _setThrew(1, 0);
  }
}




// === Auto-generated postamble setup entry stuff ===

if (!Object.getOwnPropertyDescriptor(Module, "intArrayFromString")) Module["intArrayFromString"] = function() { abort("'intArrayFromString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "intArrayToString")) Module["intArrayToString"] = function() { abort("'intArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ccall")) Module["ccall"] = function() { abort("'ccall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "cwrap")) Module["cwrap"] = function() { abort("'cwrap' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setValue")) Module["setValue"] = function() { abort("'setValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getValue")) Module["getValue"] = function() { abort("'getValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "allocate")) Module["allocate"] = function() { abort("'allocate' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "UTF8ArrayToString")) Module["UTF8ArrayToString"] = function() { abort("'UTF8ArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "UTF8ToString")) Module["UTF8ToString"] = function() { abort("'UTF8ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF8Array")) Module["stringToUTF8Array"] = function() { abort("'stringToUTF8Array' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF8")) Module["stringToUTF8"] = function() { abort("'stringToUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF8")) Module["lengthBytesUTF8"] = function() { abort("'lengthBytesUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stackTrace")) Module["stackTrace"] = function() { abort("'stackTrace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "addOnPreRun")) Module["addOnPreRun"] = function() { abort("'addOnPreRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "addOnInit")) Module["addOnInit"] = function() { abort("'addOnInit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "addOnPreMain")) Module["addOnPreMain"] = function() { abort("'addOnPreMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "addOnExit")) Module["addOnExit"] = function() { abort("'addOnExit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "addOnPostRun")) Module["addOnPostRun"] = function() { abort("'addOnPostRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeStringToMemory")) Module["writeStringToMemory"] = function() { abort("'writeStringToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeArrayToMemory")) Module["writeArrayToMemory"] = function() { abort("'writeArrayToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeAsciiToMemory")) Module["writeAsciiToMemory"] = function() { abort("'writeAsciiToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "addRunDependency")) Module["addRunDependency"] = function() { abort("'addRunDependency' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "removeRunDependency")) Module["removeRunDependency"] = function() { abort("'removeRunDependency' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createFolder")) Module["FS_createFolder"] = function() { abort("'FS_createFolder' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createPath")) Module["FS_createPath"] = function() { abort("'FS_createPath' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createDataFile")) Module["FS_createDataFile"] = function() { abort("'FS_createDataFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createPreloadedFile")) Module["FS_createPreloadedFile"] = function() { abort("'FS_createPreloadedFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createLazyFile")) Module["FS_createLazyFile"] = function() { abort("'FS_createLazyFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createLink")) Module["FS_createLink"] = function() { abort("'FS_createLink' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_createDevice")) Module["FS_createDevice"] = function() { abort("'FS_createDevice' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "FS_unlink")) Module["FS_unlink"] = function() { abort("'FS_unlink' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you") };
if (!Object.getOwnPropertyDescriptor(Module, "getLEB")) Module["getLEB"] = function() { abort("'getLEB' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getFunctionTables")) Module["getFunctionTables"] = function() { abort("'getFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "alignFunctionTables")) Module["alignFunctionTables"] = function() { abort("'alignFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerFunctions")) Module["registerFunctions"] = function() { abort("'registerFunctions' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "addFunction")) Module["addFunction"] = function() { abort("'addFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "removeFunction")) Module["removeFunction"] = function() { abort("'removeFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getFuncWrapper")) Module["getFuncWrapper"] = function() { abort("'getFuncWrapper' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "prettyPrint")) Module["prettyPrint"] = function() { abort("'prettyPrint' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "makeBigInt")) Module["makeBigInt"] = function() { abort("'makeBigInt' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "dynCall")) Module["dynCall"] = function() { abort("'dynCall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getCompilerSetting")) Module["getCompilerSetting"] = function() { abort("'getCompilerSetting' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "print")) Module["print"] = function() { abort("'print' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "printErr")) Module["printErr"] = function() { abort("'printErr' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getTempRet0")) Module["getTempRet0"] = function() { abort("'getTempRet0' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setTempRet0")) Module["setTempRet0"] = function() { abort("'setTempRet0' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "callMain")) Module["callMain"] = function() { abort("'callMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "abort")) Module["abort"] = function() { abort("'abort' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stringToNewUTF8")) Module["stringToNewUTF8"] = function() { abort("'stringToNewUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setFileTime")) Module["setFileTime"] = function() { abort("'setFileTime' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emscripten_realloc_buffer")) Module["emscripten_realloc_buffer"] = function() { abort("'emscripten_realloc_buffer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ENV")) Module["ENV"] = function() { abort("'ENV' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ERRNO_CODES")) Module["ERRNO_CODES"] = function() { abort("'ERRNO_CODES' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ERRNO_MESSAGES")) Module["ERRNO_MESSAGES"] = function() { abort("'ERRNO_MESSAGES' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setErrNo")) Module["setErrNo"] = function() { abort("'setErrNo' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "DNS")) Module["DNS"] = function() { abort("'DNS' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getHostByName")) Module["getHostByName"] = function() { abort("'getHostByName' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "GAI_ERRNO_MESSAGES")) Module["GAI_ERRNO_MESSAGES"] = function() { abort("'GAI_ERRNO_MESSAGES' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "Protocols")) Module["Protocols"] = function() { abort("'Protocols' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "Sockets")) Module["Sockets"] = function() { abort("'Sockets' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getRandomDevice")) Module["getRandomDevice"] = function() { abort("'getRandomDevice' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "traverseStack")) Module["traverseStack"] = function() { abort("'traverseStack' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "UNWIND_CACHE")) Module["UNWIND_CACHE"] = function() { abort("'UNWIND_CACHE' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "withBuiltinMalloc")) Module["withBuiltinMalloc"] = function() { abort("'withBuiltinMalloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "readAsmConstArgsArray")) Module["readAsmConstArgsArray"] = function() { abort("'readAsmConstArgsArray' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "readAsmConstArgs")) Module["readAsmConstArgs"] = function() { abort("'readAsmConstArgs' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "mainThreadEM_ASM")) Module["mainThreadEM_ASM"] = function() { abort("'mainThreadEM_ASM' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "jstoi_q")) Module["jstoi_q"] = function() { abort("'jstoi_q' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "jstoi_s")) Module["jstoi_s"] = function() { abort("'jstoi_s' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getExecutableName")) Module["getExecutableName"] = function() { abort("'getExecutableName' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "listenOnce")) Module["listenOnce"] = function() { abort("'listenOnce' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "autoResumeAudioContext")) Module["autoResumeAudioContext"] = function() { abort("'autoResumeAudioContext' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "dynCallLegacy")) Module["dynCallLegacy"] = function() { abort("'dynCallLegacy' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getDynCaller")) Module["getDynCaller"] = function() { abort("'getDynCaller' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "dynCall")) Module["dynCall"] = function() { abort("'dynCall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "callRuntimeCallbacks")) Module["callRuntimeCallbacks"] = function() { abort("'callRuntimeCallbacks' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "abortStackOverflow")) Module["abortStackOverflow"] = function() { abort("'abortStackOverflow' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "reallyNegative")) Module["reallyNegative"] = function() { abort("'reallyNegative' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "unSign")) Module["unSign"] = function() { abort("'unSign' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "reSign")) Module["reSign"] = function() { abort("'reSign' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "formatString")) Module["formatString"] = function() { abort("'formatString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "PATH")) Module["PATH"] = function() { abort("'PATH' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "PATH_FS")) Module["PATH_FS"] = function() { abort("'PATH_FS' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "SYSCALLS")) Module["SYSCALLS"] = function() { abort("'SYSCALLS' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "syscallMmap2")) Module["syscallMmap2"] = function() { abort("'syscallMmap2' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "syscallMunmap")) Module["syscallMunmap"] = function() { abort("'syscallMunmap' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getSocketFromFD")) Module["getSocketFromFD"] = function() { abort("'getSocketFromFD' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getSocketAddress")) Module["getSocketAddress"] = function() { abort("'getSocketAddress' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "JSEvents")) Module["JSEvents"] = function() { abort("'JSEvents' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerKeyEventCallback")) Module["registerKeyEventCallback"] = function() { abort("'registerKeyEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "specialHTMLTargets")) Module["specialHTMLTargets"] = function() { abort("'specialHTMLTargets' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "maybeCStringToJsString")) Module["maybeCStringToJsString"] = function() { abort("'maybeCStringToJsString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "findEventTarget")) Module["findEventTarget"] = function() { abort("'findEventTarget' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "findCanvasEventTarget")) Module["findCanvasEventTarget"] = function() { abort("'findCanvasEventTarget' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getBoundingClientRect")) Module["getBoundingClientRect"] = function() { abort("'getBoundingClientRect' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillMouseEventData")) Module["fillMouseEventData"] = function() { abort("'fillMouseEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerMouseEventCallback")) Module["registerMouseEventCallback"] = function() { abort("'registerMouseEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerWheelEventCallback")) Module["registerWheelEventCallback"] = function() { abort("'registerWheelEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerUiEventCallback")) Module["registerUiEventCallback"] = function() { abort("'registerUiEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerFocusEventCallback")) Module["registerFocusEventCallback"] = function() { abort("'registerFocusEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillDeviceOrientationEventData")) Module["fillDeviceOrientationEventData"] = function() { abort("'fillDeviceOrientationEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerDeviceOrientationEventCallback")) Module["registerDeviceOrientationEventCallback"] = function() { abort("'registerDeviceOrientationEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillDeviceMotionEventData")) Module["fillDeviceMotionEventData"] = function() { abort("'fillDeviceMotionEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerDeviceMotionEventCallback")) Module["registerDeviceMotionEventCallback"] = function() { abort("'registerDeviceMotionEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "screenOrientation")) Module["screenOrientation"] = function() { abort("'screenOrientation' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillOrientationChangeEventData")) Module["fillOrientationChangeEventData"] = function() { abort("'fillOrientationChangeEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerOrientationChangeEventCallback")) Module["registerOrientationChangeEventCallback"] = function() { abort("'registerOrientationChangeEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillFullscreenChangeEventData")) Module["fillFullscreenChangeEventData"] = function() { abort("'fillFullscreenChangeEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerFullscreenChangeEventCallback")) Module["registerFullscreenChangeEventCallback"] = function() { abort("'registerFullscreenChangeEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerRestoreOldStyle")) Module["registerRestoreOldStyle"] = function() { abort("'registerRestoreOldStyle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "hideEverythingExceptGivenElement")) Module["hideEverythingExceptGivenElement"] = function() { abort("'hideEverythingExceptGivenElement' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "restoreHiddenElements")) Module["restoreHiddenElements"] = function() { abort("'restoreHiddenElements' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setLetterbox")) Module["setLetterbox"] = function() { abort("'setLetterbox' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "currentFullscreenStrategy")) Module["currentFullscreenStrategy"] = function() { abort("'currentFullscreenStrategy' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "restoreOldWindowedStyle")) Module["restoreOldWindowedStyle"] = function() { abort("'restoreOldWindowedStyle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "softFullscreenResizeWebGLRenderTarget")) Module["softFullscreenResizeWebGLRenderTarget"] = function() { abort("'softFullscreenResizeWebGLRenderTarget' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "doRequestFullscreen")) Module["doRequestFullscreen"] = function() { abort("'doRequestFullscreen' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillPointerlockChangeEventData")) Module["fillPointerlockChangeEventData"] = function() { abort("'fillPointerlockChangeEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerPointerlockChangeEventCallback")) Module["registerPointerlockChangeEventCallback"] = function() { abort("'registerPointerlockChangeEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerPointerlockErrorEventCallback")) Module["registerPointerlockErrorEventCallback"] = function() { abort("'registerPointerlockErrorEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "requestPointerLock")) Module["requestPointerLock"] = function() { abort("'requestPointerLock' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillVisibilityChangeEventData")) Module["fillVisibilityChangeEventData"] = function() { abort("'fillVisibilityChangeEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerVisibilityChangeEventCallback")) Module["registerVisibilityChangeEventCallback"] = function() { abort("'registerVisibilityChangeEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerTouchEventCallback")) Module["registerTouchEventCallback"] = function() { abort("'registerTouchEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillGamepadEventData")) Module["fillGamepadEventData"] = function() { abort("'fillGamepadEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerGamepadEventCallback")) Module["registerGamepadEventCallback"] = function() { abort("'registerGamepadEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerBeforeUnloadEventCallback")) Module["registerBeforeUnloadEventCallback"] = function() { abort("'registerBeforeUnloadEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "fillBatteryEventData")) Module["fillBatteryEventData"] = function() { abort("'fillBatteryEventData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "battery")) Module["battery"] = function() { abort("'battery' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerBatteryEventCallback")) Module["registerBatteryEventCallback"] = function() { abort("'registerBatteryEventCallback' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setCanvasElementSize")) Module["setCanvasElementSize"] = function() { abort("'setCanvasElementSize' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getCanvasElementSize")) Module["getCanvasElementSize"] = function() { abort("'getCanvasElementSize' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "polyfillSetImmediate")) Module["polyfillSetImmediate"] = function() { abort("'polyfillSetImmediate' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "demangle")) Module["demangle"] = function() { abort("'demangle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "demangleAll")) Module["demangleAll"] = function() { abort("'demangleAll' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "jsStackTrace")) Module["jsStackTrace"] = function() { abort("'jsStackTrace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stackTrace")) Module["stackTrace"] = function() { abort("'stackTrace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getEnvStrings")) Module["getEnvStrings"] = function() { abort("'getEnvStrings' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "checkWasiClock")) Module["checkWasiClock"] = function() { abort("'checkWasiClock' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "flush_NO_FILESYSTEM")) Module["flush_NO_FILESYSTEM"] = function() { abort("'flush_NO_FILESYSTEM' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToI64")) Module["writeI53ToI64"] = function() { abort("'writeI53ToI64' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToI64Clamped")) Module["writeI53ToI64Clamped"] = function() { abort("'writeI53ToI64Clamped' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToI64Signaling")) Module["writeI53ToI64Signaling"] = function() { abort("'writeI53ToI64Signaling' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToU64Clamped")) Module["writeI53ToU64Clamped"] = function() { abort("'writeI53ToU64Clamped' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToU64Signaling")) Module["writeI53ToU64Signaling"] = function() { abort("'writeI53ToU64Signaling' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "readI53FromI64")) Module["readI53FromI64"] = function() { abort("'readI53FromI64' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "readI53FromU64")) Module["readI53FromU64"] = function() { abort("'readI53FromU64' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "convertI32PairToI53")) Module["convertI32PairToI53"] = function() { abort("'convertI32PairToI53' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "convertU32PairToI53")) Module["convertU32PairToI53"] = function() { abort("'convertU32PairToI53' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "uncaughtExceptionCount")) Module["uncaughtExceptionCount"] = function() { abort("'uncaughtExceptionCount' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "exceptionLast")) Module["exceptionLast"] = function() { abort("'exceptionLast' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "exceptionCaught")) Module["exceptionCaught"] = function() { abort("'exceptionCaught' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ExceptionInfoAttrs")) Module["ExceptionInfoAttrs"] = function() { abort("'ExceptionInfoAttrs' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ExceptionInfo")) Module["ExceptionInfo"] = function() { abort("'ExceptionInfo' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "CatchInfo")) Module["CatchInfo"] = function() { abort("'CatchInfo' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "exception_addRef")) Module["exception_addRef"] = function() { abort("'exception_addRef' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "exception_decRef")) Module["exception_decRef"] = function() { abort("'exception_decRef' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "Browser")) Module["Browser"] = function() { abort("'Browser' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "funcWrappers")) Module["funcWrappers"] = function() { abort("'funcWrappers' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getFuncWrapper")) Module["getFuncWrapper"] = function() { abort("'getFuncWrapper' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setMainLoop")) Module["setMainLoop"] = function() { abort("'setMainLoop' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "tempFixedLengthArray")) Module["tempFixedLengthArray"] = function() { abort("'tempFixedLengthArray' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "miniTempWebGLFloatBuffers")) Module["miniTempWebGLFloatBuffers"] = function() { abort("'miniTempWebGLFloatBuffers' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "heapObjectForWebGLType")) Module["heapObjectForWebGLType"] = function() { abort("'heapObjectForWebGLType' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "heapAccessShiftForWebGLHeap")) Module["heapAccessShiftForWebGLHeap"] = function() { abort("'heapAccessShiftForWebGLHeap' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "GL")) Module["GL"] = function() { abort("'GL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGet")) Module["emscriptenWebGLGet"] = function() { abort("'emscriptenWebGLGet' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "computeUnpackAlignedImageSize")) Module["computeUnpackAlignedImageSize"] = function() { abort("'computeUnpackAlignedImageSize' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGetTexPixelData")) Module["emscriptenWebGLGetTexPixelData"] = function() { abort("'emscriptenWebGLGetTexPixelData' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGetUniform")) Module["emscriptenWebGLGetUniform"] = function() { abort("'emscriptenWebGLGetUniform' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGetVertexAttrib")) Module["emscriptenWebGLGetVertexAttrib"] = function() { abort("'emscriptenWebGLGetVertexAttrib' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "writeGLArray")) Module["writeGLArray"] = function() { abort("'writeGLArray' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "AL")) Module["AL"] = function() { abort("'AL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "SDL_unicode")) Module["SDL_unicode"] = function() { abort("'SDL_unicode' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "SDL_ttfContext")) Module["SDL_ttfContext"] = function() { abort("'SDL_ttfContext' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "SDL_audio")) Module["SDL_audio"] = function() { abort("'SDL_audio' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "SDL")) Module["SDL"] = function() { abort("'SDL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "SDL_gfx")) Module["SDL_gfx"] = function() { abort("'SDL_gfx' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "GLUT")) Module["GLUT"] = function() { abort("'GLUT' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "EGL")) Module["EGL"] = function() { abort("'EGL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "GLFW_Window")) Module["GLFW_Window"] = function() { abort("'GLFW_Window' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "GLFW")) Module["GLFW"] = function() { abort("'GLFW' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "GLEW")) Module["GLEW"] = function() { abort("'GLEW' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "IDBStore")) Module["IDBStore"] = function() { abort("'IDBStore' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "runAndAbortIfError")) Module["runAndAbortIfError"] = function() { abort("'runAndAbortIfError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emval_handle_array")) Module["emval_handle_array"] = function() { abort("'emval_handle_array' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emval_free_list")) Module["emval_free_list"] = function() { abort("'emval_free_list' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emval_symbols")) Module["emval_symbols"] = function() { abort("'emval_symbols' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "init_emval")) Module["init_emval"] = function() { abort("'init_emval' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "count_emval_handles")) Module["count_emval_handles"] = function() { abort("'count_emval_handles' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "get_first_emval")) Module["get_first_emval"] = function() { abort("'get_first_emval' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getStringOrSymbol")) Module["getStringOrSymbol"] = function() { abort("'getStringOrSymbol' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "requireHandle")) Module["requireHandle"] = function() { abort("'requireHandle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emval_newers")) Module["emval_newers"] = function() { abort("'emval_newers' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "craftEmvalAllocator")) Module["craftEmvalAllocator"] = function() { abort("'craftEmvalAllocator' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emval_get_global")) Module["emval_get_global"] = function() { abort("'emval_get_global' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emval_methodCallers")) Module["emval_methodCallers"] = function() { abort("'emval_methodCallers' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "InternalError")) Module["InternalError"] = function() { abort("'InternalError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "BindingError")) Module["BindingError"] = function() { abort("'BindingError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "UnboundTypeError")) Module["UnboundTypeError"] = function() { abort("'UnboundTypeError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "PureVirtualError")) Module["PureVirtualError"] = function() { abort("'PureVirtualError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "init_embind")) Module["init_embind"] = function() { abort("'init_embind' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "throwInternalError")) Module["throwInternalError"] = function() { abort("'throwInternalError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "throwBindingError")) Module["throwBindingError"] = function() { abort("'throwBindingError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "throwUnboundTypeError")) Module["throwUnboundTypeError"] = function() { abort("'throwUnboundTypeError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ensureOverloadTable")) Module["ensureOverloadTable"] = function() { abort("'ensureOverloadTable' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "exposePublicSymbol")) Module["exposePublicSymbol"] = function() { abort("'exposePublicSymbol' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "replacePublicSymbol")) Module["replacePublicSymbol"] = function() { abort("'replacePublicSymbol' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "extendError")) Module["extendError"] = function() { abort("'extendError' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "createNamedFunction")) Module["createNamedFunction"] = function() { abort("'createNamedFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registeredInstances")) Module["registeredInstances"] = function() { abort("'registeredInstances' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getBasestPointer")) Module["getBasestPointer"] = function() { abort("'getBasestPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerInheritedInstance")) Module["registerInheritedInstance"] = function() { abort("'registerInheritedInstance' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "unregisterInheritedInstance")) Module["unregisterInheritedInstance"] = function() { abort("'unregisterInheritedInstance' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getInheritedInstance")) Module["getInheritedInstance"] = function() { abort("'getInheritedInstance' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getInheritedInstanceCount")) Module["getInheritedInstanceCount"] = function() { abort("'getInheritedInstanceCount' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getLiveInheritedInstances")) Module["getLiveInheritedInstances"] = function() { abort("'getLiveInheritedInstances' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registeredTypes")) Module["registeredTypes"] = function() { abort("'registeredTypes' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "awaitingDependencies")) Module["awaitingDependencies"] = function() { abort("'awaitingDependencies' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "typeDependencies")) Module["typeDependencies"] = function() { abort("'typeDependencies' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registeredPointers")) Module["registeredPointers"] = function() { abort("'registeredPointers' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "registerType")) Module["registerType"] = function() { abort("'registerType' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "whenDependentTypesAreResolved")) Module["whenDependentTypesAreResolved"] = function() { abort("'whenDependentTypesAreResolved' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "embind_charCodes")) Module["embind_charCodes"] = function() { abort("'embind_charCodes' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "embind_init_charCodes")) Module["embind_init_charCodes"] = function() { abort("'embind_init_charCodes' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "readLatin1String")) Module["readLatin1String"] = function() { abort("'readLatin1String' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getTypeName")) Module["getTypeName"] = function() { abort("'getTypeName' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "heap32VectorToArray")) Module["heap32VectorToArray"] = function() { abort("'heap32VectorToArray' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "requireRegisteredType")) Module["requireRegisteredType"] = function() { abort("'requireRegisteredType' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "getShiftFromSize")) Module["getShiftFromSize"] = function() { abort("'getShiftFromSize' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "integerReadValueFromPointer")) Module["integerReadValueFromPointer"] = function() { abort("'integerReadValueFromPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "enumReadValueFromPointer")) Module["enumReadValueFromPointer"] = function() { abort("'enumReadValueFromPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "floatReadValueFromPointer")) Module["floatReadValueFromPointer"] = function() { abort("'floatReadValueFromPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "simpleReadValueFromPointer")) Module["simpleReadValueFromPointer"] = function() { abort("'simpleReadValueFromPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "runDestructors")) Module["runDestructors"] = function() { abort("'runDestructors' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "new_")) Module["new_"] = function() { abort("'new_' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "craftInvokerFunction")) Module["craftInvokerFunction"] = function() { abort("'craftInvokerFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "embind__requireFunction")) Module["embind__requireFunction"] = function() { abort("'embind__requireFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "tupleRegistrations")) Module["tupleRegistrations"] = function() { abort("'tupleRegistrations' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "structRegistrations")) Module["structRegistrations"] = function() { abort("'structRegistrations' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "genericPointerToWireType")) Module["genericPointerToWireType"] = function() { abort("'genericPointerToWireType' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "constNoSmartPtrRawPointerToWireType")) Module["constNoSmartPtrRawPointerToWireType"] = function() { abort("'constNoSmartPtrRawPointerToWireType' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "nonConstNoSmartPtrRawPointerToWireType")) Module["nonConstNoSmartPtrRawPointerToWireType"] = function() { abort("'nonConstNoSmartPtrRawPointerToWireType' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "init_RegisteredPointer")) Module["init_RegisteredPointer"] = function() { abort("'init_RegisteredPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "RegisteredPointer")) Module["RegisteredPointer"] = function() { abort("'RegisteredPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "RegisteredPointer_getPointee")) Module["RegisteredPointer_getPointee"] = function() { abort("'RegisteredPointer_getPointee' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "RegisteredPointer_destructor")) Module["RegisteredPointer_destructor"] = function() { abort("'RegisteredPointer_destructor' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "RegisteredPointer_deleteObject")) Module["RegisteredPointer_deleteObject"] = function() { abort("'RegisteredPointer_deleteObject' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "RegisteredPointer_fromWireType")) Module["RegisteredPointer_fromWireType"] = function() { abort("'RegisteredPointer_fromWireType' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "runDestructor")) Module["runDestructor"] = function() { abort("'runDestructor' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "releaseClassHandle")) Module["releaseClassHandle"] = function() { abort("'releaseClassHandle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "finalizationGroup")) Module["finalizationGroup"] = function() { abort("'finalizationGroup' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "detachFinalizer_deps")) Module["detachFinalizer_deps"] = function() { abort("'detachFinalizer_deps' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "detachFinalizer")) Module["detachFinalizer"] = function() { abort("'detachFinalizer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "attachFinalizer")) Module["attachFinalizer"] = function() { abort("'attachFinalizer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "makeClassHandle")) Module["makeClassHandle"] = function() { abort("'makeClassHandle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "init_ClassHandle")) Module["init_ClassHandle"] = function() { abort("'init_ClassHandle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ClassHandle")) Module["ClassHandle"] = function() { abort("'ClassHandle' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ClassHandle_isAliasOf")) Module["ClassHandle_isAliasOf"] = function() { abort("'ClassHandle_isAliasOf' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "throwInstanceAlreadyDeleted")) Module["throwInstanceAlreadyDeleted"] = function() { abort("'throwInstanceAlreadyDeleted' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ClassHandle_clone")) Module["ClassHandle_clone"] = function() { abort("'ClassHandle_clone' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ClassHandle_delete")) Module["ClassHandle_delete"] = function() { abort("'ClassHandle_delete' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "deletionQueue")) Module["deletionQueue"] = function() { abort("'deletionQueue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ClassHandle_isDeleted")) Module["ClassHandle_isDeleted"] = function() { abort("'ClassHandle_isDeleted' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "ClassHandle_deleteLater")) Module["ClassHandle_deleteLater"] = function() { abort("'ClassHandle_deleteLater' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "flushPendingDeletes")) Module["flushPendingDeletes"] = function() { abort("'flushPendingDeletes' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "delayFunction")) Module["delayFunction"] = function() { abort("'delayFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "setDelayFunction")) Module["setDelayFunction"] = function() { abort("'setDelayFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "RegisteredClass")) Module["RegisteredClass"] = function() { abort("'RegisteredClass' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "shallowCopyInternalPointer")) Module["shallowCopyInternalPointer"] = function() { abort("'shallowCopyInternalPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "downcastPointer")) Module["downcastPointer"] = function() { abort("'downcastPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "upcastPointer")) Module["upcastPointer"] = function() { abort("'upcastPointer' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "validateThis")) Module["validateThis"] = function() { abort("'validateThis' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "char_0")) Module["char_0"] = function() { abort("'char_0' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "char_9")) Module["char_9"] = function() { abort("'char_9' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "makeLegalFunctionName")) Module["makeLegalFunctionName"] = function() { abort("'makeLegalFunctionName' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGetIndexed")) Module["emscriptenWebGLGetIndexed"] = function() { abort("'emscriptenWebGLGetIndexed' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "warnOnce")) Module["warnOnce"] = function() { abort("'warnOnce' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stackSave")) Module["stackSave"] = function() { abort("'stackSave' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stackRestore")) Module["stackRestore"] = function() { abort("'stackRestore' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stackAlloc")) Module["stackAlloc"] = function() { abort("'stackAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "AsciiToString")) Module["AsciiToString"] = function() { abort("'AsciiToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stringToAscii")) Module["stringToAscii"] = function() { abort("'stringToAscii' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "UTF16ToString")) Module["UTF16ToString"] = function() { abort("'UTF16ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF16")) Module["stringToUTF16"] = function() { abort("'stringToUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF16")) Module["lengthBytesUTF16"] = function() { abort("'lengthBytesUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "UTF32ToString")) Module["UTF32ToString"] = function() { abort("'UTF32ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF32")) Module["stringToUTF32"] = function() { abort("'stringToUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF32")) Module["lengthBytesUTF32"] = function() { abort("'lengthBytesUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "allocateUTF8")) Module["allocateUTF8"] = function() { abort("'allocateUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
if (!Object.getOwnPropertyDescriptor(Module, "allocateUTF8OnStack")) Module["allocateUTF8OnStack"] = function() { abort("'allocateUTF8OnStack' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") };
Module["writeStackCookie"] = writeStackCookie;
Module["checkStackCookie"] = checkStackCookie;
if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_NORMAL")) Object.defineProperty(Module, "ALLOC_NORMAL", { configurable: true, get: function() { abort("'ALLOC_NORMAL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") } });
if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_STACK")) Object.defineProperty(Module, "ALLOC_STACK", { configurable: true, get: function() { abort("'ALLOC_STACK' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)") } });

var calledRun;

/**
 * @constructor
 * @this {ExitStatus}
 */
function ExitStatus(status) {
  this.name = "ExitStatus";
  this.message = "Program terminated with exit(" + status + ")";
  this.status = status;
}

var calledMain = false;

dependenciesFulfilled = function runCaller() {
  // If run has never been called, and we should call run (INVOKE_RUN is true, and Module.noInitialRun is not false)
  if (!calledRun) run();
  if (!calledRun) dependenciesFulfilled = runCaller; // try this again later, after new deps are fulfilled
};

function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  _emscripten_stack_init();
  writeStackCookie();
}

/** @type {function(Array=)} */
function run(args) {
  args = args || arguments_;

  if (runDependencies > 0) {
    return;
  }

  stackCheckInit();

  preRun();

  // a preRun added a dependency, run will be called later
  if (runDependencies > 0) {
    return;
  }

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
    if (calledRun) return;
    calledRun = true;
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

    preMain();

    readyPromiseResolve(Module);
    if (Module['onRuntimeInitialized']) Module['onRuntimeInitialized']();

    assert(!Module['_main'], 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');

    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      doRun();
    }, 1);
  } else
  {
    doRun();
  }
  checkStackCookie();
}
Module['run'] = run;

function checkUnflushedContent() {
  // Compiler settings do not allow exiting the runtime, so flushing
  // the streams is not possible. but in ASSERTIONS mode we check
  // if there was something to flush, and if so tell the user they
  // should request that the runtime be exitable.
  // Normally we would not even include flush() at all, but in ASSERTIONS
  // builds we do so just for this check, and here we see if there is any
  // content to flush, that is, we check if there would have been
  // something a non-ASSERTIONS build would have not seen.
  // How we flush the streams depends on whether we are in SYSCALLS_REQUIRE_FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var oldOut = out;
  var oldErr = err;
  var has = false;
  out = err = function(x) {
    has = true;
  }
  try { // it doesn't matter if it fails
    var flush = flush_NO_FILESYSTEM;
    if (flush) flush();
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the FAQ), or make sure to emit a newline when you printf etc.');
    warnOnce('(this may also be due to not including full filesystem support - try building with -s FORCE_FILESYSTEM=1)');
  }
}

/** @param {boolean|number=} implicit */
function exit(status, implicit) {
  checkUnflushedContent();

  // if this is just main exit-ing implicitly, and the status is 0, then we
  // don't need to do anything here and can just leave. if the status is
  // non-zero, though, then we need to report it.
  // (we may have warned about this earlier, if a situation justifies doing so)
  if (implicit && noExitRuntime && status === 0) {
    return;
  }

  if (noExitRuntime) {
    // if exit() was called, we may warn the user if the runtime isn't actually being shut down
    if (!implicit) {
      var msg = 'program exited (with status: ' + status + '), but EXIT_RUNTIME is not set, so halting execution but not exiting the runtime or preventing further async execution (build with EXIT_RUNTIME=1, if you want a true shutdown)';
      readyPromiseReject(msg);
      err(msg);
    }
  } else {

    EXITSTATUS = status;

    exitRuntime();

    if (Module['onExit']) Module['onExit'](status);

    ABORT = true;
  }

  quit_(status, new ExitStatus(status));
}

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}

noExitRuntime = true;

run();







  return CanvasKitInit.ready
}
);
})();
if (typeof exports === 'object' && typeof module === 'object')
  module.exports = CanvasKitInit;
else if (typeof define === 'function' && define['amd'])
  define([], function() { return CanvasKitInit; });
else if (typeof exports === 'object')
  exports["CanvasKitInit"] = CanvasKitInit;
