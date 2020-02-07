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
#include "include/gpu/GrConfig.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "tools/ToolUtils.h"

#include <utility>

namespace skiagm {
/**
 * This GM directly exercises GrConstColorProcessor.
 */
class ConstColorProcessor : public GpuGM {
public:
    ConstColorProcessor() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() override {
        return SkString("const_color_processor");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onOnceBeforeDraw() override {
        SkColor colors[] = { 0xFFFF0000, 0x2000FF00, 0xFF0000FF};
        SkPoint pts[] = { SkPoint::Make(0, 0), SkPoint::Make(kRectSize, kRectSize) };
        fShader = SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                               SkTileMode::kClamp);
    }

    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {
        constexpr GrColor kColors[] = {
            0xFFFFFFFF,
            0xFFFF00FF,
            0x80000000,
            0x00000000,
        };

        constexpr SkColor kPaintColors[] = {
            0xFFFFFFFF,
            0xFFFF0000,
            0x80FF0000,
            0x00000000,
        };

        const char* kModeStrs[] {
            "kIgnore",
            "kModulateRGBA",
            "kModulateA",
        };
        static_assert(SK_ARRAY_COUNT(kModeStrs) == GrConstColorProcessor::kInputModeCnt);

        SkScalar y = kPad;
        SkScalar x = kPad;
        SkScalar maxW = 0;
        for (size_t paintType = 0; paintType < SK_ARRAY_COUNT(kPaintColors) + 1; ++paintType) {
            for (size_t procColor = 0; procColor < SK_ARRAY_COUNT(kColors); ++procColor) {
                for (int m = 0; m < GrConstColorProcessor::kInputModeCnt; ++m) {
                    // translate by x,y for the canvas draws and the test target draws.
                    canvas->save();
                    canvas->translate(x, y);
                    const SkMatrix viewMatrix = SkMatrix::MakeTrans(x, y);

                    // rect to draw
                    SkRect renderRect = SkRect::MakeXYWH(0, 0, kRectSize, kRectSize);

                    GrPaint grPaint;
                    SkPaint skPaint;
                    if (paintType >= SK_ARRAY_COUNT(kPaintColors)) {
                        skPaint.setShader(fShader);
                    } else {
                        skPaint.setColor(kPaintColors[paintType]);
                    }
                    SkAssertResult(SkPaintToGrPaint(context, renderTargetContext->colorInfo(),
                                                    skPaint, viewMatrix, &grPaint));

                    GrConstColorProcessor::InputMode mode = (GrConstColorProcessor::InputMode) m;
                    SkPMColor4f color = SkPMColor4f::FromBytes_RGBA(kColors[procColor]);
                    auto fp = GrConstColorProcessor::Make(color, mode);

                    grPaint.addColorFragmentProcessor(std::move(fp));
                    renderTargetContext->priv().testingOnly_addDrawOp(
                            GrFillRectOp::MakeNonAARect(context, std::move(grPaint),
                                                        viewMatrix, renderRect));

                    // Draw labels for the input to the processor and the processor to the right of
                    // the test rect. The input label appears above the processor label.
                    SkFont labelFont;
                    labelFont.setTypeface(ToolUtils::create_portable_typeface());
                    labelFont.setEdging(SkFont::Edging::kAntiAlias);
                    labelFont.setSize(10.f);
                    SkPaint labelPaint;
                    labelPaint.setAntiAlias(true);
                    SkString inputLabel;
                    inputLabel.set("Input: ");
                    if (paintType >= SK_ARRAY_COUNT(kPaintColors)) {
                        inputLabel.append("gradient");
                    } else {
                        inputLabel.appendf("0x%08x", kPaintColors[paintType]);
                    }
                    SkString procLabel;
                    procLabel.printf("Proc: [0x%08x, %s]", kColors[procColor], kModeStrs[m]);

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
        }
    }

private:
    // Use this as a way of generating and input FP
    sk_sp<SkShader> fShader;

    static constexpr SkScalar       kPad = 10.f;
    static constexpr SkScalar       kRectSize = 20.f;
    static constexpr int            kWidth  = 820;
    static constexpr int            kHeight = 500;

    typedef GM INHERITED;
};

DEF_GM(return new ConstColorProcessor;)
}
