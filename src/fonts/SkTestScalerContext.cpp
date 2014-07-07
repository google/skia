/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDescriptor.h"
#include "SkGlyph.h"
#include "SkMask.h"
// #include "SkOTUtils.h"
#include "SkScalerContext.h"
#include "SkTestScalerContext.h"
#include "SkTypefaceCache.h"

class SkTestTypeface : public SkTypeface {
public:
    SkTestTypeface(SkPaint::FontMetrics (*funct)(SkTDArray<SkPath*>& , SkTDArray<SkFixed>& ),
                   SkTypeface::Style style) 
        : SkTypeface(style, SkTypefaceCache::NewFontID(), false) {
        fMetrics = (*funct)(fPaths, fWidths);
    }

    virtual ~SkTestTypeface() {
        fPaths.deleteAll();
    }

    void getAdvance(SkGlyph* glyph) {
        glyph->fAdvanceX = fWidths[SkGlyph::ID2Code(glyph->fID)];
        glyph->fAdvanceY = 0;
    }

    void getFontMetrics(SkPaint::FontMetrics* metrics) {
        *metrics = fMetrics;
    }

    void getMetrics(SkGlyph* glyph) {
        glyph->fAdvanceX = fWidths[SkGlyph::ID2Code(glyph->fID)];
        glyph->fAdvanceY = 0;
    }

    void getPath(const SkGlyph& glyph, SkPath* path) {
        *path = *fPaths[SkGlyph::ID2Code(glyph.fID)];
    }

protected:
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor* desc) const SK_OVERRIDE;

    virtual void onFilterRec(SkScalerContextRec* rec) const SK_OVERRIDE {
        rec->setHinting(SkPaint::kNo_Hinting);
        rec->fMaskFormat = SkMask::kA8_Format;
    }

    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                    SkAdvancedTypefaceMetrics::PerGlyphInfo ,
                                    const uint32_t* glyphIDs,
                                    uint32_t glyphIDsCount) const SK_OVERRIDE {
    // pdf only
        SkAdvancedTypefaceMetrics* info = new SkAdvancedTypefaceMetrics;
        info->fEmSize = 0;
        info->fLastGlyphID = SkToU16(onCountGlyphs() - 1);
        info->fStyle = 0;
        info->fFontName.set("SkiaTest");
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
        info->fItalicAngle = 0;
        info->fAscent = 0;
        info->fDescent = 0;
        info->fStemV = 0;
        info->fCapHeight = 0;
        info->fBBox = SkIRect::MakeEmpty();
        return info;
    }

    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE {
        SkASSERT(0);  // don't expect to get here
        return NULL;
    }

    virtual void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const SK_OVERRIDE {
        SkASSERT(0);  // don't expect to get here
    }

    virtual int onCharsToGlyphs(const void* chars, Encoding encoding,
                                uint16_t glyphs[], int glyphCount) const SK_OVERRIDE {
        SkASSERT(encoding == kUTF8_Encoding);
        for (int index = 0; index < glyphCount; ++index) {
            int ch = ((unsigned char*) chars)[index];
            SkASSERT(ch < 0x7F);
            if (ch < 0x20) {
                glyphs[index] = 0;
            } else {
                glyphs[index] = ch - 0x20;
            }
        }
        return glyphCount;
    }

    virtual int onCountGlyphs() const SK_OVERRIDE {
        return fPaths.count();
    }

    virtual int onGetUPEM() const SK_OVERRIDE {
        SkASSERT(0);  // don't expect to get here
        return 1;
    }

    virtual SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const SK_OVERRIDE {
        SkString familyName("SkiaTest");
        SkString language("und"); //undetermined
    SkASSERT(0);  // incomplete
        return NULL;
   //     return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
    }

    virtual int onGetTableTags(SkFontTableTag tags[]) const SK_OVERRIDE {
        return 0;
    }

    virtual size_t onGetTableData(SkFontTableTag tag, size_t offset,
                                  size_t length, void* data) const SK_OVERRIDE {
        return 0;
    }

private:
    SkTDArray<SkPath* > fPaths;
    SkTDArray<SkFixed> fWidths;
    SkPaint::FontMetrics fMetrics;
    friend class SkTestScalerContext;
};

SkTypeface* CreateTestTypeface(SkPaint::FontMetrics (*funct)(SkTDArray<SkPath*>& pathArray,
                               SkTDArray<SkFixed>& widthArray),
                               SkTypeface::Style style) {
    SkTypeface* test = SkNEW_ARGS(SkTestTypeface, (funct, style));
    return test;
}

class SkTestScalerContext : public SkScalerContext {
public:
    SkTestScalerContext(SkTestTypeface* face, const SkDescriptor* desc)
        : SkScalerContext(face, desc)
        , fFace(face)
    {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

    virtual ~SkTestScalerContext() {
    }

protected:
    virtual unsigned generateGlyphCount() SK_OVERRIDE {
        return fFace->onCountGlyphs();
    }

    virtual uint16_t generateCharToGlyph(SkUnichar uni) SK_OVERRIDE {
        uint8_t ch = (uint8_t) uni;
        SkASSERT(ch < 0x7f);
        uint16_t glyph;
        (void) fFace->onCharsToGlyphs((const void *) &ch, SkTypeface::kUTF8_Encoding, &glyph, 1);
        return glyph;
    }

    virtual void generateAdvance(SkGlyph* glyph) SK_OVERRIDE {
        fFace->getAdvance(glyph);

        SkVector advance;
        fMatrix.mapXY(SkFixedToScalar(glyph->fAdvanceX),
                      SkFixedToScalar(glyph->fAdvanceY), &advance);
        glyph->fAdvanceX = SkScalarToFixed(advance.fX);
        glyph->fAdvanceY = SkScalarToFixed(advance.fY);
    }

    virtual void generateMetrics(SkGlyph* glyph) SK_OVERRIDE {
        fFace->getMetrics(glyph);

        SkVector advance;
        fMatrix.mapXY(SkFixedToScalar(glyph->fAdvanceX),
                      SkFixedToScalar(glyph->fAdvanceY), &advance);
        glyph->fAdvanceX = SkScalarToFixed(advance.fX);
        glyph->fAdvanceY = SkScalarToFixed(advance.fY);

        SkPath path;
        fFace->getPath(*glyph, &path);
        path.transform(fMatrix);

        SkRect storage;
        const SkPaint paint;
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

    virtual void generateImage(const SkGlyph& glyph) SK_OVERRIDE {
        SkPath path;
        fFace->getPath(glyph, &path);

        SkBitmap bm;
        bm.installPixels(SkImageInfo::MakeN32Premul(glyph.fWidth, glyph.fHeight),
                            glyph.fImage, glyph.rowBytes());
        bm.eraseColor(0);

        SkCanvas canvas(bm);
        canvas.translate(-SkIntToScalar(glyph.fLeft),
                            -SkIntToScalar(glyph.fTop));
        canvas.concat(fMatrix);
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas.drawPath(path, paint);
    }

    virtual void generatePath(const SkGlyph& glyph, SkPath* path) SK_OVERRIDE {
        fFace->getPath(glyph, path);
        path->transform(fMatrix);
    }

    virtual void generateFontMetrics(SkPaint::FontMetrics* metrics) SK_OVERRIDE {
        fFace->getFontMetrics(metrics);
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

private:
    SkTestTypeface*  fFace;
    SkMatrix         fMatrix;
};

SkScalerContext* SkTestTypeface::onCreateScalerContext(const SkDescriptor* desc) const {
    return SkNEW_ARGS(SkTestScalerContext, (const_cast<SkTestTypeface*>(this), desc));
}
