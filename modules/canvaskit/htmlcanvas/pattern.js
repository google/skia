function CanvasPattern(image, repetition) {
  this._shader = null;
  // image should be an Image returned from HTMLCanvas.decodeImage()
  this._image = image;
  this._transform = CanvasKit.Matrix.identity();

  if (repetition === '') {
    repetition = 'repeat';
  }
  switch(repetition) {
    case 'repeat-x':
      this._tileX = CanvasKit.TileMode.Repeat;
      // Skia's 'clamp' mode repeats the last row/column
      // which looks very very strange.
      // Decal mode does just transparent copying, which
      // is exactly what the spec wants.
      this._tileY = CanvasKit.TileMode.Decal;
      break;
    case 'repeat-y':
      this._tileX = CanvasKit.TileMode.Decal;
      this._tileY = CanvasKit.TileMode.Repeat;
      break;
    case 'repeat':
      this._tileX = CanvasKit.TileMode.Repeat;
      this._tileY = CanvasKit.TileMode.Repeat;
      break;
    case 'no-repeat':
      this._tileX = CanvasKit.TileMode.Decal;
      this._tileY = CanvasKit.TileMode.Decal;
      break;
    default:
      throw 'invalid repetition mode ' + repetition;
  }

  // Takes a DOMMatrix like object. e.g. the identity would be:
  // {a:1, b: 0, c: 0, d: 1, e: 0, f: 0}
  // @param {DOMMatrix} m
  this.setTransform = function(m) {
    var t = [m.a, m.c, m.e,
             m.b, m.d, m.f,
               0,   0,   1];
    if (allAreFinite(t)) {
      this._transform = t;
    }
  };

  this._copy = function() {
    var cp = new CanvasPattern();
    cp._tileX = this._tileX;
    cp._tileY = this._tileY;
    return cp;
  };

  this._dispose = function() {
    if (this._shader) {
      this._shader.delete();
      this._shader = null;
    }
  };

  this._getShader = function(currentTransform) {
    // Ignore currentTransform since it will be applied later
    this._dispose();
    // A shader with cubic sampling options is high quality.
    this._shader = this._image.makeShaderCubic(this._tileX, this._tileY, 1/3, 1/3, this._transform);
    return this._shader;
  }

}
