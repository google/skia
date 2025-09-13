/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/fonts/RandomScalerContext.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPath.h"
#include "include/core/SkStream.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkTHash.h"

using namespace skia_private;

class SkDescriptor;

class RandomScalerContext : public SkScalerContext {
public:
    RandomScalerContext(SkRandomTypeface&,
                        const SkScalerContextEffects&,
                        const SkDescriptor*,
                        bool fFakeIt);

protected:
    GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override;
    void     generateImage(const SkGlyph&, void*) override;
    std::optional<GeneratedPath> generatePath(const SkGlyph&) override;
    sk_sp<SkDrawable> generateDrawable(const SkGlyph&) override;
    void     generateFontMetrics(SkFontMetrics*) override;

private:
    SkRandomTypeface* getRandomTypeface() const {
        return static_cast<SkRandomTypeface*>(this->getTypeface());
    }
    std::unique_ptr<SkScalerContext>   fProxy;
    // Many of the SkGlyphs returned are the same as those created by the fProxy.
    // When they are not, the originals are kept here.
    THashMap<SkPackedGlyphID, SkGlyph> fProxyGlyphs;
    bool                               fFakeIt;
};

RandomScalerContext::RandomScalerContext(SkRandomTypeface& face,
                                         const SkScalerContextEffects& effects,
                                         const SkDescriptor* desc,
                                         bool fakeIt)
        : SkScalerContext(face, effects, desc)
        , fProxy(getRandomTypeface()->proxy()->createScalerContext(SkScalerContextEffects(), desc))
        , fFakeIt(fakeIt) {}

SkScalerContext::GlyphMetrics RandomScalerContext::generateMetrics(const SkGlyph& origGlyph,
                                                                   SkArenaAlloc* alloc) {
    // Here we will change the mask format of the glyph
    // NOTE: this may be overridden by the base class (e.g. if a mask filter is applied).
    SkMask::Format format = SkMask::kA8_Format;
    switch (origGlyph.getGlyphID() % 4) {
        case 0: format = SkMask::kLCD16_Format; break;
        case 1: format = SkMask::kA8_Format; break;
        case 2: format = SkMask::kARGB32_Format; break;
        case 3: format = SkMask::kBW_Format; break;
    }

    auto glyph = fProxy->internalMakeGlyph(origGlyph.getPackedID(), format, alloc);

    GlyphMetrics mx(SkMask::kA8_Format);
    mx.advance = glyph.advanceVector();
    mx.bounds = glyph.rect();
    mx.maskFormat = glyph.maskFormat();
    mx.extraBits = glyph.extraBits();

    if (fFakeIt || (glyph.getGlyphID() % 4) != 2) {
        mx.neverRequestPath = glyph.setPathHasBeenCalled() && !glyph.path();
        mx.computeFromPath = !mx.neverRequestPath;
        return mx;
    }

    fProxy->getPath(glyph, alloc);
    if (!glyph.path()) {
        mx.neverRequestPath = true;
        return mx;
    }

    // The proxy glyph has a path, but this glyph does not.
    // Stash the proxy glyph so it can be used later.
    const auto packedID = glyph.getPackedID();
    const SkGlyph* proxyGlyph = fProxyGlyphs.set(packedID, std::move(glyph));
    const SkPath& proxyPath = *proxyGlyph->path();

    mx.neverRequestPath = true;
    mx.maskFormat = SkMask::kARGB32_Format;
    mx.advance = proxyGlyph->advanceVector();
    mx.extraBits = proxyGlyph->extraBits();

    SkRect         storage;
    const SkPaint& paint = this->getRandomTypeface()->paint();
    const SkRect&  newBounds =
            paint.doComputeFastBounds(proxyPath.getBounds(), &storage, SkPaint::kFill_Style);
    newBounds.roundOut(&mx.bounds);

    return mx;
}

