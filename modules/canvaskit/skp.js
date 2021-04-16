CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  // data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
  CanvasKit.MakePicture = function(data) {
    data = new Uint8Array(data);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    // The skp takes ownership of the malloc'd data.
    var pic = CanvasKit._MakePicture(iptr, data.byteLength);
    if (!pic) {
      Debug('Could not decode picture');
      return null;
    }
    return pic;
  };
});
