/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

DEF_GANESH_TEST(GrContext_oomed, reporter, originalOptions, CtsEnforcement::kApiLevel_T) {
    GrContextOptions options = originalOptions;
    options.fRandomGLOOM = true;
    options.fSkipGLErrorChecks = GrContextOptions::Enable::kNo;
    sk_gpu_test::GrContextFactory factory(options);
    for (int ct = 0; ct < skgpu::kContextTypeCount; ++ct) {
        auto contextType = static_cast<skgpu::ContextType>(ct);
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
                        SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info, 1, nullptr);
                SkPaint paint;
                surf->getCanvas()->drawRect(SkRect::MakeLTRB(100, 100, 2000, 2000), paint);
                context->flushAndSubmit(surf.get(), GrSyncCpu::kNo);
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
