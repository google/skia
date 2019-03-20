/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RandomScalerContext.h"
#include "SkAdvancedTypefaceMetrics.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGlyph.h"
#include "SkMakeUnique.h"
#include "SkPath.h"
#include "SkRectPriv.h"

class SkDescriptor;

class RandomScalerContext : public SkScalerContext {
public:
    RandomScalerContext(sk_sp<SkRandomTypeface>,
                        const SkScalerContextEffects&,
                        const SkDescriptor*,
                        bool fFakeIt);

protected:
    unsigned generateGlyphCount() override;
    uint16_t generateCharToGlyph(SkUnichar) override;
    bool     generateAdvance(SkGlyph*) override;
    void     generateMetrics(SkGlyph*) override;
    void     generateImage(const SkGlyph&) override;
    bool     generatePath(SkGlyphID, SkPath*) override;
    void     generateFontMetrics(SkFontMetrics*) override;

private:
    SkRandomTypeface* getRandomTypeface() const {
        return static_cast<SkRandomTypeface*>(this->getTypeface());
    }
    std::unique_ptr<SkScalerContext> fProxy;
    bool                             fFakeIt;
};

RandomScalerContext::RandomScalerContext(sk_sp<SkRandomTypeface>       face,
                                         const SkScalerContextEffects& effects,
                                         const SkDescriptor*           desc,
                                         bool                          fakeIt)
        : SkScalerContext(std::move(face), effects, desc)
        , fProxy(getRandomTypeface()->proxy()->createScalerContext(SkScalerContextEffects(), desc))
        , fFakeIt(fakeIt) {
    fProxy->forceGenerateImageFromPath();
}

unsigned RandomScalerContext::generateGlyphCount() { return fProxy->getGlyphCount(); }

uint16_t RandomScalerContext::generateCharToGlyph(SkUnichar uni) {
    return fProxy->charToGlyphID(uni);
}

bool RandomScalerContext::generateAdvance(SkGlyph* glyph) { return fProxy->generateAdvance(glyph); }

void RandomScalerContext::generateMetrics(SkGlyph* glyph) {
    // Here we will change the mask format of the glyph
    // NOTE: this may be overridden by the base class (e.g. if a mask filter is applied).
    switch (glyph->getGlyphID() % 4) {
        case 0: glyph->fMaskFormat = SkMask::kLCD16_Format; break;
        case 1: glyph->fMaskFormat = SkMask::kA8_Format; break;
        case 2: glyph->fMaskFormat = SkMask::kARGB32_Format; break;
        case 3: glyph->fMaskFormat = SkMask::kBW_Format; break;
    }

    fProxy->getMetrics(glyph);

    if (fFakeIt || (glyph->getGlyphID() % 4) != 2) {
        return;
    }

    SkPath path;
    if (!fProxy->getPath(glyph->getPackedID(), &path)) {
        return;
    }
    glyph->fMaskFormat = SkMask::kARGB32_Format;

    SkRect         storage;
    const SkPaint& paint = this->getRandomTypeface()->paint();
    const SkRect&  newBounds =
            paint.doComputeFastBounds(path.getBounds(), &storage, SkPaint::kFill_Style);
    SkIRect ibounds;
    newBounds.roundOut(&ibounds);
    glyph->fLeft   = ibounds.fLeft;
    glyph->fTop    = ibounds.fTop;
    glyph->fWidth  = ibounds.width();
    glyph->fHeight = ibounds.height();
}

