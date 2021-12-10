/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteTypeface_DEFINED
#define SkRemoteTypeface_DEFINED

#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTypeface.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkScalerContext.h"

class SkTypefaceProxy;
class SkStrikeCache;

class SkScalerContextProxy : public SkScalerContext {
public:
    SkScalerContextProxy(sk_sp<SkTypeface> tf,
                         const SkScalerContextEffects& effects,
                         const SkDescriptor* desc,
                         sk_sp<SkStrikeClient::DiscardableHandleManager> manager);

protected:
    bool generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph, SkArenaAlloc*) override;
    void generateImage(const SkGlyph& glyph) override;
    bool generatePath(const SkGlyph& glyphID, SkPath* path) override;
    void generateFontMetrics(SkFontMetrics* metrics) override;
    SkTypefaceProxy* getProxyTypeface() const;

private:
    sk_sp<SkStrikeClient::DiscardableHandleManager> fDiscardableManager;
    using INHERITED = SkScalerContext;
};

class SkTypefaceProxy : public SkTypeface {
public:
    SkTypefaceProxy(SkFontID fontId,
                    int glyphCount,
                    const SkFontStyle& style,
                    bool isFixed,
                    bool glyphMaskNeedsCurrentColor,
                    sk_sp<SkStrikeClient::DiscardableHandleManager> manager,
                    bool isLogging = true)
            : INHERITED{style, false}
            , fFontId{fontId}
            , fGlyphCount{glyphCount}
            , fIsLogging{isLogging}
            , fGlyphMaskNeedsCurrentColor(glyphMaskNeedsCurrentColor)
            , fDiscardableManager{std::move(manager)} {}
    SkFontID remoteTypefaceID() const {return fFontId;}
    int glyphCount() const {return fGlyphCount;}
    bool isLogging() const {return fIsLogging;}

protected:
    int onGetUPEM() const override { SK_ABORT("Should never be called."); }
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        SK_ABORT("Should never be called.");
    }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        SK_ABORT("Should never be called.");
    }
    bool onGlyphMaskNeedsCurrentColor() const override {
        return fGlyphMaskNeedsCurrentColor;
    }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override {
        SK_ABORT("Should never be called.");
    }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override {
        SK_ABORT("Should never be called.");
    }
    void onGetFamilyName(SkString* familyName) const override {
        // Used by SkStrikeCache::DumpMemoryStatistics.
        *familyName = "";
    }
    bool onGetPostScriptName(SkString*) const override {
        SK_ABORT("Should never be called.");
    }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        SK_ABORT("Should never be called.");
    }
    int onGetTableTags(SkFontTableTag tags[]) const override {
        SK_ABORT("Should never be called.");
    }
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override {
        SK_ABORT("Should never be called.");
    }
    std::unique_ptr<SkScalerContext> onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const override
    {
        return std::make_unique<SkScalerContextProxy>(
                sk_ref_sp(const_cast<SkTypefaceProxy*>(this)), effects, desc, fDiscardableManager);
    }
    void onFilterRec(SkScalerContextRec* rec) const override {
        // The rec filtering is already applied by the server when generating
        // the glyphs.
    }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override {
        SK_ABORT("Should never be called.");
    }
    void getGlyphToUnicodeMap(SkUnichar*) const override {
        SK_ABORT("Should never be called.");
    }

    void getPostScriptGlyphNames(SkString*) const override {
        SK_ABORT("Should never be called.");
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        SK_ABORT("Should never be called.");
    }
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override {
        SK_ABORT("Should never be called.");
    }
    int onCountGlyphs() const override {
        return this->glyphCount();
    }

    void* onGetCTFontRef() const override {
        SK_ABORT("Should never be called.");
    }

private:
    const SkFontID                                  fFontId;
    const int                                       fGlyphCount;
    const bool                                      fIsLogging;
    const bool                                      fGlyphMaskNeedsCurrentColor;
    sk_sp<SkStrikeClient::DiscardableHandleManager> fDiscardableManager;


    using INHERITED = SkTypeface;
};

#endif  // SkRemoteTypeface_DEFINED
