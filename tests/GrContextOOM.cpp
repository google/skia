/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "tests/Test.h"

DEF_GPUTEST(GrContext_oomed, reporter, originalOptions) {
    GrContextOptions options = originalOptions;
    options.fRandomGLOOM = true;
    options.fSkipGLErrorChecks = GrContextOptions::Enable::kNo;
    sk_gpu_test::GrContextFactory factory(options);
    for (int ct = 0; ct < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++ct) {
        auto contextType = static_cast<sk_gpu_test::GrContextFactory::ContextType>(ct);
        auto context = factory.get(contextType);
        if (!context) {
            continue;
        }
        if (context->backend() != GrBackendApi::kOpenGL) {
            continue;
        }
        auto info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        for (int run = 0; run < 20; ++run) {
            bool oomed = false;
            for (int i = 0; i < 500; ++i) {
                // Make sure we're actually making a significant number of GL calls and not just
                // issuing a small number calls by reusing scratch resources created in a previous
                // iteration.
                context->freeGpuResources();
                auto surf =
                        SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info, 1, nullptr);
                SkPaint paint;
                surf->getCanvas()->drawRect(SkRect::MakeLTRB(100, 100, 2000, 2000), paint);
                surf->flushAndSubmit();
                if ((oomed = context->oomed())) {
                    REPORTER_ASSERT(reporter, !context->oomed(), "oomed() wasn't cleared");
                    break;
                }
            }
            if (!oomed) {
                ERRORF(reporter, "Did not OOM on %dth run.", run);
            }
        }
    }
}