void RandomScalerContext::generateImage(const SkGlyph& glyph, void* imageBuffer) {
    if (fFakeIt) {
        sk_bzero(imageBuffer, glyph.imageSize());
        return;
    }

    SkGlyph* proxyGlyph = fProxyGlyphs.find(glyph.getPackedID());
    if (!proxyGlyph || !proxyGlyph->path()) {
        fProxy->getImage(glyph);
        return;
    }
    const SkPath& path = *proxyGlyph->path();
    const bool hairline = proxyGlyph->pathIsHairline();

    SkBitmap bm;
    bm.installPixels(SkImageInfo::MakeN32Premul(glyph.width(), glyph.height()),
                     imageBuffer, glyph.rowBytes());
    bm.eraseColor(0);

    SkCanvas canvas(bm);
    canvas.translate(-SkIntToScalar(glyph.left()), -SkIntToScalar(glyph.top()));
    SkPaint paint = this->getRandomTypeface()->paint();
    if (hairline) {
        // We have a device path with effects already applied which is normally a fill path.
        // However here we do not have a fill path and there is no area to fill.
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStroke(0);
    }
    canvas.drawPath(path, paint); //Need to modify the paint if the devPath is hairline
}

std::optional<SkScalerContext::GeneratedPath>
RandomScalerContext::generatePath(const SkGlyph& glyph) {
    SkGlyph* shadowProxyGlyph = fProxyGlyphs.find(glyph.getPackedID());
    if (shadowProxyGlyph && shadowProxyGlyph->path()) {
        return {};
    }
    return fProxy->generatePath(glyph);
}

sk_sp<SkDrawable> RandomScalerContext::generateDrawable(const SkGlyph& glyph) {
    SkGlyph* shadowProxyGlyph = fProxyGlyphs.find(glyph.getPackedID());
    if (shadowProxyGlyph && shadowProxyGlyph->path()) {
        return nullptr;
    }
    return fProxy->generateDrawable(glyph);
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

std::unique_ptr<SkScalerContext> SkRandomTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<RandomScalerContext>(
            *const_cast<SkRandomTypeface*>(this), effects, desc, fFakeIt);
}

void SkRandomTypeface::onFilterRec(SkScalerContextRec* rec) const {
    fProxy->filterRec(rec);
    rec->setHinting(SkFontHinting::kNone);
    rec->fMaskFormat = SkMask::kARGB32_Format;
}

void SkRandomTypeface::getGlyphToUnicodeMap(SkSpan<SkUnichar> glyphToUnicode) const {
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

void SkRandomTypeface::onCharsToGlyphs(SkSpan<const SkUnichar> uni,
                                       SkSpan<SkGlyphID> glyphs) const {
    fProxy->unicharsToGlyphs(uni, glyphs);
}

int SkRandomTypeface::onCountGlyphs() const { return fProxy->countGlyphs(); }

int SkRandomTypeface::onGetUPEM() const { return fProxy->getUnitsPerEm(); }

void SkRandomTypeface::onGetFamilyName(SkString* familyName) const {
    fProxy->getFamilyName(familyName);
}

bool SkRandomTypeface::onGetPostScriptName(SkString* postScriptName) const {
    return fProxy->getPostScriptName(postScriptName);
}

SkTypeface::LocalizedStrings* SkRandomTypeface::onCreateFamilyNameIterator() const {
    return fProxy->createFamilyNameIterator();
}

void SkRandomTypeface::getPostScriptGlyphNames(SkString* names) const {
    return fProxy->getPostScriptGlyphNames(names);
}

bool SkRandomTypeface::onGlyphMaskNeedsCurrentColor() const {
    return fProxy->glyphMaskNeedsCurrentColor();
}

int SkRandomTypeface::onGetVariationDesignPosition(
                       SkSpan<SkFontArguments::VariationPosition::Coordinate> coordinates) const {
    return fProxy->onGetVariationDesignPosition(coordinates);
}

int SkRandomTypeface::onGetVariationDesignParameters(
                                     SkSpan<SkFontParameters::Variation::Axis> parameters) const {
    return fProxy->onGetVariationDesignParameters(parameters);
}

int SkRandomTypeface::onGetTableTags(SkSpan<SkFontTableTag> tags) const {
    return fProxy->readTableTags(tags);
}

size_t SkRandomTypeface::onGetTableData(SkFontTableTag tag,
                                        size_t         offset,
                                        size_t         length,
                                        void*          data) const {
    return fProxy->getTableData(tag, offset, length, data);
}
