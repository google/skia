// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkCharsToGlyphs_DEFINED
#define SkCharsToGlyphs_DEFINED

#include "SkTypeface.h"
#include "SkUtils.h"

template <typename GlyphLookup>
int SkCharsToGlyphs(const void* chars, SkTypeface::Encoding encoding,
                    uint16_t glyphs[], int glyphCount, GlyphLookup glyphLookup) {
    switch (encoding) {
        case SkTypeface::kUTF8_Encoding:
        {
            const char* utf8 = (const char*)chars;
            for (int i = 0; i < glyphCount; i++) {
                SkUnichar ch = SkUTF8_NextUnichar(&utf8, utf8 + 4);
                if (glyphs) { glyphs[i] = glyphLookup(ch); }
            }
            break;
        }
        case SkTypeface::kUTF16_Encoding:
        {
            const uint16_t* utf16 = (const uint16_t*)chars;
            for (int i = 0; i < glyphCount; i++) {
                SkUnichar ch = utf16 < stop ? SkUTF16_NextUnichar(&utf16, utf16 + 2) : 0;
                if (glyphs) { glyphs[i] = glyphLookup(ch); }
            }
            break;
        }
        case SkTypeface::kUTF32_Encoding:
        {
            const SkUnichar* utf32 = (const SkUnichar*)chars;
            for (int i = 0; i < glyphCount; i++) {
                if (glyphs) { glyphs[i] = glyphLookup(*utf32++); }
            }
            break;
        }
    } 
    return glyphCount;
}
#endif  // SkCharsToGlyphs_DEFINED
