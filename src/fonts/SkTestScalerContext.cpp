/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDescriptor.h"
#include "SkFontDescriptor.h"
#include "SkGlyph.h"
#include "SkMask.h"
// #include "SkOTUtils.h"
#include "SkScalerContext.h"
#include "SkTestScalerContext.h"
#include "SkTypefaceCache.h"

SkTestFont::SkTestFont(const SkTestFontData& fontData)
    : INHERITED()
    , fCharCodes(fontData.fCharCodes)
    , fCharCodesCount(fontData.fCharCodesCount)
    , fWidths(fontData.fWidths)
    , fMetrics(fontData.fMetrics)
    , fName(fontData.fName)
    , fPaths(nullptr)
{
    init(fontData.fPoints, fontData.fVerbs);
#ifdef SK_DEBUG
    sk_bzero(fDebugBits, sizeof(fDebugBits));
    sk_bzero(fDebugOverage, sizeof(fDebugOverage));
#endif
}

SkTestFont::~SkTestFont() {
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        delete fPaths[index];
    }
    delete[] fPaths;
}

#ifdef SK_DEBUG

#include "SkMutex.h"
SK_DECLARE_STATIC_MUTEX(gUsedCharsMutex);

#endif

int SkTestFont::codeToIndex(SkUnichar charCode) const {
#ifdef SK_DEBUG  // detect missing test font data
    {
        SkAutoMutexAcquire ac(gUsedCharsMutex);
        if (charCode >= ' ' && charCode <= '~') {
            int bitOffset = charCode - ' ';
            fDebugBits[bitOffset >> 3] |= 1 << (bitOffset & 7);
        } else {
            int index = 0;
            while (fDebugOverage[index] != 0 && fDebugOverage[index] != charCode
                    && index < (int) sizeof(fDebugOverage)) {
                ++index;
            }
            SkASSERT(index < (int) sizeof(fDebugOverage));
            if (fDebugOverage[index] == 0) {
                fDebugOverage[index] = charCode;
            }
        }
    }
#endif
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        if (fCharCodes[index] == (unsigned) charCode) {
            return (int) index;
        }
    }
    SkDEBUGF(("missing '%c' (%d) from %s %d\n", (char) charCode, charCode,
            fDebugName, fDebugStyle));
    return 0;
}

void SkTestFont::init(const SkScalar* pts, const unsigned char* verbs) {
    fPaths = new SkPath* [fCharCodesCount];
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        SkPath* path = new SkPath;
        SkPath::Verb verb;
        while ((verb = (SkPath::Verb) *verbs++) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                    path->moveTo(pts[0], pts[1]);
                    pts += 2;
                    break;
                case SkPath::kLine_Verb:
                    path->lineTo(pts[0], pts[1]);
                    pts += 2;
                    break;
                case SkPath::kQuad_Verb:
                    path->quadTo(pts[0], pts[1], pts[2], pts[3]);
                    pts += 4;
                    break;
                case SkPath::kCubic_Verb:
                    path->cubicTo(pts[0], pts[1], pts[2], pts[3], pts[4], pts[5]);
                    pts += 6;
                    break;
                case SkPath::kClose_Verb:
                    path->close();
                    break;
                default:
                    SkDEBUGFAIL("bad verb");
                    return;
            }
        }
        // This should make SkPath::getBounds() queries threadsafe.
        path->updateBoundsCache();
        fPaths[index] = path;
    }
}

SkTestTypeface::SkTestTypeface(SkTestFont* testFont, const SkFontStyle& style)
    : SkTypeface(style, SkTypefaceCache::NewFontID(), false)
    , fTestFont(testFont) {
}

void SkTestTypeface::getAdvance(SkGlyph* glyph) {
    // TODO(benjaminwagner): Update users to use floats.
    glyph->fAdvanceX = SkFixedToFloat(fTestFont->fWidths[glyph->getGlyphID()]);
    glyph->fAdvanceY = 0;
}

void SkTestTypeface::getFontMetrics(SkPaint::FontMetrics* metrics) {
    *metrics = fTestFont->fMetrics;
}

void SkTestTypeface::getMetrics(SkGlyph* glyph) {
    // TODO(benjaminwagner): Update users to use floats.
    glyph->fAdvanceX = SkFixedToFloat(fTestFont->fWidths[glyph->getGlyphID()]);
    glyph->fAdvanceY = 0;
}

void SkTestTypeface::getPath(const SkGlyph& glyph, SkPath* path) {
    *path = *fTestFont->fPaths[glyph.getGlyphID()];
}

void SkTestTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkPaint::kNo_Hinting);
}

SkAdvancedTypefaceMetrics* SkTestTypeface::onGetAdvancedTypefaceMetrics(
                                PerGlyphInfo ,
                                const uint32_t* glyphIDs,
                                uint32_t glyphIDsCount) const {
// pdf only
    SkAdvancedTypefaceMetrics* info = new SkAdvancedTypefaceMetrics;
    info->fFontName.set(fTestFont->fName);
    info->fLastGlyphID = SkToU16(onCountGlyphs() - 1);
    return info;
}

void SkTestTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    desc->setFamilyName(fTestFont->fName);
    *isLocal = false;
}

int SkTestTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                            uint16_t glyphs[], int glyphCount) const {
    SkASSERT(encoding == kUTF16_Encoding);
    for (int index = 0; index < glyphCount; ++index) {
        SkUnichar ch = ((SkUnichar*) chars)[index];
        glyphs[index] = fTestFont->codeToIndex(ch);
    }
    return glyphCount;
}

void SkTestTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = fTestFont->fName;
}

SkTypeface::LocalizedStrings* SkTestTypeface::onCreateFamilyNameIterator() const {
    SkString familyName(fTestFont->fName);
    SkString language("und"); //undetermined
SkASSERT(0);  // incomplete
    return nullptr;
//     return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
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
    unsigned generateGlyphCount() override {
        return fFace->onCountGlyphs();
    }

    uint16_t generateCharToGlyph(SkUnichar uni) override {
        uint16_t glyph;
        (void) fFace->onCharsToGlyphs((const void *) &uni, SkTypeface::kUTF16_Encoding, &glyph, 1);
        return glyph;
    }

    void generateAdvance(SkGlyph* glyph) override {
        fFace->getAdvance(glyph);

        const SkVector advance = fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                                               SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);
    }

    void generateMetrics(SkGlyph* glyph) override {
        fFace->getMetrics(glyph);

        const SkVector advance = fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX),
                                               SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);

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
    }

    void generateImage(const SkGlyph& glyph) override {
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

    void generatePath(const SkGlyph& glyph, SkPath* path) override {
        fFace->getPath(glyph, path);
        path->transform(fMatrix);
    }

    void generateFontMetrics(SkPaint::FontMetrics* metrics) override {
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
    return new SkTestScalerContext(const_cast<SkTestTypeface*>(this), desc);
}
