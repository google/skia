/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDescriptor.h"
#include "SkGScalerContext.h"
#include "SkGlyph.h"
#include "SkPaintPriv.h"
#include "SkPath.h"
#include "SkMakeUnique.h"

#define STD_SIZE    1

class SkGScalerContext : public SkScalerContext {
public:
    SkGScalerContext(sk_sp<SkGTypeface> face, const SkScalerContextEffects& effects,
                     const SkDescriptor* desc)
        : SkScalerContext(std::move(face), effects, desc)
    {

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

        fProxy = this->getGTypeface()->proxy()->createScalerContext(effects, newDesc);

        fRec.getSingleMatrix(&fMatrix);
        fMatrix.preScale(SK_Scalar1 / STD_SIZE, SK_Scalar1 / STD_SIZE);
    }

protected:
    SkGTypeface* getGTypeface() { return static_cast<SkGTypeface*>(this->getTypeface()); }

    unsigned generateGlyphCount() override;
    uint16_t generateCharToGlyph(SkUnichar) override;
    void generateAdvance(SkGlyph*) override;
    void generateMetrics(SkGlyph*) override;
    void generateImage(const SkGlyph&) override;
    void generatePath(SkGlyphID, SkPath*) override;
    void generateFontMetrics(SkPaint::FontMetrics*) override;

private:
    std::unique_ptr<SkScalerContext> fProxy;
    SkMatrix         fMatrix;
};

unsigned SkGScalerContext::generateGlyphCount() {
    return fProxy->getGlyphCount();
}

uint16_t SkGScalerContext::generateCharToGlyph(SkUnichar uni) {
    return fProxy->charToGlyphID(uni);
}

void SkGScalerContext::generateAdvance(SkGlyph* glyph) {
    fProxy->getAdvance(glyph);

    SkVector advance;
    fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                  SkFloatToScalar(glyph->fAdvanceY), &advance);
    glyph->fAdvanceX = SkScalarToFloat(advance.fX);
    glyph->fAdvanceY = SkScalarToFloat(advance.fY);
}

void SkGScalerContext::generateMetrics(SkGlyph* glyph) {
    fProxy->getMetrics(glyph);

    SkVector advance;
    fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                  SkFloatToScalar(glyph->fAdvanceY), &advance);
    glyph->fAdvanceX = SkScalarToFloat(advance.fX);
    glyph->fAdvanceY = SkScalarToFloat(advance.fY);

    SkPath path;
    fProxy->getPath(glyph->getPackedID(), &path);
    path.transform(fMatrix);

    SkRect storage;
    const SkPaint& paint = this->getGTypeface()->paint();
    const SkRect& newBounds = paint.doComputeFastBounds(path.getBounds(),
                                                        &storage,
                                                        SkPaint::kFill_Style);
    SkIRect ibounds;
    newBounds.roundOut(&ibounds);
    glyph->fLeft = ibounds.fLeft;
    glyph->fTop = ibounds.fTop;
    glyph->fWidth = ibounds.width();
    glyph->fHeight = ibounds.height();
    glyph->fMaskFormat = SkMask::kARGB32_Format;
}

void SkGScalerContext::generateImage(const SkGlyph& glyph) {
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        SkPath path;
        fProxy->getPath(glyph.getPackedID(), &path);

        SkBitmap bm;
        bm.installPixels(SkImageInfo::MakeN32Premul(glyph.fWidth, glyph.fHeight),
                         glyph.fImage, glyph.rowBytes());
        bm.eraseColor(0);

        SkCanvas canvas(bm);
        canvas.translate(-SkIntToScalar(glyph.fLeft),
                         -SkIntToScalar(glyph.fTop));
        canvas.concat(fMatrix);
        canvas.drawPath(path, this->getGTypeface()->paint());
    } else {
        fProxy->getImage(glyph);
    }
}

void SkGScalerContext::generatePath(SkGlyphID glyph, SkPath* path) {
    fProxy->getPath(SkPackedGlyphID(glyph), path);
    path->transform(fMatrix);
}

void SkGScalerContext::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    fProxy->getFontMetrics(metrics);
    SkPaintPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
}

///////////////////////////////////////////////////////////////////////////////

#include "SkTypefaceCache.h"

SkGTypeface::SkGTypeface(sk_sp<SkTypeface> proxy, const SkPaint& paint)
    : SkTypeface(proxy->fontStyle(), false)
    , fProxy(std::move(proxy))
    , fPaint(paint)
{}

SkScalerContext* SkGTypeface::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                    const SkDescriptor* desc) const {
    return new SkGScalerContext(sk_ref_sp(const_cast<SkGTypeface*>(this)), effects, desc);
}

void SkGTypeface::onFilterRec(SkScalerContextRec* rec) const {
    fProxy->filterRec(rec);
    rec->setHinting(SkPaint::kNo_Hinting);
    rec->fMaskFormat = SkMask::kARGB32_Format;
}

SkAdvancedTypefaceMetrics* SkGTypeface::onGetAdvancedTypefaceMetrics(
                                PerGlyphInfo info,
                                const uint32_t* glyphIDs,
                                uint32_t glyphIDsCount) const {
    return fProxy->getAdvancedTypefaceMetrics(info, glyphIDs, glyphIDsCount);
}

SkStreamAsset* SkGTypeface::onOpenStream(int* ttcIndex) const {
    return fProxy->openStream(ttcIndex);
}

void SkGTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                      bool* isLocal) const {
    fProxy->getFontDescriptor(desc, isLocal);
}

int SkGTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                                 uint16_t glyphs[], int glyphCount) const {
    return fProxy->charsToGlyphs(chars, encoding, glyphs, glyphCount);
}

int SkGTypeface::onCountGlyphs() const {
    return fProxy->countGlyphs();
}

int SkGTypeface::onGetUPEM() const {
    return fProxy->getUnitsPerEm();
}

void SkGTypeface::onGetFamilyName(SkString* familyName) const {
    fProxy->getFamilyName(familyName);
}

SkTypeface::LocalizedStrings* SkGTypeface::onCreateFamilyNameIterator() const {
    return fProxy->createFamilyNameIterator();
}

int SkGTypeface::onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const
{
    return fProxy->onGetVariationDesignPosition(coordinates, coordinateCount);
}

int SkGTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    return fProxy->getTableTags(tags);
}

size_t SkGTypeface::onGetTableData(SkFontTableTag tag, size_t offset,
                                    size_t length, void* data) const {
    return fProxy->getTableData(tag, offset, length, data);
}

///////////////////////////////////////////////////////////////////////////////

#if 0
// under construction -- defining a font purely in terms of skia primitives
// ala an SVG-font.
class SkGFont : public SkRefCnt {
public:
    virtual ~SkGFont();

    int unicharToGlyph(SkUnichar) const;

    int countGlyphs() const { return fCount; }

    float getAdvance(int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return fGlyphs[index].fAdvance;
    }

    const SkPath& getPath(int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return fGlyphs[index].fPath;
    }

private:
    struct Glyph {
        SkUnichar   fUni;
        float       fAdvance;
        SkPath      fPath;
    };
    int fCount;
    Glyph* fGlyphs;

    friend class SkGFontBuilder;
    SkGFont(int count, Glyph* array);
};

class SkGFontBuilder {
public:

};
#endif
