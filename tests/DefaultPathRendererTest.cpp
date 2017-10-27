/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrRenderTargetContext.h"
#include "GrStyle.h"
#include "GrTypesPriv.h"
#include "effects/GrPorterDuffXferProcessor.h"

static void modify_context_options(GrContextOptions* options) {
    // Only allow the default path renderer
    options->fGpuPathRenderers = GpuPathRenderers::kNone;
}


// From crbug.com/769898:
//   create an approx fit render target context that will have extra space (i.e., npot)
//   draw an inverse wound concave path into it - forcing use of the GrDefaultPathRenderer
//   throw it away so the backing GrSurface/GrStencilBuffer can be reused
//   create a new render target context that will reuse the prior GrSurface
//   draw into the portion outside of the approx fit RTC's content rect
//
// When the bug manifests the GrDefaultPathRenderer was leaving the stencil buffer outside
// of the content rect in a bad state and the second draw would be incorrect.
DEF_GPUTEST_FOR_ALL_CONTEXTS_WITH_FILTER(GrDefaultPathRendererTest, reporter, ctxInfo, modify_context_options) {
    GrContext* const ctx = ctxInfo.grContext();

    {
        auto rtc =  ctx->makeDeferredRenderTargetContext(SkBackingFit::kApprox, 1025, 1025,
                                                         kRGBA_8888_GrPixelConfig, nullptr);

        SkPath p;
        p.addRect(SkRect::MakeXYWH(256, 256, 512, 512), SkPath::kCW_Direction);
        p.addRect(SkRect::MakeXYWH(257, 257, 510, 510), SkPath::kCCW_Direction);
        p.setFillType(SkPath::kInverseWinding_FillType);

        GrPaint paint;
        paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
    //    if (fp) {
    //        paint.addColorFragmentProcessor(std::move(fp));
    //    }

        GrStyle style(SkStrokeRec::kFill_InitStyle);

        rtc->drawPath(GrNoClip::GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), p, style);

        rtc->prepareForExternalIO(0, nullptr);
    }

    {
        auto rtc = ctx->makeDeferredRenderTargetContext(SkBackingFit::kExact, 2048, 2048,
                                                        kRGBA_8888_GrPixelConfig, nullptr);

        SkRect r = SkRect::MakeXYWH(1, 1, 2046, 2046);

        GrPaint paint;
        paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));



        rtc->drawRect(GrNoClip::GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r, nullptr);

        SkImageInfo dstII = SkImageInfo::MakeN32Premul(2048, 2048);

        SkBitmap bm;
        bm.allocPixels(dstII);

        rtc->readPixels(dstII, bm.getAddr(0, 0), bm.rowBytes(), 0, 0, 0);
    }

}

#endif
