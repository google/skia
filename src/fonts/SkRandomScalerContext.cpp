/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRandomScalerContext.h"
#include "SkGlyph.h"
#include "SkPath.h"
#include "SkCanvas.h"
#include "SkRasterizer.h"

class SkRandomScalerContext : public SkScalerContext {
public:
    SkRandomScalerContext(SkRandomTypeface*, const SkDescriptor*, bool fFakeIt);
    virtual ~SkRandomScalerContext();

protected:
    unsigned generateGlyphCount() override;
    uint16_t generateCharToGlyph(SkUnichar) override;
    void generateAdvance(SkGlyph*) override;
    void generateMetrics(SkGlyph*) override;
    void generateImage(const SkGlyph&) override;
    void generatePath(const SkGlyph&, SkPath*) override;
    void generateFontMetrics(SkPaint::FontMetrics*) override;

private:
    SkRandomTypeface*     fFace;
    SkScalerContext* fProxy;
    bool fFakeIt;
};

#define STD_SIZE    1

#include "SkDescriptor.h"

SkRandomScalerContext::SkRandomScalerContext(SkRandomTypeface* face, const SkDescriptor* desc,
                                             bool fakeIt)
        : SkScalerContext(face, desc)
        , fFace(face)
        , fFakeIt(fakeIt) {
    fProxy = face->proxy()->createScalerContext(desc);
}

SkRandomScalerContext::~SkRandomScalerContext() { delete fProxy; }

unsigned SkRandomScalerContext::generateGlyphCount() {
    return fProxy->getGlyphCount();
}

uint16_t SkRandomScalerContext::generateCharToGlyph(SkUnichar uni) {
    return fProxy->charToGlyphID(uni);
}

void SkRandomScalerContext::generateAdvance(SkGlyph* glyph) {
    fProxy->getAdvance(glyph);
}

void SkRandomScalerContext::generateMetrics(SkGlyph* glyph) {
    // Here we will change the mask format of the glyph
    // NOTE this is being overridden by the base class
    SkMask::Format format;
    switch (glyph->getGlyphID() % 4) {
        case 0:
            format = SkMask::kLCD16_Format;
            break;
        case 1:
            format = SkMask::kA8_Format;
            break;
        case 2:
            format = SkMask::kARGB32_Format;
            break;
        case 3:
            format = SkMask::kBW_Format;
            break;
    }

    fProxy->getMetrics(glyph);

    glyph->fMaskFormat = format;
    if (fFakeIt) {
        return;
    }
    if (SkMask::kARGB32_Format == format) {
        SkPath path;
        fProxy->getPath(*glyph, &path);

        SkRect storage;
        const SkPaint& paint = fFace->paint();
        const SkRect& newBounds = paint.doComputeFastBounds(path.getBounds(),
                                                            &storage,
                                                            SkPaint::kFill_Style);
        SkIRect ibounds;
        newBounds.roundOut(&ibounds);
        glyph->fLeft = ibounds.fLeft;
        glyph->fTop = ibounds.fTop;
        glyph->fWidth = ibounds.width();
        glyph->fHeight = ibounds.height();
    } else {
        SkPath      devPath, fillPath;
        SkMatrix    fillToDevMatrix;

        this->internalGetPath(*glyph, &fillPath, &devPath, &fillToDevMatrix);

        // just use devPath
        const SkIRect ir = devPath.getBounds().roundOut();

        if (ir.isEmpty() || !ir.is16Bit()) {
            glyph->fLeft    = 0;
            glyph->fTop     = 0;
            glyph->fWidth   = 0;
            glyph->fHeight  = 0;
            return;
        }
        glyph->fLeft    = ir.fLeft;
        glyph->fTop     = ir.fTop;
        glyph->fWidth   = SkToU16(ir.width());
        glyph->fHeight  = SkToU16(ir.height());

        if (glyph->fWidth > 0) {
            switch (glyph->fMaskFormat) {
            case SkMask::kLCD16_Format:
                glyph->fWidth += 2;
                glyph->fLeft -= 1;
                break;
            default:
                break;
            }
        }
    }
}

