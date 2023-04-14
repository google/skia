/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMetrics.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations.h"

namespace {
/* Placeholder glyph example until we extract paths through fontations, representing the capital H
 * from Roboto Regular. */
void drawCapitalH(SkPath* path) {
    path->setFillType(SkPathFillType::kWinding);
    path->moveTo(1096, -0);
    path->lineTo(1096, -673);
    path->lineTo(362, -673);
    path->lineTo(362, -0);
    path->lineTo(169, -0);
    path->lineTo(169, -1456);
    path->lineTo(362, -1456);
    path->lineTo(362, -830);
    path->lineTo(1096, -830);
    path->lineTo(1096, -1456);
    path->lineTo(1288, -1456);
    path->lineTo(1288, -0);
    path->lineTo(1096, -0);
    path->close();
}

constexpr SkScalar capitalHAdvance = 1461;
}

int SkTypeface_Fontations::onGetUPEM() const { return 2048; }

void SkTypeface_Fontations::onCharsToGlyphs(const SkUnichar* chars,
                                            int count,
                                            SkGlyphID glyphs[]) const {
    sk_bzero(glyphs, count * sizeof(glyphs[0]));
}

void SkTypeface_Fontations::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

class SkFontationsScalerContext : public SkScalerContext {
public:
    SkFontationsScalerContext(sk_sp<SkTypeface_Fontations> face,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

protected:

    bool generateAdvance(SkGlyph* glyph) override {
        const SkVector advance = fMatrix.mapXY(capitalHAdvance / getTypeface()->getUnitsPerEm(),
                                               SkFloatToScalar(0.f));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);
        return true;
    }

    void generateMetrics(SkGlyph* glyph, SkArenaAlloc*) override {
        glyph->fMaskFormat = fRec.fMaskFormat;
        glyph->zeroMetrics();
        this->generateAdvance(glyph);
        // Always generates from paths, so SkScalerContext::makeGlyph will figure the bounds.
    }

    void generateImage(const SkGlyph&) override { SK_ABORT("Should have generated from path."); }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        drawCapitalH(path);
        SkMatrix scaled = fMatrix.preScale(1.0f / getTypeface()->getUnitsPerEm(),
                                           1.0f / getTypeface()->getUnitsPerEm());
        *path = path->makeTransform(scaled);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        /* Hard-coded Roboto Regular metrics, to be replaced with Fontations calls. */
        metrics->fTop = -2163.f / getTypeface()->getUnitsPerEm();
        metrics->fAscent = -2146.f / getTypeface()->getUnitsPerEm();
        metrics->fDescent = 555.f / getTypeface()->getUnitsPerEm();
        metrics->fBottom = 555.f / getTypeface()->getUnitsPerEm();
        metrics->fLeading = 0;
        metrics->fAvgCharWidth = 0;
        metrics->fMaxCharWidth = 0;
        metrics->fXMin = -1825.f / getTypeface()->getUnitsPerEm();
        metrics->fXMax = 4188.f / getTypeface()->getUnitsPerEm();
        metrics->fXHeight = -1082.f / getTypeface()->getUnitsPerEm();
        metrics->fCapHeight = -1456.f / getTypeface()->getUnitsPerEm();
        metrics->fFlags = 0;

        SkFontPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix fMatrix;
};

std::unique_ptr<SkScalerContext> SkTypeface_Fontations::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkFontationsScalerContext>(
            sk_ref_sp(const_cast<SkTypeface_Fontations*>(this)), effects, desc);
}
