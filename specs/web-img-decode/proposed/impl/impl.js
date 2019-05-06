(function(window) {

let CanvasKit = null;

window.loadPolyfill = () => {
  return CanvasKitInit({
    locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.6.0/bin/'+file,
  }).ready().then((CK) => {
    CanvasKit = CK;
  });
}

window.createImageData = (src, options) => {
  const skImg = CanvasKit.MakeImageFromEncoded(src);
  // we know width and height
  const imageInfo = {
    width: options.resizeWidth || skImg.width(),
    height: options.resizeHeight || skImg.height(),
    alphaType: options.premul ? CanvasKit.AlphaType.Premul : CanvasKit.AlphaType.Unpremul,
  }
  switch (options.colorType) {
    case "float32":
      imageInfo.colorType = CanvasKit.ColorType.RGBA_F32;
      break;

    case "uint8":
    default:
      imageInfo.colorType = CanvasKit.ColorType.RGBA_8888;
      break;
  }

  const pixels = skImg.readPixels(imageInfo, 0, 0);
  let output;
  // ImageData at the moment only supports Uint8, so we have to convert our numbers to that
  switch (options.colorType) {
    case "float32":
      // This will make an extra copy, which is a limitation of the native Browser's
      // ImageData support.
      output = new Uint8ClampedArray(pixels);
      break;

    case "uint8":
    default:
      // We can cast w/o another copy
      output = new Uint8ClampedArray(pixels.buffer);
      break;
  }


  const ret = new ImageData(output, imageInfo.width, imageInfo.height);
  skImg.delete();

  return ret;
}


})(window);
