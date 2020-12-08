/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkColorMatrix.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(MatrixColorFilter_TransparentBlack, reporter, info) {
    auto context = info.directContext();
    // Make a transparent black image rather than use a paint color to avoid an optimization that
    // applies the color filter on the CPU to paint colors.
    auto imgSurf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                               SkImageInfo::MakeN32(5, 5, kPremul_SkAlphaType));
    imgSurf->getCanvas()->drawColor(0x0000000);
    auto shader = imgSurf->makeImageSnapshot()->makeShader(SkSamplingOptions());
    SkColorMatrix m;
    m.setScale(0, 0, 0, 127.f);
    SkPaint p;
    p.setColorFilter(SkColorFilters::Matrix(m));
    p.setShader(shader);
    p.setBlendMode(SkBlendMode::kSrc);
    auto surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                            SkImageInfo::MakeN32(5, 5, kPremul_SkAlphaType));
    // Seed the output surface with red so we would notice if we failed to draw at all.
    surf->getCanvas()->clear(SK_ColorRED);
    surf->getCanvas()->drawPaint(p);
    SkAutoPixmapStorage pixels;
    pixels.alloc(surf->imageInfo());
    surf->readPixels(pixels, 0, 0);
    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter](int x, int y, const float diffs[4]) {
                ERRORF(reporter, "Expected transparent black, instead got (%f, %f, %f, %f)",
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    static constexpr float kTol[] = {0, 0, 0, 0};
    CheckSolidPixels({0, 0, 0, 0}, pixels, kTol, error);
}
