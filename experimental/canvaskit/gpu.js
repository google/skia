// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the GPU version of canvaskit.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      CanvasKit.MakeCanvasSurface = function(htmlID) {
        var canvas = document.getElementById(htmlID);
        if (!canvas) {
          throw 'Canvas with id ' + htmlID + ' was not found';
        }
        // Maybe better to use clientWidth/height.  See:
        // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
        return this._getWebGLSurface(htmlID, canvas.width, canvas.height);
      };

      CanvasKit.SkSurface.prototype.flush = function() {
        this._flush();
      }

      CanvasKit.SkSurface.prototype.dispose = function() {
        this.delete();
      }
    });
}(Module)); // When this file is loaded in, the high level object is "Module";
