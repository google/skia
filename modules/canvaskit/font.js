CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
CanvasKit._extraInitializations.push(function() {

  CanvasKit.Canvas.prototype.drawText = function(str, x, y, paint, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    var strLen = lengthBytesUTF8(str);
    // Add 1 for null terminator, which we need when copying/converting, but can ignore
    // when we call into Skia.
    var strPtr = CanvasKit._malloc(strLen + 1);
    stringToUTF8(str, strPtr, strLen + 1);
    this._drawSimpleText(strPtr, strLen, x, y, font, paint);
    CanvasKit._free(strPtr);
  };

  CanvasKit.Canvas.prototype.drawGlyphs = function(glyphs, positions, x, y, font, paint) {
    if (!(glyphs.length*2 <= positions.length)) {
        throw 'Not enough positions for the array of gyphs';
    }
    CanvasKit.setCurrentContext(this._context);
    const glyphs_ptr    = copy1dArray(glyphs, 'HEAPU16');
    const positions_ptr = copy1dArray(positions, 'HEAPF32');

    this._drawGlyphs(glyphs.length, glyphs_ptr, positions_ptr, x, y, font, paint);

    freeArraysThatAreNotMallocedByUsers(positions_ptr, positions);
    freeArraysThatAreNotMallocedByUsers(glyphs_ptr,    glyphs);
  };

  // Glyphs should be a Uint16Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  // The return value will be a Float32Array that is 4 times as long as the input array. For each
  // glyph, there will be 4 floats for left, top, right, bottom (relative to 0, 0) for that glyph.
  CanvasKit.Font.prototype.getGlyphBounds = function(glyphs, paint, optionalOutputArray) {
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerRect = 4 * 4;
    var rectPtr = CanvasKit._malloc(glyphs.length * bytesPerRect);
    this._getGlyphWidthBounds(glyphPtr, glyphs.length, nullptr, rectPtr, paint || null);

    var rects = new Float32Array(CanvasKit.HEAPU8.buffer, rectPtr, glyphs.length * 4);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (optionalOutputArray) {
      optionalOutputArray.set(rects);
      CanvasKit._free(rectPtr);
      return optionalOutputArray;
    }
    var rv = Float32Array.from(rects);
    CanvasKit._free(rectPtr);
    return rv;
  };

  CanvasKit.Font.prototype.getGlyphIDs = function(str, numGlyphIDs, optionalOutputArray) {
    if (!numGlyphIDs) {
      numGlyphIDs = str.length;
    }
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strBytes = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strBytes);
    stringToUTF8(str, strPtr, strBytes); // This includes the null terminator

    var bytesPerGlyph = 2;
    var glyphPtr = CanvasKit._malloc(numGlyphIDs * bytesPerGlyph);
    // We don't need to compute the id for the null terminator, so subtract 1.
    var actualIDs = this._getGlyphIDs(strPtr, strBytes - 1, numGlyphIDs, glyphPtr);
    CanvasKit._free(strPtr);
    if (actualIDs < 0) {
      Debug('Could not get glyphIDs');
      CanvasKit._free(glyphPtr);
      return null;
    }
    var glyphs = new Uint16Array(CanvasKit.HEAPU8.buffer, glyphPtr, actualIDs);
    if (optionalOutputArray) {
      optionalOutputArray.set(glyphs);
      CanvasKit._free(glyphPtr);
      return optionalOutputArray;
    }
    var rv = Uint16Array.from(glyphs);
    CanvasKit._free(glyphPtr);
    return rv;
  };

  CanvasKit.Font.prototype.getGlyphIntercepts = function(glyphs, positions, top, bottom) {
    var gPtr = copy1dArray(glyphs, 'HEAPU16');
    var pPtr = copy1dArray(positions, 'HEAPF32');
    return this._getGlyphIntercepts(gPtr, glyphs.length, !wasMalloced(glyphs),
                                    pPtr, positions.length, !wasMalloced(positions),
                                    top, bottom);
  };

  // Glyphs should be a Uint16Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  // The return value will be a Float32Array that has one width per input glyph.
  CanvasKit.Font.prototype.getGlyphWidths = function(glyphs, paint, optionalOutputArray) {
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerWidth = 4;
    var widthPtr = CanvasKit._malloc(glyphs.length * bytesPerWidth);
    this._getGlyphWidthBounds(glyphPtr, glyphs.length, widthPtr, nullptr, paint || null);

    var widths = new Float32Array(CanvasKit.HEAPU8.buffer, widthPtr, glyphs.length);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (optionalOutputArray) {
      optionalOutputArray.set(widths);
      CanvasKit._free(widthPtr);
      return optionalOutputArray;
    }
    var rv = Float32Array.from(widths);
    CanvasKit._free(widthPtr);
    return rv;
  };

  // arguments should all be arrayBuffers or be an array of arrayBuffers.
  CanvasKit.FontMgr.FromData = function() {
    if (!arguments.length) {
      Debug('Could not make FontMgr from no font sources');
      return null;
    }
    var fonts = arguments;
    if (fonts.length === 1 && Array.isArray(fonts[0])) {
      fonts = arguments[0];
    }
    if (!fonts.length) {
      Debug('Could not make FontMgr from no font sources');
      return null;
    }
    var dPtrs = [];
    var sizes = [];
    for (var i = 0; i < fonts.length; i++) {
      var data = new Uint8Array(fonts[i]);
      var dptr = copy1dArray(data, 'HEAPU8');
      dPtrs.push(dptr);
      sizes.push(data.byteLength);
    }
    // Pointers are 32 bit unsigned ints
    var datasPtr = copy1dArray(dPtrs, 'HEAPU32');
    var sizesPtr = copy1dArray(sizes, 'HEAPU32');
    var fm = CanvasKit.FontMgr._fromData(datasPtr, sizesPtr, fonts.length);
    // The FontMgr has taken ownership of the bytes we allocated in the for loop.
    CanvasKit._free(datasPtr);
    CanvasKit._free(sizesPtr);
    return fm;
  };

  CanvasKit.Typeface.MakeTypefaceFromData = function(fontData) {
    var data = new Uint8Array(fontData);

    var fptr = copy1dArray(data, 'HEAPU8');
    var font = CanvasKit.Typeface._MakeTypefaceFromData(fptr, data.byteLength);
    if (!font) {
      Debug('Could not decode font data');
      // We do not need to free the data since the C++ will do that for us
      // when the font is deleted (or fails to decode);
      return null;
    }
    return font;
  };

  // TODO(kjlubick) remove this after clients have migrated.
  CanvasKit.Typeface["MakeFreeTypeFaceFromData"] = CanvasKit.Typeface.MakeTypefaceFromData;

  CanvasKit.Typeface.prototype.getGlyphIDs = function(str, numGlyphIDs, optionalOutputArray) {
    if (!numGlyphIDs) {
      numGlyphIDs = str.length;
    }
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strBytes = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strBytes);
    stringToUTF8(str, strPtr, strBytes); // This includes the null terminator

    var bytesPerGlyph = 2;
    var glyphPtr = CanvasKit._malloc(numGlyphIDs * bytesPerGlyph);
    // We don't need to compute the id for the null terminator, so subtract 1.
    var actualIDs = this._getGlyphIDs(strPtr, strBytes - 1, numGlyphIDs, glyphPtr);
    CanvasKit._free(strPtr);
    if (actualIDs < 0) {
      Debug('Could not get glyphIDs');
      CanvasKit._free(glyphPtr);
      return null;
    }
    var glyphs = new Uint16Array(CanvasKit.HEAPU8.buffer, glyphPtr, actualIDs);
    if (optionalOutputArray) {
      optionalOutputArray.set(glyphs);
      CanvasKit._free(glyphPtr);
      return optionalOutputArray;
    }
    var rv = Uint16Array.from(glyphs);
    CanvasKit._free(glyphPtr);
    return rv;
  };

  CanvasKit.TextBlob.MakeOnPath = function(str, path, font, initialOffset) {
    if (!str || !str.length) {
      Debug('ignoring 0 length string');
      return;
    }
    if (!path || !path.countPoints()) {
      Debug('ignoring empty path');
      return;
    }
    if (path.countPoints() === 1) {
      Debug('path has 1 point, returning normal textblob');
      return this.MakeFromText(str, font);
    }

    if (!initialOffset) {
      initialOffset = 0;
    }

    var ids = font.getGlyphIDs(str);
    var widths = font.getGlyphWidths(ids);

    var rsx = [];
    var meas = new CanvasKit.ContourMeasureIter(path, false, 1);
    var cont = meas.next();
    var dist = initialOffset;
    var xycs = new Float32Array(4);
    for (var i = 0; i < str.length && cont; i++) {
      var width = widths[i];
      dist += width/2;
      if (dist > cont.length()) {
        // jump to next contour
        cont.delete();
        cont = meas.next();
        if (!cont) {
          // We have come to the end of the path - terminate the string
          // right here.
          str = str.substring(0, i);
          break;
        }
        dist = width/2;
      }

      // Gives us the (x, y) coordinates as well as the cos/sin of the tangent
      // line at that position.
      cont.getPosTan(dist, xycs);
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
    cont && cont.delete();
    meas.delete();
    return retVal;
  };

  CanvasKit.TextBlob.MakeFromRSXform = function(str, rsxForms, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);

    var rPtr = copy1dArray(rsxForms, 'HEAPF32');

    var blob = CanvasKit.TextBlob._MakeFromRSXform(strPtr, strLen - 1, rPtr, font);
    CanvasKit._free(strPtr);
    if (!blob) {
      Debug('Could not make textblob from string "' + str + '"');
      return null;
    }
    return blob;
  };

  // Glyphs should be a Uint32Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  CanvasKit.TextBlob.MakeFromRSXformGlyphs = function(glyphs, rsxForms, font) {
    // Currently on the C++ side, glyph ids are 16bit, but there is an effort to change that.
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerGlyph = 2;

    var rPtr = copy1dArray(rsxForms, 'HEAPF32');

    var blob = CanvasKit.TextBlob._MakeFromRSXformGlyphs(glyphPtr, glyphs.length * bytesPerGlyph, rPtr, font);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (!blob) {
      Debug('Could not make textblob from glyphs "' + glyphs + '"');
      return null;
    }
    return blob;
  };

  // Glyphs should be a Uint32Array of glyph ids, e.g. provided by Font.getGlyphIDs.
  // If using a Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs() to get the right type.
  CanvasKit.TextBlob.MakeFromGlyphs = function(glyphs, font) {
    // Currently on the C++ side, glyph ids are 16bit, but there is an effort to change that.
    var glyphPtr = copy1dArray(glyphs, 'HEAPU16');
    var bytesPerGlyph = 2;
    var blob = CanvasKit.TextBlob._MakeFromGlyphs(glyphPtr, glyphs.length * bytesPerGlyph, font);
    freeArraysThatAreNotMallocedByUsers(glyphPtr, glyphs);
    if (!blob) {
      Debug('Could not make textblob from glyphs "' + glyphs + '"');
      return null;
    }
    return blob;
  };

  CanvasKit.TextBlob.MakeFromText = function(str, font) {
    // lengthBytesUTF8 and stringToUTF8Array are defined in the emscripten
    // JS.  See https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#stringToUTF8
    // Add 1 for null terminator
    var strLen = lengthBytesUTF8(str) + 1;
    var strPtr = CanvasKit._malloc(strLen);
    // Add 1 for the null terminator.
    stringToUTF8(str, strPtr, strLen);

    var blob = CanvasKit.TextBlob._MakeFromText(strPtr, strLen - 1, font);
    CanvasKit._free(strPtr);
    if (!blob) {
      Debug('Could not make textblob from string "' + str + '"');
      return null;
    }
    return blob;
  };

  // A helper to return the right type for GlyphIDs stored internally. When that changes, this
  // will also be changed, which will help avoid future breakages.
  CanvasKit.MallocGlyphIDs = function(numGlyphIDs) {
    return CanvasKit.Malloc(Uint16Array, numGlyphIDs);
  }
});
