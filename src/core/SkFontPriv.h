/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontPriv_DEFINED
#define SkFontPriv_DEFINED

#include "SkFont.h"
#include "SkMatrix.h"
#include "SkTypeface.h"

class SkFontPriv {
public:
    /**
     *  Return a matrix that applies the paint's text values: size, scale, skew
     */
    static SkMatrix MakeTextMatrix(SkScalar size, SkScalar scaleX, SkScalar skewX) {
        SkMatrix m = SkMatrix::MakeScale(size * scaleX, size);
        if (skewX) {
            m.postSkew(skewX, 0);
        }
        return m;
    }

    static SkMatrix MakeTextMatrix(const SkFont& font) {
        return MakeTextMatrix(font.getSize(), font.getScaleX(), font.getSkewX());
    }

    static void ScaleFontMetrics(SkFontMetrics*, SkScalar);

    // returns -1 if buffer is invalid for specified encoding
    static int ValidCountText(const void* text, size_t length, SkTextEncoding);

    static SkTypeface* GetTypefaceOrDefault(const SkFont& font) {
        return font.getTypeface() ? font.getTypeface() : SkTypeface::GetDefaultTypeface();
    }

    static sk_sp<SkTypeface> RefTypefaceOrDefault(const SkFont& font) {
        return font.getTypeface() ? font.refTypeface() : SkTypeface::MakeDefault();
    }

    typedef const SkGlyph& (*GlyphCacheProc)(SkGlyphCache*, const char**, const char*);

    static GlyphCacheProc GetGlyphCacheProc(SkTextEncoding encoding, bool needFullMetrics);
};

#endif
