
// Adds any extra JS functions/helpers we want to add to CanvasKit.
// Wrapped in a function to avoid leaking global variables.
(function(CanvasKit){

  function clamp(c) {
    return Math.round(Math.max(0, Math.min(c || 0, 255)));
  }

  // Colors are just a 32 bit number with 8 bits each of a, r, g, b
  // The API is the same as CSS's representation of color rgba(), that is
  // r,g,b are 0-255, and a is 0.0 to 1.0.
  // if a is omitted, it will be assumed to be 1.0
  CanvasKit.Color = function(r, g, b, a) {
    if (a === undefined) {
        a = 1;
    }
    return (clamp(a*255) << 24) | (clamp(r) << 16) | (clamp(g) << 8) | (clamp(b) << 0);
  }

  // returns [r, g, b, a] from a color
  // where a is scaled between 0 and 1.0
  CanvasKit.getColorComponents = function(color) {
    return [
       (color >> 16) & 0xFF,
       (color >>  8) & 0xFF,
       (color >>  0) & 0xFF,
      ((color >> 24) & 0xFF) / 255,
    ]
  }

  CanvasKit.multiplyByAlpha = function(color, alpha) {
    if (alpha === 1) {
      return color;
    }
    // extract as int from 0 to 255
    var a = (color >> 24) & 0xFF;
    a *= alpha;
    // mask off the old alpha
    color &= 0xFFFFFF;
    return clamp(a) << 24 | color;

  }
}(Module)); // When this file is loaded in, the high level object is "Module";

// See https://stackoverflow.com/a/31090240
// This contraption keeps closure from minifying away the check
// if btoa is defined *and* prevents runtime "btoa" or "window" is not defined.
// Defined outside any scopes to make it available in all files.
var isNode = !(new Function("try {return this===window;}catch(e){ return false;}")());

function almostEqual(floata, floatb) {
  return Math.abs(floata - floatb) < 0.00001;
}