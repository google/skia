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
        if (floatArray[i+4] === 1) {
          r['direction'] = CanvasKit.TextDirection.RTL;
        } else {
          r['direction'] = CanvasKit.TextDirection.LTR;
        }
        ret.push(r);
      }
      CanvasKit._free(floatArray.byteOffset);
      return ret;
    }

    // These helpers fill out all fields, because emscripten complains if we
    // have undefined and it expects, for example, a float.
    CanvasKit.ParagraphStyle = function(s) {
      // Use [''] to tell closure not to minify the names
      // TODO(kjlubick): strutStyle
      s['disableHinting'] = s['disableHinting'] || false;
      if (s['ellipsis']) {
        var str = s['ellipsis'];
        var strLen = lengthBytesUTF8(str) + 1;
        var strPtr = CanvasKit._malloc(strLen);
        stringToUTF8(str, strPtr, strLen);
        s['_ellipsisPtr'] = strPtr;
        s['_ellipsisLen'] = strLen;
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
    }

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
      if (!isCanvasKitColor(s['color'])) {
        s['color'] = CanvasKit.BLACK;
      }
      s['foregroundColor'] = s['foregroundColor'] || CanvasKit.TRANSPARENT;
      s['backgroundColor'] = s['backgroundColor'] || CanvasKit.TRANSPARENT;
      s['decoration'] = s['decoration'] || 0;
      s['decorationThickness'] = s['decorationThickness'] || 0;
      s['fontSize'] = s['fontSize'] || 0;
      if (Array.isArray(s['fontFamilies']) && s['fontFamilies'].length) {
        var sPtr = naiveCopyStrArray(s['fontFamilies']);
        s['_fontFamilies'] = sPtr;
        s['_numFontFamilies'] = s['fontFamilies'].length;
      } else {
        s['_fontFamilies'] = nullptr;
        s['_numFontFamilies'] = 0;
        SkDebug("no font families provided, text may draw wrong or not at all")
      }
      s['fontStyle'] = fontStyle(s['fontStyle']);
      return s;
    }

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
        var str = strings[i];
        // Add 1 for null terminator, which we need when copying/converting
        var strLen = lengthBytesUTF8(str) + 1;
        var strPtr = CanvasKit._malloc(strLen);
        stringToUTF8(str, strPtr, strLen);
        sPtrs.push(strPtr);
      }
      return copy1dArray(sPtrs, CanvasKit.HEAPU32);
    }
});
}(Module)); // When this file is loaded in, the high level object is "Module";