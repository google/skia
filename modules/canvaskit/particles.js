// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the Particle builds of canvaskit.

// assets is a dictionary of named blobs: { key: ArrayBuffer, ... }
// The keys should be well-behaved strings - they're turned into null-terminated
// strings for the native side.
CanvasKit.MakeParticles = function(json, assets) {
  if (!CanvasKit._MakeParticles) {
    throw 'Not compiled with MakeParticles';
  }
  if (!assets) {
    return CanvasKit._MakeParticles(json, 0, nullptr, nullptr, nullptr);
  }
  var assetNamePtrs = [];
  var assetDataPtrs = [];
  var assetSizes    = [];

  var assetKeys = Object.keys(assets || {});
  for (var i = 0; i < assetKeys.length; i++) {
    var key = assetKeys[i];
    var buffer = assets[key];
    var data = new Uint8Array(buffer);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    assetDataPtrs.push(iptr);
    assetSizes.push(data.byteLength);

    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(key) + 1;
    var strPtr = CanvasKit._malloc(strLen);

    stringToUTF8(key, strPtr, strLen);
    assetNamePtrs.push(strPtr);
  }

  // Not entirely sure if it matters, but the uintptr_t are 32 bits
  // we want to copy our array of uintptr_t into the right size memory.
  var namesPtr      = copy1dArray(assetNamePtrs, CanvasKit.HEAPU32);
  var assetsPtr     = copy1dArray(assetDataPtrs, CanvasKit.HEAPU32);
  var assetSizesPtr = copy1dArray(assetSizes,    CanvasKit.HEAPU32);

  var particles = CanvasKit._MakeParticles(json, assetKeys.length,
                                           namesPtr, assetsPtr, assetSizesPtr);

  // The C++ code has made copies of the asset and string data, so free our copies.
  CanvasKit._free(namesPtr);
  CanvasKit._free(assetsPtr);
  CanvasKit._free(assetSizesPtr);

  return particles;
};
