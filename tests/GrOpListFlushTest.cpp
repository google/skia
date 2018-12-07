/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "Test.h"

DEF_GPUTEST_FOR_ALL_CONTEXTS(GrOpListFlushCount, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGpu* gpu = context->contextPriv().getGpu();

    SkImageInfo imageInfo = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface1 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, imageInfo);
    if (!surface1) {
        return;
    }
    sk_sp<SkSurface> surface2 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, imageInfo);
    if (!surface2) {
        return;
    }

    surface1->getCanvas()->clear(SK_ColorGREEN);
    surface2->getCanvas()->clear(SK_ColorRED);

    for (int i = 0; i < 1000; ++i) {
        surface2->draw(surface1->getCanvas(), 0, 0, nullptr);
        surface1->draw(surface2->getCanvas(), 0, 0, nullptr);
    }
    context->flush();

    // In total we make 2000 oplists. Our current limit on max oplists between flushes is 100, so we
    // should do 20 flushes while executing oplists. Additionaly we always do 1 flush at the end of
    // executing all oplists. So in total we should see 21 flushes here.
    REPORTER_ASSERT(reporter, gpu->stats()->numFinishFlushes() == 21);
}
