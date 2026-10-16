/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
(function(SkBidi) {
  SkBidi['onRuntimeInitialized'] = function() {
    SkBidi['Bidi'] = SkBidi['Bidi'] || {};

    SkBidi['Bidi']['getBidiRegions'] = function(text, textDirection) {
      let dir = textDirection === SkBidi['TextDirection']['LTR'] ? 1 : 0;
      let textLength = text.length;
      let textPtr = SkBidi['_malloc'](textLength * 2);
      let textView = new Uint16Array(SkBidi['HEAPU8'].buffer, textPtr, textLength);
      for (let i = 0; i < textLength; i++) {
        textView[i] = text.charCodeAt(i);
      }

      let outPtr = SkBidi['_malloc'](textLength * 3 * 4);
      let regionCount = SkBidi['_skbidi_getBidiRegions'](textPtr, textLength, dir, outPtr);

      let outView = new Int32Array(SkBidi['HEAPU8'].buffer, outPtr, regionCount * 3);
      let ret = [];
      for (let i = 0; i < regionCount * 3; i += 3) {
        ret.push({
          'start': outView[i],
          'end': outView[i+1],
          'level': outView[i+2]
        });
      }

      SkBidi['_free'](textPtr);
      SkBidi['_free'](outPtr);
      return ret;
    }

    SkBidi['Bidi']['reorderVisual'] = function(visualRuns) {
      if (!visualRuns || !visualRuns.length) return [];
      let vPtr = SkBidi['_malloc'](visualRuns.length);
      SkBidi['HEAPU8'].set(visualRuns, vPtr);
      let outPtr = SkBidi['_malloc'](visualRuns.length * 4);

      let success = SkBidi['_skbidi_reorderVisual'](vPtr, visualRuns.length, outPtr);

      let ret = [];
      if (success) {
        let outView = new Int32Array(SkBidi['HEAPU8'].buffer, outPtr, visualRuns.length);
        for (let i = 0; i < visualRuns.length; i++) {
          ret.push({ 'index': outView[i] });
        }
      }

      SkBidi['_free'](vPtr);
      SkBidi['_free'](outPtr);
      return ret;
    }

    SkBidi['CodeUnits'] = SkBidi['CodeUnits'] || {};
    SkBidi['CodeUnits']['compute'] = function(text) {
      let textLength = text.length;
      let textPtr = SkBidi['_malloc'](textLength * 2);
      let textView = new Uint16Array(SkBidi['HEAPU8'].buffer, textPtr, textLength);
      for (let i = 0; i < textLength; i++) {
        textView[i] = text.charCodeAt(i);
      }

      let outPtr = SkBidi['_malloc'](textLength * 2);
      let success = SkBidi['_skbidi_computeCodeUnitFlags'](textPtr, textLength, outPtr);

      let ret = [];
      if (success) {
        let outView = new Uint16Array(SkBidi['HEAPU8'].buffer, outPtr, textLength);
        for (let i = 0; i < textLength; i++) {
          ret.push({ 'flags': outView[i] });
        }
      }

      SkBidi['_free'](textPtr);
      SkBidi['_free'](outPtr);
      return ret;
    }

    if (!SkBidi['TextDirection']) {
      SkBidi['TextDirection'] = {
        'LTR': { 'value': 1 },
        'RTL': { 'value': 0 },
      }
    }
  };
})(Module);
