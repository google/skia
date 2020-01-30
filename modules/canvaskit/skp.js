CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  // data is a TypedArray or ArrayBuffer e.g. from fetch().then(resp.arrayBuffer())
  CanvasKit.MakeSkPicture = function(data) {
    data = new Uint8Array(data);

    var iptr = CanvasKit._malloc(data.byteLength);
    CanvasKit.HEAPU8.set(data, iptr);
    var pic = CanvasKit._MakeSkPicture(iptr, data.byteLength);
    if (!pic) {
      SkDebug('Could not decode picture');
      return null;
    }
    return pic;
  }

  // The serialized format of an SkPicture (informally called an "skp"), is not something
  // that clients should ever rely on. The format may change at anytime and no promises
  // are made for backwards or forward compatibility.
  CanvasKit.SkPicture.prototype.saveAsFile = function(skpName) {
    var data = this.serialize();
    if (!data) {
      SkDebug('Could not serialize to skpicture.');
      return;
    }
    var bytes = CanvasKit.getSkDataBytes(data);
    saveBytesToFile(bytes, skpName);
    data.delete();
  }
});