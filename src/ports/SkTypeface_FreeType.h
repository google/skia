/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_Freetype_DEFINED
#define SkTypeface_Freetype_DEFINED

#include "include/core/SkFontScanner.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkSharedMutex.h"
#include "src/utils/SkCharToGlyphCache.h"

class SkFontData;

// These are forward declared to avoid pimpl but also hide the FreeType implementation.
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_StreamRec_* FT_Stream;
typedef signed long FT_Pos;
typedef struct FT_BBox_ FT_BBox;

class SkTypeface_FreeType : public SkTypeface {
public:
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

    std::unique_ptr<SkFontData> cloneFontData(const SkFontArguments&, SkFontStyle* style) const;
    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects&,
                                                           const SkDescriptor*) const override;
    std::unique_ptr<SkScalerContext> onCreateScalerContextAsProxyTypeface(
                                                           const SkScalerContextEffects&,
                                                           const SkDescriptor*,
                                                           sk_sp<SkTypeface>) const override;
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

#endif // SkTypeface_Freetype_DEFINED
