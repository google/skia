// Adds JS functions to augment the SkottieKit interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.

// SkottieKit.onRuntimeInitialized is called after the WASM library has loaded.
// Anything that modifies an exposed class (e.g. SkPath) should be set
// after onRuntimeInitialized, otherwise, it can happen outside of that scope.
SkottieKit.onRuntimeInitialized = function() {
  // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

  // Run through the JS files that are added at compile time.
  if (SkottieKit._extraInitializations) {
    SkottieKit._extraInitializations.forEach(function(init) {
      init();
    });
  }
}; // end SkottieKit.onRuntimeInitialized, that is, anything changing prototypes or dynamic.

// assets is a dictionary of named blobs: { key: ArrayBuffer, ... }
// The keys should be well-behaved strings - they're turned into null-terminated
// strings for the native side.
SkottieKit.MakeManagedAnimation = function(json, assets) {
  if (!SkottieKit.managed_skottie) {
    throw 'Not compiled with MakeManagedAnimation';
  }
  if (!assets) {
    return SkottieKit._MakeManagedAnimation(json, 0, nullptr, nullptr, nullptr);
  }
  var assetNamePtrs = [];
  var assetDataPtrs = [];
  var assetSizes    = [];

  var assetKeys = Object.keys(assets || {});
  for (var i = 0; i < assetKeys.length; i++) {
    var key = assetKeys[i];
    var buffer = assets[key];
    var data = new Uint8Array(buffer);

    var iptr = SkottieKit._malloc(data.byteLength);
    SkottieKit.HEAPU8.set(data, iptr);
    assetDataPtrs.push(iptr);
    assetSizes.push(data.byteLength);

    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(key) + 1;
    var strPtr = SkottieKit._malloc(strLen);

    stringToUTF8(key, strPtr, strLen);
    assetNamePtrs.push(strPtr);
  }

  // Not entirely sure if it matters, but the uintptr_t are 32 bits
  // we want to copy our array of uintptr_t into the right size memory.
  var namesPtr      = copy1dArray(assetNamePtrs, SkottieKit.HEAPU32);
  var assetsPtr     = copy1dArray(assetDataPtrs, SkottieKit.HEAPU32);
  var assetSizesPtr = copy1dArray(assetSizes,    SkottieKit.HEAPU32);

  var anim = SkottieKit._MakeManagedAnimation(json, assetKeys.length, namesPtr,
                                              assetsPtr, assetSizesPtr);

  // The C++ code has made copies of the asset and string data, so free our copies.
  SkottieKit._free(namesPtr);
  SkottieKit._free(assetsPtr);
  SkottieKit._free(assetSizesPtr);

  return anim;
};
