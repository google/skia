/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKFONTHOST_FREETYPE_COMMON_H_
#define SKFONTHOST_FREETYPE_COMMON_H_

#include "SkGlyph.h"
#include "SkScalerContext.h"
#include "SkTypeface.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef SK_DEBUG
    #define SkASSERT_CONTINUE(pred)                                                         \
        do {                                                                                \
            if (!(pred))                                                                    \
                SkDebugf("file %s:%d: assert failed '" #pred "'\n", __FILE__, __LINE__);    \
        } while (false)
#else
    #define SkASSERT_CONTINUE(pred)
#endif


class SkScalerContext_FreeType_Base : public SkScalerContext {
protected:
    // See http://freetype.sourceforge.net/freetype2/docs/reference/ft2-bitmap_handling.html#FT_Bitmap_Embolden
    // This value was chosen by eyeballing the result in Firefox and trying to match it.
    static const FT_Pos kBitmapEmboldenStrength = 1 << 6;

    SkScalerContext_FreeType_Base(SkTypeface* typeface, const SkDescriptor *desc)
    : INHERITED(typeface, desc)
    {}

    void generateGlyphImage(FT_Face face, const SkGlyph& glyph);
    void generateGlyphPath(FT_Face face, SkPath* path);
    void emboldenOutline(FT_Face face, FT_Outline* outline);

private:
    typedef SkScalerContext INHERITED;
};

class SkTypeface_FreeType : public SkTypeface {
protected:
    SkTypeface_FreeType(Style style, SkFontID uniqueID, bool isFixedPitch)
        : INHERITED(style, uniqueID, isFixedPitch) {}

    virtual SkScalerContext* onCreateScalerContext(
                                        const SkDescriptor*) const SK_OVERRIDE;
    virtual void onFilterRec(SkScalerContextRec*) const SK_OVERRIDE;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo,
                                const uint32_t*, uint32_t) const SK_OVERRIDE;
    virtual int onGetUPEM() const SK_OVERRIDE;

private:
    typedef SkTypeface INHERITED;
};

#endif // SKFONTHOST_FREETYPE_COMMON_H_
