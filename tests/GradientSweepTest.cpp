/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkImageInfo.h"
#include "Test.h"

static const int kWidth = 32;
static const int kHeight = 32;

DEF_TEST(GradientSweep, reporter) {
    SkPaint p;
    const SkScalar cx = SkIntToScalar(kWidth) / 2;
    const SkScalar cy = SkIntToScalar(kHeight) / 2;
    const SkColor colors[4] = {SK_ColorTRANSPARENT, SK_ColorRED, SK_ColorBLUE, SK_ColorWHITE};
    p.setShader(SkGradientShader::MakeSweep(cx, cy, colors, /* pos */ nullptr, 4));

    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(kWidth, kHeight, kN32_SkColorType, kPremul_SkAlphaType));
    SkCanvas canvas(bm);
    canvas.drawPaint(p);
    canvas.flush();
    for (int j = 0; j < kHeight; j++) {
        for (int i = 0; i < kWidth; i++) {
            uint32_t c = *bm.getAddr32(i, j);
            uint32_t a = SkGetPackedA32(c);
            REPORTER_ASSERT(reporter, a >= SkGetPackedR32(c));
            REPORTER_ASSERT(reporter, a >= SkGetPackedG32(c));
            REPORTER_ASSERT(reporter, a >= SkGetPackedB32(c));
        }
    }
}
