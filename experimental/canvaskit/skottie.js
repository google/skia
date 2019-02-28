// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the Skottie builds of canvaskit.


CanvasKit.MakeManagedAnimation = function(json, imgs, fonts) {
  if (!CanvasKit._MakeManagedAnimation) {
    throw 'Not compiled with MakeManagedAnimation';
  }
  if (!imgs && !fonts) {
    return CanvasKit._MakeManagedAnimation(json, 0, nullptr, nullptr, nullptr,
                                                 0, nullptr, nullptr, nullptr);
  }
  var imgNamePtrs = [];
  var imgDataPtrs = [];
  var imgSizes    = [];

  var imgKeys = Object.keys(imgs || {});
  for (var i = 0; i < imgKeys.length; i++) {
    var key = imgKeys[i];
    var buffer = imgs[key];
    var data = new Uint8Array(buffer);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    imgDataPtrs.push(iptr);
    imgSizes.push(data.byteLength);

    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(key) + 1;
    var strPtr = CanvasKit._malloc(strLen);

    stringToUTF8(key, strPtr, strLen);
    imgNamePtrs.push(strPtr);
  }

  var fontNamePtrs = [];
  var fontDataPtrs = [];
  var fontSizes    = [];

  var fontKeys = Object.keys(fonts || {});
  for (var i = 0; i < fontKeys.length; i++) {
    var key = fontKeys[i];
    var buffer = fonts[key];
    var data = new Uint8Array(buffer);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    fontDataPtrs.push(iptr);
    fontSizes.push(data.byteLength);

    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(key) + 1;
    var strPtr = CanvasKit._malloc(strLen);

    stringToUTF8(key, strPtr, strLen);
    fontNamePtrs.push(strPtr);
  }

  // Not entirely sure if it matters, but the uintptr_t are 32 bits
  // we want to copy our array of uintptr_t into the right size memory.
  var iNamesPtr    = copy1dArray(imgNamePtrs, CanvasKit.HEAPU32);
  var imgsPtr      = copy1dArray(imgDataPtrs, CanvasKit.HEAPU32);
  var imgSizesPtr  = copy1dArray(imgSizes,    CanvasKit.HEAPU32);

  var fNamesPtr    = copy1dArray(fontNamePtrs, CanvasKit.HEAPU32);
  var fontsPtr     = copy1dArray(fontDataPtrs, CanvasKit.HEAPU32);
  var fontSizesPtr = copy1dArray(fontSizes,    CanvasKit.HEAPU32);

  var anim = CanvasKit._MakeManagedAnimation(json, imgKeys.length, iNamesPtr, imgsPtr, imgSizesPtr,
                                             fontKeys.length, fNamesPtr, fontsPtr, fontSizesPtr);

  // We leave the image data arrays and string data live and assume
  // it is now owned by the C++ code
  CanvasKit._free(iNamesPtr);
  CanvasKit._free(imgsPtr);
  CanvasKit._free(imgSizesPtr);

  CanvasKit._free(fNamesPtr);
  CanvasKit._free(fontsPtr);
  CanvasKit._free(fontSizesPtr);
  return anim;
};
