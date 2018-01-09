/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteTypeface_DEFINED
#define SkRemoteTypeface_DEFINED

#include "SkDescriptor.h"
#include "SkFontStyle.h"
#include "SkScalerContext.h"
#include "SkTypeface.h"

#include <thread>

class SkTypefaceProxy;

class RemoteScalerContext {
public:
    virtual ~RemoteScalerContext() {}
    // TODO: do metrics need effects?
    virtual void generateFontMetrics(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            SkPaint::FontMetrics*) = 0;
    virtual void generateMetrics(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            SkGlyph* glyph) = 0;
    virtual void generateImage(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            const SkGlyph& glyph)  = 0;
    virtual void generatePath(
            const SkTypefaceProxy& tf,
            const SkScalerContextRec& rec,
            SkGlyphID glyph, SkPath* path) = 0;
};

class SkScalerContextProxy : public SkScalerContext {
public:
    SkScalerContextProxy(
            sk_sp<SkTypeface> tf,
            const SkScalerContextEffects& effects,
            const SkDescriptor* desc,
            RemoteScalerContext* rsc);

protected:
    unsigned generateGlyphCount(void) override { SK_ABORT("Should never be called."); }
    uint16_t generateCharToGlyph(SkUnichar uni) override { SK_ABORT("Should never be called."); }
    void generateAdvance(SkGlyph* glyph) override { this->generateMetrics(glyph); }
    void generateMetrics(SkGlyph* glyph) override;
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
            SkFontID fontId,
            std::thread::id threadId,
            const SkFontStyle& style,
            bool isFixed,
            RemoteScalerContext* rsc)
            : INHERITED{style, false}
            , fFontId{fontId}
            , fThreadId{threadId}
            , fRsc{rsc} { }
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
                                         desc, fRsc);

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
    RemoteScalerContext* const fRsc;

    typedef SkTypeface INHERITED;
};

class Op {
public:
    Op() {}
    int32_t op;
    SkFontID typeface_id;
    union {
        // op 0
        SkPaint::FontMetrics fontMetrics;
        // op 1 and 2
        SkGlyph glyph;
        // op 3
        struct {
            SkGlyphID glyphId;
            size_t pathSize;
        };
    };
    alignas(uint32_t) uint8_t descriptor[sizeof(SkDescriptor)
                                         + sizeof(SkDescriptor::Entry)
                                         + sizeof(SkScalerContextRec)
                                         + 8];
};

#endif  // SkRemoteTypeface_DEFINED
