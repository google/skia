// Adds JS functions to augment the Skia WASM interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.
(function(Skia){
  // Skia.onRuntimeInitialized is called after the WASM library has loaded.
  // Anything that modifies an exposed class (e.g. SkPath) should be set
  // after onRuntimeInitialized, otherwise, it can happen outside of that scope.
  Skia.onRuntimeInitialized = function() {
    // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

  }

  Skia.getWebGLSurface = function(htmlID) {
    var canvas = document.getElementById(htmlID);
    if (!canvas) {
      throw 'Canvas with id ' + htmlID + ' was not found';
    }
    return this._getWebGLSurface(htmlID, canvas.width, canvas.height);
  }

}(Module)); // When this file is loaded in, the high level object is "Module";