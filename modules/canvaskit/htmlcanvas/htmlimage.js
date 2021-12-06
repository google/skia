function HTMLImage(skImage) {
  this._skImage = skImage;
  // These are writable but have no effect, just like HTMLImageElement
  this.width = skImage.width();
  this.height = skImage.height();
  this.naturalWidth = this.width;
  this.naturalHeight = this.height;
  this.getSkImage = function() {
    return skImage;
  }
}