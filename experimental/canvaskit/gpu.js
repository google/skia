// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the GPU version of canvaskit.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      CanvasKit.MakeWebGLCanvasSurface = function(htmlID) {
        var canvas = document.getElementById(htmlID);
        if (!canvas) {
          throw 'Canvas with id ' + htmlID + ' was not found';
        }
        // Maybe better to use clientWidth/height.  See:
        // https://webglfundamentals.org/webgl/lessons/webgl-anti-patterns.html
        var surface = this._getWebGLSurface(htmlID, canvas.width, canvas.height);
        if (!surface) {
          SkDebug('falling back from GPU implementation to a SW based one');
          return CanvasKit.MakeSWCanvasSurface(htmlID);
        }
        return surface;
      };
      // Default to trying WebGL first.
      CanvasKit.MakeCanvasSurface = CanvasKit.MakeWebGLCanvasSurface;
    });
}(Module)); // When this file is loaded in, the high level object is "Module";
