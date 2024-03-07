/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_mac_ct_DEFINED
#define SkTypeface_mac_ct_DEFINED

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkOnce.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <memory>

class SkData;
class SkDescriptor;
class SkFontData;
class SkFontDescriptor;
class SkScalerContext;
class SkString;
struct SkAdvancedTypefaceMetrics;
struct SkScalerContextEffects;
struct SkScalerContextRec;

struct OpszVariation {
    bool isSet = false;
    double value = 0;
};

struct CTFontVariation {
    SkUniqueCFRef<CFDictionaryRef> variation;
    SkUniqueCFRef<CFDictionaryRef> wrongOpszVariation;
    OpszVariation opsz;
};

SkFontStyle SkCTFontDescriptorGetSkFontStyle(CTFontDescriptorRef desc, bool fromDataProvider);

CGFloat SkCTFontCTWeightForCSSWeight(int fontstyleWeight);
CGFloat SkCTFontCTWidthForCSSWidth(int fontstyleWidth);

void SkStringFromCFString(CFStringRef src, SkString* dst);

class SkTypeface_Mac : public SkTypeface {
private:
    SkTypeface_Mac(SkUniqueCFRef<CTFontRef> fontRef, const SkFontStyle& fs, bool isFixedPitch,
                   OpszVariation opszVariation, std::unique_ptr<SkStreamAsset> providedData)
        : SkTypeface(fs, isFixedPitch)
        , fFontRef(std::move(fontRef))
        , fOpszVariation(opszVariation)
        , fHasColorGlyphs(
                SkToBool(CTFontGetSymbolicTraits(fFontRef.get()) & kCTFontColorGlyphsTrait))
        , fStream(std::move(providedData))
        , fIsFromStream(fStream)
    {
        SkASSERT(fFontRef);
    }

public:
    static sk_sp<SkTypeface> Make(SkUniqueCFRef<CTFontRef> font,
                                  OpszVariation opszVariation,
                                  std::unique_ptr<SkStreamAsset> providedData);

    static constexpr SkTypeface::FactoryId FactoryId = SkSetFourByteTag('c','t','x','t');
    static sk_sp<SkTypeface> SK_SPI MakeFromStream(std::unique_ptr<SkStreamAsset>,
                                                   const SkFontArguments&);

    SkUniqueCFRef<CTFontRef> fFontRef;
    const OpszVariation fOpszVariation;
    const bool fHasColorGlyphs;

    /**
     * CTFontCopyVariationAxes provides the localized name of all axes, making it very slow.
     * This is unfortunate, its result is needed just to see if there are any axes at all.
     * To avoid calling internal APIs cache the result of CTFontCopyVariationAxes.
     * https://github.com/WebKit/WebKit/commit/1842365d413ed87868e7d33d4fad1691fa3a8129
     * https://bugs.webkit.org/show_bug.cgi?id=232690
     */
    CFArrayRef getVariationAxes() const;

protected:
    int onGetUPEM() const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkStreamAsset> onOpenExistingStream(int* ttcIndex) const override;
    bool onGlyphMaskNeedsCurrentColor() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    void onGetFamilyName(SkString* familyName) const override;
    bool onGetPostScriptName(SkString*) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override;
    sk_sp<SkData> onCopyTableData(SkFontTableTag) const override;
    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects&,
                                                           const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    void getGlyphToUnicodeMap(SkUnichar*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;
    void getPostScriptGlyphNames(SkString*) const override {}
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const override;

    void* onGetCTFontRef() const override { return (void*)fFontRef.get(); }

private:
    mutable std::unique_ptr<SkStreamAsset> fStream;
    mutable SkUniqueCFRef<CFArrayRef> fVariationAxes;
    bool fIsFromStream;
    mutable SkOnce fInitStream;
    mutable SkOnce fInitVariationAxes;

    using INHERITED = SkTypeface;
};

#endif
#endif //SkTypeface_mac_ct_DEFINED
