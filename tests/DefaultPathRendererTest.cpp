/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmap.h"

#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrRenderTargetContext.h"
#include "GrStyle.h"
#include "GrTypesPriv.h"

#include "effects/GrConstColorProcessor.h"

static void allow_default_and_msaa(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kMSAA;
}

static void only_allow_default(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kNone;
}

static SkBitmap read_back(GrRenderTargetContext* rtc, int width, int height) {

    SkImageInfo dstII = SkImageInfo::MakeN32Premul(width, height);

    SkBitmap bm;
    bm.allocPixels(dstII);

    rtc->readPixels(dstII, bm.getAddr(0, 0), bm.rowBytes(), 0, 0, 0);

    return bm;
}

static SkPath make_path(const SkRect& outer, int inset, SkPath::FillType fill) {
    SkPath p;

    p.addRect(outer, SkPath::kCW_Direction);
    p.addRect(outer.makeInset(inset, inset), SkPath::kCCW_Direction);
    p.setFillType(fill);
    return p;
}


static const int kBigSize = 64; // This should be a power of 2
static const int kPad = 3;

// From crbug.com/769898:
//   create an approx fit render target context that will have extra space (i.e., npot)
//   draw an inverse wound concave path into it - forcing use of the stencil-using path renderer
//   throw the RTC away so the backing GrSurface/GrStencilBuffer can be reused
//   create a new render target context that will reuse the prior GrSurface
//   draw a normally wound concave path that touches outside of the approx fit RTC's content rect
//
// When the bug manifests the GrDefaultPathRenderer/GrMSAAPathRenderer is/was leaving the stencil
// buffer outside of the first content rect in a bad state and the second draw would be incorrect.

static void run_test(GrContext* ctx, skiatest::Reporter* reporter) {
    SkPath invPath = make_path(SkRect::MakeXYWH(0, 0, kBigSize, kBigSize),
                               kBigSize/2-1, SkPath::kInverseWinding_FillType);
    SkPath path = make_path(SkRect::MakeXYWH(0, 0, kBigSize, kBigSize),
                            kPad, SkPath::kWinding_FillType);

    GrStyle style(SkStrokeRec::kFill_InitStyle);

    {
        auto rtc =  ctx->makeDeferredRenderTargetContext(SkBackingFit::kApprox,
                                                         kBigSize/2+1, kBigSize/2+1,
                                                         kRGBA_8888_GrPixelConfig, nullptr);

        rtc->clear(nullptr, GrColorPackRGBA(0x0, 0x0, 0x0, 0xFF),
                   GrRenderTargetContext::CanClearFullscreen::kYes);

        GrPaint paint;

        const GrColor4f color = { 1.0f, 0.0f, 0.0f, 1.0f };
        auto fp = GrConstColorProcessor::Make(color, GrConstColorProcessor::InputMode::kIgnore);
        paint.addColorFragmentProcessor(std::move(fp));

        rtc->drawPath(GrNoClip(), std::move(paint), GrAA::kNo,
                      SkMatrix::I(), invPath, style);

        rtc->prepareForExternalIO(0, nullptr);
    }

    {
        auto rtc = ctx->makeDeferredRenderTargetContext(SkBackingFit::kExact, kBigSize, kBigSize,
                                                        kRGBA_8888_GrPixelConfig, nullptr);

        rtc->clear(nullptr, GrColorPackRGBA(0x0, 0x0, 0x0, 0xFF),
                   GrRenderTargetContext::CanClearFullscreen::kYes);

        GrPaint paint;

        const GrColor4f color = { 0.0f, 1.0f, 0.0f, 1.0f };
        auto fp = GrConstColorProcessor::Make(color, GrConstColorProcessor::InputMode::kIgnore);
        paint.addColorFragmentProcessor(std::move(fp));

        rtc->drawPath(GrNoClip(), std::move(paint), GrAA::kNo,
                      SkMatrix::I(), path, style);

        SkBitmap bm = read_back(rtc.get(), kBigSize, kBigSize);

        bool correct = true;
        for (int y = kBigSize/2+1; y < kBigSize-kPad-1 && correct; ++y) {
            for (int x = kPad+1; x < kBigSize-kPad-1 && correct; ++x) {
                correct = bm.getColor(x, y) == SK_ColorBLACK;
                REPORTER_ASSERT(reporter, correct);
            }
        }
    }

}

DEF_GPUTEST_FOR_CONTEXTS(GrDefaultPathRendererTest,
                         sk_gpu_test::GrContextFactory::IsRenderingContext,
                         reporter, ctxInfo, only_allow_default) {
    GrContext* ctx = ctxInfo.grContext();

    run_test(ctx, reporter);
}

DEF_GPUTEST_FOR_CONTEXTS(GrMSAAPathRendererTest,
                         sk_gpu_test::GrContextFactory::IsRenderingContext,
                         reporter, ctxInfo, allow_default_and_msaa) {
    GrContext* ctx = ctxInfo.grContext();

    if (!ctx->caps()->sampleShadingSupport()) {   // The MSAAPathRenderer requires this
        return;
    }

    run_test(ctx, reporter);
}

#endif
