/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/fonts/TestSVGTypeface.h"

#if defined(SK_ENABLE_SVG)

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkUtils.h"
#include "src/sfnt/SkOTUtils.h"
#include "tools/Resources.h"

#include <utility>

class SkDescriptor;

TestSVGTypeface::TestSVGTypeface(const char*                              name,
                                 int                                      upem,
                                 const SkFontMetrics&                     fontMetrics,
                                 SkSpan<const SkSVGTestTypefaceGlyphData> data,
                                 const SkFontStyle&                       style)
        : SkTypeface(style, false)
        , fName(name)
        , fUpem(upem)
        , fFontMetrics(fontMetrics)
        , fGlyphs(new Glyph[data.size()])
        , fGlyphCount(data.size()) {
    for (size_t i = 0; i < data.size(); ++i) {
        const SkSVGTestTypefaceGlyphData& datum  = data[i];
        fCMap.set(datum.fUnicode, i);
        fGlyphs[i].fAdvance      = datum.fAdvance;
        fGlyphs[i].fOrigin       = datum.fOrigin;
        fGlyphs[i].fResourcePath = datum.fSvgResourcePath;
    }
}

template <typename Fn>
void TestSVGTypeface::Glyph::withSVG(Fn&& fn) const {
    SkAutoMutexExclusive lock(fSvgMutex);

    if (!fParsedSvg) {
        fParsedSvg = true;

        std::unique_ptr<SkStreamAsset> stream = GetResourceAsStream(fResourcePath);
        if (!stream) {
            return;
        }

        sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromStream(*stream);
        if (!svg) {
            return;
        }

        if (svg->containerSize().isEmpty()) {
            return;
        }

        fSvg = std::move(svg);
    }

    if (fSvg) {
        fn(*fSvg);
    }
}

SkSize TestSVGTypeface::Glyph::size() const {
    SkSize size = SkSize::MakeEmpty();
    this->withSVG([&](const SkSVGDOM& svg){
        size = svg.containerSize();
    });
    return size;
}

void TestSVGTypeface::Glyph::render(SkCanvas* canvas) const {
    this->withSVG([&](const SkSVGDOM& svg){
        svg.render(canvas);
    });
}

TestSVGTypeface::~TestSVGTypeface() {}

TestSVGTypeface::Glyph::Glyph() : fOrigin{0, 0}, fAdvance(0) {}
TestSVGTypeface::Glyph::~Glyph() {}

void TestSVGTypeface::getAdvance(SkGlyph* glyph) const {
    SkGlyphID glyphID = glyph->getGlyphID();
    glyphID           = glyphID < fGlyphCount ? glyphID : 0;

    glyph->fAdvanceX = fGlyphs[glyphID].fAdvance;
    glyph->fAdvanceY = 0;
}

void TestSVGTypeface::getFontMetrics(SkFontMetrics* metrics) const { *metrics = fFontMetrics; }

void TestSVGTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

void TestSVGTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    SkDEBUGCODE(unsigned glyphCount = this->countGlyphs());
    fCMap.foreach ([=](const SkUnichar& c, const SkGlyphID& g) {
        SkASSERT(g < glyphCount);
        glyphToUnicode[g] = c;
    });
}

std::unique_ptr<SkAdvancedTypefaceMetrics> TestSVGTypeface::onGetAdvancedMetrics() const {
    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);
    info->fFontName = fName;
    return info;
}

void TestSVGTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    desc->setFamilyName(fName.c_str());
    desc->setStyle(this->fontStyle());
    *isLocal = false;
}

void TestSVGTypeface::onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    for (int i = 0; i < count; i++) {
        SkGlyphID* g = fCMap.find(uni[i]);
        glyphs[i]    = g ? *g : 0;
    }
}

void TestSVGTypeface::onGetFamilyName(SkString* familyName) const { *familyName = fName; }

bool TestSVGTypeface::onGetPostScriptName(SkString*) const { return false; }

SkTypeface::LocalizedStrings* TestSVGTypeface::onCreateFamilyNameIterator() const {
    SkString familyName(fName);
    SkString language("und");  // undetermined
    return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
}

class SkTestSVGScalerContext : public SkScalerContext {
public:
    SkTestSVGScalerContext(sk_sp<TestSVGTypeface>        face,
                           const SkScalerContextEffects& effects,
                           const SkDescriptor*           desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        SkScalar upem = this->getTestSVGTypeface()->fUpem;
        fMatrix.preScale(1.f / upem, 1.f / upem);
    }

protected:
    TestSVGTypeface* getTestSVGTypeface() const {
        return static_cast<TestSVGTypeface*>(this->getTypeface());
    }

    bool generateAdvance(SkGlyph* glyph) override {
        this->getTestSVGTypeface()->getAdvance(glyph);

        const SkVector advance =
                fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX), SkFloatToScalar(glyph->fAdvanceY));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);
        return true;
    }

    void generateMetrics(SkGlyph* glyph, SkArenaAlloc* alloc) override {
        SkGlyphID glyphID = glyph->getGlyphID();
        glyphID           = glyphID < this->getTestSVGTypeface()->fGlyphCount ? glyphID : 0;

        glyph->zeroMetrics();
        glyph->fMaskFormat = SkMask::kARGB32_Format;
        glyph->setPath(alloc, nullptr, false);
        this->generateAdvance(glyph);

        TestSVGTypeface::Glyph& glyphData = this->getTestSVGTypeface()->fGlyphs[glyphID];

        SkSize containerSize = glyphData.size();
        SkRect newBounds = SkRect::MakeXYWH(glyphData.fOrigin.fX,
                                           -glyphData.fOrigin.fY,
                                            containerSize.fWidth,
                                            containerSize.fHeight);
        fMatrix.mapRect(&newBounds);
        SkScalar dx = SkFixedToScalar(glyph->getSubXFixed());
        SkScalar dy = SkFixedToScalar(glyph->getSubYFixed());
        newBounds.offset(dx, dy);

        SkIRect ibounds;
        newBounds.roundOut(&ibounds);
        glyph->fLeft   = ibounds.fLeft;
        glyph->fTop    = ibounds.fTop;
        glyph->fWidth  = ibounds.width();
        glyph->fHeight = ibounds.height();
    }

    void generateImage(const SkGlyph& glyph) override {
        SkGlyphID glyphID = glyph.getGlyphID();
        glyphID           = glyphID < this->getTestSVGTypeface()->fGlyphCount ? glyphID : 0;

        SkBitmap bm;
        // TODO: this should be SkImageInfo::MakeS32 when that passes all the tests.
        bm.installPixels(SkImageInfo::MakeN32(glyph.fWidth, glyph.fHeight, kPremul_SkAlphaType),
                         glyph.fImage,
                         glyph.rowBytes());
        bm.eraseColor(0);

        TestSVGTypeface::Glyph& glyphData = this->getTestSVGTypeface()->fGlyphs[glyphID];

        SkScalar dx = SkFixedToScalar(glyph.getSubXFixed());
        SkScalar dy = SkFixedToScalar(glyph.getSubYFixed());

        SkCanvas canvas(bm);
        canvas.translate(-glyph.fLeft, -glyph.fTop);
        canvas.translate(dx, dy);
        canvas.concat(fMatrix);
        canvas.translate(glyphData.fOrigin.fX, -glyphData.fOrigin.fY);

        glyphData.render(&canvas);
    }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        // Should never get here since generateMetrics always sets the path to not exist.
        SK_ABORT("Path requested, but it should have been indicated that there isn't one.");
        path->reset();
        return false;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        this->getTestSVGTypeface()->getFontMetrics(metrics);
        SkFontPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix fMatrix;
};

