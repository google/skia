CanvasKit.MakeCanvas = function(width, height) {
  // TODO(kjlubick) do fonts the "correct" way
  CanvasKit.initFonts();
  var surf = CanvasKit.MakeSurface(width, height);
  if (surf) {
    return new HTMLCanvas(surf);
  }
  return null;
}

function HTMLCanvas(skSurface) {
  this._surface = skSurface;
  this._context = new CanvasRenderingContext2D(skSurface.getCanvas());
  this._imgs = [];

  // Data is either an ArrayBuffer, a TypedArray, or a Node Buffer
  this.decodeImage = function(data) {
    var img = CanvasKit.MakeImageFromEncoded(data);
    if (!img) {
      throw 'Invalid input';
    }
    this._imgs.push(img);
    return img;
  }

  // A normal <canvas> requires that clients call getContext
  this.getContext = function(type) {
    if (type === '2d') {
      return this._context;
    }
    return null;
  }

  this.toDataURL = function(codec, quality) {
    // TODO(kjlubick): maybe support other codecs (webp?)
    // For now, just to png and jpeg
    this._surface.flush();

    var img = this._surface.makeImageSnapshot();
    if (!img) {
      SkDebug('no snapshot');
      return;
    }
    var codec = codec || 'image/png';
    var format = CanvasKit.ImageFormat.PNG;
    if (codec === 'image/jpeg') {
      format = CanvasKit.ImageFormat.JPEG;
    }
    var quality = quality || 0.92;
    var skimg = img.encodeToData(format, quality);
    if (!skimg) {
      SkDebug('encoding failure');
      return
    }
    var imgBytes = CanvasKit.getSkDataBytes(skimg);
    return 'data:' + codec + ';base64,' + toBase64String(imgBytes);
  }

  this.dispose = function() {
    this._context._dispose();
    this._imgs.forEach(function(i) {
      i.delete();
    });
    this._surface.dispose();
  }
}