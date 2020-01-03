// Adds in the code to use pathops with SkPath
CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  CanvasKit.SkPath.prototype.op = function(otherPath, op) {
    if (this._op(otherPath, op)) {
      return this;
    }
    return null;
  };

  CanvasKit.SkPath.prototype.simplify = function() {
    if (this._simplify()) {
      return this;
    }
    return null;
  };
});