std::unique_ptr<SkScalerContext> TestSVGTypeface::onCreateScalerContext(
    const SkScalerContextEffects& e, const SkDescriptor* desc) const
{
    return std::make_unique<SkTestSVGScalerContext>(
            sk_ref_sp(const_cast<TestSVGTypeface*>(this)), e, desc);
}

sk_sp<TestSVGTypeface> TestSVGTypeface::Default() {
    // Recommended that the first four be .notdef, .null, CR, space
    constexpr const static SkSVGTestTypefaceGlyphData glyphs[] = {
            {"fonts/svg/notdef.svg", {100, 800}, 800, 0x0},      // .notdef
            {"fonts/svg/empty.svg", {0, 0}, 800, 0x0020},        // space
            {"fonts/svg/diamond.svg", {100, 800}, 800, 0x2662},  // ♢
            {"fonts/svg/smile.svg", {0, 800}, 800, 0x1F600},     // 😀
    };
    SkFontMetrics metrics;
    metrics.fFlags = SkFontMetrics::kUnderlineThicknessIsValid_Flag |
                     SkFontMetrics::kUnderlinePositionIsValid_Flag |
                     SkFontMetrics::kStrikeoutThicknessIsValid_Flag |
                     SkFontMetrics::kStrikeoutPositionIsValid_Flag;
    metrics.fTop                = -800;
    metrics.fAscent             = -800;
    metrics.fDescent            = 200;
    metrics.fBottom             = 200;
    metrics.fLeading            = 100;
    metrics.fAvgCharWidth       = 1000;
    metrics.fMaxCharWidth       = 1000;
    metrics.fXMin               = 0;
    metrics.fXMax               = 1000;
    metrics.fXHeight            = 500;
    metrics.fCapHeight          = 700;
    metrics.fUnderlineThickness = 40;
    metrics.fUnderlinePosition  = 20;
    metrics.fStrikeoutThickness = 20;
    metrics.fStrikeoutPosition  = -400;

    class DefaultTypeface : public TestSVGTypeface {
        using TestSVGTypeface::TestSVGTypeface;

        bool getPathOp(SkColor color, SkPathOp* op) const override {
            if ((SkColorGetR(color) + SkColorGetG(color) + SkColorGetB(color)) / 3 > 0x20) {
                *op = SkPathOp::kDifference_SkPathOp;
            } else {
                *op = SkPathOp::kUnion_SkPathOp;
            }
            return true;
        }
    };
    return sk_make_sp<DefaultTypeface>("Emoji",
                                       1000,
                                       metrics,
                                       SkMakeSpan(glyphs),
                                       SkFontStyle::Normal());
}

sk_sp<TestSVGTypeface> TestSVGTypeface::Planets() {
    // Recommended that the first four be .notdef, .null, CR, space
    constexpr const static SkSVGTestTypefaceGlyphData glyphs[] = {
            {"fonts/svg/planets/pluto.svg", {0, 20}, 60, 0x0},             // .notdef
            {"fonts/svg/empty.svg", {0, 0}, 400, 0x0020},                  // space
            {"fonts/svg/planets/mercury.svg", {0, 45}, 120, 0x263F},       // ☿
            {"fonts/svg/planets/venus.svg", {0, 100}, 240, 0x2640},        // ♀
            {"fonts/svg/planets/earth.svg", {0, 100}, 240, 0x2641},        // ♁
            {"fonts/svg/planets/mars.svg", {0, 50}, 130, 0x2642},          // ♂
            {"fonts/svg/planets/jupiter.svg", {0, 1000}, 2200, 0x2643},    // ♃
            {"fonts/svg/planets/saturn.svg", {-300, 1500}, 2600, 0x2644},  // ♄
            {"fonts/svg/planets/uranus.svg", {0, 375}, 790, 0x2645},       // ♅
            {"fonts/svg/planets/neptune.svg", {0, 350}, 740, 0x2646},      // ♆
    };
    SkFontMetrics metrics;
    metrics.fFlags = SkFontMetrics::kUnderlineThicknessIsValid_Flag |
                     SkFontMetrics::kUnderlinePositionIsValid_Flag |
                     SkFontMetrics::kStrikeoutThicknessIsValid_Flag |
                     SkFontMetrics::kStrikeoutPositionIsValid_Flag;
    metrics.fTop                = -1500;
    metrics.fAscent             = -200;
    metrics.fDescent            = 50;
    metrics.fBottom             = 1558;
    metrics.fLeading            = 10;
    metrics.fAvgCharWidth       = 200;
    metrics.fMaxCharWidth       = 200;
    metrics.fXMin               = -300;
    metrics.fXMax               = 2566;
    metrics.fXHeight            = 100;
    metrics.fCapHeight          = 180;
    metrics.fUnderlineThickness = 8;
    metrics.fUnderlinePosition  = 2;
    metrics.fStrikeoutThickness = 2;
    metrics.fStrikeoutPosition  = -80;

    class PlanetTypeface : public TestSVGTypeface {
        using TestSVGTypeface::TestSVGTypeface;

        bool getPathOp(SkColor color, SkPathOp* op) const override {
            *op = SkPathOp::kUnion_SkPathOp;
            return true;
        }
    };
    return sk_make_sp<PlanetTypeface>("Planets",
                                      200,
                                      metrics,
                                      SkMakeSpan(glyphs),
                                      SkFontStyle::Normal());
}

