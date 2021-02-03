CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {

  // sksl is the shader code.
  // errorCallback is a function that will be called with an error string if the
  // effect cannot be made. If not provided, the error will be logged.
  CanvasKit.RuntimeEffect.Make = function(sksl, errorCallback) {
    // The easiest way to pass a function into C++ code is to wrap it in an object and
    // treat it as an emscripten::val on the other side.
    var callbackObj = {
      'onError': errorCallback || function(err) {
        console.log('RuntimeEffect error', err);
      },
    };
    return CanvasKit.RuntimeEffect._Make(sksl, callbackObj);
  };

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
