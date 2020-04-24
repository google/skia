/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"

// This passes by not crashing.
static void test(SkCanvas* canvas) {
    canvas->scale(63, 0);
    canvas->drawString("A", 50, 50, SkFont(), SkPaint());
}

DEF_TEST(skbug5221, r) {
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(256, 256)));
    test(surface->getCanvas());
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(skbug5221_GPU, r, contextInfo) {
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(
            contextInfo.grContext(), SkBudgeted::kYes,
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType)));
    test(surface->getCanvas());
}