void TestSVGTypeface::exportTtxCommon(SkWStream*                out,
                                      const char*               type,
                                      const SkTArray<GlyfInfo>* glyfInfo) const {
    int totalGlyphs = fGlyphCount;
    out->writeText("  <GlyphOrder>\n");
    for (int i = 0; i < fGlyphCount; ++i) {
        out->writeText("    <GlyphID name=\"glyf");
        out->writeHexAsText(i, 4);
        out->writeText("\"/>\n");
    }
    if (glyfInfo) {
        for (int i = 0; i < fGlyphCount; ++i) {
            for (int j = 0; j < (*glyfInfo)[i].fLayers.count(); ++j) {
                out->writeText("    <GlyphID name=\"glyf");
                out->writeHexAsText(i, 4);
                out->writeText("l");
                out->writeHexAsText(j, 4);
                out->writeText("\"/>\n");
                ++totalGlyphs;
            }
        }
    }
    out->writeText("  </GlyphOrder>\n");

    out->writeText("  <head>\n");
    out->writeText("    <tableVersion value=\"1.0\"/>\n");
    out->writeText("    <fontRevision value=\"1.0\"/>\n");
    out->writeText("    <checkSumAdjustment value=\"0xa9c3274\"/>\n");
    out->writeText("    <magicNumber value=\"0x5f0f3cf5\"/>\n");
    out->writeText("    <flags value=\"00000000 00011011\"/>\n");
    out->writeText("    <unitsPerEm value=\"");
    out->writeDecAsText(fUpem);
    out->writeText("\"/>\n");
    out->writeText("    <created value=\"Thu Feb 15 12:55:49 2018\"/>\n");
    out->writeText("    <modified value=\"Thu Feb 15 12:55:49 2018\"/>\n");
    // TODO: not recalculated for bitmap fonts?
    out->writeText("    <xMin value=\"");
    out->writeScalarAsText(fFontMetrics.fXMin);
    out->writeText("\"/>\n");
    out->writeText("    <yMin value=\"");
    out->writeScalarAsText(-fFontMetrics.fBottom);
    out->writeText("\"/>\n");
    out->writeText("    <xMax value=\"");
    out->writeScalarAsText(fFontMetrics.fXMax);
    out->writeText("\"/>\n");
    out->writeText("    <yMax value=\"");
    out->writeScalarAsText(-fFontMetrics.fTop);
    out->writeText("\"/>\n");

    char macStyle[16] = {
            '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
    if (this->fontStyle().weight() >= SkFontStyle::Bold().weight()) {
        macStyle[0xF - 0x0] = '1';  // Bold
    }
    switch (this->fontStyle().slant()) {
        case SkFontStyle::kUpright_Slant: break;
        case SkFontStyle::kItalic_Slant:
            macStyle[0xF - 0x1] = '1';  // Italic
            break;
        case SkFontStyle::kOblique_Slant:
            macStyle[0xF - 0x1] = '1';  // Italic
            break;
        default: SK_ABORT("Unknown slant.");
    }
    if (this->fontStyle().width() <= SkFontStyle::kCondensed_Width) {
        macStyle[0xF - 0x5] = '1';  // Condensed
    } else if (this->fontStyle().width() >= SkFontStyle::kExpanded_Width) {
        macStyle[0xF - 0x6] = '1';  // Extended
    }
    out->writeText("    <macStyle value=\"");
    out->write(macStyle, 8);
    out->writeText(" ");
    out->write(macStyle + 8, 8);
    out->writeText("\"/>\n");
    out->writeText("    <lowestRecPPEM value=\"8\"/>\n");
    out->writeText("    <fontDirectionHint value=\"2\"/>\n");
    out->writeText("    <indexToLocFormat value=\"0\"/>\n");
    out->writeText("    <glyphDataFormat value=\"0\"/>\n");
    out->writeText("  </head>\n");

    out->writeText("  <hhea>\n");
    out->writeText("    <tableVersion value=\"0x00010000\"/>\n");
    out->writeText("    <ascent value=\"");
    out->writeDecAsText(-fFontMetrics.fAscent);
    out->writeText("\"/>\n");
    out->writeText("    <descent value=\"");
    out->writeDecAsText(-fFontMetrics.fDescent);
    out->writeText("\"/>\n");
    out->writeText("    <lineGap value=\"");
    out->writeDecAsText(fFontMetrics.fLeading);
    out->writeText("\"/>\n");
    out->writeText("    <advanceWidthMax value=\"0\"/>\n");
    out->writeText("    <minLeftSideBearing value=\"0\"/>\n");
    out->writeText("    <minRightSideBearing value=\"0\"/>\n");
    out->writeText("    <xMaxExtent value=\"");
    out->writeScalarAsText(fFontMetrics.fXMax - fFontMetrics.fXMin);
    out->writeText("\"/>\n");
    out->writeText("    <caretSlopeRise value=\"1\"/>\n");
    out->writeText("    <caretSlopeRun value=\"0\"/>\n");
    out->writeText("    <caretOffset value=\"0\"/>\n");
    out->writeText("    <reserved0 value=\"0\"/>\n");
    out->writeText("    <reserved1 value=\"0\"/>\n");
    out->writeText("    <reserved2 value=\"0\"/>\n");
    out->writeText("    <reserved3 value=\"0\"/>\n");
    out->writeText("    <metricDataFormat value=\"0\"/>\n");
    out->writeText("    <numberOfHMetrics value=\"0\"/>\n");
    out->writeText("  </hhea>\n");

    // Some of this table is going to be re-calculated, but we have to write it out anyway.
    out->writeText("  <maxp>\n");
    out->writeText("    <tableVersion value=\"0x10000\"/>\n");
    out->writeText("    <numGlyphs value=\"");
    out->writeDecAsText(totalGlyphs);
    out->writeText("\"/>\n");
    out->writeText("    <maxPoints value=\"4\"/>\n");
    out->writeText("    <maxContours value=\"1\"/>\n");
    out->writeText("    <maxCompositePoints value=\"0\"/>\n");
    out->writeText("    <maxCompositeContours value=\"0\"/>\n");
    out->writeText("    <maxZones value=\"1\"/>\n");
    out->writeText("    <maxTwilightPoints value=\"0\"/>\n");
    out->writeText("    <maxStorage value=\"0\"/>\n");
    out->writeText("    <maxFunctionDefs value=\"10\"/>\n");
    out->writeText("    <maxInstructionDefs value=\"0\"/>\n");
    out->writeText("    <maxStackElements value=\"512\"/>\n");
    out->writeText("    <maxSizeOfInstructions value=\"24\"/>\n");
    out->writeText("    <maxComponentElements value=\"0\"/>\n");
    out->writeText("    <maxComponentDepth value=\"0\"/>\n");
    out->writeText("  </maxp>\n");

    out->writeText("  <OS_2>\n");
    out->writeText("    <version value=\"4\"/>\n");
    out->writeText("    <xAvgCharWidth value=\"");
    out->writeScalarAsText(fFontMetrics.fAvgCharWidth);
    out->writeText("\"/>\n");
    out->writeText("    <usWeightClass value=\"");
    out->writeDecAsText(this->fontStyle().weight());
    out->writeText("\"/>\n");
    out->writeText("    <usWidthClass value=\"");
    out->writeDecAsText(this->fontStyle().width());
    out->writeText("\"/>\n");
    out->writeText("    <fsType value=\"00000000 00000000\"/>\n");
    out->writeText("    <ySubscriptXSize value=\"665\"/>\n");
    out->writeText("    <ySubscriptYSize value=\"716\"/>\n");
    out->writeText("    <ySubscriptXOffset value=\"0\"/>\n");
    out->writeText("    <ySubscriptYOffset value=\"143\"/>\n");
    out->writeText("    <ySuperscriptXSize value=\"665\"/>\n");
    out->writeText("    <ySuperscriptYSize value=\"716\"/>\n");
    out->writeText("    <ySuperscriptXOffset value=\"0\"/>\n");
    out->writeText("    <ySuperscriptYOffset value=\"491\"/>\n");
    out->writeText("    <yStrikeoutSize value=\"");
    out->writeScalarAsText(fFontMetrics.fStrikeoutThickness);
    out->writeText("\"/>\n");
    out->writeText("    <yStrikeoutPosition value=\"");
    out->writeScalarAsText(-fFontMetrics.fStrikeoutPosition);
    out->writeText("\"/>\n");
    out->writeText("    <sFamilyClass value=\"0\"/>\n");
    out->writeText("    <panose>\n");
    out->writeText("      <bFamilyType value=\"0\"/>\n");
    out->writeText("      <bSerifStyle value=\"0\"/>\n");
    out->writeText("      <bWeight value=\"0\"/>\n");
    out->writeText("      <bProportion value=\"0\"/>\n");
    out->writeText("      <bContrast value=\"0\"/>\n");
    out->writeText("      <bStrokeVariation value=\"0\"/>\n");
    out->writeText("      <bArmStyle value=\"0\"/>\n");
    out->writeText("      <bLetterForm value=\"0\"/>\n");
    out->writeText("      <bMidline value=\"0\"/>\n");
    out->writeText("      <bXHeight value=\"0\"/>\n");
    out->writeText("    </panose>\n");
    out->writeText("    <ulUnicodeRange1 value=\"00000000 00000000 00000000 00000001\"/>\n");
    out->writeText("    <ulUnicodeRange2 value=\"00010000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <ulUnicodeRange3 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <ulUnicodeRange4 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <achVendID value=\"Skia\"/>\n");
    char fsSelection[16] = {
            '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
    fsSelection[0xF - 0x7] = '1';  // Use typo metrics
    if (this->fontStyle().weight() >= SkFontStyle::Bold().weight()) {
        fsSelection[0xF - 0x5] = '1';  // Bold
    }
    switch (this->fontStyle().slant()) {
        case SkFontStyle::kUpright_Slant:
            if (this->fontStyle().weight() < SkFontStyle::Bold().weight()) {
                fsSelection[0xF - 0x6] = '1';  // Not bold or italic, is regular
            }
            break;
        case SkFontStyle::kItalic_Slant:
            fsSelection[0xF - 0x0] = '1';  // Italic
            break;
        case SkFontStyle::kOblique_Slant:
            fsSelection[0xF - 0x0] = '1';  // Italic
            fsSelection[0xF - 0x9] = '1';  // Oblique
            break;
        default: SK_ABORT("Unknown slant.");
    }
    out->writeText("    <fsSelection value=\"");
    out->write(fsSelection, 8);
    out->writeText(" ");
    out->write(fsSelection + 8, 8);
    out->writeText("\"/>\n");
    out->writeText("    <usFirstCharIndex value=\"0\"/>\n");
    out->writeText("    <usLastCharIndex value=\"0\"/>\n");
    out->writeText("    <sTypoAscender value=\"");
    out->writeScalarAsText(-fFontMetrics.fAscent);
    out->writeText("\"/>\n");
    out->writeText("    <sTypoDescender value=\"");
    out->writeScalarAsText(-fFontMetrics.fDescent);
    out->writeText("\"/>\n");
    out->writeText("    <sTypoLineGap value=\"");
    out->writeScalarAsText(fFontMetrics.fLeading);
    out->writeText("\"/>\n");
    out->writeText("    <usWinAscent value=\"");
    out->writeScalarAsText(-fFontMetrics.fAscent);
    out->writeText("\"/>\n");
    out->writeText("    <usWinDescent value=\"");
    out->writeScalarAsText(fFontMetrics.fDescent);
    out->writeText("\"/>\n");
    out->writeText("    <ulCodePageRange1 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <ulCodePageRange2 value=\"00000000 00000000 00000000 00000000\"/>\n");
    out->writeText("    <sxHeight value=\"");
    out->writeScalarAsText(fFontMetrics.fXHeight);
    out->writeText("\"/>\n");
    out->writeText("    <sCapHeight value=\"");
    out->writeScalarAsText(fFontMetrics.fCapHeight);
    out->writeText("\"/>\n");
    out->writeText("    <usDefaultChar value=\"0\"/>\n");
    out->writeText("    <usBreakChar value=\"32\"/>\n");
    out->writeText("    <usMaxContext value=\"0\"/>\n");
    out->writeText("  </OS_2>\n");

    out->writeText("  <hmtx>\n");
    for (int i = 0; i < fGlyphCount; ++i) {
        out->writeText("    <mtx name=\"glyf");
        out->writeHexAsText(i, 4);
        out->writeText("\" width=\"");
        out->writeDecAsText(fGlyphs[i].fAdvance);
        out->writeText("\" lsb=\"");
        int lsb = fGlyphs[i].fOrigin.fX;
        if (glyfInfo) {
            lsb += (*glyfInfo)[i].fBounds.fLeft;
        }
        out->writeDecAsText(lsb);
        out->writeText("\"/>\n");
    }
    if (glyfInfo) {
        for (int i = 0; i < fGlyphCount; ++i) {
            for (int j = 0; j < (*glyfInfo)[i].fLayers.count(); ++j) {
                out->writeText("    <mtx name=\"glyf");
                out->writeHexAsText(i, 4);
                out->writeText("l");
                out->writeHexAsText(j, 4);
                out->writeText("\" width=\"");
                out->writeDecAsText(fGlyphs[i].fAdvance);
                out->writeText("\" lsb=\"");
                int32_t lsb = fGlyphs[i].fOrigin.fX + (*glyfInfo)[i].fLayers[j].fBounds.fLeft;
                out->writeDecAsText(lsb);
                out->writeText("\"/>\n");
            }
        }
    }
    out->writeText("  </hmtx>\n");

    bool hasNonBMP = false;
    out->writeText("  <cmap>\n");
    out->writeText("    <tableVersion version=\"0\"/>\n");
    out->writeText("    <cmap_format_4 platformID=\"3\" platEncID=\"1\" language=\"0\">\n");
    fCMap.foreach ([&out, &hasNonBMP](const SkUnichar& c, const SkGlyphID& g) {
        if (0xFFFF < c) {
            hasNonBMP = true;
            return;
        }
        out->writeText("      <map code=\"0x");
        out->writeHexAsText(c, 4);
        out->writeText("\" name=\"glyf");
        out->writeHexAsText(g, 4);
        out->writeText("\"/>\n");
    });
    out->writeText("    </cmap_format_4>\n");
    if (hasNonBMP) {
        out->writeText(
                "    <cmap_format_12 platformID=\"3\" platEncID=\"10\" format=\"12\" "
                "reserved=\"0\" length=\"1\" language=\"0\" nGroups=\"0\">\n");
        fCMap.foreach ([&out](const SkUnichar& c, const SkGlyphID& g) {
            out->writeText("      <map code=\"0x");
            out->writeHexAsText(c, 6);
            out->writeText("\" name=\"glyf");
            out->writeHexAsText(g, 4);
            out->writeText("\"/>\n");
        });
        out->writeText("    </cmap_format_12>\n");
    }
    out->writeText("  </cmap>\n");

    out->writeText("  <name>\n");
    out->writeText(
            "    <namerecord nameID=\"1\" platformID=\"3\" platEncID=\"1\" langID=\"0x409\">\n");
    out->writeText("      ");
    out->writeText(fName.c_str());
    out->writeText(" ");
    out->writeText(type);
    out->writeText("\n");
    out->writeText("    </namerecord>\n");
    out->writeText(
            "    <namerecord nameID=\"2\" platformID=\"3\" platEncID=\"1\" langID=\"0x409\">\n");
    out->writeText("      Regular\n");
    out->writeText("    </namerecord>\n");
    out->writeText("  </name>\n");

    out->writeText("  <post>\n");
    out->writeText("    <formatType value=\"3.0\"/>\n");
    out->writeText("    <italicAngle value=\"0.0\"/>\n");
    out->writeText("    <underlinePosition value=\"");
    out->writeScalarAsText(fFontMetrics.fUnderlinePosition);
    out->writeText("\"/>\n");
    out->writeText("    <underlineThickness value=\"");
    out->writeScalarAsText(fFontMetrics.fUnderlineThickness);
    out->writeText("\"/>\n");
    out->writeText("    <isFixedPitch value=\"0\"/>\n");
    out->writeText("    <minMemType42 value=\"0\"/>\n");
    out->writeText("    <maxMemType42 value=\"0\"/>\n");
    out->writeText("    <minMemType1 value=\"0\"/>\n");
    out->writeText("    <maxMemType1 value=\"0\"/>\n");
    out->writeText("  </post>\n");
}

void TestSVGTypeface::exportTtxCbdt(SkWStream* out, SkSpan<unsigned> strikeSizes) const {
    SkPaint paint;
    SkFont  font;
    font.setTypeface(sk_ref_sp(const_cast<TestSVGTypeface*>(this)));
    SkString name;
    this->getFamilyName(&name);

    // The CBDT/CBLC format is quite restrictive. Only write strikes which fully fit.
    SkSTArray<8, int> goodStrikeSizes;
    for (size_t strikeIndex = 0; strikeIndex < strikeSizes.size(); ++strikeIndex) {
        font.setSize(strikeSizes[strikeIndex]);

        // CBLC limits
        SkFontMetrics fm;
        font.getMetrics(&fm);
        if (!SkTFitsIn<int8_t>((int)(-fm.fTop)) || !SkTFitsIn<int8_t>((int)(-fm.fBottom)) ||
            !SkTFitsIn<uint8_t>((int)(fm.fXMax - fm.fXMin))) {
            SkDebugf("Metrics too big cbdt font size %f for %s.\n", font.getSize(), name.c_str());
            continue;
        }

        // CBDT limits
        auto exceedsCbdtLimits = [&]() {
            for (int i = 0; i < fGlyphCount; ++i) {
                SkGlyphID gid = i;
                SkScalar  advance;
                SkRect    bounds;
                font.getWidthsBounds(&gid, 1, &advance, &bounds, nullptr);
                SkIRect ibounds = bounds.roundOut();
                if (!SkTFitsIn<int8_t>(ibounds.fLeft) || !SkTFitsIn<int8_t>(ibounds.fTop) ||
                    !SkTFitsIn<uint8_t>(ibounds.width()) || !SkTFitsIn<uint8_t>(ibounds.height()) ||
                    !SkTFitsIn<uint8_t>((int)advance)) {
                    return true;
                }
            }
            return false;
        };
        if (exceedsCbdtLimits()) {
            SkDebugf("Glyphs too big cbdt font size %f for %s.\n", font.getSize(), name.c_str());
            continue;
        }

        goodStrikeSizes.emplace_back(strikeSizes[strikeIndex]);
    }

    if (goodStrikeSizes.empty()) {
        SkDebugf("No strike size fit for cbdt font for %s.\n", name.c_str());
        return;
    }

    out->writeText("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    out->writeText("<ttFont sfntVersion=\"\\x00\\x01\\x00\\x00\" ttLibVersion=\"3.19\">\n");
    this->exportTtxCommon(out, "CBDT");

    out->writeText("  <CBDT>\n");
    out->writeText("    <header version=\"2.0\"/>\n");
    for (size_t strikeIndex = 0; strikeIndex < goodStrikeSizes.size(); ++strikeIndex) {
        font.setSize(goodStrikeSizes[strikeIndex]);

        out->writeText("    <strikedata index=\"");
        out->writeDecAsText(strikeIndex);
        out->writeText("\">\n");
        for (int i = 0; i < fGlyphCount; ++i) {
            SkGlyphID gid = i;
            SkScalar  advance;
            SkRect    bounds;
            font.getWidthsBounds(&gid, 1, &advance, &bounds, nullptr);
            SkIRect ibounds = bounds.roundOut();
            if (ibounds.isEmpty()) {
                continue;
            }
            SkImageInfo image_info = SkImageInfo::MakeN32Premul(ibounds.width(), ibounds.height());
            sk_sp<SkSurface> surface(SkSurface::MakeRaster(image_info));
            SkASSERT(surface);
            SkCanvas* canvas = surface->getCanvas();
            canvas->clear(0);
            SkPixmap pix;
            surface->peekPixels(&pix);
            canvas->drawSimpleText(&gid,
                                   sizeof(gid),
                                   SkTextEncoding::kGlyphID,
                                   -bounds.fLeft,
                                   -bounds.fTop,
                                   font,
                                   paint);
            surface->flushAndSubmit();
            sk_sp<SkImage> image = surface->makeImageSnapshot();
            sk_sp<SkData>  data  = image->encodeToData(SkEncodedImageFormat::kPNG, 100);

            out->writeText("      <cbdt_bitmap_format_17 name=\"glyf");
            out->writeHexAsText(i, 4);
            out->writeText("\">\n");
            out->writeText("        <SmallGlyphMetrics>\n");
            out->writeText("          <height value=\"");
            out->writeDecAsText(image->height());
            out->writeText("\"/>\n");
            out->writeText("          <width value=\"");
            out->writeDecAsText(image->width());
            out->writeText("\"/>\n");
            out->writeText("          <BearingX value=\"");
            out->writeDecAsText(ibounds.fLeft);
            out->writeText("\"/>\n");
            out->writeText("          <BearingY value=\"");
            out->writeDecAsText(-ibounds.fTop);
            out->writeText("\"/>\n");
            out->writeText("          <Advance value=\"");
            out->writeDecAsText((int)advance);
            out->writeText("\"/>\n");
            out->writeText("        </SmallGlyphMetrics>\n");
            out->writeText("        <rawimagedata>");
            uint8_t const* bytes = data->bytes();
            for (size_t j = 0; j < data->size(); ++j) {
                if ((j % 0x10) == 0x0) {
                    out->writeText("\n          ");
                } else if (((j - 1) % 0x4) == 0x3) {
                    out->writeText(" ");
                }
                out->writeHexAsText(bytes[j], 2);
            }
            out->writeText("\n");
            out->writeText("        </rawimagedata>\n");
            out->writeText("      </cbdt_bitmap_format_17>\n");
        }
        out->writeText("    </strikedata>\n");
    }
    out->writeText("  </CBDT>\n");

    SkFontMetrics fm;
    out->writeText("  <CBLC>\n");
    out->writeText("    <header version=\"2.0\"/>\n");
    for (size_t strikeIndex = 0; strikeIndex < goodStrikeSizes.size(); ++strikeIndex) {
        font.setSize(goodStrikeSizes[strikeIndex]);
        font.getMetrics(&fm);
        out->writeText("    <strike index=\"");
        out->writeDecAsText(strikeIndex);
        out->writeText("\">\n");
        out->writeText("      <bitmapSizeTable>\n");
        out->writeText("        <sbitLineMetrics direction=\"hori\">\n");
        out->writeText("          <ascender value=\"");
        out->writeDecAsText((int)(-fm.fTop));
        out->writeText("\"/>\n");
        out->writeText("          <descender value=\"");
        out->writeDecAsText((int)(-fm.fBottom));
        out->writeText("\"/>\n");
        out->writeText("          <widthMax value=\"");
        out->writeDecAsText((int)(fm.fXMax - fm.fXMin));
        out->writeText("\"/>\n");
        out->writeText("          <caretSlopeNumerator value=\"0\"/>\n");
        out->writeText("          <caretSlopeDenominator value=\"0\"/>\n");
        out->writeText("          <caretOffset value=\"0\"/>\n");
        out->writeText("          <minOriginSB value=\"0\"/>\n");
        out->writeText("          <minAdvanceSB value=\"0\"/>\n");
        out->writeText("          <maxBeforeBL value=\"0\"/>\n");
        out->writeText("          <minAfterBL value=\"0\"/>\n");
        out->writeText("          <pad1 value=\"0\"/>\n");
        out->writeText("          <pad2 value=\"0\"/>\n");
        out->writeText("        </sbitLineMetrics>\n");
        out->writeText("        <sbitLineMetrics direction=\"vert\">\n");
        out->writeText("          <ascender value=\"");
        out->writeDecAsText((int)(-fm.fTop));
        out->writeText("\"/>\n");
        out->writeText("          <descender value=\"");
        out->writeDecAsText((int)(-fm.fBottom));
        out->writeText("\"/>\n");
        out->writeText("          <widthMax value=\"");
        out->writeDecAsText((int)(fm.fXMax - fm.fXMin));
        out->writeText("\"/>\n");
        out->writeText("          <caretSlopeNumerator value=\"0\"/>\n");
        out->writeText("          <caretSlopeDenominator value=\"0\"/>\n");
        out->writeText("          <caretOffset value=\"0\"/>\n");
        out->writeText("          <minOriginSB value=\"0\"/>\n");
        out->writeText("          <minAdvanceSB value=\"0\"/>\n");
        out->writeText("          <maxBeforeBL value=\"0\"/>\n");
        out->writeText("          <minAfterBL value=\"0\"/>\n");
        out->writeText("          <pad1 value=\"0\"/>\n");
        out->writeText("          <pad2 value=\"0\"/>\n");
        out->writeText("        </sbitLineMetrics>\n");
        out->writeText("        <colorRef value=\"0\"/>\n");
        out->writeText("        <startGlyphIndex value=\"1\"/>\n");
        out->writeText("        <endGlyphIndex value=\"1\"/>\n");
        out->writeText("        <ppemX value=\"");
        out->writeDecAsText(goodStrikeSizes[strikeIndex]);
        out->writeText("\"/>\n");
        out->writeText("        <ppemY value=\"");
        out->writeDecAsText(goodStrikeSizes[strikeIndex]);
        out->writeText("\"/>\n");
        out->writeText("        <bitDepth value=\"32\"/>\n");
        out->writeText("        <flags value=\"1\"/>\n");
        out->writeText("      </bitmapSizeTable>\n");
        out->writeText(
                "      <eblc_index_sub_table_1 imageFormat=\"17\" firstGlyphIndex=\"1\" "
                "lastGlyphIndex=\"1\">\n");
        for (int i = 0; i < fGlyphCount; ++i) {
            SkGlyphID gid = i;
            SkRect    bounds;
            font.getBounds(&gid, 1, &bounds, nullptr);
            if (bounds.isEmpty()) {
                continue;
            }
            out->writeText("        <glyphLoc name=\"glyf");
            out->writeHexAsText(i, 4);
            out->writeText("\"/>\n");
        }
        out->writeText("      </eblc_index_sub_table_1>\n");
        out->writeText("    </strike>\n");
    }
    out->writeText("  </CBLC>\n");

    out->writeText("</ttFont>\n");
}

/**
 * UnitsPerEm is generally 1000 here. Versions of macOS older than 10.13
 * have problems in CoreText determining the glyph bounds of bitmap glyphs
 * with unitsPerEm set to 1024 or numbers not divisible by 100 when the
 * contour is not closed. The bounds of sbix fonts on macOS appear to be those
 * of the outline in the 'glyf' table. If this countour is closed it will be
 * drawn, as the 'glyf' outline is to be drawn on top of any bitmap. (There is
 * a bit which is supposed to control this, but it cannot be relied on.) So
 * make the glyph contour a degenerate line with points at the edge of the
 * bounding box of the glyph.
 */
void TestSVGTypeface::exportTtxSbix(SkWStream* out, SkSpan<unsigned> strikeSizes) const {
    out->writeText("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    out->writeText("<ttFont sfntVersion=\"\\x00\\x01\\x00\\x00\" ttLibVersion=\"3.19\">\n");
    this->exportTtxCommon(out, "sbix");

    SkPaint paint;
    SkFont  font;
    font.setTypeface(sk_ref_sp(const_cast<TestSVGTypeface*>(this)));

    out->writeText("  <glyf>\n");
    for (int i = 0; i < fGlyphCount; ++i) {
        const TestSVGTypeface::Glyph& glyphData = this->fGlyphs[i];

        SkSize containerSize = glyphData.size();
        SkRect  bounds  = SkRect::MakeXYWH(glyphData.fOrigin.fX,
                                         -glyphData.fOrigin.fY,
                                         containerSize.fWidth,
                                         containerSize.fHeight);
        SkIRect ibounds = bounds.roundOut();
        out->writeText("    <TTGlyph name=\"glyf");
        out->writeHexAsText(i, 4);
        out->writeText("\" xMin=\"");
        out->writeDecAsText(ibounds.fLeft);
        out->writeText("\" yMin=\"");
        out->writeDecAsText(-ibounds.fBottom);
        out->writeText("\" xMax=\"");
        out->writeDecAsText(ibounds.fRight);
        out->writeText("\" yMax=\"");
        out->writeDecAsText(-ibounds.fTop);
        out->writeText("\">\n");
        out->writeText("      <contour>\n");
        out->writeText("        <pt x=\"");
        out->writeDecAsText(ibounds.fLeft);
        out->writeText("\" y=\"");
        out->writeDecAsText(-ibounds.fBottom);
        out->writeText("\" on=\"1\"/>\n");
        out->writeText("      </contour>\n");
        out->writeText("      <contour>\n");
        out->writeText("        <pt x=\"");
        out->writeDecAsText(ibounds.fRight);
        out->writeText("\" y=\"");
        out->writeDecAsText(-ibounds.fTop);
        out->writeText("\" on=\"1\"/>\n");
        out->writeText("      </contour>\n");
        out->writeText("      <instructions/>\n");
        out->writeText("    </TTGlyph>\n");
    }
    out->writeText("  </glyf>\n");

    // The loca table will be re-calculated, but if we don't write one we don't get one.
    out->writeText("  <loca/>\n");

    out->writeText("  <sbix>\n");
    out->writeText("    <version value=\"1\"/>\n");
    out->writeText("    <flags value=\"00000000 00000001\"/>\n");
    for (size_t strikeIndex = 0; strikeIndex < strikeSizes.size(); ++strikeIndex) {
        font.setSize(strikeSizes[strikeIndex]);
        out->writeText("    <strike>\n");
        out->writeText("      <ppem value=\"");
        out->writeDecAsText(strikeSizes[strikeIndex]);
        out->writeText("\"/>\n");
        out->writeText("      <resolution value=\"72\"/>\n");
        for (int i = 0; i < fGlyphCount; ++i) {
            SkGlyphID gid = i;
            SkScalar  advance;
            SkRect    bounds;
            font.getWidthsBounds(&gid, 1, &advance, &bounds, nullptr);
            SkIRect ibounds = bounds.roundOut();
            if (ibounds.isEmpty()) {
                continue;
            }
            SkImageInfo image_info = SkImageInfo::MakeN32Premul(ibounds.width(), ibounds.height());
            sk_sp<SkSurface> surface(SkSurface::MakeRaster(image_info));
            SkASSERT(surface);
            SkCanvas* canvas = surface->getCanvas();
            canvas->clear(0);
            SkPixmap pix;
            surface->peekPixels(&pix);
            canvas->drawSimpleText(&gid,
                                   sizeof(gid),
                                   SkTextEncoding::kGlyphID,
                                   -bounds.fLeft,
                                   -bounds.fTop,
                                   font,
                                   paint);
            surface->flushAndSubmit();
            sk_sp<SkImage> image = surface->makeImageSnapshot();
            sk_sp<SkData>  data  = image->encodeToData(SkEncodedImageFormat::kPNG, 100);

            // The originOffset values are difficult to use as DirectWrite and FreeType interpret
            // the origin to be the initial glyph position on the baseline, but CoreGraphics
            // interprets the origin to be the lower left of the cbox of the outline in the 'glyf'
            // table.
            //#define SK_SBIX_LIKE_FT
            //#define SK_SBIX_LIKE_DW
            out->writeText("      <glyph name=\"glyf");
            out->writeHexAsText(i, 4);
            out->writeText("\" graphicType=\"png \" originOffsetX=\"");
#if defined(SK_SBIX_LIKE_FT) || defined(SK_SBIX_LIKE_DW)
            out->writeDecAsText(bounds.fLeft);
#else
            out->writeDecAsText(0);
#endif
            // DirectWrite and CoreGraphics use positive values of originOffsetY to push the
            // image visually up (but from different origins).
            // FreeType uses positive values to push the image down.
            out->writeText("\" originOffsetY=\"");
#if defined(SK_SBIX_LIKE_FT)
            out->writeScalarAsText(bounds.fBottom);
#elif defined(SK_SBIX_LIKE_DW)
            out->writeScalarAsText(-bounds.fBottom);
#else
            out->writeDecAsText(0);
#endif
            out->writeText("\">\n");

            out->writeText("        <hexdata>");
            uint8_t const* bytes = data->bytes();
            for (size_t j = 0; j < data->size(); ++j) {
                if ((j % 0x10) == 0x0) {
                    out->writeText("\n          ");
                } else if (((j - 1) % 0x4) == 0x3) {
                    out->writeText(" ");
                }
                out->writeHexAsText(bytes[j], 2);
            }
            out->writeText("\n");
            out->writeText("        </hexdata>\n");
            out->writeText("      </glyph>\n");
        }
        out->writeText("    </strike>\n");
    }
    out->writeText("  </sbix>\n");
    out->writeText("</ttFont>\n");
}

namespace {

void convert_noninflect_cubic_to_quads(const SkPoint            p[4],
                                       SkScalar                 toleranceSqd,
                                       SkTArray<SkPoint, true>* quads,
                                       int                      sublevel = 0) {
    // Notation: Point a is always p[0]. Point b is p[1] unless p[1] == p[0], in which case it is
    // p[2]. Point d is always p[3]. Point c is p[2] unless p[2] == p[3], in which case it is p[1].

    SkVector ab = p[1] - p[0];
    SkVector dc = p[2] - p[3];

    if (SkPointPriv::LengthSqd(ab) < SK_ScalarNearlyZero) {
        if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
            SkPoint* degQuad = quads->push_back_n(3);
            degQuad[0]       = p[0];
            degQuad[1]       = p[0];
            degQuad[2]       = p[3];
            return;
        }
        ab = p[2] - p[0];
    }
    if (SkPointPriv::LengthSqd(dc) < SK_ScalarNearlyZero) {
        dc = p[1] - p[3];
    }

    static const SkScalar kLengthScale = 3 * SK_Scalar1 / 2;
    static const int      kMaxSubdivs  = 10;

    ab.scale(kLengthScale);
    dc.scale(kLengthScale);

    // e0 and e1 are extrapolations along vectors ab and dc.
    SkVector c0 = p[0];
    c0 += ab;
    SkVector c1 = p[3];
    c1 += dc;

    SkScalar dSqd = sublevel > kMaxSubdivs ? 0 : SkPointPriv::DistanceToSqd(c0, c1);
    if (dSqd < toleranceSqd) {
        SkPoint cAvg = c0;
        cAvg += c1;
        cAvg.scale(SK_ScalarHalf);

        SkPoint* pts = quads->push_back_n(3);
        pts[0]       = p[0];
        pts[1]       = cAvg;
        pts[2]       = p[3];
        return;
    }
    SkPoint choppedPts[7];
    SkChopCubicAtHalf(p, choppedPts);
    convert_noninflect_cubic_to_quads(choppedPts + 0, toleranceSqd, quads, sublevel + 1);
    convert_noninflect_cubic_to_quads(choppedPts + 3, toleranceSqd, quads, sublevel + 1);
}

void convertCubicToQuads(const SkPoint p[4], SkScalar tolScale, SkTArray<SkPoint, true>* quads) {
    if (!p[0].isFinite() || !p[1].isFinite() || !p[2].isFinite() || !p[3].isFinite()) {
        return;
    }
    SkPoint chopped[10];
    int     count = SkChopCubicAtInflections(p, chopped);

    const SkScalar tolSqd = SkScalarSquare(tolScale);

    for (int i = 0; i < count; ++i) {
        SkPoint* cubic = chopped + 3 * i;
        convert_noninflect_cubic_to_quads(cubic, tolSqd, quads);
    }
}

void path_to_quads(const SkPath& path, SkPath* quadPath) {
    quadPath->reset();
    SkTArray<SkPoint, true> qPts;
    SkAutoConicToQuads      converter;
    const SkPoint*          quadPts;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove: quadPath->moveTo(pts[0].fX, pts[0].fY); break;
            case SkPathVerb::kLine: quadPath->lineTo(pts[1].fX, pts[1].fY); break;
            case SkPathVerb::kQuad:
                quadPath->quadTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPathVerb::kCubic:
                qPts.reset();
                convertCubicToQuads(pts, SK_Scalar1, &qPts);
                for (int i = 0; i < qPts.count(); i += 3) {
                    quadPath->quadTo(
                            qPts[i + 1].fX, qPts[i + 1].fY, qPts[i + 2].fX, qPts[i + 2].fY);
                }
                break;
            case SkPathVerb::kConic:
                quadPts = converter.computeQuads(pts, *w, SK_Scalar1);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    quadPath->quadTo(quadPts[i * 2 + 1].fX,
                                     quadPts[i * 2 + 1].fY,
                                     quadPts[i * 2 + 2].fX,
                                     quadPts[i * 2 + 2].fY);
                }
                break;
            case SkPathVerb::kClose: quadPath->close(); break;
        }
    }
}

