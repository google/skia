CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {

  // str can be either a text string or a ShapedText object
  CanvasKit.SkCanvas.prototype.drawText = function(str, x, y, paint, font) {
    if (typeof str === 'string') {
      // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
      // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
      var strLen = lengthBytesUTF8(str);
      // Add 1 for null terminator, which we need when copying/converting, but can ignore
      // when we call into Skia.
      var strPtr = CanvasKit._malloc(strLen + 1);
      stringToUTF8(str, strPtr, strLen + 1);
      this._drawSimpleText(strPtr, strLen, x, y, font, paint);
    } else {
      this._drawShapedText(str, x, y, paint);
    }
  }

  // Returns an array of the widths of the glyphs in this string.
  CanvasKit.SkFont.prototype.getWidths = function(str) {
    // add 1 for null terminator
    var codePoints = str.length + 1;
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strBytes = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strBytes);
    stringToUTF8(str, strPtr, strBytes);

    var bytesPerFloat = 4;
    // allocate widths == numCodePoints
    var widthPtr = CanvasKit._malloc(codePoints * bytesPerFloat);
    if (!this._getWidths(strPtr, strBytes, codePoints, widthPtr)) {
      SkDebug('Could not compute widths');
      CanvasKit._free(strPtr);
      CanvasKit._free(widthPtr);
      return null;
    }
    // reminder, this shouldn't copy the data, just is a nice way to
    // wrap 4 bytes together into a float.
    var widths = new Float32Array(CanvasKit.HEAPU8.buffer, widthPtr, codePoints);
    // This copies the data so we can free the CanvasKit memory
    var retVal = Array.from(widths);
    CanvasKit._free(strPtr);
    CanvasKit._free(widthPtr);
    return retVal;
  }

  // arguments should all be arrayBuffers or be an array of arrayBuffers.
  CanvasKit.SkFontMgr.FromData = function() {
    if (!arguments.length) {
      SkDebug('Could not make SkFontMgr from no font sources');
      return null;
    }
    var fonts = arguments;
    if (fonts.length === 1 && Array.isArray(fonts[0])) {
      fonts = arguments[0];
    }
    if (!fonts.length) {
      SkDebug('Could not make SkFontMgr from no font sources');
      return null;
    }
    var dPtrs = [];
    var sizes = [];
    for (var i = 0; i < fonts.length; i++) {
      var data = new Uint8Array(fonts[i]);
      var dptr = copy1dArray(data, CanvasKit.HEAPU8);
      dPtrs.push(dptr);
      sizes.push(data.byteLength);
    }
    // Pointers are 32 bit unsigned ints
    var datasPtr = copy1dArray(dPtrs, CanvasKit.HEAPU32);
    var sizesPtr = copy1dArray(sizes, CanvasKit.HEAPU32);
    var fm = CanvasKit.SkFontMgr._fromData(datasPtr, sizesPtr, fonts.length);
    // The SkFontMgr has taken ownership of the bytes we allocated in the for loop.
    CanvasKit._free(datasPtr);
    CanvasKit._free(sizesPtr);
    return fm;
  }

  // fontData should be an arrayBuffer
  CanvasKit.SkFontMgr.prototype.MakeTypefaceFromData = function(fontData) {
    var data = new Uint8Array(fontData);

    var fptr = copy1dArray(data, CanvasKit.HEAPU8);
    var font = this._makeTypefaceFromData(fptr, data.byteLength);
    if (!font) {
      SkDebug('Could not decode font data');
      // We do not need to free the data since the C++ will do that for us
      // when the font is deleted (or fails to decode);
      return null;
    }
    return font;
  }

  CanvasKit.SkTextBlob.MakeOnPath = function(str, path, font, initialOffset) {
    if (!str || !str.length) {
      SkDebug('ignoring 0 length string');
      return;
    }
    if (!path || !path.countPoints()) {
      SkDebug('ignoring empty path');
      return;
    }
    if (path.countPoints() === 1) {
      SkDebug('path has 1 point, returning normal textblob');
      return this.MakeFromText(str, font);
    }

    if (!initialOffset) {
      initialOffset = 0;
    }

    var widths = font.getWidths(str);

    var rsx = new CanvasKit.RSXFormBuilder();
    var meas = new CanvasKit.SkPathMeasure(path, false, 1);
    var dist = initialOffset;
    for (var i = 0; i < str.length; i++) {
      var width = widths[i];
      dist += width/2;
      if (dist > meas.getLength()) {
        // jump to next contour
        if (!meas.nextContour()) {
          // We have come to the end of the path - terminate the string
          // right here.
          str = str.substring(0, i);
          break;
        }
        dist = width/2;
      }

      // Gives us the (x, y) coordinates as well as the cos/sin of the tangent
      // line at that position.
      var xycs = meas.getPosTan(dist);
      var cx = xycs[0];
      var cy = xycs[1];
      var cosT = xycs[2];
      var sinT = xycs[3];

      var adjustedX = cx - (width/2 * cosT);
      var adjustedY = cy - (width/2 * sinT);

      rsx.push(cosT, sinT, adjustedX, adjustedY);
      dist += width/2;
    }
    var retVal = this.MakeFromRSXform(str, rsx, font);
    rsx.delete();
    meas.delete();
    return retVal;
  }

  CanvasKit.SkTextBlob.MakeFromRSXform = function(str, rsxBuilder, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);
    var rptr = rsxBuilder.build();

    var blob = CanvasKit.SkTextBlob._MakeFromRSXform(strPtr, strLen - 1, rptr, font);
    if (!blob) {
      SkDebug('Could not make textblob from string "' + str + '"');
      return null;
    }

    var origDelete = blob.delete.bind(blob);
    blob.delete = function() {
      CanvasKit._free(strPtr);
      origDelete();
    }
    return blob;
  }

  CanvasKit.SkTextBlob.MakeFromText = function(str, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);

    var blob = CanvasKit.SkTextBlob._MakeFromText(strPtr, strLen - 1, font);
    if (!blob) {
      SkDebug('Could not make textblob from string "' + str + '"');
      return null;
    }

    var origDelete = blob.delete.bind(blob);
    blob.delete = function() {
      CanvasKit._free(strPtr);
      origDelete();
    }
    return blob;
  }
});
