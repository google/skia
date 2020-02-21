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
});
CanvasKit._extraInitializations.push(function() {
  // children is an array of
  CanvasKit.SkRuntimeEffect.prototype.makeShaderWithChildren = function(floats, children, isOpaque, matrix) {
    var fptr = copy1dArray(floats, CanvasKit.HEAPF32);
    var chilptr = copy1dArray(children, CanvasKit.HEAPU32);
    if (!matrix) {
      return this._makeShaderWithChildren(fptr, floats.length*4, chilptr, children.length*4, !!isOpaque);
    }
    return this._makeShaderWithChildren(fptr, floats.length*4, chilptr, children.length*4, !!isOpaque, matrix);
  }
});