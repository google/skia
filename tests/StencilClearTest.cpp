/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

namespace {

SkPath make_star() {
    SkPathBuilder starPath;
    starPath.moveTo(0.0f, -33.3333f);
    starPath.lineTo(9.62f, -16.6667f);
    starPath.lineTo(28.867f, -16.6667f);
    starPath.lineTo(19.24f, 0.0f);
    starPath.lineTo(28.867f, 16.6667f);
    starPath.lineTo(9.62f, 16.6667f);
    starPath.lineTo(0.0f, 33.3333f);
    starPath.lineTo(-9.62f, 16.6667f);
    starPath.lineTo(-28.867f, 16.6667f);
    starPath.lineTo(-19.24f, 0.0f);
    starPath.lineTo(-28.867f, -16.6667f);
    starPath.lineTo(-9.62f, -16.6667f);
    starPath.close();

    return starPath.detach();
}

sk_sp<SkSurface> gpu_surface(GrDirectContext* dContext) {
    SkSurfaceProps props(SkSurfaceProps::kDynamicMSAA_Flag, kUnknown_SkPixelGeometry);

    SkImageInfo ii = SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    return SkSurfaces::RenderTarget(dContext,
                                    skgpu::Budgeted::kYes,
                                    ii,
                                    /* sampleCount= */ 4,
                                    kTopLeft_GrSurfaceOrigin,
                                    &props,
                                    /*shouldCreateWithMips=*/true);
}

static void disable_split_reduction(GrContextOptions* options) {
    options->fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
}

} // anonymous namespace

// This test exercises b/502351526.
// The ultimate goal is to:
//    create an OpsTask (O1) that uses a stencil buffer (S) and clears it
//    create a new OpsTask (O2) that doesn't use a stencil
//    later, get O2 to add a stencil (and, specifically, reuse S)
// In the bug this will allow uninitialized values to creep into the
// stencil buffer and, potentially, impact later stencil-based rendering.
// When executed this test will trigger an assert if the bug reoccurs.
DEF_GANESH_TEST_FOR_CONTEXTS(StencilClearTest,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             disable_split_reduction,
                             CtsEnforcement::kNextRelease) {
    GrDirectContext* dContext = ctxInfo.directContext();

    SkPath star = make_star();

    // This first draw clears the stencil buffer and fills it with the first star.
    // The OpsTask (O1) has clear/store stencilOps and marks the stencil buffer as cleared.
    sk_sp<SkSurface> s1 = gpu_surface(dContext);
    if (!s1) {
        return;  // Dynamic MSAA isn't supported everywhere
    }

    {
        SkCanvas* canvas = s1->getCanvas();

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(true);

        canvas->concat(SkMatrix::ScaleTranslate(4, 4, 128, 128));
        canvas->drawPath(star, paint);
    }

    // This starts a new Task that doesn't (yet) need stencil.
    // Previously, this OpsTask (O2) would have discard/store stencilOps. With
    // the fix this will now have clear/store stencilOps.
    sk_sp<SkSurface> s2 = gpu_surface(dContext);
    {
        SkCanvas* canvas = s2->getCanvas();

        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeWH(256, 256), paint);
    }

    // This block just serves to close the active Task from s2
    {
        sk_sp<SkSurface> s3 = gpu_surface(dContext);
        SkCanvas* canvas = s3->getCanvas();

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeWH(256, 256), paint);
    }

    // This adds a new Task on s2 that requires a stencil. It retroactively sets the
    // needsStencil flag on the proxy used by OpsTask2.
    // The OpsTask (O3) has load/store stencilOps.
    {
        SkCanvas* canvas = s2->getCanvas();

        SkPaint paint;
        paint.setColor(SK_ColorYELLOW);
        paint.setAntiAlias(true);

        canvas->concat(SkMatrix::ScaleTranslate(4, 4, 128, 128));
        canvas->drawPath(star, paint);
    }

    dContext->flush();
    dContext->submit(GrSyncCpu::kYes);
}
