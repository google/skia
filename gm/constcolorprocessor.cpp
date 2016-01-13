
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrPipelineBuilder.h"
#include "SkGrPriv.h"
#include "SkGradientShader.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "effects/GrConstColorProcessor.h"

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
        fShader.reset(SkGradientShader::CreateLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                       SkShader::kClamp_TileMode));
    }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTarget* rt = canvas->internal_private_accessTopLayerRenderTarget();
        if (nullptr == rt) {
            return;
        }
        GrContext* context = rt->getContext();
        if (nullptr == context) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(rt));
        if (!drawContext) {
            return;
        }

        static const GrColor kColors[] = {
            0xFFFFFFFF,
            0xFFFF00FF,
            0x80000000,
            0x00000000,
        };

        static const SkColor kPaintColors[] = {
            0xFFFFFFFF,
            0xFFFF0000,
            0x80FF0000,
            0x00000000,
        };

        static const char* kModeStrs[] {
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
                    SkAssertResult(SkPaintToGrPaint(context, skPaint, viewMatrix, &grPaint));

                    GrConstColorProcessor::InputMode mode = (GrConstColorProcessor::InputMode) m;
                    GrColor color = kColors[procColor];
                    SkAutoTUnref<GrFragmentProcessor> fp(GrConstColorProcessor::Create(color, mode));

                    GrClip clip;
                    GrPipelineBuilder pipelineBuilder(grPaint, rt, clip);
                    pipelineBuilder.addColorFragmentProcessor(fp);

                    SkAutoTUnref<GrDrawBatch> batch(
                            GrRectBatchFactory::CreateNonAAFill(grPaint.getColor(), viewMatrix,
                                                                renderRect, nullptr, nullptr));
                    drawContext->internal_drawBatch(pipelineBuilder, batch);

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
                    canvas->drawText(inputLabel.c_str(), inputLabel.size(),
                                     renderRect.fRight + kPad,
                                     -inputLabelBounds.fTop, labelPaint);
                    // update the bounds to reflect the offset we used to draw it.
                    inputLabelBounds.offset(renderRect.fRight + kPad, -inputLabelBounds.fTop);

                    SkRect procLabelBounds;
                    labelPaint.measureText(procLabel.c_str(), procLabel.size(),
                                           &procLabelBounds);
                    canvas->drawText(procLabel.c_str(), procLabel.size(),
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
    SkAutoTUnref<SkShader>      fShader;

    static const SkScalar       kPad;
    static const SkScalar       kRectSize;
    static const int            kWidth  = 820;
    static const int            kHeight = 500;

    typedef GM INHERITED;
};

const SkScalar ConstColorProcessor::kPad = 10.f;
const SkScalar ConstColorProcessor::kRectSize = 20.f;

DEF_GM(return new ConstColorProcessor;)
}

#endif
