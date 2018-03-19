/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkSurface.h"
#include "SkTableColorFilter.h"

#include "Test.h"

#if SK_SUPPORT_GPU // These are all GPU-backend specific tests


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureStripAtlasManagerGradientTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    static const SkColor gColors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    static const SkScalar gPos[] = { 0, SK_ScalarHalf, SK_Scalar1 };

    SkPaint p;
    p.setShader(SkGradientShader::MakeTwoPointConical(SkPoint::Make(0, 0),
                                                      1.0f,
                                                      SkPoint::Make(10.0f, 20.0f),
                                                      2.0f,
                                                      gColors,
                                                      gPos,
                                                      3,
                                                      SkShader::kClamp_TileMode));

    SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    SkRect r = SkRect::MakeXYWH(10, 10, 100, 100);

    canvas->drawRect(r, p);

    context->abandonContext();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureStripAtlasManagerColorFilterTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    uint8_t identity[256];
    for (int i = 0; i < 256; i++) {
        identity[i] = i;
    }

    SkPaint p;
    p.setColorFilter(SkTableColorFilter::Make(identity));

    SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    SkRect r = SkRect::MakeXYWH(10, 10, 100, 100);

    canvas->drawRect(r, p);

    context->abandonContext();
}

#endif
