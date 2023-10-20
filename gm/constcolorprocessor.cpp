/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/TestOps.h"

#include <utility>

namespace skiagm {
/**
 * This GM directly exercises Color and ModulateRGBA.
 */
class ColorProcessor : public GpuGM {
public:
    enum class TestMode {
        kConstColor,
        kModulateRGBA
    };

    ColorProcessor(TestMode mode) : fMode(mode) {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString getName() const override {
        switch (fMode) {
            case TestMode::kConstColor:    return SkString("const_color_processor");
            case TestMode::kModulateRGBA:  return SkString("modulate_rgba");
        }
        SkUNREACHABLE;
    }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onOnceBeforeDraw() override {
        SkColor colors[] = { 0xFFFF0000, 0x2000FF00, 0xFF0000FF};
        SkPoint pts[] = { SkPoint::Make(0, 0), SkPoint::Make(kRectSize, kRectSize) };
        fShader = SkGradientShader::MakeLinear(pts, colors, nullptr, std::size(colors),
                                               SkTileMode::kClamp);
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);
        if (!sdc) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        constexpr GrColor kColors[] = {
            0xFFFFFFFF,
            0xFFFF00FF,
            0x80000000,
            0x00000000,
        };

        constexpr GrColor kPaintColors[] = {
            0xFFFFFFFF,
            0xFF0000FF,
            0x80000080,
            0x00000000,
        };

        SkScalar y = kPad;
        SkScalar x = kPad;
        SkScalar maxW = 0;
        for (size_t paintType = 0; paintType < std::size(kPaintColors) + 1; ++paintType) {
            for (size_t procColor = 0; procColor < std::size(kColors); ++procColor) {
                // translate by x,y for the canvas draws and the test target draws.
                canvas->save();
                canvas->translate(x, y);

                // rect to draw
                SkRect renderRect = SkRect::MakeXYWH(0, 0, kRectSize, kRectSize);

                // Create a base-layer FP for the const color processor to draw on top of.
                std::unique_ptr<GrFragmentProcessor> baseFP;
                if (paintType >= std::size(kPaintColors)) {
                    GrColorInfo colorInfo;
                    SkSurfaceProps props;
                    GrFPArgs args(rContext, &colorInfo, props, GrFPArgs::Scope::kDefault);
                    baseFP = GrFragmentProcessors::Make(fShader.get(), args, SkMatrix::I());
                } else {
                    baseFP = GrFragmentProcessor::MakeColor(
                            SkPMColor4f::FromBytes_RGBA(kPaintColors[paintType]));
                }

                // Layer a color/modulation FP on top of the base layer, using various colors.
                std::unique_ptr<GrFragmentProcessor> colorFP;
                switch (fMode) {
                    case TestMode::kConstColor:
                        colorFP = GrFragmentProcessor::MakeColor(
                                SkPMColor4f::FromBytes_RGBA(kColors[procColor]));
                        break;

                    case TestMode::kModulateRGBA:
                        colorFP = GrFragmentProcessor::ModulateRGBA(
                                std::move(baseFP), SkPMColor4f::FromBytes_RGBA(kColors[procColor]));
                        break;
                }

                // Render the FP tree.
                if (auto op = sk_gpu_test::test_ops::MakeRect(rContext,
                                                              std::move(colorFP),
                                                              renderRect.makeOffset(x, y),
                                                              renderRect,
                                                              SkMatrix::I())) {
                    sdc->addDrawOp(std::move(op));
                }

                // Draw labels for the input to the processor and the processor to the right of
                // the test rect. The input label appears above the processor label.
                SkFont labelFont;
                labelFont.setTypeface(ToolUtils::DefaultPortableTypeface());
                labelFont.setEdging(SkFont::Edging::kAntiAlias);
                labelFont.setSize(10.f);
                SkPaint labelPaint;
                labelPaint.setAntiAlias(true);
                SkString inputLabel("Input: ");
                if (paintType >= std::size(kPaintColors)) {
                    inputLabel.append("gradient");
                } else {
                    inputLabel.appendf("0x%08x", kPaintColors[paintType]);
                }
                SkString procLabel;
                procLabel.printf("Proc: [0x%08x]", kColors[procColor]);

                SkRect inputLabelBounds;
                // get the bounds of the text in order to position it
                labelFont.measureText(inputLabel.c_str(), inputLabel.size(),
                                      SkTextEncoding::kUTF8, &inputLabelBounds);
                canvas->drawString(inputLabel, renderRect.fRight + kPad, -inputLabelBounds.fTop,
                                   labelFont, labelPaint);
                // update the bounds to reflect the offset we used to draw it.
                inputLabelBounds.offset(renderRect.fRight + kPad, -inputLabelBounds.fTop);

                SkRect procLabelBounds;
                labelFont.measureText(procLabel.c_str(), procLabel.size(),
                                      SkTextEncoding::kUTF8, &procLabelBounds);
                canvas->drawString(procLabel, renderRect.fRight + kPad,
                                   inputLabelBounds.fBottom + 2.f - procLabelBounds.fTop,
                                   labelFont, labelPaint);
                procLabelBounds.offset(renderRect.fRight + kPad,
                                       inputLabelBounds.fBottom + 2.f - procLabelBounds.fTop);

                labelPaint.setStrokeWidth(0);
                labelPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawRect(renderRect, labelPaint);

                canvas->restore();

                // update x and y for the next test case.
                SkScalar height = renderRect.height();
                SkScalar width = std::max(inputLabelBounds.fRight, procLabelBounds.fRight);
                maxW = std::max(maxW, width);
                y += height + kPad;
                if (y + height > kHeight) {
                    y = kPad;
                    x += maxW + kPad;
                    maxW = 0;
                }
            }
        }

        return DrawResult::kOk;
    }

private:
    // Use this as a way of generating an input FP
    sk_sp<SkShader> fShader;
    TestMode        fMode;

    inline static constexpr SkScalar       kPad = 10.f;
    inline static constexpr SkScalar       kRectSize = 20.f;
    inline static constexpr int            kWidth  = 820;
    inline static constexpr int            kHeight = 500;

    using INHERITED = GM;
};

DEF_GM(return new ColorProcessor{ColorProcessor::TestMode::kConstColor};)
DEF_GM(return new ColorProcessor{ColorProcessor::TestMode::kModulateRGBA};)

}  // namespace skiagm
