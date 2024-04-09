/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
#include "include/ports/SkTypeface_fontations.h"
#endif

#include <string.h>
#include <initializer_list>

namespace skiagm {

namespace {
const SkScalar kTextSizes[] = {12, 18, 30, 120};
const char kTestFontName[] = "fonts/test_glyphs-glyf_colr_1.ttf";
const char kTestFontNameVariable[] = "fonts/test_glyphs-glyf_colr_1_variable.ttf";
const SkScalar xWidth = 1200;
const SkScalar xTranslate = 200;
}  // namespace

class ColrV1GM : public GM {
public:
    ColrV1GM(const char* testName,
             SkSpan<const uint32_t> codepoints,
             SkScalar skewX,
             SkScalar rotateDeg,
             std::initializer_list<SkFontArguments::VariationPosition::Coordinate>
                     specifiedVariations)
            : fTestName(testName), fCodepoints(codepoints), fSkewX(skewX), fRotateDeg(rotateDeg) {
        fVariationPosition.coordinateCount = specifiedVariations.size();
        fCoordinates = std::make_unique<SkFontArguments::VariationPosition::Coordinate[]>(
                specifiedVariations.size());
        for (size_t i = 0; i < specifiedVariations.size(); ++i) {
            fCoordinates[i] = std::data(specifiedVariations)[i];
        }

        fVariationPosition.coordinates = fCoordinates.get();
    }

protected:
    void onOnceBeforeDraw() override {
        if (fVariationPosition.coordinateCount) {
            fTypeface = ToolUtils::CreateTypefaceFromResource(kTestFontNameVariable, 0);
        } else {
            fTypeface = ToolUtils::CreateTypefaceFromResource(kTestFontName, 0);
        }
        fVariationSliders = ToolUtils::VariationSliders(fTypeface.get(), fVariationPosition);
    }

    SkString getName() const override {
        SkASSERT(!fTestName.isEmpty());
        SkString gm_name = SkStringPrintf("colrv1_%s", fTestName.c_str());

        if (fSkewX) {
            gm_name.append(SkStringPrintf("_skew_%.2f", fSkewX));
        }

        if (fRotateDeg) {
            gm_name.append(SkStringPrintf("_rotate_%.2f", fRotateDeg));
        }

        for (int i = 0; i < fVariationPosition.coordinateCount; ++i) {
            SkString tagName = ToolUtils::VariationSliders::tagToString(
                    fVariationPosition.coordinates[i].axis);
            gm_name.append(SkStringPrintf(
                    "_%s_%.2f", tagName.c_str(), fVariationPosition.coordinates[i].value));
        }

        return gm_name;
    }

    bool onGetControls(SkMetaData* controls) override {
        return fVariationSliders.writeControls(controls);
    }

    void onSetControls(const SkMetaData& controls) override {
        return fVariationSliders.readControls(controls);
    }

    SkISize getISize() override {
        // Sweep tests get a slightly wider canvas so that glyphs from one group fit in one row.
        if (fTestName.equals("sweep_varsweep")) {
            return SkISize::Make(xWidth + 500, xWidth);
        }
        return SkISize::Make(xWidth, xWidth);
    }

