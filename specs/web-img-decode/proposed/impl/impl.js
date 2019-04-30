(function(window) {

let CanvasKit = null;

window.loadPolyfill = () => {
  return CanvasKitInit({
    locateFile: (file) => 'https://unpkg.com/canvaskit-wasm@0.5.1/bin/'+file,
  }).ready().then((CK) => {
    CanvasKit = CK;
  });
}

window.createImageData = (src, options) => {
  console.log("Loading src", src);

  const skImg = CanvasKit.MakeImageFromEncoded(src);
  // we know width and height
  const imageInfo = {
    width: skImg.width,
    height: skImg.height,
    alphaType: options.premul ? CanvasKit.AlphaType.Premul : CanvasKit.AlphaType.Unpremul,
  }
  switch (src.colorType) {
    case "float32":
      imageInfo.colorType = CanvasKit.ColorType.RGBA_F32; // Correct?
      break;

    case "uint16":
      imageInfo.colorType = CanvasKit.ColorType.RGBA_1010102; // Correct?
      break;

    case "uint8":
    default:
      imageInfo.colorType = CanvasKit.ColorType.RGBA_8888;
      break;
  }

  const pixels = skImg.readPixels(imageInfo, 0, 0);
  const output = new Uint8ClampedArray(pixels.buffer);
  const ret = new ImageData(output, skImg.width, skImg.height);
  skImg.delete();

  return ret;
}


})(window);