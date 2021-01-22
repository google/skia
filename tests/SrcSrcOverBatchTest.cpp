/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// We want to make sure that if we collapse src-over down to src when blending, that batching still
// works correctly with a draw that explicitly requests src.

#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SrcSrcOverBatchTest, reporter, ctxInfo) {
    auto ctx = ctxInfo.directContext();

    static const int kSize = 8;
    const SkImageInfo ii = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);

    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo,
                                                         ii, 0, kTopLeft_GrSurfaceOrigin,
                                                         nullptr));

    auto canvas = surface->getCanvas();

    SkPaint paint;
    // Setting a shader so that we actually build a processor set and don't fallback to all
    // defaults.
    paint.setShader(SkShaders::Color(SK_ColorRED));

    SkIRect rect = SkIRect::MakeWH(2, 2);

    canvas->drawIRect(rect, paint);

    // Now draw a rect with src blend mode. If we collapsed the previous draw to src blend mode (a
    // setting on caps plus not having any coverage), then we expect this second draw to try to
    // batch with it. This test is a success if we don't hit any asserts, specifically making sure
    // that both things we decided can be batched together claim to have the same value for
    // CompatibleWithCoverageAsAlpha.
    canvas->translate(3, 0);
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawIRect(rect, paint);
}
