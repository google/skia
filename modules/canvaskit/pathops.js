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
