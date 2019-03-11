
// General purpose utility functions go in this file.


function allAreFinite(args) {
  for (var i = 0; i < args.length; i++) {
    if (args[i] !== undefined && !Number.isFinite(args[i])) {
      return false;
    }
  }
  return true;
}

function toBase64String(bytes) {
  if (isNode) {
    return Buffer.from(bytes).toString('base64');
  } else {
    // From https://stackoverflow.com/a/25644409
    // because the naive solution of
    //     btoa(String.fromCharCode.apply(null, bytes));
    // would occasionally throw "Maximum call stack size exceeded"
    var CHUNK_SIZE = 0x8000; //arbitrary number
    var index = 0;
    var length = bytes.length;
    var result = '';
    var slice;
    while (index < length) {
      slice = bytes.slice(index, Math.min(index + CHUNK_SIZE, length));
      result += String.fromCharCode.apply(null, slice);
      index += CHUNK_SIZE;
    }
    return btoa(result);
  }
}

