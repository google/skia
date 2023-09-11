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
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkSharedMutex.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkScalerContext.h"
#include "src/utils/SkCharToGlyphCache.h"

struct SkAdvancedTypefaceMetrics;
class SkFontDescriptor;
class SkFontData;

// These are forward declared to avoid pimpl but also hide the FreeType implementation.
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_StreamRec_* FT_Stream;
typedef signed long FT_Pos;
typedef struct FT_BBox_ FT_BBox;


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
    bool generateFacePath(FT_Face, SkGlyphID, uint32_t loadGlyphFlags, SkPath*);

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
        using AxisDefinitions = skia_private::STArray<4, AxisDefinition, true>;
        bool recognizedFont(SkStreamAsset* stream, int* numFonts) const;
        bool scanFont(SkStreamAsset* stream, int ttcIndex,
                      SkString* name, SkFontStyle* style, bool* isFixedPitch,
                      AxisDefinitions* axes) const;
        static void computeAxisValues(
            AxisDefinitions axisDefinitions,
            const SkFontArguments::VariationPosition position,
            SkFixed* axisValues,
            const SkString& name,
            const SkFontArguments::VariationPosition::Coordinate* currentPosition = nullptr);
        static bool GetAxes(FT_Face face, AxisDefinitions* axes);
    private:
        FT_Face openFace(SkStreamAsset* stream, int ttcIndex, FT_Stream ftStream) const;
        FT_Library fLibrary;
        mutable SkMutex fLibraryMutex;
    };

    /** Fetch units/EM from "head" table if needed (ie for bitmap fonts) */
    static int GetUnitsPerEm(FT_Face face);

    /** Return the font data, or nullptr on failure. */
    std::unique_ptr<SkFontData> makeFontData() const;
    class FaceRec;
    FaceRec* getFaceRec() const;

    static constexpr SkTypeface::FactoryId FactoryId = SkSetFourByteTag('f','r','e','e');
    static sk_sp<SkTypeface> MakeFromStream(std::unique_ptr<SkStreamAsset>, const SkFontArguments&);

protected:
    SkTypeface_FreeType(const SkFontStyle& style, bool isFixedPitch);
    ~SkTypeface_FreeType() override;

    std::unique_ptr<SkFontData> cloneFontData(const SkFontArguments&) const;
    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects&,
                                                           const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    void getGlyphToUnicodeMap(SkUnichar*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void getPostScriptGlyphNames(SkString* dstArray) const override;
    bool onGetPostScriptName(SkString*) const override;
    int onGetUPEM() const override;
    bool onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                     int32_t adjustments[]) const override;
    void onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;

    LocalizedStrings* onCreateFamilyNameIterator() const override;

    bool onGlyphMaskNeedsCurrentColor() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset,
                          size_t length, void* data) const override;
    sk_sp<SkData> onCopyTableData(SkFontTableTag) const override;

    virtual std::unique_ptr<SkFontData> onMakeFontData() const = 0;
    /** Utility to fill out the SkFontDescriptor palette information from the SkFontData. */
    static void FontDataPaletteToDescriptorPalette(const SkFontData&, SkFontDescriptor*);

private:
    mutable SkOnce fFTFaceOnce;
    mutable std::unique_ptr<FaceRec> fFaceRec;

    mutable SkSharedMutex fC2GCacheMutex;
    mutable SkCharToGlyphCache fC2GCache;

    mutable SkOnce fGlyphMasksMayNeedCurrentColorOnce;
    mutable bool fGlyphMasksMayNeedCurrentColor;

    using INHERITED = SkTypeface;
};

class SkTypeface_FreeTypeStream : public SkTypeface_FreeType {
public:
    SkTypeface_FreeTypeStream(std::unique_ptr<SkFontData> fontData, const SkString familyName,
                              const SkFontStyle& style, bool isFixedPitch);
    ~SkTypeface_FreeTypeStream() override;

protected:
    void onGetFamilyName(SkString* familyName) const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool* serialize) const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const override;

private:
    const SkString fFamilyName;
    const std::unique_ptr<const SkFontData> fData;
};

#endif // SKFONTHOST_FREETYPE_COMMON_H_
