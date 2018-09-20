
// Adds any extra JS functions/helpers we want to CanvasKit.
// Wrapped in a function to avoid leaking global variables.
(function(CanvasKit){

  function clamp(c) {
    return Math.round(Math.max(0, Math.min(c || 0, 255)));
  }

  // Colors are just a 32 bit number with 8 bits each of a, r, g, b
  // The API is the same as CSS's representation of color rgba(), that is
  // r,g,b are 0-255, and a is 0.0 to 1.0.
  CanvasKit.Color = function(r, g, b, a) {
    return (clamp(a*255) << 24) | (clamp(r) << 16) | (clamp(g) << 8) | (clamp(b) << 0);
  }
}(Module)); // When this file is loaded in, the high level object is "Module";