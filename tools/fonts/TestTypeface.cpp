/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkUtils.h"
#include "src/sfnt/SkOTUtils.h"
#include "tools/fonts/TestTypeface.h"

#include <utility>

class SkDescriptor;

SkTestFont::SkTestFont(const SkTestFontData& fontData)
        : INHERITED()
        , fCharCodes(fontData.fCharCodes)
        , fCharCodesCount(fontData.fCharCodes ? fontData.fCharCodesCount : 0)
        , fWidths(fontData.fWidths)
        , fMetrics(fontData.fMetrics)
        , fName(fontData.fName)
        , fPaths(nullptr) {
    init(fontData.fPoints, fontData.fVerbs);
}

SkTestFont::~SkTestFont() {
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        delete fPaths[index];
    }
    delete[] fPaths;
}

SkGlyphID SkTestFont::glyphForUnichar(SkUnichar charCode) const {
    for (size_t index = 0; index < fCharCodesCount; ++index) {
        if (fCharCodes[index] == charCode) {
            return SkTo<SkGlyphID>(index);
        }
    }
    return 0;
}

void SkTestFont::init(const SkScalar* pts, const unsigned char* verbs) {
    fPaths = new SkPath*[fCharCodesCount];
    for (unsigned index = 0; index < fCharCodesCount; ++index) {
        SkPath*      path = new SkPath;
        SkPath::Verb verb;
        while ((verb = (SkPath::Verb)*verbs++) != SkPath::kDone_Verb) {
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
                case SkPath::kClose_Verb: path->close(); break;
                default: SkDEBUGFAIL("bad verb"); return;
            }
        }
        // This should make SkPath::getBounds() queries threadsafe.
        path->updateBoundsCache();
        fPaths[index] = path;
    }
}

TestTypeface::TestTypeface(sk_sp<SkTestFont> testFont, const SkFontStyle& style)
        : SkTypeface(style, false), fTestFont(std::move(testFont)) {}

void TestTypeface::getAdvance(SkGlyph* glyph) {
    SkGlyphID glyphID = glyph->getGlyphID();
    glyphID           = glyphID < fTestFont->fCharCodesCount ? glyphID : 0;

    // TODO(benjaminwagner): Update users to use floats.
    glyph->fAdvanceX = SkFixedToFloat(fTestFont->fWidths[glyphID]);
    glyph->fAdvanceY = 0;
}

void TestTypeface::getFontMetrics(SkFontMetrics* metrics) { *metrics = fTestFont->fMetrics; }

void TestTypeface::getPath(SkGlyphID glyphID, SkPath* path) {
    glyphID = glyphID < fTestFont->fCharCodesCount ? glyphID : 0;
    *path   = *fTestFont->fPaths[glyphID];
}

void TestTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(kNo_SkFontHinting);
}

void TestTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    unsigned glyphCount = fTestFont->fCharCodesCount;
    for (unsigned gid = 0; gid < glyphCount; ++gid) {
        glyphToUnicode[gid] = SkTo<SkUnichar>(fTestFont->fCharCodes[gid]);
    }
}

std::unique_ptr<SkAdvancedTypefaceMetrics> TestTypeface::onGetAdvancedMetrics() const {  // pdf only
    std::unique_ptr<SkAdvancedTypefaceMetrics>info(new SkAdvancedTypefaceMetrics);
    info->fFontName.set(fTestFont->fName);
    return info;
}

void TestTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    desc->setFamilyName(fTestFont->fName);
    desc->setStyle(this->fontStyle());
    *isLocal = false;
}

void TestTypeface::onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    for (int i = 0; i < count; ++i) {
        glyphs[i] = fTestFont->glyphForUnichar(uni[i]);
    }
}

void TestTypeface::onGetFamilyName(SkString* familyName) const { *familyName = fTestFont->fName; }

SkTypeface::LocalizedStrings* TestTypeface::onCreateFamilyNameIterator() const {
    SkString familyName(fTestFont->fName);
    SkString language("und");  // undetermined
    return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
}

class SkTestScalerContext : public SkScalerContext {
public:
    SkTestScalerContext(sk_sp<TestTypeface>           face,
                        const SkScalerContextEffects& effects,
                        const SkDescriptor*           desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

protected:
    TestTypeface* getTestTypeface() const {
        return static_cast<TestTypeface*>(this->getTypeface());
    }

    unsigned generateGlyphCount() override { return this->getTestTypeface()->onCountGlyphs(); }

    bool generateAdvance(SkGlyph* glyph) override {
        this->getTestTypeface()->getAdvance(glyph);

        const SkVector advance =
                fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX), SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);
        return true;
    }

    void generateMetrics(SkGlyph* glyph) override {
        glyph->zeroMetrics();
        this->generateAdvance(glyph);
        // Always generates from paths, so SkScalerContext::getMetrics will figure the bounds.
    }

    void generateImage(const SkGlyph&) override { SK_ABORT("Should have generated from path."); }

    bool generatePath(SkGlyphID glyph, SkPath* path) override {
        this->getTestTypeface()->getPath(glyph, path);
        path->transform(fMatrix);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        this->getTestTypeface()->getFontMetrics(metrics);
        SkFontPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix fMatrix;
};

SkScalerContext* TestTypeface::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                     const SkDescriptor*           desc) const {
    return new SkTestScalerContext(sk_ref_sp(const_cast<TestTypeface*>(this)), effects, desc);
}
