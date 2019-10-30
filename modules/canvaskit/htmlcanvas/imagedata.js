function ImageData(arr, width, height) {
  if (!width || height === 0) {
    throw 'invalid dimensions, width and height must be non-zero';
  }
  if (arr.length % 4) {
    throw 'arr must be a multiple of 4';
  }
  height = height || arr.length/(4*width);

  Object.defineProperty(this, 'data', {
    value: arr,
    writable: false
  });
  Object.defineProperty(this, 'height', {
    value: height,
    writable: false
  });
  Object.defineProperty(this, 'width', {
    value: width,
    writable: false
  });
}

CanvasKit.ImageData = function() {
  if (arguments.length === 2) {
    var width = arguments[0];
    var height = arguments[1];
    var byteLength = 4 * width * height;
    return new ImageData(new Uint8ClampedArray(byteLength),
                         width, height);
  } else if (arguments.length === 3) {
    var arr = arguments[0];
    if (arr.prototype.constructor !== Uint8ClampedArray ) {
      throw 'bytes must be given as a Uint8ClampedArray';
    }
    var width = arguments[1];
    var height = arguments[2];
    if (arr % 4) {
      throw 'bytes must be given in a multiple of 4';
    }
    if (arr % width) {
      throw 'bytes must divide evenly by width';
    }
    if (height && (height !== (arr / (width * 4)))) {
      throw 'invalid height given';
    }
    height = arr / (width * 4);
    return new ImageData(arr, width, height);
  } else {
    throw 'invalid number of arguments - takes 2 or 3, saw ' + arguments.length;
  }
}