class SkCOLRCanvas : public SkNoDrawCanvas {
public:
    SkCOLRCanvas(SkRect                     glyphBounds,
                 const TestSVGTypeface&     typeface,
                 SkGlyphID                  glyphId,
                 TestSVGTypeface::GlyfInfo* glyf,
                 SkTHashMap<SkColor, int>*  colors,
                 SkWStream*                 out)
            : SkNoDrawCanvas(glyphBounds.roundOut().width(), glyphBounds.roundOut().height())
            , fBaselineOffset(glyphBounds.top())
            , fTypeface(typeface)
            , fGlyphId(glyphId)
            , fGlyf(glyf)
            , fColors(colors)
            , fOut(out)
            , fLayerId(0) {}

    void writePoint(SkScalar x, SkScalar y, bool on) {
        fOut->writeText("        <pt x=\"");
        fOut->writeDecAsText(SkScalarRoundToInt(x));
        fOut->writeText("\" y=\"");
        fOut->writeDecAsText(SkScalarRoundToInt(y));
        fOut->writeText("\" on=\"");
        fOut->write8(on ? '1' : '0');
        fOut->writeText("\"/>\n");
    }
    SkIRect writePath(const SkPath& path, bool layer) {
        // Convert to quads.
        SkPath quads;
        path_to_quads(path, &quads);

        SkRect  bounds  = quads.computeTightBounds();
        SkIRect ibounds = bounds.roundOut();
        // The bounds will be re-calculated anyway.
        fOut->writeText("    <TTGlyph name=\"glyf");
        fOut->writeHexAsText(fGlyphId, 4);
        if (layer) {
            fOut->writeText("l");
            fOut->writeHexAsText(fLayerId, 4);
        }
        fOut->writeText("\" xMin=\"");
        fOut->writeDecAsText(ibounds.fLeft);
        fOut->writeText("\" yMin=\"");
        fOut->writeDecAsText(ibounds.fTop);
        fOut->writeText("\" xMax=\"");
        fOut->writeDecAsText(ibounds.fRight);
        fOut->writeText("\" yMax=\"");
        fOut->writeDecAsText(ibounds.fBottom);
        fOut->writeText("\">\n");

        bool contourOpen = false;
        for (auto [verb, pts, w] : SkPathPriv::Iterate(quads)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    if (contourOpen) {
                        fOut->writeText("      </contour>\n");
                        contourOpen = false;
                    }
                    break;
                case SkPathVerb::kLine:
                    if (!contourOpen) {
                        fOut->writeText("      <contour>\n");
                        this->writePoint(pts[0].fX, pts[0].fY, true);
                        contourOpen = true;
                    }
                    this->writePoint(pts[1].fX, pts[1].fY, true);
                    break;
                case SkPathVerb::kQuad:
                    if (!contourOpen) {
                        fOut->writeText("      <contour>\n");
                        this->writePoint(pts[0].fX, pts[0].fY, true);
                        contourOpen = true;
                    }
                    this->writePoint(pts[1].fX, pts[1].fY, false);
                    this->writePoint(pts[2].fX, pts[2].fY, true);
                    break;
                case SkPathVerb::kClose:
                    if (contourOpen) {
                        fOut->writeText("      </contour>\n");
                        contourOpen = false;
                    }
                    break;
                default: SkDEBUGFAIL("bad verb"); return ibounds;
            }
        }
        if (contourOpen) {
            fOut->writeText("      </contour>\n");
        }

