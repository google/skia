CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
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