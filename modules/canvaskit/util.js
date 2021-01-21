/**
 * This file houses miscellaneous helper functions and constants.
 */

var nullptr = 0; // emscripten doesn't like to take null as uintptr_t


function radiansToDegrees(rad) {
  return (rad / Math.PI) * 180;
}

function degreesToRadians(deg) {
  return (deg / 180) * Math.PI;
}

// See https://stackoverflow.com/a/31090240
// This contraption keeps closure from minifying away the check
// if btoa is defined *and* prevents runtime 'btoa' or 'window' is not defined.
// Defined outside any scopes to make it available in all files.
var isNode = !(new Function('try {return this===window;}catch(e){ return false;}')());

function almostEqual(floata, floatb) {
  return Math.abs(floata - floatb) < 0.00001;
}

function saveBytesToFile(bytes, fileName) {
  if (!isNode) {
    // https://stackoverflow.com/a/32094834
    var blob = new Blob([bytes], {type: 'application/octet-stream'});
    url = window.URL.createObjectURL(blob);
    var a = document.createElement('a');
    document.body.appendChild(a);
    a.href = url;
    a.download = fileName;
    a.click();
    // clean up after because FF might not download it synchronously
    setTimeout(function() {
      URL.revokeObjectURL(url);
      a.remove();
    }, 50);
  } else {
    var fs = require('fs');
    // https://stackoverflow.com/a/42006750
    // https://stackoverflow.com/a/47018122
    fs.writeFile(fileName, new Buffer(bytes), function(err) {
      if (err) throw err;
    });
  }
}