/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/NearestDecalSubsetTestCases.h"
#include "tests/Test.h"

struct GrContextOptions;

// Tests that GrTextureEffect's kClampToBorder_Nearest shader mode clamps texture lookup
// coordinates to the texel-center-inset subset rect before sampling. The cases are shared with
// Graphite's ImageShader_NearestDecalSubset (see tests/NearestDecalSubsetTestCases.h); Ganesh
// additionally runs them against both top-left and bottom-left origin source textures.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrTextureEffect_NearestDecalSubset,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrDirectContext* dContext = ctxInfo.directContext();
    if (dContext->supportsProtectedContent()) {
        return;
    }

    for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        skiatest::ReporterContext originCtx(
                reporter,
                SkString(origin == kTopLeft_GrSurfaceOrigin ? "TopLeft" : "BottomLeft"));

        skiatest::NearestDecalSubset::Factories factories;
        factories.makeSourceSurface = [dContext, origin](const SkImageInfo& info) {
            return SkSurfaces::RenderTarget(
                    dContext, skgpu::Budgeted::kYes, info, /*sampleCount=*/1, origin, nullptr);
        };
        factories.makeTargetSurface = [dContext](const SkImageInfo& info) {
            return SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, info);
        };
        factories.makeImage = [](sk_sp<SkSurface> surface) {
            return surface->makeImageSnapshot();
        };
        skiatest::NearestDecalSubset::Run(reporter, factories);
    }
}
