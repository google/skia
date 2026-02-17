/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlurTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMask.h"
#include "src/core/SkPathPriv.h"
#include "tests/Test.h"

#include <memory>

// b/476169487
DEF_TEST(DrawToMask_RejectsBigPath, reporter) {
    SkPathBuilder pb;
    pb.moveTo(2.44905932e-05f, 0.0625f);
    pb.quadTo({2.44905932e-05f, -2.12843282e+38f}, {2.44905932e-05f, 0.0625f});
    pb.lineTo(2.44905932e-05f, 0.0625f);

    auto raw = SkPathPriv::Raw(pb, SkResolveConvexity::kNo);
    REPORTER_ASSERT(reporter, raw.has_value());

    SkIRect clip = {0, 0, 0, 0};
    SkMaskBuilder dst;
    auto filter = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f);
    SkMatrix matrix = SkMatrix::I();

    // This crashes without the fix
    skcpu::DrawToMask(*raw,
                      clip,
                      filter.get(),
                      &matrix,
                      &dst,
                      SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode,
                      SkStrokeRec::kHairline_InitStyle);
    SkAutoMaskFreeImage autoFree(dst.image());
}
