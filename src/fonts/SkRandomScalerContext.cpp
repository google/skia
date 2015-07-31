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

class SkRandomScalerContext : public SkScalerContext {
public:
    SkRandomScalerContext(SkRandomTypeface*, const SkDescriptor*);
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
    SkMatrix         fMatrix;
};

#define STD_SIZE    1

#include "SkDescriptor.h"

SkRandomScalerContext::SkRandomScalerContext(SkRandomTypeface* face, const SkDescriptor* desc)
        : SkScalerContext(face, desc)
        , fFace(face) {
    size_t  descSize = SkDescriptor::ComputeOverhead(1) + sizeof(SkScalerContext::Rec);
    SkAutoDescriptor ad(descSize);
    SkDescriptor*    newDesc = ad.getDesc();

    newDesc->init();
    void* entry = newDesc->addEntry(kRec_SkDescriptorTag,
                                    sizeof(SkScalerContext::Rec), &fRec);
    {
        SkScalerContext::Rec* rec = (SkScalerContext::Rec*)entry;
        rec->fTextSize = STD_SIZE;
        rec->fPreScaleX = SK_Scalar1;
        rec->fPreSkewX = 0;
        rec->fPost2x2[0][0] = rec->fPost2x2[1][1] = SK_Scalar1;
        rec->fPost2x2[1][0] = rec->fPost2x2[0][1] = 0;
    }
    SkASSERT(descSize == newDesc->getLength());
    newDesc->computeChecksum();

    fProxy = face->proxy()->createScalerContext(newDesc);

    fRec.getSingleMatrix(&fMatrix);
    fMatrix.preScale(SK_Scalar1 / STD_SIZE, SK_Scalar1 / STD_SIZE);
}

SkRandomScalerContext::~SkRandomScalerContext() {
    SkDELETE(fProxy);
}

unsigned SkRandomScalerContext::generateGlyphCount() {
    return fProxy->getGlyphCount();
}

uint16_t SkRandomScalerContext::generateCharToGlyph(SkUnichar uni) {
    return fProxy->charToGlyphID(uni);
}

void SkRandomScalerContext::generateAdvance(SkGlyph* glyph) {
    fProxy->getAdvance(glyph);

    SkVector advance;
    fMatrix.mapXY(SkFixedToScalar(glyph->fAdvanceX),
                  SkFixedToScalar(glyph->fAdvanceY), &advance);
    glyph->fAdvanceX = SkScalarToFixed(advance.fX);
    glyph->fAdvanceY = SkScalarToFixed(advance.fY);
}

void SkRandomScalerContext::generateMetrics(SkGlyph* glyph) {
    fProxy->getAdvance(glyph);

    SkVector advance;
    fMatrix.mapXY(SkFixedToScalar(glyph->fAdvanceX),
                  SkFixedToScalar(glyph->fAdvanceY), &advance);
    glyph->fAdvanceX = SkScalarToFixed(advance.fX);
    glyph->fAdvanceY = SkScalarToFixed(advance.fY);

    SkPath path;
    fProxy->getPath(*glyph, &path);
    path.transform(fMatrix);

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

    // Here we will change the mask format of the glyph
    // NOTE this is being overridden by the base class
    SkMask::Format format;
    switch (glyph->getGlyphID() % 6) {
        case 0:
            format = SkMask::kLCD16_Format;
            break;
        case 1:
            format = SkMask::kA8_Format;
            break;
        case 2:
            format = SkMask::kARGB32_Format;
            break;
        default:
            // we will fiddle with these in generate image
            format = (SkMask::Format)MASK_FORMAT_UNKNOWN;
    }

    glyph->fMaskFormat = format;
}

void SkRandomScalerContext::generateImage(const SkGlyph& glyph) {
    SkMask::Format format = (SkMask::Format)glyph.fMaskFormat;
    switch (glyph.getGlyphID() % 6) {
        case 0:
        case 1:
        case 2:
            break;
        case 3:
            format = SkMask::kLCD16_Format;
            break;
        case 4:
            format = SkMask::kA8_Format;
            break;
        case 5:
            format = SkMask::kARGB32_Format;
            break;
    }
    const_cast<SkGlyph&>(glyph).fMaskFormat = format;

    // if the format is ARGB, we just draw the glyph from path ourselves.  Otherwise, we force
    // our proxy context to generate the image from paths.
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
        canvas.concat(fMatrix);
        canvas.drawPath(path, fFace->paint());
    } else {
        this->forceGenerateImageFromPath();
        fProxy->getImage(glyph);
        this->forceOffGenerateImageFromPath();
    }
}

void SkRandomScalerContext::generatePath(const SkGlyph& glyph, SkPath* path) {
    fProxy->getPath(glyph, path);
    path->transform(fMatrix);
}

void SkRandomScalerContext::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    fProxy->getFontMetrics(metrics);
    if (metrics) {
        SkScalar scale = fMatrix.getScaleY();
        metrics->fTop = SkScalarMul(metrics->fTop, scale);
        metrics->fAscent = SkScalarMul(metrics->fAscent, scale);
        metrics->fDescent = SkScalarMul(metrics->fDescent, scale);
        metrics->fBottom = SkScalarMul(metrics->fBottom, scale);
        metrics->fLeading = SkScalarMul(metrics->fLeading, scale);
        metrics->fAvgCharWidth = SkScalarMul(metrics->fAvgCharWidth, scale);
        metrics->fXMin = SkScalarMul(metrics->fXMin, scale);
        metrics->fXMax = SkScalarMul(metrics->fXMax, scale);
        metrics->fXHeight = SkScalarMul(metrics->fXHeight, scale);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkTypefaceCache.h"

SkRandomTypeface::SkRandomTypeface(SkTypeface* proxy, const SkPaint& paint)
    : SkTypeface(proxy->fontStyle(), SkTypefaceCache::NewFontID(), false)
    , fProxy(SkRef(proxy))
    , fPaint(paint) {}

SkRandomTypeface::~SkRandomTypeface() {
    fProxy->unref();
}

SkScalerContext* SkRandomTypeface::onCreateScalerContext(
                                            const SkDescriptor* desc) const {
    return SkNEW_ARGS(SkRandomScalerContext, (const_cast<SkRandomTypeface*>(this), desc));
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

