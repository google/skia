/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"

DEF_GPUTEST_FOR_ALL_CONTEXTS(skbug12214, r, contextInfo) {
    auto imageInfo = SkImageInfo::Make(/*width=*/32, /*height=*/32, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    sk_sp<SkSurface> surface1 = SkSurface::MakeRenderTarget(contextInfo.directContext(),
                                                            SkBudgeted::kNo, imageInfo);
    sk_sp<SkSurface> surface2 = SkSurface::MakeRaster(imageInfo);

    // The test succeeds if this draw does not crash. (See skia:12214)
    surface1->draw(surface2->getCanvas(), /*x=*/0, /*y=*/0);
}
