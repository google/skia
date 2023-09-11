/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

struct GrContextOptions;

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(skbug12214, r, contextInfo, CtsEnforcement::kApiLevel_T) {
    auto imageInfo = SkImageInfo::Make(/*width=*/32, /*height=*/32, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    sk_sp<SkSurface> surface1 =
            SkSurfaces::RenderTarget(contextInfo.directContext(), skgpu::Budgeted::kNo, imageInfo);
    sk_sp<SkSurface> surface2 = SkSurfaces::Raster(imageInfo);

    // The test succeeds if this draw does not crash. (See skia:12214)
    surface1->draw(surface2->getCanvas(), /*x=*/0, /*y=*/0);
}
