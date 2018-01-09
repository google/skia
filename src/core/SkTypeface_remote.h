/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteTypeface_DEFINED
#define SkRemoteTypeface_DEFINED

#include "SkFontStyle.h"
#include "SkScalerContext.h"
#include "SkTypeface.h"

#include <thread>

class RemoteScalerContext;
class SkTypefaceProxy;

class SkScalerContextProxy : public SkScalerContext {
public:
    SkScalerContextProxy(
            sk_sp<SkTypeface> tf,
            const SkScalerContextEffects& effects,
            const SkDescriptor* desc);

protected:
    unsigned generateGlyphCount(void) override { SK_ABORT("Should never be called."); }
    uint16_t generateCharToGlyph(SkUnichar uni) override { SK_ABORT("Should never be called."); }
    void generateAdvance(SkGlyph* glyph) override { this->generateImage(*glyph); }
    void generateMetrics(SkGlyph* glyph) override { this->generateImage(*glyph); }
    void generateImage(const SkGlyph& glyph) override;
    void generatePath(SkGlyphID glyphID, SkPath* path) override;
    void generateFontMetrics(SkPaint::FontMetrics* metrics) override;

private:
    SkTypefaceProxy* typefaceProxy();
    RemoteScalerContext* const fRemote;
    typedef SkScalerContext INHERITED;
};

class SkTypefaceProxy : public SkTypeface {
public:
    SkTypefaceProxy(
            SkFontID fontId, std::thread::id threadId, const SkFontStyle& style, bool isFixed)
            : INHERITED{style, false}
            , fFontId{fontId}
            , fThreadId{threadId} {}
    SkFontID fontID() const {return fFontId;}
protected:
    int onGetUPEM() const override { SK_ABORT("Should never be called."); }
    SkStreamAsset* onOpenStream(int* ttcIndex) const override {
        SK_ABORT("Should never be called.");
    }
    std::unique_ptr<SkFontData> onMakeFontData() const override {
        SK_ABORT("Should never be called.");
    }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override {
        SK_ABORT("Should never be called.");
    }
    void onGetFamilyName(SkString* familyName) const override {
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
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc) const override {
        //std::cout << fFontId << fThreadId;

        return new SkScalerContextProxy(sk_ref_sp(const_cast<SkTypefaceProxy*>(this)), effects,
                                         desc);

    }
    void onFilterRec(SkScalerContextRec*) const override { /* Do nothing at this point */  }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override {
        SK_ABORT("Should never be called.");
    }
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        SK_ABORT("Should never be called.");
    }
    int onCharsToGlyphs(const void* chars, Encoding,
                        uint16_t glyphs[], int glyphCount) const override {
        SK_ABORT("Should never be called.");
    }
    int onCountGlyphs() const override { SK_ABORT("Should never be called."); }

    void* onGetCTFontRef() const override { SK_ABORT("Should never be called."); }

private:
    const SkFontID fFontId;
    const std::thread::id fThreadId;

    typedef SkTypeface INHERITED;
};

#endif  // SkRemoteTypeface_DEFINED