        // Required to write out an instructions tag.
        fOut->writeText("      <instructions/>\n");
        fOut->writeText("    </TTGlyph>\n");
        return ibounds;
    }

    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        SkPath path;
        path.addRect(rect);
        this->drawPath(path, paint);
    }

    void onDrawOval(const SkRect& oval, const SkPaint& paint) override {
        SkPath path;
        path.addOval(oval);
        this->drawPath(path, paint);
    }

    void onDrawArc(const SkRect&  oval,
                   SkScalar       startAngle,
                   SkScalar       sweepAngle,
                   bool           useCenter,
                   const SkPaint& paint) override {
        SkPath path;
        bool fillNoPathEffect = SkPaint::kFill_Style == paint.getStyle() && !paint.getPathEffect();
        SkPathPriv::CreateDrawArcPath(
                &path, oval, startAngle, sweepAngle, useCenter, fillNoPathEffect);
        this->drawPath(path, paint);
    }

    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        SkPath path;
        path.addRRect(rrect);
        this->drawPath(path, paint);
    }

    void onDrawPath(const SkPath& platonicPath, const SkPaint& originalPaint) override {
        SkPaint paint = originalPaint;
        SkPath  path  = platonicPath;

        // Apply the path effect.
        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
            bool fill = paint.getFillPath(path, &path);

            paint.setPathEffect(nullptr);
            if (fill) {
                paint.setStyle(SkPaint::kFill_Style);
            } else {
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeWidth(0);
            }
        }

        // Apply the matrix.
        SkMatrix m = this->getTotalMatrix();
        // If done to the canvas then everything would get clipped out.
        m.postTranslate(0, fBaselineOffset);  // put the baseline at 0
        m.postScale(1, -1);                   // and flip it since OpenType is y-up.
        path.transform(m);

        // While creating the default glyf, union with dark colors and intersect with bright colors.
        SkColor  color = paint.getColor();
        SkPathOp op;
        if (fTypeface.getPathOp(color, &op)) {
            fBasePath.add(path, op);
        }
        SkIRect bounds = this->writePath(path, true);

        // The CPAL table has the concept of a 'current color' which is index 0xFFFF.
        // Mark any layer drawn in 'currentColor' as having this special index.
        // The value of 'currentColor' here should a color which causes this layer to union into the
        // default glyf.
        constexpr SkColor currentColor = 0xFF2B0000;

        int colorIndex;
        if (color == currentColor) {
            colorIndex = 0xFFFF;
        } else {
            int* colorIndexPtr = fColors->find(color);
            if (colorIndexPtr) {
                colorIndex = *colorIndexPtr;
            } else {
                colorIndex = fColors->count();
                fColors->set(color, colorIndex);
            }
        }
        fGlyf->fLayers.emplace_back(colorIndex, bounds);

        ++fLayerId;
    }

    void finishGlyph() {
        SkPath baseGlyph;
        fBasePath.resolve(&baseGlyph);
        fGlyf->fBounds = this->writePath(baseGlyph, false);
    }

