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

#include "SkFontMgr.h"

// These are forward declared to avoid pimpl but also hide the FreeType implementation.
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_StreamRec_* FT_Stream;
typedef signed long FT_Pos;


#ifdef SK_DEBUG
const char* SkTraceFtrGetError(int);
#define SK_TRACEFTR(ERR, MSG, ...) \
    SkDebugf("%s:%lu:1: error: 0x%x '%s' " MSG "\n", __FILE__, __LINE__, ERR, \
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

    void generateGlyphImage(FT_Face face, const SkGlyph& glyph, const SkMatrix& bitmapTransform);
    bool generateGlyphPath(FT_Face face, SkPath* path);
    bool generateFacePath(FT_Face face, SkGlyphID glyphID, SkPath* path);
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
        bool recognizedFont(SkStreamAsset* stream, int* numFonts) const;
        bool scanFont(SkStreamAsset* stream, int ttcIndex,
                      SkString* name, SkFontStyle* style, bool* isFixedPitch,
                      AxisDefinitions* axes) const;
        static void computeAxisValues(
            AxisDefinitions axisDefinitions,
            const SkFontArguments::VariationPosition position,
            SkFixed* axisValues,
            const SkString& name);
        static bool GetAxes(FT_Face face, AxisDefinitions* axes);

    private:
        FT_Face openFace(SkStreamAsset* stream, int ttcIndex, FT_Stream ftStream) const;
        FT_Library fLibrary;
        mutable SkMutex fLibraryMutex;
    };

    /** Fetch units/EM from "head" table if needed (ie for bitmap fonts) */
    static int GetUnitsPerEm(FT_Face face);
protected:
    SkTypeface_FreeType(const SkFontStyle& style, bool isFixedPitch)
        : INHERITED(style, isFixedPitch)
    {}

    std::unique_ptr<SkFontData> cloneFontData(const SkFontArguments&) const;
    virtual SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                                   const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    void getGlyphToUnicodeMap(SkUnichar*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void getPostScriptGlyphNames(SkString* dstArray) const override;
    int onGetUPEM() const override;
    bool onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                     int32_t adjustments[]) const override;
    void onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;

    LocalizedStrings* onCreateFamilyNameIterator() const override;

    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset,
                          size_t length, void* data) const override;

private:
    typedef SkTypeface INHERITED;
};

#endif // SKFONTHOST_FREETYPE_COMMON_H_
