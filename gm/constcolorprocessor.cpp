/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"
#include "sk_tool_utils.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrRenderTargetContextPriv.h"
#include "SkGr.h"
#include "SkGradientShader.h"
#include "effects/GrConstColorProcessor.h"
#include "ops/GrDrawOp.h"
#include "ops/GrRectOpFactory.h"

namespace skiagm {
/**
 * This GM directly exercises GrConstColorProcessor.
 */
class ConstColorProcessor : public GM {
public:
    ConstColorProcessor() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
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
                                               SkShader::kClamp_TileMode);
    }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

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
        GR_STATIC_ASSERT(SK_ARRAY_COUNT(kModeStrs) == GrConstColorProcessor::kInputModeCnt);

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
                    SkAssertResult(SkPaintToGrPaint(context, renderTargetContext, skPaint,
                                                    viewMatrix, &grPaint));

                    GrConstColorProcessor::InputMode mode = (GrConstColorProcessor::InputMode) m;
                    GrColor4f color = GrColor4f::FromGrColor(kColors[procColor]);
                    sk_sp<GrFragmentProcessor> fp(GrConstColorProcessor::Make(color, mode));

                    grPaint.addColorFragmentProcessor(std::move(fp));
                    renderTargetContext->priv().testingOnly_addDrawOp(
                            GrRectOpFactory::MakeNonAAFill(std::move(grPaint), viewMatrix,
                                                           renderRect, GrAAType::kNone));

                    // Draw labels for the input to the processor and the processor to the right of
                    // the test rect. The input label appears above the processor label.
                    SkPaint labelPaint;
                    sk_tool_utils::set_portable_typeface(&labelPaint);
                    labelPaint.setAntiAlias(true);
                    labelPaint.setTextSize(10.f);
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
                    labelPaint.measureText(inputLabel.c_str(), inputLabel.size(),
                                           &inputLabelBounds);
                    canvas->drawString(inputLabel,
                                     renderRect.fRight + kPad,
                                     -inputLabelBounds.fTop, labelPaint);
                    // update the bounds to reflect the offset we used to draw it.
                    inputLabelBounds.offset(renderRect.fRight + kPad, -inputLabelBounds.fTop);

                    SkRect procLabelBounds;
                    labelPaint.measureText(procLabel.c_str(), procLabel.size(),
                                           &procLabelBounds);
                    canvas->drawString(procLabel,
                                     renderRect.fRight + kPad,
                                     inputLabelBounds.fBottom + 2.f - procLabelBounds.fTop,
                                     labelPaint);
                    procLabelBounds.offset(renderRect.fRight + kPad,
                                           inputLabelBounds.fBottom + 2.f - procLabelBounds.fTop);

                    labelPaint.setStrokeWidth(0);
                    labelPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(renderRect, labelPaint);

                    canvas->restore();

                    // update x and y for the next test case.
                    SkScalar height = renderRect.height();
                    SkScalar width = SkTMax(inputLabelBounds.fRight, procLabelBounds.fRight);
                    maxW = SkTMax(maxW, width);
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

#endif