void RandomScalerContext::generateImage(const SkGlyph& glyph) {
    // TODO: can force down but not up
    /*
    SkMask::Format format = (SkMask::Format)glyph.fMaskFormat;
    switch (glyph.getGlyphID() % 4) {
        case 0: format = SkMask::kLCD16_Format; break;
        case 1: format = SkMask::kA8_Format; break;
        case 2: format = SkMask::kARGB32_Format; break;
        case 3: format = SkMask::kBW_Format; break;
    }
    const_cast<SkGlyph&>(glyph).fMaskFormat = format;
    */

    if (fFakeIt) {
        sk_bzero(glyph.fImage, glyph.computeImageSize());
        return;
    }

    if (SkMask::kARGB32_Format != glyph.fMaskFormat) {
        fProxy->getImage(glyph);
        return;
    }

    // If the format is ARGB, just draw the glyph from path.
    SkPath path;
    if (!fProxy->getPath(glyph.getPackedID(), &path)) {
        fProxy->getImage(glyph);
        return;
    }

    SkBitmap bm;
    bm.installPixels(SkImageInfo::MakeN32Premul(glyph.fWidth, glyph.fHeight),
                     glyph.fImage,
                     glyph.rowBytes());
    bm.eraseColor(0);

    SkCanvas canvas(bm);
    canvas.translate(-SkIntToScalar(glyph.fLeft), -SkIntToScalar(glyph.fTop));
    canvas.drawPath(path, this->getRandomTypeface()->paint());
}

bool RandomScalerContext::generatePath(SkGlyphID glyph, SkPath* path) {
    return fProxy->generatePath(glyph, path);
}

void RandomScalerContext::generateFontMetrics(SkFontMetrics* metrics) {
    fProxy->getFontMetrics(metrics);
}

///////////////////////////////////////////////////////////////////////////////

SkRandomTypeface::SkRandomTypeface(sk_sp<SkTypeface> proxy, const SkPaint& paint, bool fakeIt)
        : SkTypeface(proxy->fontStyle(), false)
        , fProxy(std::move(proxy))
        , fPaint(paint)
        , fFakeIt(fakeIt) {}

SkScalerContext* SkRandomTypeface::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                         const SkDescriptor*           desc) const {
    return new RandomScalerContext(
            sk_ref_sp(const_cast<SkRandomTypeface*>(this)), effects, desc, fFakeIt);
}

void SkRandomTypeface::onFilterRec(SkScalerContextRec* rec) const {
    fProxy->filterRec(rec);
    rec->setHinting(kNo_SkFontHinting);
    rec->fMaskFormat = SkMask::kARGB32_Format;
}

void SkRandomTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    fProxy->getGlyphToUnicodeMap(glyphToUnicode);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkRandomTypeface::onGetAdvancedMetrics() const {
    return fProxy->getAdvancedMetrics();
}

std::unique_ptr<SkStreamAsset> SkRandomTypeface::onOpenStream(int* ttcIndex) const {
    return fProxy->openStream(ttcIndex);
}

sk_sp<SkTypeface> SkRandomTypeface::onMakeClone(const SkFontArguments& args) const {
    sk_sp<SkTypeface> proxy = fProxy->makeClone(args);
    if (!proxy) {
        return nullptr;
    }
    return sk_make_sp<SkRandomTypeface>(proxy, fPaint, fFakeIt);
}

void SkRandomTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    // TODO: anything that uses this typeface isn't correctly serializable, since this typeface
    // cannot be deserialized.
    fProxy->getFontDescriptor(desc, isLocal);
}

int SkRandomTypeface::onCharsToGlyphs(const void* chars,
                                      Encoding    encoding,
                                      uint16_t    glyphs[],
                                      int         glyphCount) const {
    return fProxy->charsToGlyphs(chars, encoding, glyphs, glyphCount);
}

int SkRandomTypeface::onCountGlyphs() const { return fProxy->countGlyphs(); }

int SkRandomTypeface::onGetUPEM() const { return fProxy->getUnitsPerEm(); }

void SkRandomTypeface::onGetFamilyName(SkString* familyName) const {
    fProxy->getFamilyName(familyName);
}

SkTypeface::LocalizedStrings* SkRandomTypeface::onCreateFamilyNameIterator() const {
    return fProxy->createFamilyNameIterator();
}

int SkRandomTypeface::onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[],
        int                                            coordinateCount) const {
    return fProxy->onGetVariationDesignPosition(coordinates, coordinateCount);
}

int SkRandomTypeface::onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                                     int parameterCount) const {
    return fProxy->onGetVariationDesignParameters(parameters, parameterCount);
}

int SkRandomTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    return fProxy->getTableTags(tags);
}

size_t SkRandomTypeface::onGetTableData(SkFontTableTag tag,
                                        size_t         offset,
                                        size_t         length,
                                        void*          data) const {
    return fProxy->getTableData(tag, offset, length, data);
}
