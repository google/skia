/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKFONTHOST_FREETYPE_COMMON_H_
#define SKFONTHOST_FREETYPE_COMMON_H_

#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkScalerContext.h"

// These are forward declared to avoid pimpl but also hide the FreeType implementation.
typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_StreamRec_* FT_Stream;
typedef signed long FT_Pos;


#ifdef SK_DEBUG
const char* SkTraceFtrGetError(int);
#define SK_TRACEFTR(ERR, MSG, ...) \
    SkDebugf("%s:%d:1: error: 0x%x '%s' " MSG "\n", __FILE__, __LINE__, ERR, \
            SkTraceFtrGetError((int)(ERR)), __VA_ARGS__)
#else
#define SK_TRACEFTR(ERR, ...) do { sk_ignore_unused_variable(ERR); } while (false)
#endif


class SkScalerContext_FreeType_Base : public SkScalerContext {
protected:
    // See http://freetype.sourceforge.net/freetype2/docs/reference/ft2-bitmap_handling.html#FT_Bitmap_Embolden
    // This value was chosen by eyeballing the result in Firefox and trying to match it.
    static const FT_Pos kBitmapEmboldenStrength = 1 << 6;

    SkScalerContext_FreeType_Base(sk_sp<SkTypeface> typeface, const SkScalerContextEffects& effects,
                                  const SkDescriptor *desc)
    : INHERITED(std::move(typeface), effects, desc)
    {}

    bool drawCOLRv0Glyph(FT_Face, const SkGlyph&, uint32_t loadGlyphFlags,
                         SkSpan<SkColor> palette, SkCanvas*);
    bool drawCOLRv1Glyph(FT_Face, const SkGlyph&, uint32_t loadGlyphFlags,
                         SkSpan<SkColor> palette, SkCanvas*);
    bool drawSVGGlyph(FT_Face, const SkGlyph&, uint32_t loadGlyphFlags,
                      SkSpan<SkColor> palette, SkCanvas*);
    void generateGlyphImage(FT_Face, const SkGlyph&, void*, const SkMatrix& bitmapTransform);
    bool generateGlyphPath(FT_Face, SkPath*);

    /** Computes a bounding box for a COLRv1 glyph.
     *
     *  This method may change the configured size and transforms on FT_Face. Make sure to
     *  configure size, matrix and load glyphs as needed after using this function to restore the
     *  state of FT_Face.
     */
    static bool computeColrV1GlyphBoundingBox(FT_Face, SkGlyphID, SkRect* bounds);

    struct ScalerContextBits {
        static const constexpr uint32_t COLRv0 = 1;
        static const constexpr uint32_t COLRv1 = 2;
        static const constexpr uint32_t SVG    = 3;
    };

private:
    using INHERITED = SkScalerContext;

    bool generateFacePath(FT_Face, SkGlyphID, uint32_t loadGlyphFlags, SkPath*);
};

#endif // SKFONTHOST_FREETYPE_COMMON_H_
