/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#if SK_SUPPORT_GPU
#include "GrTest.h"
#include "effects/GrRRectEffect.h"
#include "SkDevice.h"
#include "SkRRect.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

class BigRRectAAEffectGM : public GM {
public:
    BigRRectAAEffectGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
        this->setUpRRects();
    }

protected:
    SkString onShortName() override {
        return SkString("big_rrect_aa_effect");
    }

    SkISize onISize() override { return SkISize::Make(kImageWidth, kImageHeight); }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTarget* rt = canvas->internal_private_accessTopLayerRenderTarget();
        GrContext* context = rt ? rt->getContext() : NULL;
        if (!context) {
            this->drawGpuOnlyMessage(canvas);
            return;
        }

        SkPaint paint;

#ifdef SK_DEBUG
        static const SkRect kMaxRRectBound = SkRect::MakeWH(SkIntToScalar(kMaxSize),
                                                            SkIntToScalar(kMaxSize));
        static const SkRect kMaxImageBound = SkRect::MakeWH(SkIntToScalar(kImageWidth),
                                                            SkIntToScalar(kImageHeight));
#endif

        int y = kPad;
        int x = kPad;
        static const GrPrimitiveEdgeType kEdgeTypes[] = {
            kFillAA_GrProcessorEdgeType,
            kInverseFillAA_GrProcessorEdgeType,
        };
        for (size_t et = 0; et < SK_ARRAY_COUNT(kEdgeTypes); ++et) {
            GrPrimitiveEdgeType edgeType = kEdgeTypes[et];
            for (int curRRect = 0; curRRect < fRRects.count(); ++curRRect) {
#ifdef SK_DEBUG
                SkASSERT(kMaxRRectBound.contains(fRRects[curRRect].getBounds()));
                SkRect imageSpaceBounds = fRRects[curRRect].getBounds();
                imageSpaceBounds.offset(SkIntToScalar(x), SkIntToScalar(y));
                SkASSERT(kMaxImageBound.contains(imageSpaceBounds));
#endif
                canvas->save();
                    canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
                    GrTestTarget tt;
                    context->getTestTarget(&tt);
                    if (NULL == tt.target()) {
                        SkDEBUGFAIL("Couldn't get Gr test target.");
                        return;
                    }
                    GrPipelineBuilder pipelineBuilder;

                    SkRRect rrect = fRRects[curRRect];
                    rrect.offset(SkIntToScalar(x), SkIntToScalar(y));
                    SkAutoTUnref<GrFragmentProcessor> fp(GrRRectEffect::Create(edgeType, rrect));
                    SkASSERT(fp);
                    if (fp) {
                        pipelineBuilder.addCoverageProcessor(fp);
                        pipelineBuilder.setRenderTarget(rt);

                        SkRect bounds = SkRect::MakeWH(SkIntToScalar(kMaxSize),
                                                       SkIntToScalar(kMaxSize));
                        bounds.outset(2.f, 2.f);
                        bounds.offset(SkIntToScalar(x), SkIntToScalar(y));

                        tt.target()->drawSimpleRect(pipelineBuilder,
                                                    0xff000000,
                                                    SkMatrix::I(),
                                                    bounds);
                    }
                canvas->restore();
                x = x + kDrawOffset;
                if (x + kMaxSize> kImageWidth) {
                    x = kPad;
                    y += kDrawOffset;
                }
            }
        }
    }

    void setUpRRects() {
        SkScalar maxSize = SkIntToScalar(kMaxSize);
        fRRects.push()->setRect(SkRect::MakeWH(maxSize, maxSize));
        fRRects.push()->setOval(SkRect::MakeWH(maxSize, maxSize));
        fRRects.push()->setOval(SkRect::MakeWH(maxSize - 1.f, maxSize - 10.f));
        fRRects.push()->setRectXY(SkRect::MakeWH(maxSize - 1.f, maxSize - 10.f),
                                  maxSize/2.f - 10.f, maxSize/2.f - 10.f);
        fRRects.push()->setRectXY(SkRect::MakeWH(maxSize - 1.f, maxSize - 10),
                                  maxSize/2.f - 10.f, maxSize/2.f - 20.f);
    }

private:
    static const int kPad = 5;
    static const int kMaxSize = 300;
    static const int kDrawOffset = kMaxSize + kPad;

    static const int kImageWidth = 4 * kDrawOffset + kPad;
    static const int kImageHeight = 3 * kDrawOffset + kPad;


    SkTDArray<SkRRect> fRRects;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new BigRRectAAEffectGM (); )

}
#endif
