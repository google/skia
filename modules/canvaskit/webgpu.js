// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
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

      // TODO(skia:12512): Implement WebGPU functionality.
    });
}(Module));  // When this file is loaded in, the high level object is "Module".
