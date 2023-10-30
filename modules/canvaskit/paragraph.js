(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

    CanvasKit.Paragraph.prototype.getRectsForRange = function(start, end, hStyle, wStyle) {
    /**
     * @type {Float32Array}
     */
      var floatArray = this._getRectsForRange(start, end, hStyle, wStyle);
      return floatArrayToRects(floatArray);
    }

    CanvasKit.Paragraph.prototype.getRectsForPlaceholders = function() {
        /**
        * @type {Float32Array}
        */
        var floatArray = this._getRectsForPlaceholders();
        return floatArrayToRects(floatArray);
    }

    function convertDirection(glyphInfo) {
      if (glyphInfo) {
        if (glyphInfo['dir'] === 0) {
          glyphInfo['dir'] = CanvasKit.TextDirection.RTL;
        } else {
          glyphInfo['dir'] = CanvasKit.TextDirection.LTR;
        }
      }
      return glyphInfo;
    }

    CanvasKit.Paragraph.prototype.getGlyphInfoAt = function(index) {
      return convertDirection(this._getGlyphInfoAt(index));
    }

    CanvasKit.Paragraph.prototype.getClosestGlyphInfoAtCoordinate = function(dx, dy) {
      return convertDirection(this._getClosestGlyphInfoAtCoordinate(dx, dy));
    }

    function floatArrayToRects(floatArray) {
        if (!floatArray || !floatArray.length) {
            return [];
        }
        var ret = [];
        for (var i = 0; i < floatArray.length; i+=5) {
            var rect = CanvasKit.LTRBRect(floatArray[i], floatArray[i+1], floatArray[i+2], floatArray[i+3]);
            var dir = CanvasKit.TextDirection.LTR;
            if (floatArray[i+4] === 0) {
                dir = CanvasKit.TextDirection.RTL;
            }
            ret.push({'rect': rect, 'dir': dir});
        }
        CanvasKit._free(floatArray.byteOffset);
        return ret;
    }

    // Registers the font (provided as an arrayBuffer) with the alias `family`.
    CanvasKit.TypefaceFontProvider.prototype.registerFont = function(font, family) {
      var typeface = CanvasKit.Typeface.MakeTypefaceFromData(font);
      if (!typeface) {
          Debug('Could not decode font data');
          // We do not need to free the data since the C++ will do that for us
          // when the font is deleted (or fails to decode);
          return null;
      }
      var familyPtr = cacheOrCopyString(family);
      this._registerFont(typeface, familyPtr);
    }

    // These helpers fill out all fields, because emscripten complains if we
    // have undefined and it expects, for example, a float.
    // TODO(kjlubick) For efficiency, we should probably just return opaque WASM objects so we do
    //   not have to keep copying them across the wire.
    CanvasKit.ParagraphStyle = function(s) {
      // Use [''] to tell closure not to minify the names
      s['disableHinting'] = s['disableHinting'] || false;
      if (s['ellipsis']) {
        var str = s['ellipsis'];
        s['_ellipsisPtr'] = cacheOrCopyString(str);
        s['_ellipsisLen'] = lengthBytesUTF8(str);
      } else {
        s['_ellipsisPtr'] = nullptr;
        s['_ellipsisLen'] = 0;
      }

      if (s['heightMultiplier'] == null) {
        s['heightMultiplier'] = -1
      }
      s['maxLines'] = s['maxLines'] || 0;
      s['replaceTabCharacters'] = s['replaceTabCharacters'] || false;
      s['strutStyle'] = strutStyle(s['strutStyle']);
      s['textAlign'] = s['textAlign'] || CanvasKit.TextAlign.Start;
      s['textDirection'] = s['textDirection'] || CanvasKit.TextDirection.LTR;
      s['textHeightBehavior'] = s['textHeightBehavior'] || CanvasKit.TextHeightBehavior.All;
      s['textStyle'] = CanvasKit.TextStyle(s['textStyle']);
      s['applyRoundingHack'] = s['applyRoundingHack'] !== false;
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

    function strutStyle(s) {
        s = s || {};
        s['strutEnabled'] = s['strutEnabled'] || false;

        if (s['strutEnabled'] && Array.isArray(s['fontFamilies']) && s['fontFamilies'].length) {
            s['_fontFamiliesPtr'] = naiveCopyStrArray(s['fontFamilies']);
            s['_fontFamiliesLen'] = s['fontFamilies'].length;
        } else {
            s['_fontFamiliesPtr'] = nullptr;
            s['_fontFamiliesLen'] = 0;
        }
        s['fontStyle'] = fontStyle(s['fontStyle']);
        if (s['fontSize'] == null) {
          s['fontSize'] = -1
        }
        if (s['heightMultiplier'] == null) {
          s['heightMultiplier'] = -1
        }
        s['halfLeading'] = s['halfLeading'] || false;
        s['leading'] = s['leading'] || 0;
        s['forceStrutHeight'] = s['forceStrutHeight'] || false;
        return s;
    }

    CanvasKit.TextStyle = function(s) {
       // Use [''] to tell closure not to minify the names
      if (!s['color']) {
        s['color'] = CanvasKit.BLACK;
      }

      s['decoration'] = s['decoration'] || 0;
      s['decorationThickness'] = s['decorationThickness'] || 0;
      s['decorationStyle'] = s['decorationStyle'] || CanvasKit.DecorationStyle.Solid;
      s['textBaseline'] = s['textBaseline'] || CanvasKit.TextBaseline.Alphabetic;
      if (s['fontSize'] == null) {
        s['fontSize'] = -1
      }
      s['letterSpacing'] = s['letterSpacing'] || 0;
      s['wordSpacing'] = s['wordSpacing'] || 0;
      if (s['heightMultiplier'] == null) {
        s['heightMultiplier'] = -1
      }
      s['halfLeading'] = s['halfLeading'] || false;
      s['fontStyle'] = fontStyle(s['fontStyle']);

      // Properties which need to be Malloc'ed are set in `copyArrays`.

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
      return copy1dArray(sPtrs, 'HEAPU32');
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
    var scratchDecorationColorPtr = CanvasKit._malloc(4 * 4); // room for 4 32bit floats

    function copyArrays(textStyle) {
      // These color fields were arrays, but will set to WASM pointers before we pass this
      // object over the WASM interface.
      textStyle['_colorPtr'] = copyColorToWasm(textStyle['color']);
      textStyle['_foregroundColorPtr'] = nullptr; // nullptr is 0, from helper.js
      textStyle['_backgroundColorPtr'] = nullptr;
      textStyle['_decorationColorPtr'] = nullptr;
      if (textStyle['foregroundColor']) {
        textStyle['_foregroundColorPtr'] = copyColorToWasm(textStyle['foregroundColor'], scratchForegroundColorPtr);
      }
      if (textStyle['backgroundColor']) {
        textStyle['_backgroundColorPtr'] = copyColorToWasm(textStyle['backgroundColor'], scratchBackgroundColorPtr);
      }
      if (textStyle['decorationColor']) {
        textStyle['_decorationColorPtr'] = copyColorToWasm(textStyle['decorationColor'], scratchDecorationColorPtr);
      }

      if (Array.isArray(textStyle['fontFamilies']) && textStyle['fontFamilies'].length) {
        textStyle['_fontFamiliesPtr'] = naiveCopyStrArray(textStyle['fontFamilies']);
        textStyle['_fontFamiliesLen'] = textStyle['fontFamilies'].length;
      } else {
        textStyle['_fontFamiliesPtr'] = nullptr;
        textStyle['_fontFamiliesLen'] = 0;
        Debug('no font families provided, text may draw wrong or not at all');
      }

      if (textStyle['locale']) {
        var str = textStyle['locale'];
        textStyle['_localePtr'] = cacheOrCopyString(str);
        textStyle['_localeLen'] = lengthBytesUTF8(str);
      } else {
        textStyle['_localePtr'] = nullptr;
        textStyle['_localeLen'] = 0;
      }

      if (Array.isArray(textStyle['shadows']) && textStyle['shadows'].length) {
        var shadows = textStyle['shadows'];
        var shadowColors = shadows.map(function (s) { return s['color'] || CanvasKit.BLACK; });
        var shadowBlurRadii = shadows.map(function (s) { return s['blurRadius'] || 0.0; });
        textStyle['_shadowLen'] = shadows.length;
        // 2 floats per point, 4 bytes per float
        var ptr = CanvasKit._malloc(shadows.length * 2 * 4);
        var adjustedPtr = ptr / 4;  // 4 bytes per float
        for (var i = 0; i < shadows.length; i++) {
          var offset = shadows[i]['offset'] || [0, 0];
          CanvasKit.HEAPF32[adjustedPtr] = offset[0];
          CanvasKit.HEAPF32[adjustedPtr + 1] = offset[1];
          adjustedPtr += 2;
        }
        textStyle['_shadowColorsPtr'] = copyFlexibleColorArray(shadowColors).colorPtr;
        textStyle['_shadowOffsetsPtr'] = ptr;
        textStyle['_shadowBlurRadiiPtr'] = copy1dArray(shadowBlurRadii, 'HEAPF32');
      } else {
        textStyle['_shadowLen'] = 0;
        textStyle['_shadowColorsPtr'] = nullptr;
        textStyle['_shadowOffsetsPtr'] = nullptr;
        textStyle['_shadowBlurRadiiPtr'] = nullptr;
      }

      if (Array.isArray(textStyle['fontFeatures']) && textStyle['fontFeatures'].length) {
        var fontFeatures = textStyle['fontFeatures'];
        var fontFeatureNames = fontFeatures.map(function (s) { return s['name']; });
        var fontFeatureValues = fontFeatures.map(function (s) { return s['value']; });
        textStyle['_fontFeatureLen'] = fontFeatures.length;
        textStyle['_fontFeatureNamesPtr'] = naiveCopyStrArray(fontFeatureNames);
        textStyle['_fontFeatureValuesPtr'] = copy1dArray(fontFeatureValues, 'HEAPU32');
      } else {
        textStyle['_fontFeatureLen'] = 0;
        textStyle['_fontFeatureNamesPtr'] = nullptr;
        textStyle['_fontFeatureValuesPtr'] = nullptr;
      }

      if (Array.isArray(textStyle['fontVariations']) && textStyle['fontVariations'].length) {
        var fontVariations = textStyle['fontVariations'];
        var fontVariationAxes = fontVariations.map(function (s) { return s['axis']; });
        var fontVariationValues = fontVariations.map(function (s) { return s['value']; });
        textStyle['_fontVariationLen'] = fontVariations.length;
        textStyle['_fontVariationAxesPtr'] = naiveCopyStrArray(fontVariationAxes);
        textStyle['_fontVariationValuesPtr'] = copy1dArray(fontVariationValues, 'HEAPF32');
      } else {
        textStyle['_fontVariationLen'] = 0;
        textStyle['_fontVariationAxesPtr'] = nullptr;
        textStyle['_fontVariationValuesPtr'] = nullptr;
      }
    }

    function freeArrays(textStyle) {
      // The font family strings will get copied to a vector on the C++ side, which is owned by
      // the text style.
      CanvasKit._free(textStyle['_fontFamiliesPtr']);
      CanvasKit._free(textStyle['_shadowColorsPtr']);
      CanvasKit._free(textStyle['_shadowOffsetsPtr']);
      CanvasKit._free(textStyle['_shadowBlurRadiiPtr']);
      CanvasKit._free(textStyle['_fontFeatureNamesPtr']);
      CanvasKit._free(textStyle['_fontFeatureValuesPtr']);
      CanvasKit._free(textStyle['_fontVariationAxesPtr']);
      CanvasKit._free(textStyle['_fontVariationValuesPtr']);
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

    CanvasKit.ParagraphBuilder.MakeFromFontCollection = function(paragraphStyle, fontCollection) {
        copyArrays(paragraphStyle['textStyle']);

        var result = CanvasKit.ParagraphBuilder._MakeFromFontCollection(
	    paragraphStyle, fontCollection);
        freeArrays(paragraphStyle['textStyle']);
        return result;
    };

    CanvasKit.ParagraphBuilder.ShapeText = function(text, blocks, width) {
        let length = 0;
        for (const b of blocks) {
            length += b.length;
        }
        if (length !== text.length) {
            throw "Accumulated block lengths must equal text.length";
        }
        return CanvasKit.ParagraphBuilder._ShapeText(text, blocks, width);
    };

    CanvasKit.ParagraphBuilder.prototype.pushStyle = function(textStyle) {
      copyArrays(textStyle);
      this._pushStyle(textStyle);
      freeArrays(textStyle);
    };

    CanvasKit.ParagraphBuilder.prototype.pushPaintStyle = function(textStyle, fg, bg) {
      copyArrays(textStyle);
      this._pushPaintStyle(textStyle, fg, bg);
      freeArrays(textStyle);
    };

    CanvasKit.ParagraphBuilder.prototype.addPlaceholder =
          function(width, height, alignment, baseline, offset) {
      width = width || 0;
      height = height || 0;
      alignment = alignment || CanvasKit.PlaceholderAlignment.Baseline;
      baseline = baseline || CanvasKit.TextBaseline.Alphabetic;
      offset = offset || 0;
      this._addPlaceholder(width, height, alignment, baseline, offset);
    };

    CanvasKit.ParagraphBuilder.prototype.setWordsUtf8 = function(words) {
      var bPtr = copy1dArray(words, 'HEAPU32');
      this._setWordsUtf8(bPtr, words && words.length || 0);
      freeArraysThatAreNotMallocedByUsers(bPtr,     words);
    };
    CanvasKit.ParagraphBuilder.prototype.setWordsUtf16 = function(words) {
      var bPtr = copy1dArray(words, 'HEAPU32');
      this._setWordsUtf16(bPtr, words && words.length || 0);
      freeArraysThatAreNotMallocedByUsers(bPtr, words);
    };

    CanvasKit.ParagraphBuilder.prototype.setGraphemeBreaksUtf8 = function(graphemeBreaks) {
      var bPtr = copy1dArray(graphemeBreaks, 'HEAPU32');
      this._setGraphemeBreaksUtf8(bPtr, graphemeBreaks && graphemeBreaks.length || 0);
      freeArraysThatAreNotMallocedByUsers(bPtr,     graphemeBreaks);
    };
    CanvasKit.ParagraphBuilder.prototype.setGraphemeBreaksUtf16 = function(graphemeBreaks) {
      var bPtr = copy1dArray(graphemeBreaks, 'HEAPU32');
      this._setGraphemeBreaksUtf16(bPtr, graphemeBreaks && graphemeBreaks.length || 0);
      freeArraysThatAreNotMallocedByUsers(bPtr, graphemeBreaks);
    };

    CanvasKit.ParagraphBuilder.prototype.setLineBreaksUtf8 = function(lineBreaks) {
      var bPtr = copy1dArray(lineBreaks, 'HEAPU32');
      this._setLineBreaksUtf8(bPtr, lineBreaks && lineBreaks.length || 0);
      freeArraysThatAreNotMallocedByUsers(bPtr,     lineBreaks);
    };
    CanvasKit.ParagraphBuilder.prototype.setLineBreaksUtf16 = function(lineBreaks) {
      var bPtr = copy1dArray(lineBreaks, 'HEAPU32');
      this._setLineBreaksUtf16(bPtr, lineBreaks && lineBreaks.length || 0);
      freeArraysThatAreNotMallocedByUsers(bPtr, lineBreaks);
    };
});
}(Module)); // When this file is loaded in, the high level object is "Module";
