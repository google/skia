/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkCanvas.h"
#include "SkSurface.h"

// This passes by not crashing.
static void test(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->scale(63, 0);
    static const char kTxt[] = "A";
    canvas->drawText(kTxt, SK_ARRAY_COUNT(kTxt), 50, 50, paint);
}

DEF_TEST(skbug5221, r) {
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(256, 256)));
    test(surface->getCanvas());
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_ALL_CONTEXTS(skbug5221_GPU, r, contextInfo) {
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(
            contextInfo.grContext(), SkBudgeted::kYes,
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType)));
    test(surface->getCanvas());
}
#endif
