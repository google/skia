CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {
  // The serialized format of an SkPicture (informally called an "skp"), is not something
  // that clients should ever rely on. It is useful when filing bug reports, but that's
  // about it. The format may change at anytime and no promises are made for backwards
  // or forward compatibility.
  CanvasKit.SkPicture.prototype.DEBUGONLY_saveAsFile = function(skpName) {
    var data = this.DEBUGONLY_serialize();
    if (!data) {
      SkDebug('Could not serialize to skpicture.');
      return;
    }
    var bytes = CanvasKit.getSkDataBytes(data);
    saveBytesToFile(bytes, skpName);
    data.delete();
  }
});