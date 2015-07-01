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
#include "SkMutex.h"
#include "SkScalerContext.h"
#include "SkTypeface.h"
#include "SkTypes.h"

#include <ft2build.h>
#include FT_FREETYPE_H

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

private:
    typedef SkScalerContext INHERITED;
};

class SkTypeface_FreeType : public SkTypeface {
public:
    /** For SkFontMgrs to make use of our ability to extract
     *  name and style from a stream, using FreeType's API.
     */
    class Scanner : ::SkNoncopyable {
    public:
        Scanner();
        ~Scanner();
        struct AxisDefinition {
            SkFourByteTag fTag;
            SkFixed fMinimum;
            SkFixed fDefault;
            SkFixed fMaximum;
        };
        using AxisDefinitions = SkSTArray<4, AxisDefinition, true>;
        bool recognizedFont(SkStream* stream, int* numFonts) const;
        bool scanFont(SkStream* stream, int ttcIndex,
                      SkString* name, SkFontStyle* style, bool* isFixedPitch,
                      AxisDefinitions* axes) const;
    private:
        FT_Face openFace(SkStream* stream, int ttcIndex, FT_Stream ftStream) const;
        FT_Library fLibrary;
        mutable SkMutex fLibraryMutex;
    };

protected:
    SkTypeface_FreeType(const SkFontStyle& style, SkFontID uniqueID, bool isFixedPitch)
        : INHERITED(style, uniqueID, isFixedPitch)
        , fGlyphCount(-1)
    {}

    virtual SkScalerContext* onCreateScalerContext(
                                        const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                        PerGlyphInfo, const uint32_t*, uint32_t) const override;
    int onGetUPEM() const override;
    virtual bool onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                       int32_t adjustments[]) const override;
    virtual int onCharsToGlyphs(const void* chars, Encoding, uint16_t glyphs[],
                                int glyphCount) const override;
    int onCountGlyphs() const override;

    LocalizedStrings* onCreateFamilyNameIterator() const override;

    int onGetTableTags(SkFontTableTag tags[]) const override;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const override;

private:
    mutable int fGlyphCount;

    typedef SkTypeface INHERITED;
};

#endif // SKFONTHOST_FREETYPE_COMMON_H_