private:
    SkScalar                   fBaselineOffset;
    const TestSVGTypeface&     fTypeface;
    SkGlyphID                  fGlyphId;
    TestSVGTypeface::GlyfInfo* fGlyf;
    SkTHashMap<SkColor, int>*  fColors;
    SkWStream* const           fOut;
    SkOpBuilder                fBasePath;
    int                        fLayerId;
};

}  // namespace

void TestSVGTypeface::exportTtxColr(SkWStream* out) const {
    out->writeText("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    out->writeText("<ttFont sfntVersion=\"\\x00\\x01\\x00\\x00\" ttLibVersion=\"3.19\">\n");

    SkTHashMap<SkColor, int> colors;
    SkTArray<GlyfInfo>       glyfInfos(fGlyphCount);

    // Need to know all the glyphs up front for the common tables.
    SkDynamicMemoryWStream glyfOut;
    glyfOut.writeText("  <glyf>\n");
    for (int i = 0; i < fGlyphCount; ++i) {
        const TestSVGTypeface::Glyph& glyphData = this->fGlyphs[i];

        SkSize containerSize = glyphData.size();
        SkRect       bounds = SkRect::MakeXYWH(glyphData.fOrigin.fX,
                                         -glyphData.fOrigin.fY,
                                         containerSize.fWidth,
                                         containerSize.fHeight);
        SkCOLRCanvas canvas(bounds, *this, i, &glyfInfos.emplace_back(), &colors, &glyfOut);
        glyphData.render(&canvas);
        canvas.finishGlyph();
    }
    glyfOut.writeText("  </glyf>\n");

    this->exportTtxCommon(out, "COLR", &glyfInfos);

    // The loca table will be re-calculated, but if we don't write one we don't get one.
    out->writeText("  <loca/>\n");

    std::unique_ptr<SkStreamAsset> glyfStream = glyfOut.detachAsStream();
    out->writeStream(glyfStream.get(), glyfStream->getLength());

    out->writeText("  <COLR>\n");
    out->writeText("    <version value=\"0\"/>\n");
    for (int i = 0; i < fGlyphCount; ++i) {
        if (glyfInfos[i].fLayers.empty()) {
            continue;
        }
        if (glyfInfos[i].fBounds.isEmpty()) {
            SkDebugf("Glyph %d is empty but has layers.\n", i);
        }
        out->writeText("    <ColorGlyph name=\"glyf");
        out->writeHexAsText(i, 4);
        out->writeText("\">\n");
        for (int j = 0; j < glyfInfos[i].fLayers.count(); ++j) {
            const int colorIndex = glyfInfos[i].fLayers[j].fLayerColorIndex;
            out->writeText("      <layer colorID=\"");
            out->writeDecAsText(colorIndex);
            out->writeText("\" name=\"glyf");
            out->writeHexAsText(i, 4);
            out->writeText("l");
            out->writeHexAsText(j, 4);
            out->writeText("\"/>\n");
        }
        out->writeText("    </ColorGlyph>\n");
    }
    out->writeText("  </COLR>\n");

    // The colors must be written in order, the 'index' is ignored by ttx.
    SkAutoTMalloc<SkColor> colorsInOrder(colors.count());
    colors.foreach ([&colorsInOrder](const SkColor& c, const int* i) { colorsInOrder[*i] = c; });
    out->writeText("  <CPAL>\n");
    out->writeText("    <version value=\"0\"/>\n");
    out->writeText("    <numPaletteEntries value=\"");
    out->writeDecAsText(colors.count());
    out->writeText("\"/>\n");
    out->writeText("    <palette index=\"0\">\n");
    for (int i = 0; i < colors.count(); ++i) {
        SkColor c = colorsInOrder[i];
        out->writeText("      <color index=\"");
        out->writeDecAsText(i);
        out->writeText("\" value=\"#");
        out->writeHexAsText(SkColorGetR(c), 2);
        out->writeHexAsText(SkColorGetG(c), 2);
        out->writeHexAsText(SkColorGetB(c), 2);
        out->writeHexAsText(SkColorGetA(c), 2);
        out->writeText("\"/>\n");
    }
    out->writeText("    </palette>\n");
    out->writeText("  </CPAL>\n");

    out->writeText("</ttFont>\n");
}
#endif  // SK_ENABLE_SVG