    sk_sp<SkTypeface> makeVariedTypeface() {
        if (!fTypeface) {
            return nullptr;
        }
        SkSpan<const SkFontArguments::VariationPosition::Coordinate> coords =
                fVariationSliders.getCoordinates();
        SkFontArguments::VariationPosition varPos = {coords.data(),
                                                     static_cast<int>(coords.size())};
        SkFontArguments args;
        args.setVariationDesignPosition(varPos);
        return fTypeface->makeClone(args);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(xTranslate, 20);

        if (!fTypeface) {
            *errorMsg = "Did not recognize COLR v1 font format.";
            return DrawResult::kSkip;
        }

        canvas->rotate(fRotateDeg);
        canvas->skew(fSkewX, 0);

        SkFont font(makeVariedTypeface());

        SkFontMetrics metrics;
        SkScalar y = 0;
        std::vector<SkColor> paint_colors = {
                SK_ColorBLACK, SK_ColorGREEN, SK_ColorRED, SK_ColorBLUE};
        auto paint_color_iterator = paint_colors.begin();
        for (SkScalar textSize : kTextSizes) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            SkScalar y_shift = -(metrics.fAscent + metrics.fDescent + metrics.fLeading) * 1.2;
            y += y_shift;
            paint.setColor(*paint_color_iterator);
            int x = 0;
            // Perform simple line breaking to fit more glyphs into the GM canvas.
            for (size_t i = 0; i < fCodepoints.size(); ++i) {
                SkScalar glyphAdvance = font.measureText(
                        &fCodepoints[i], sizeof(uint32_t), SkTextEncoding::kUTF32, nullptr);
                if (0 < x && getISize().width() - xTranslate < x + glyphAdvance) {
                    y += y_shift;
                    x = 0;
                }
                canvas->drawSimpleText(&fCodepoints[i],
                                       sizeof(uint32_t),
                                       SkTextEncoding::kUTF32,
                                       x,
                                       y,
                                       font,
                                       paint);
                x += glyphAdvance + glyphAdvance * 0.05f;
            }
            paint_color_iterator++;
        }
        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    SkString fTestName;
    sk_sp<SkTypeface> fTypeface;
    SkSpan<const uint32_t> fCodepoints;
    SkScalar fSkewX;
    SkScalar fRotateDeg;
    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoordinates;
    SkFontArguments::VariationPosition fVariationPosition;
    ToolUtils::VariationSliders fVariationSliders;
};

// clang-format off
// Generated using test glyphs generator script from https://github.com/googlefonts/color-fonts:
// $ python3 config/test_glyphs-glyf_colr_1.py -vvv  --generate-descriptions fonts/
// Regenerate descriptions and paste the generated arrays here when updating the test font.
namespace ColrV1TestDefinitions {
const uint32_t gradient_stops_repeat[] = {0xf0100, 0xf0101, 0xf0102, 0xf0103};
const uint32_t sweep_varsweep[] = {
        0xf0200, 0xf0201, 0xf0202, 0xf0203, 0xf0204, 0xf0205, 0xf0206, 0xf0207, 0xf0208,
        0xf0209, 0xf020a, 0xf020b, 0xf020c, 0xf020d, 0xf020e, 0xf020f, 0xf0210, 0xf0211,
        0xf0212, 0xf0213, 0xf0214, 0xf0215, 0xf0216, 0xf0217, 0xf0218, 0xf0219, 0xf021a,
        0xf021b, 0xf021c, 0xf021d, 0xf021e, 0xf021f, 0xf0220, 0xf0221, 0xf0222, 0xf0223,
        0xf0224, 0xf0225, 0xf0226, 0xf0227, 0xf0228, 0xf0229, 0xf022a, 0xf022b, 0xf022c,
        0xf022d, 0xf022e, 0xf022f, 0xf0230, 0xf0231, 0xf0232, 0xf0233, 0xf0234, 0xf0235,
        0xf0236, 0xf0237, 0xf0238, 0xf0239, 0xf023a, 0xf023b, 0xf023c, 0xf023d, 0xf023e,
        0xf023f, 0xf0240, 0xf0241, 0xf0242, 0xf0243, 0xf0244, 0xf0245, 0xf0246, 0xf0247};
const uint32_t paint_scale[] = {0xf0300, 0xf0301, 0xf0302, 0xf0303, 0xf0304, 0xf0305};
const uint32_t extend_mode[] = {
        0xf0500, 0xf0501, 0xf0502, 0xf0503, 0xf0504, 0xf0505, 0xf0506, 0xf0507, 0xf0508};
const uint32_t paint_rotate[] = {0xf0600, 0xf0601, 0xf0602, 0xf0603};
const uint32_t paint_skew[] = {0xf0700, 0xf0701, 0xf0702, 0xf0703, 0xf0704, 0xf0705};
const uint32_t paint_transform[] = {0xf0800, 0xf0801, 0xf0802, 0xf0803};
const uint32_t paint_translate[] = {0xf0900, 0xf0901, 0xf0902, 0xf0903, 0xf0904, 0xf0905, 0xf0906};
const uint32_t composite_mode[] = {0xf0a00, 0xf0a01, 0xf0a02, 0xf0a03, 0xf0a04, 0xf0a05, 0xf0a06,
                                   0xf0a07, 0xf0a08, 0xf0a09, 0xf0a0a, 0xf0a0b, 0xf0a0c, 0xf0a0d,
                                   0xf0a0e, 0xf0a0f, 0xf0a10, 0xf0a11, 0xf0a12, 0xf0a13, 0xf0a14,
                                   0xf0a15, 0xf0a16, 0xf0a17, 0xf0a18, 0xf0a19, 0xf0a1a, 0xf0a1b};
const uint32_t foreground_color[] = {
        0xf0b00, 0xf0b01, 0xf0b02, 0xf0b03, 0xf0b04, 0xf0b05, 0xf0b06, 0xf0b07};
const uint32_t clipbox[] = {0xf0c00, 0xf0c01, 0xf0c02, 0xf0c03, 0xf0c04};
const uint32_t gradient_p2_skewed[] = {0xf0d00};
const uint32_t variable_alpha[] = {0xf1000};
const uint32_t paintcolrglyph_cycle[] = { 0xf1100, 0xf1101, 0xf1200 };
const uint32_t sweep_coincident[] = { 0xf1300, 0xf1301, 0xf1302, 0xf1303, 0xf1304, 0xf1305,
                                      0xf1306, 0xf1307, 0xf1308, 0xf1309, 0xf130a, 0xf130b,
                                      0xf130c, 0xf130d, 0xf130e, 0xf130f, 0xf1310, 0xf1311,
                                      0xf1312, 0xf1313, 0xf1314, 0xf1315, 0xf1316, 0xf1317};
const uint32_t paint_glyph_nested[] = { 0xf1400, 0xf1401, 0xf1402, 0xf1403,
                                        0xf1404, 0xf1405, 0xf1406, 0xf1407,
                                        0xf1408, 0xf1409, 0xf140a, 0xf140b,
                                        0xf140c, 0xf140d, 0xf140e, 0xf140f };
// clang-format on

};  // namespace ColrV1TestDefinitions

namespace {
std::unique_ptr<ColrV1GM> F(
        const char* name,
        SkSpan<const uint32_t> codepoints,
        SkScalar skewX,
        SkScalar rotateDeg,
        std::initializer_list<SkFontArguments::VariationPosition::Coordinate> variations) {
    return std::make_unique<ColrV1GM>(name, codepoints, skewX, rotateDeg, variations);
}

SkFourByteTag constexpr operator"" _t(const char* tagName, size_t size) {
    SkASSERT(size == 4);
    return SkSetFourByteTag(tagName[0], tagName[1], tagName[2], tagName[3]);
}
}  // namespace

// clang-format off
#define C(TEST_CATEGORY) #TEST_CATEGORY, ColrV1TestDefinitions::TEST_CATEGORY
DEF_GM(return F(C(clipbox),                0.0f,  0.0f, {}))
DEF_GM(return F(C(clipbox),                0.0f,  0.0f, {{"CLIO"_t, 200.f}}))
DEF_GM(return F(C(composite_mode),         0.0f,  0.0f, {}))
DEF_GM(return F(C(composite_mode),        -0.5f,  0.0f, {}))
DEF_GM(return F(C(composite_mode),        -0.5f, 20.0f, {}))
DEF_GM(return F(C(composite_mode),         0.0f, 20.0f, {}))
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {}))
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"COL1"_t, -0.25f}, {"COL3"_t, 0.25f}}))
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"COL1"_t, 0.5f}, {"COL3"_t, -0.5f}}))
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"COL3"_t, 0.5f}}))
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"COL3"_t, 1.f}}))
// Radial gradient tests where radii become negative
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"COL1"_t, -1.5f}}))
// Both radii negative and equal, nothing should render.
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"GRR0"_t, -200.f}, {"GRR1"_t, -300.f}}))
// Small cones opening to the right.
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"GRX0"_t, -1000.f}, {"GRX1"_t, -1000.f}, {"GRR0"_t, -1000.f}, {"GRR1"_t, -900.f}}))
// Small cones opening to the left.
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"GRX0"_t, 1000.f}, {"GRX1"_t, -1000.f}, {"GRR0"_t, -1000.f}, {"GRR1"_t, 200.f}}))
// Pad cone should appear green.
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"GRR0"_t, -50.f}, {"COL3"_t, -2.f}, {"COL2"_t, -2.f}, {"COL1"_t, -0.9f}}))
// Pad cone should appear red.
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"GRR0"_t, -50.f}, {"COL3"_t, -2.f}, {"COL2"_t, -2.f}, {"COL1"_t, -1.1f}}))
// Hard boundary for pad mode, should appear on the right inside the glyph for linear and radial.
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"COL3"_t, 1.f}, {"COL2"_t, 1.5f}, {"COL1"_t, 2.f}}))
// Extend mode with rotation or skew below.
DEF_GM(return F(C(extend_mode),           -0.5f,  0.0f, {}))
DEF_GM(return F(C(extend_mode),           -0.5f, 20.0f, {}))
DEF_GM(return F(C(extend_mode),            0.0f, 20.0f, {}))
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"COL2"_t, -0.3f}}))
DEF_GM(return F(C(extend_mode),            0.0f,  0.0f, {{"GRR0"_t, 430.f}, {"GRR1"_t, 40.f}}))
DEF_GM(return F(C(foreground_color),       0.0f,  0.0f, {}))
DEF_GM(return F(C(gradient_p2_skewed),     0.0f,  0.0f, {}))
DEF_GM(return F(C(gradient_stops_repeat),  0.0f,  0.0f, {}))
DEF_GM(return F(C(gradient_stops_repeat), -0.5f,  0.0f, {}))
DEF_GM(return F(C(gradient_stops_repeat), -0.5f, 20.0f, {}))
DEF_GM(return F(C(gradient_stops_repeat),  0.0f, 20.0f, {}))
DEF_GM(return F(C(paint_rotate),           0.0f,  0.0f, {}))
DEF_GM(return F(C(paint_rotate),           0.0f,  0.0f, {{"ROTA"_t, 40.f}}))
DEF_GM(return F(C(paint_rotate),           0.0f,  0.0f, {{"ROTX"_t, -250.f}, {"ROTY"_t, -250.f}}))
DEF_GM(return F(C(paint_scale),            0.0f,  0.0f, {}))
DEF_GM(return F(C(paint_scale),            0.0f,  0.0f, {{"SCOX"_t, 200.f}, {"SCOY"_t, 200.f}}))
DEF_GM(return F(C(paint_scale),            0.0f,  0.0f, {{"SCSX"_t, 0.25f}, {"SCOY"_t, 0.25f}}))
DEF_GM(return F(C(paint_scale),            0.0f,  0.0f, {{"SCSX"_t, -1.f}, {"SCOY"_t, -1.f}}))
DEF_GM(return F(C(paint_skew),             0.0f,  0.0f, {}))
DEF_GM(return F(C(paint_skew),             0.0f,  0.0f, {{"SKXA"_t, 20.f}}))
DEF_GM(return F(C(paint_skew),             0.0f,  0.0f, {{"SKYA"_t, 20.f}}))
DEF_GM(return F(C(paint_skew),             0.0f,  0.0f, {{"SKCX"_t, 200.f},{"SKCY"_t, 200.f}}))
DEF_GM(return F(C(paint_transform),        0.0f,  0.0f, {}))
DEF_GM(return F(C(paint_translate),        0.0f,  0.0f, {}))
DEF_GM(return F(C(paint_translate),        0.0f,  0.0f, {{"TLDX"_t, 100.f}, {"TLDY"_t, 100.f}}))
DEF_GM(return F(C(sweep_varsweep),         0.0f,  0.0f, {}))
DEF_GM(return F(C(sweep_varsweep),        -0.5f,  0.0f, {}))
DEF_GM(return F(C(sweep_varsweep),        -0.5f, 20.0f, {}))
DEF_GM(return F(C(sweep_varsweep),         0.0f, 20.0f, {}))
DEF_GM(return F(C(sweep_varsweep),         0.0f,  0.0f, {{"SWPS"_t, 0.f}}))
DEF_GM(return F(C(sweep_varsweep),         0.0f,  0.0f, {{"SWPS"_t, 90.f}}))
DEF_GM(return F(C(sweep_varsweep),         0.0f,  0.0f, {{"SWPE"_t, -90.f}}))
DEF_GM(return F(C(sweep_varsweep),         0.0f,  0.0f, {{"SWPE"_t, -45.f}}))
DEF_GM(return F(C(sweep_varsweep),         0.0f,  0.0f, {{"SWPS"_t, -45.f},{"SWPE"_t, 45.f}}))
DEF_GM(return F(C(sweep_varsweep),
                0.0f,
                0.0f,
                {{"SWC1"_t, -0.25f},
                 {"SWC2"_t, 0.083333333f},
                 {"SWC3"_t, 0.083333333f},
                 {"SWC4"_t, +0.25f}}))
DEF_GM(return F(C(variable_alpha),         0.0f,  0.0f, {}))
DEF_GM(return F(C(variable_alpha),         0.0f,  0.0f, {{"APH1"_t, -0.7f}}))
DEF_GM(return F(C(variable_alpha),         0.0f,  0.0f, {{"APH2"_t, -0.7f}, {"APH3"_t, -0.2f}}))
DEF_GM(return F(C(paintcolrglyph_cycle),   0.0f,  0.0f, {}))
DEF_GM(return F(C(sweep_coincident),       0.0f,  0.0f, {}))
DEF_GM(return F(C(paint_glyph_nested),     0.0f,  0.0f, {}))
// clang-format on

}  // namespace skiagm
