CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  CanvasKit.SkRuntimeEffect.prototype.makeShader = function(floats, isOpaque, matrix) {
    var fptr = copy1dArray(floats, CanvasKit.HEAPF32);
    // Our array has 4 bytes per float, so be sure to account for that before
    // sending it over the wire.
    if (!matrix) {
      return this._makeShader(fptr, floats.length * 4, !!isOpaque);
    }
    return this._makeShader(fptr, floats.length * 4, !!isOpaque, matrix);
  }

  // childrenWithShaders is an array of other shaders (e.g. SkImage.makeShader())
  CanvasKit.SkRuntimeEffect.prototype.makeShaderWithChildren = function(floats, isOpaque, childrenShaders, matrix) {
    var fptr = copy1dArray(floats, CanvasKit.HEAPF32);
    var barePointers = [];
    for (var i = 0; i<childrenShaders.length;i++) {
      // childrenShaders are emscriptens smart pointer type. We want to get the bare pointer
      // and send that over the wire, so it can be re-wrapped as an sk_sp.
      barePointers.push(childrenShaders[i].$$.ptr);
    }
    var childrenPointers = copy1dArray(barePointers, CanvasKit.HEAPU32);
    // Our array has 4 bytes per float, so be sure to account for that before
    // sending it over the wire.
    if (!matrix) {
      return this._makeShaderWithChildren(fptr, floats.length * 4, !!isOpaque, childrenPointers, barePointers.length);
    }
    return this._makeShaderWithChildren(fptr, floats.length * 4, !!isOpaque, childrenPointers, barePointers.length, matrix);
  }
});