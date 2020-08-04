(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

    CanvasKit.Paragraph.prototype.getRectsForRange = function(start, end, hStyle, wStyle) {
    /**
     * This is bytes, but we'll want to think about them as float32s
     * @type {Float32Array}
     */
      var floatArray = this._getRectsForRange(start, end, hStyle, wStyle);

      if (!floatArray || !floatArray.length) {
        return [];
      }
      var ret = [];
      for (var i = 0; i < floatArray.length; i+=5) {
        var r = CanvasKit.LTRBRect(floatArray[i], floatArray[i+1], floatArray[i+2], floatArray[i+3]);
        if (floatArray[i+4] === 0) {
          r['direction'] = CanvasKit.TextDirection.RTL;
        } else {
          r['direction'] = CanvasKit.TextDirection.LTR;
        }
        ret.push(r);
      }
      CanvasKit._free(floatArray.byteOffset);
      return ret;
    }

    // Registers the font (provided as an arrayBuffer) with the alias `family`.
    CanvasKit.TypefaceFontProvider.prototype.registerFont = function(font, family) {
      var typeface = CanvasKit.SkFontMgr.RefDefault().MakeTypefaceFromData(font);
      if (!typeface) {
          SkDebug('Could not decode font data');
          // We do not need to free the data since the C++ will do that for us
          // when the font is deleted (or fails to decode);
          return null;
      }
      var familyPtr = cacheOrCopyString(family);
      this._registerFont(typeface, familyPtr);
    }

    // These helpers fill out all fields, because emscripten complains if we
    // have undefined and it expects, for example, a float.
    CanvasKit.ParagraphStyle = function(s) {
      // Use [''] to tell closure not to minify the names
      // TODO(kjlubick): strutStyle
      s['disableHinting'] = s['disableHinting'] || false;
      if (s['ellipsis']) {
        var str = s['ellipsis'];
        s['_ellipsisPtr'] = cacheOrCopyString(str);
        s['_ellipsisLen'] = lengthBytesUTF8(str) + 1; // add 1 for the null terminator.
      } else {
        s['_ellipsisPtr'] = nullptr;
        s['_ellipsisLen'] = 0;
      }

      s['heightMultiplier'] = s['heightMultiplier'] || 0;
      s['maxLines'] = s['maxLines'] || 0;
      s['textAlign'] = s['textAlign'] || CanvasKit.TextAlign.Start;
      s['textDirection'] = s['textDirection'] || CanvasKit.TextDirection.LTR;
      s['textStyle'] = CanvasKit.TextStyle(s['textStyle']);
      return s;
    };

    function fontStyle(s) {
      s = s || {};
      // Can't check for falsey as 0 width means "invisible".
      if (s['weight'] === undefined) {
        s['weight'] = CanvasKit.FontWeight.Normal;
      }
      s['width'] = s['width'] || CanvasKit.FontWidth.Normal;
      s['slant'] = s['slant'] || CanvasKit.FontSlant.Upright;
      return s;
    }

    CanvasKit.TextStyle = function(s) {
       // Use [''] to tell closure not to minify the names
      if (!s['color']) {
        s['color'] = CanvasKit.BLACK;
      }

      s['decoration'] = s['decoration'] || 0;
      s['decorationThickness'] = s['decorationThickness'] || 0;
      s['fontSize'] = s['fontSize'] || 0;
      s['fontStyle'] = fontStyle(s['fontStyle']);
      return s;
    };

    // returns a pointer to a place on the heap that has an array
    // of char* (effectively a char**). For now, this does the naive thing
    // and depends on the string being null-terminated. This should be used
    // for simple, well-formed things (e.g. font-families), not arbitrary
    // text that should be drawn. If we need this to handle more complex
    // strings, it should return two pointers, a pointer of the
    // string array and a pointer to an array of the strings byte lengths.
    function naiveCopyStrArray(strings) {
      if (!strings || !strings.length) {
        return nullptr;
      }
      var sPtrs = [];
      for (var i = 0; i < strings.length; i++) {
        var strPtr = cacheOrCopyString(strings[i]);
        sPtrs.push(strPtr);
      }
      return copy1dArray(sPtrs, "HEAPU32");
    }

    // maps string -> malloc'd pointer
    var stringCache = {};

    // cacheOrCopyString copies a string from JS into WASM on the heap and returns the pointer
    // to the memory of the string. It is expected that a caller to this helper will *not* free
    // that memory, so it is cached. Thus, if a future call to this function with the same string
    // will return the cached pointer, preventing the memory usage from growing unbounded (in
    // a normal use case).
    function cacheOrCopyString(str) {
      if (stringCache[str]) {
        return stringCache[str];
      }
      // Add 1 for null terminator, which we need when copying/converting
      var strLen = lengthBytesUTF8(str) + 1;
      var strPtr = CanvasKit._malloc(strLen);
      stringToUTF8(str, strPtr, strLen);
      stringCache[str] = strPtr;
      return strPtr;
    }

    // These scratch arrays are allocated once to copy the color data into, which saves us
    // having to free them after every invocation.
    var scratchForegroundColorPtr = CanvasKit._malloc(4 * 4); // room for 4 32bit floats
    var scratchBackgroundColorPtr = CanvasKit._malloc(4 * 4); // room for 4 32bit floats

    function copyArrays(textStyle) {
      // These color fields were arrays, but will set to WASM pointers before we pass this
      // object over the WASM interface.
      textStyle['_colorPtr'] = copyColorToWasm(textStyle['color']);
      textStyle['_foregroundColorPtr'] = nullptr; // nullptr is 0, from helper.js
      textStyle['_backgroundColorPtr'] = nullptr;
      if (textStyle['foregroundColor']) {
        textStyle['_foregroundColorPtr'] = copyColorToWasm(textStyle['foregroundColor'], scratchForegroundColorPtr);
      }
      if (textStyle['backgroundColor']) {
        textStyle['_backgroundColorPtr'] = copyColorToWasm(textStyle['backgroundColor'], scratchBackgroundColorPtr);
      }

      if (Array.isArray(textStyle['fontFamilies']) && textStyle['fontFamilies'].length) {
        textStyle['_fontFamiliesPtr'] = naiveCopyStrArray(textStyle['fontFamilies']);
        textStyle['_fontFamiliesLen'] = textStyle['fontFamilies'].length;
      } else {
        textStyle['_fontFamiliesPtr'] = nullptr;
        textStyle['_fontFamiliesLen'] = 0;
        SkDebug('no font families provided, text may draw wrong or not at all');
      }
    }

    function freeArrays(textStyle) {
      // The font family strings will get copied to a vector on the C++ side, which is owned by
      // the text style.
      CanvasKit._free(textStyle['_fontFamiliesPtr']);
    }

    CanvasKit.ParagraphBuilder.Make = function(paragraphStyle, fontManager) {
      copyArrays(paragraphStyle['textStyle']);

      var result =  CanvasKit.ParagraphBuilder._Make(paragraphStyle, fontManager);
      freeArrays(paragraphStyle['textStyle']);
      return result;
    };

    CanvasKit.ParagraphBuilder.MakeFromFontProvider = function(paragraphStyle, fontProvider) {
        copyArrays(paragraphStyle['textStyle']);

        var result =  CanvasKit.ParagraphBuilder._MakeFromFontProvider(paragraphStyle, fontProvider);
        freeArrays(paragraphStyle['textStyle']);
        return result;
    };

    CanvasKit.ParagraphBuilder.prototype.pushStyle = function(textStyle) {
      copyArrays(textStyle);
      this._pushStyle(textStyle);
      freeArrays(textStyle);
    }

    CanvasKit.ParagraphBuilder.prototype.pushPaintStyle = function(textStyle, fg, bg) {
      copyArrays(textStyle);
      this._pushPaintStyle(textStyle, fg, bg);
      freeArrays(textStyle);
    }
});
}(Module)); // When this file is loaded in, the high level object is "Module";
