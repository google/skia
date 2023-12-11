/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontPriv_DEFINED
#define SkFontPriv_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkTemplates.h"

class SkReadBuffer;
class SkWriteBuffer;

class SkFontPriv {
public:
    /*  This is the size we use when we ask for a glyph's path. We then
     *  post-transform it as we draw to match the request.
     *  This is done to try to re-use cache entries for the path.
     *
     *  This value is somewhat arbitrary. In theory, it could be 1, since
     *  we store paths as floats. However, we get the path from the font
     *  scaler, and it may represent its paths as fixed-point (or 26.6),
     *  so we shouldn't ask for something too big (might overflow 16.16)
     *  or too small (underflow 26.6).
     *
     *  This value could track kMaxSizeForGlyphCache, assuming the above
     *  constraints, but since we ask for unhinted paths, the two values
     *  need not match per-se.
     */
    inline static constexpr int kCanonicalTextSizeForPaths  = 64;

    /**
     *  Return a matrix that applies the paint's text values: size, scale, skew
     */
    static SkMatrix MakeTextMatrix(SkScalar size, SkScalar scaleX, SkScalar skewX) {
        SkMatrix m = SkMatrix::Scale(size * scaleX, size);
        if (skewX) {
            m.postSkew(skewX, 0);
        }
        return m;
    }

    static SkMatrix MakeTextMatrix(const SkFont& font) {
        return MakeTextMatrix(font.getSize(), font.getScaleX(), font.getSkewX());
    }

    static void ScaleFontMetrics(SkFontMetrics*, SkScalar);

    /**
        Returns the union of bounds of all glyphs.
        Returned dimensions are computed by font manager from font data,
        ignoring SkPaint::Hinting. Includes font metrics, but not fake bold or SkPathEffect.

        If text size is large, text scale is one, and text skew is zero,
        returns the bounds as:
        { SkFontMetrics::fXMin, SkFontMetrics::fTop, SkFontMetrics::fXMax, SkFontMetrics::fBottom }.

        @return  union of bounds of all glyphs
     */
    static SkRect GetFontBounds(const SkFont&);

    /** Return the approximate largest dimension of typical text when transformed by the matrix.
     *
     * @param matrix  used to transform size
     * @param textLocation  location of the text prior to matrix transformation. Used if the
     *                      matrix has perspective.
     * @return  typical largest dimension
     */
    static SkScalar ApproximateTransformedTextSize(const SkFont& font, const SkMatrix& matrix,
                                                   const SkPoint& textLocation);

    static bool IsFinite(const SkFont& font) {
        return SkScalarIsFinite(font.getSize()) &&
               SkScalarIsFinite(font.getScaleX()) &&
               SkScalarIsFinite(font.getSkewX());
    }

    // Returns the number of elements (characters or glyphs) in the array.
    static int CountTextElements(const void* text, size_t byteLength, SkTextEncoding);

    static void GlyphsToUnichars(const SkFont&, const uint16_t glyphs[], int count, SkUnichar[]);

    static void Flatten(const SkFont&, SkWriteBuffer& buffer);
    static bool Unflatten(SkFont*, SkReadBuffer& buffer);

    static inline uint8_t Flags(const SkFont& font) { return font.fFlags; }
};

class SkAutoToGlyphs {
public:
    SkAutoToGlyphs(const SkFont& font, const void* text, size_t length, SkTextEncoding encoding) {
        if (encoding == SkTextEncoding::kGlyphID || length == 0) {
            fGlyphs = reinterpret_cast<const uint16_t*>(text);
            fCount = SkToInt(length >> 1);
        } else {
            fCount = font.countText(text, length, encoding);
            if (fCount < 0) {
                fCount = 0;
            }
            fStorage.reset(fCount);
            font.textToGlyphs(text, length, encoding, fStorage.get(), fCount);
            fGlyphs = fStorage.get();
        }
    }

    int count() const { return fCount; }
    const uint16_t* glyphs() const { return fGlyphs; }

private:
    skia_private::AutoSTArray<32, uint16_t> fStorage;
    const uint16_t* fGlyphs;
    int             fCount;
};

#endif
