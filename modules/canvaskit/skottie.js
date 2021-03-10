// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the Skottie builds of canvaskit.

// assets is a dictionary of named blobs: { key: ArrayBuffer, ... }
// The keys should be well-behaved strings - they're turned into null-terminated
// strings for the native side.

// prop_filter_prefix is an optional string acting as a name filter for selecting
// "interesting" Lottie properties (surfaced in the embedded player controls)

// soundMap is an optional object that maps string names to AudioPlayers
// AudioPlayers manage a single audio layer with a seek function

// logger is an optional logging object, expected to provide two functions:
//   - onError(err_str, json_node_str)
//   - onWarning(wrn_str, json_node_str)
CanvasKit.MakeManagedAnimation = function(json, assets, prop_filter_prefix, soundMap, logger) {
  if (!CanvasKit._MakeManagedAnimation) {
    throw 'Not compiled with MakeManagedAnimation';
  }
  if (!prop_filter_prefix) {
    prop_filter_prefix = '';
  }
  if (!assets) {
    return CanvasKit._MakeManagedAnimation(json, 0, nullptr, nullptr, nullptr, prop_filter_prefix,
                                           soundMap, logger);
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
  var namesPtr      = copy1dArray(assetNamePtrs, "HEAPU32");
  var assetsPtr     = copy1dArray(assetDataPtrs, "HEAPU32");
  var assetSizesPtr = copy1dArray(assetSizes,    "HEAPU32");

  var anim = CanvasKit._MakeManagedAnimation(json, assetKeys.length, namesPtr,
                                             assetsPtr, assetSizesPtr, prop_filter_prefix,
                                             soundMap, logger);

  // The C++ code has made copies of the asset and string data, so free our copies.
  CanvasKit._free(namesPtr);
  CanvasKit._free(assetsPtr);
  CanvasKit._free(assetSizesPtr);

  return anim;
};

(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

  CanvasKit.Animation.prototype.render = function(canvas, dstRect) {
    copyRectToWasm(dstRect, _scratchFourFloatsAPtr);
    this._render(canvas, _scratchFourFloatsAPtr);
  };

  CanvasKit.Animation.prototype.size = function(optSize) {
    // This will copy 2 floats into a space for 4 floats
    this._size(_scratchFourFloatsAPtr);
    var ta = _scratchFourFloatsA['toTypedArray']();
    if (optSize) {
      // We cannot call optSize.set() because it is an error to call .set() with
      // a source bigger than the destination.
      optSize[0] = ta[0];
      optSize[1] = ta[1];
      return optSize;
    }
    // Be sure to return a copy of just the first 2 values.
    return ta.slice(0, 2);
  };

  if (CanvasKit.ManagedAnimation) {
    CanvasKit.ManagedAnimation.prototype.render = function(canvas, dstRect) {
    copyRectToWasm(dstRect, _scratchFourFloatsAPtr);
    this._render(canvas, _scratchFourFloatsAPtr);
    };

    CanvasKit.ManagedAnimation.prototype.seek = function(t, optDamageRect) {
      this._seek(t, _scratchFourFloatsAPtr);
      var ta = _scratchFourFloatsA['toTypedArray']();
      if (optDamageRect) {
        optDamageRect.set(ta);
        return optDamageRect;
      }
      return ta.slice();
    };

    CanvasKit.ManagedAnimation.prototype.seekFrame = function(frame, optDamageRect) {
      this._seekFrame(frame, _scratchFourFloatsAPtr);
      var ta = _scratchFourFloatsA['toTypedArray']();
      if (optDamageRect) {
        optDamageRect.set(ta);
        return optDamageRect;
      }
      return ta.slice();
    };

    CanvasKit.ManagedAnimation.prototype.setColor = function(key, color) {
      var cPtr = copyColorToWasm(color);
      return this._setColor(key, cPtr);
    };

    CanvasKit.ManagedAnimation.prototype.size = function(optSize) {
      // This will copy 2 floats into a space for 4 floats
      this._size(_scratchFourFloatsAPtr);
      var ta = _scratchFourFloatsA['toTypedArray']();
      if (optSize) {
        // We cannot call optSize.set() because it is an error to call .set() with
        // a source bigger than the destination.
        optSize[0] = ta[0];
        optSize[1] = ta[1];
        return optSize;
      }
      // Be sure to return a copy of just the first 2 values.
      return ta.slice(0, 2);
    };
  }


});
}(Module)); // When this file is loaded in, the high level object is "Module";
