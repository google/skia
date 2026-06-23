//
// This file houses miscellaneous helper functions and constants.
//

var nullptr = 0; // emscripten doesn't like to take null as uintptr_t


function radiansToDegrees(rad) {
  return (rad / Math.PI) * 180;
}

function degreesToRadians(deg) {
  return (deg / 180) * Math.PI;
}

function almostEqual(floata, floatb) {
  return Math.abs(floata - floatb) < 0.00001;
}

// We try to find the natural media type (for <img> and <video>), display* for
// https://developer.mozilla.org/en-US/docs/Web/API/VideoFrame and then fall back to
// the height and width (to cover <canvas>, ImageBitmap or ImageData).
CanvasKit._getWidth = function(src) {
  return src['naturalWidth'] || src['videoWidth'] || src['displayWidth'] || src['width'];
};

CanvasKit._getHeight = function(src) {
  return src['naturalHeight'] || src['videoHeight'] || src['displayHeight'] || src['height'];
};