void SkRandomScalerContext::generateImage(const SkGlyph& glyph) {
    SkMask::Format format = (SkMask::Format)glyph.fMaskFormat;
    switch (glyph.getGlyphID() % 4) {
        case 0:
            format = SkMask::kLCD16_Format;
            break;
        case 1:
            format = SkMask::kA8_Format;
            break;
        case 2:
            format = SkMask::kARGB32_Format;
            break;
        case 3:
            format = SkMask::kBW_Format;
            break;
    }
    const_cast<SkGlyph&>(glyph).fMaskFormat = format;

    // if the format is ARGB, we just draw the glyph from path ourselves.  Otherwise, we force
    // our proxy context to generate the image from paths.
    if (!fFakeIt) {
        if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
            SkPath path;
            fProxy->getPath(glyph, &path);

            SkBitmap bm;
            bm.installPixels(SkImageInfo::MakeN32Premul(glyph.fWidth, glyph.fHeight),
                             glyph.fImage, glyph.rowBytes());
            bm.eraseColor(0);

            SkCanvas canvas(bm);
            canvas.translate(-SkIntToScalar(glyph.fLeft),
                             -SkIntToScalar(glyph.fTop));
            canvas.drawPath(path, fFace->paint());
        } else {
            fProxy->forceGenerateImageFromPath();
            fProxy->getImage(glyph);
            fProxy->forceOffGenerateImageFromPath();
        }
    } else {
        sk_bzero(glyph.fImage, glyph.computeImageSize());
    }
}

void SkRandomScalerContext::generatePath(const SkGlyph& glyph, SkPath* path) {
    fProxy->getPath(glyph, path);
}

void SkRandomScalerContext::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    fProxy->getFontMetrics(metrics);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkTypefaceCache.h"

SkRandomTypeface::SkRandomTypeface(SkTypeface* proxy, const SkPaint& paint, bool fakeIt)
    : SkTypeface(proxy->fontStyle(), SkTypefaceCache::NewFontID(), false)
    , fProxy(SkRef(proxy))
    , fPaint(paint)
    , fFakeIt(fakeIt) {}

SkRandomTypeface::~SkRandomTypeface() {
    fProxy->unref();
}

SkScalerContext* SkRandomTypeface::onCreateScalerContext(
                                            const SkDescriptor* desc) const {
    return new SkRandomScalerContext(const_cast<SkRandomTypeface*>(this), desc, fFakeIt);
}

void SkRandomTypeface::onFilterRec(SkScalerContextRec* rec) const {
    fProxy->filterRec(rec);
    rec->setHinting(SkPaint::kNo_Hinting);
    rec->fMaskFormat = SkMask::kARGB32_Format;
}

SkAdvancedTypefaceMetrics* SkRandomTypeface::onGetAdvancedTypefaceMetrics(
                                PerGlyphInfo info,
                                const uint32_t* glyphIDs,
                                uint32_t glyphIDsCount) const {
    return fProxy->getAdvancedTypefaceMetrics(info, glyphIDs, glyphIDsCount);
}

SkStreamAsset* SkRandomTypeface::onOpenStream(int* ttcIndex) const {
    return fProxy->openStream(ttcIndex);
}

void SkRandomTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                      bool* isLocal) const {
    fProxy->getFontDescriptor(desc, isLocal);
}

int SkRandomTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                                 uint16_t glyphs[], int glyphCount) const {
    return fProxy->charsToGlyphs(chars, encoding, glyphs, glyphCount);
}

int SkRandomTypeface::onCountGlyphs() const {
    return fProxy->countGlyphs();
}

int SkRandomTypeface::onGetUPEM() const {
    return fProxy->getUnitsPerEm();
}

void SkRandomTypeface::onGetFamilyName(SkString* familyName) const {
    fProxy->getFamilyName(familyName);
}

SkTypeface::LocalizedStrings* SkRandomTypeface::onCreateFamilyNameIterator() const {
    return fProxy->createFamilyNameIterator();
}

int SkRandomTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    return fProxy->getTableTags(tags);
}

size_t SkRandomTypeface::onGetTableData(SkFontTableTag tag, size_t offset,
                                    size_t length, void* data) const {
    return fProxy->getTableData(tag, offset, length, data);
}

