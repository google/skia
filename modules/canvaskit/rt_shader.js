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