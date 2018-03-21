/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteTypeface_DEFINED
#define SkRemoteTypeface_DEFINED

#include "SkAdvancedTypefaceMetrics.h"
#include "SkDescriptor.h"
#include "SkFontDescriptor.h"
#include "SkFontStyle.h"
#include "SkPaint.h"
#include "SkScalerContext.h"
#include "SkTypeface.h"

class SkStrikeClient;
class SkTypefaceProxy;

class SkScalerContextProxy : public SkScalerContext {
public:
    SkScalerContextProxy(
            sk_sp<SkTypeface> tf,
            const SkScalerContextEffects& effects,
            const SkDescriptor* desc,
            SkStrikeClient* rsc);

protected:
    unsigned generateGlyphCount() override;
    uint16_t generateCharToGlyph(SkUnichar) override;
    void generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph) override;
    void generateImage(const SkGlyph& glyph) override;
    void generatePath(SkGlyphID glyphID, SkPath* path) override;
    void generateFontMetrics(SkPaint::FontMetrics* metrics) override;

private:
    // Copied from SkGlyphCache
    // so we don't grow our arrays a lot
    static constexpr size_t kMinGlyphCount = 8;
    static constexpr size_t kMinGlyphImageSize = 16 /* height */ * 8 /* width */;
    static constexpr size_t kMinAllocAmount = kMinGlyphImageSize * kMinGlyphCount;

    SkTypefaceProxy* typefaceProxy();

    SkArenaAlloc          fAlloc{kMinAllocAmount};
    SkStrikeClient* const fClient;
    typedef SkScalerContext INHERITED;
};

class SkTypefaceProxy : public SkTypeface {
public:
    SkTypefaceProxy(
            SkFontID fontId,
            int glyphCount,
            const SkFontStyle& style,
            bool isFixed,
            SkStrikeClient* rsc)
            : INHERITED{style, false}
            , fFontId{fontId}
            , fGlyphCount{glyphCount}
            , fRsc{rsc} { }
    SkFontID remoteTypefaceID() const {return fFontId;}
    int glyphCount() const {return fGlyphCount;}
    static SkTypefaceProxy* DownCast(SkTypeface* typeface) {
        // TODO: how to check the safety of the down cast?
        return (SkTypefaceProxy*)typeface;
    }

protected:
    int onGetUPEM() const override { SK_ABORT("Should never be called."); return 0; }
    SkStreamAsset* onOpenStream(int* ttcIndex) const override {
        SK_ABORT("Should never be called.");
        return nullptr;
    }
    std::unique_ptr<SkFontData> onMakeFontData() const override {
        SK_ABORT("Should never be called.");
        return nullptr;
    }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override {
        SK_ABORT("Should never be called.");
        return 0;
    }
    void onGetFamilyName(SkString* familyName) const override {
        SK_ABORT("Should never be called.");
    }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        SK_ABORT("Should never be called.");
        return nullptr;
    }
    int onGetTableTags(SkFontTableTag tags[]) const override {
        SK_ABORT("Should never be called.");
        return 0;
    }
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override {
        SK_ABORT("Should never be called.");
        return 0;
    }
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc) const override {
        return new SkScalerContextProxy(sk_ref_sp(const_cast<SkTypefaceProxy*>(this)), effects,
                                         desc, fRsc);

    }
    void onFilterRec(SkScalerContextRec* rec) const override {
        // Add all the device information here.
        //rec->fPost2x2[0][0] = 0.5f;

        // This would be the best place to run the host SkTypeface_* onFilterRec.
        // Can we move onFilterRec to the FongMgr, that way we don't need to cross the boundary to
        // filter.
    }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override {
        SK_ABORT("Should never be called.");
    }
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        SK_ABORT("Should never be called.");
        return nullptr;
    }
    int onCharsToGlyphs(const void* chars, Encoding,
                        uint16_t glyphs[], int glyphCount) const override {
        SK_ABORT("Should never be called.");
        return 0;
    }
    int onCountGlyphs() const override {
        SK_ABORT("Should never be called.");
        return 0;
    }

    void* onGetCTFontRef() const override {
        SK_ABORT("Should never be called.");
        return nullptr;
    }

private:
    const SkFontID        fFontId;
    const int             fGlyphCount;
    SkStrikeClient* const fRsc;

    typedef SkTypeface INHERITED;
};

#endif  // SkRemoteTypeface_DEFINED
