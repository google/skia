/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrPipelineBuilder.h"
#include "SkDevice.h"
#include "SkRRect.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "effects/GrRRectEffect.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

class BigRRectAAEffectGM : public GM {
public:
    BigRRectAAEffectGM(const SkRRect& rrect, const char* name)
        : fRRect(rrect)
        , fName(name) {
        this->setBGColor(sk_tool_utils::color_to_565(SK_ColorBLUE));
        // Each test case draws the rrect with gaps around it.
        fTestWidth = SkScalarCeilToInt(rrect.width()) + 2 * kGap;
        fTestHeight = SkScalarCeilToInt(rrect.height()) + 2 * kGap;

        // Add a pad between test cases.
        fTestOffsetX = fTestWidth + kPad;
        fTestOffsetY = fTestHeight + kPad;

        // We draw two tests in x (fill and inv-fill) and pad around
        // all four sides of the image.
        fWidth = 2 * fTestOffsetX + kPad;
        fHeight = fTestOffsetY + kPad;
    }

protected:
    SkString onShortName() override {
        SkString name;
        name.printf("big_rrect_%s_aa_effect", fName);
        return name;
    }

    SkISize onISize() override { return SkISize::Make(fWidth, fHeight); }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTarget* rt = canvas->internal_private_accessTopLayerRenderTarget();
        GrContext* context = rt ? rt->getContext() : nullptr;
        if (!context) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(rt));
        if (!drawContext) {
            return;
        }

        SkPaint paint;

        int y = kPad;
        int x = kPad;
        static const GrPrimitiveEdgeType kEdgeTypes[] = {
            kFillAA_GrProcessorEdgeType,
            kInverseFillAA_GrProcessorEdgeType,
        };
        SkRect testBounds = SkRect::MakeIWH(fTestWidth, fTestHeight);
        for (size_t et = 0; et < SK_ARRAY_COUNT(kEdgeTypes); ++et) {
            GrPrimitiveEdgeType edgeType = kEdgeTypes[et];
            canvas->save();
                canvas->translate(SkIntToScalar(x), SkIntToScalar(y));                

                // Draw a background for the test case
                SkPaint paint;
                paint.setColor(SK_ColorWHITE);
                canvas->drawRect(testBounds, paint);

                GrPipelineBuilder pipelineBuilder;
                pipelineBuilder.setXPFactory(
                    GrPorterDuffXPFactory::Create(SkXfermode::kSrc_Mode))->unref();

                SkRRect rrect = fRRect;
                rrect.offset(SkIntToScalar(x + kGap), SkIntToScalar(y + kGap));
                SkAutoTUnref<GrFragmentProcessor> fp(GrRRectEffect::Create(edgeType, rrect));
                SkASSERT(fp);
                if (fp) {
                    pipelineBuilder.addCoverageFragmentProcessor(fp);
                    pipelineBuilder.setRenderTarget(rt);

                    SkRect bounds = testBounds;
                    bounds.offset(SkIntToScalar(x), SkIntToScalar(y));

                    SkAutoTUnref<GrDrawBatch> batch(
                            GrRectBatchFactory::CreateNonAAFill(0xff000000, SkMatrix::I(), bounds,
                                                                nullptr, nullptr));
                    drawContext->internal_drawBatch(pipelineBuilder, batch);
                }
            canvas->restore();
            x = x + fTestOffsetX;
        }
    }

private:
    // pad between test cases
    static const int kPad = 7;
    // gap between rect for each case that is rendered and exterior of rrect
    static const int kGap = 3;

    SkRRect fRRect;
    int fWidth;
    int fHeight;
    int fTestWidth;
    int fTestHeight;
    int fTestOffsetX;
    int fTestOffsetY;
    const char* fName;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
// This value is motivated by bug chromium:477684. It has to be large to cause overflow in
// the shader
static const int kSize = 700;

DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeRect(SkRect::MakeIWH(kSize, kSize)), "rect"); )
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeOval(SkRect::MakeIWH(kSize, kSize)), "circle"); )
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeOval(SkRect::MakeIWH(kSize - 1, kSize - 10)), "ellipse"); )
// The next two have small linear segments between the corners
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeRectXY(SkRect::MakeIWH(kSize - 1, kSize - 10), kSize/2.f - 10.f, kSize/2.f - 10.f), "circular_corner"); )
DEF_GM( return new BigRRectAAEffectGM (SkRRect::MakeRectXY(SkRect::MakeIWH(kSize - 1, kSize - 10), kSize/2.f - 10.f, kSize/2.f - 15.f), "elliptical_corner"); )

}
#endif
