/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

static void only_allow_small(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kSmall;
}

DEF_GANESH_TEST_FOR_CONTEXTS(SmallPathRenderer_crbug505876830,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             only_allow_small,
                             CtsEnforcement::kNever) {
    auto ctx = ctxInfo.directContext();
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(
                ctx, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
                {100, 100}, SkSurfaceProps(), /*label=*/{});
    if (!sdc) {
        return;
    }
    sdc->clear(SK_PMColor4fBLACK);

    GrStyle style(SkStrokeRec::kFill_InitStyle);
    const SkPath smolpath = SkPathBuilder()
        .moveTo(-0.25000017881393432617f, -0.25000017881393432617f)
        .lineTo( 0.25000017881393432617f, -0.25000017881393432617f)
        .lineTo( 0.25000017881393432617f,  0.25000017881393432617f)
        .close()
        .detach();
    const SkMatrix m = SkMatrix::MakeAll(
                      -0.0f, 70368341861093281422320336896.0f, -590295810358705651712.0f,
                       2.0f,          17469300226849243136.0f,                      0.0f,
         868052350533632.0f,     4220791729285326409039872.0f,           3572.759765625f
    );

    // Passes if we don't assert.
    sdc->drawPath(nullptr, GrPaint(), GrAA::kYes, m, smolpath, style);
    ctx->flushAndSubmit();
}
