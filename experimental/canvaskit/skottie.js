// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the Skottie builds of canvaskit.
(function(CanvasKit){

  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      if (CanvasKit.SkottieAssetProvider) {
        CanvasKit.SkottieAssetProvider.prototype.setImage = function(path, name, imgData) {
          var data = new Uint8Array(imgData);

          var iptr = CanvasKit._malloc(data.byteLength);
          CanvasKit.HEAPU8.set(data, iptr);
          this._setImage(path, name, iptr, data.byteLength);
        }
      }
    });
}(Module)); // When this file is loaded in, the high level object is "Module";
