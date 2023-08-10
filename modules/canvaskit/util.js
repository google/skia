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
