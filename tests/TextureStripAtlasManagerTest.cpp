/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkTableColorFilter.h"
#include "include/gpu/GrDirectContext.h"

#include "tests/Test.h"
#include "tools/Resources.h"

// The gradient shader will use the texture strip atlas if it has too many colors. Make sure
// abandoning the context works.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureStripAtlasManagerGradientTest, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();

    static const SkColor gColors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
                                       SK_ColorCYAN, SK_ColorMAGENTA, SK_ColorYELLOW,
                                       SK_ColorBLACK };
    static const SkScalar gPos[] = { 0, 0.17f, 0.32f, 0.49f, 0.66f, 0.83f, 1.0f };

    SkPaint p;
    p.setShader(SkGradientShader::MakeTwoPointConical(SkPoint::Make(0, 0),
                                                      1.0f,
                                                      SkPoint::Make(10.0f, 20.0f),
                                                      2.0f,
                                                      gColors,
                                                      gPos,
                                                      7,
                                                      SkTileMode::kClamp));

    SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    SkRect r = SkRect::MakeXYWH(10, 10, 100, 100);

    canvas->drawRect(r, p);

    context->abandonContext();
}

// The table color filter uses the texture strip atlas. Make sure abandoning the context works.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextureStripAtlasManagerColorFilterTest, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();

    sk_sp<SkImage> img = GetResourceAsImage("images/mandrill_128.png");


    uint8_t identity[256];
    for (int i = 0; i < 256; i++) {
        identity[i] = i;
    }

    SkPaint p;
    p.setColorFilter(SkTableColorFilter::Make(identity));

    SkImageInfo info = SkImageInfo::MakeN32Premul(128, 128);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
    SkCanvas* canvas = surface->getCanvas();

    canvas->drawImage(std::move(img), 0, 0, SkSamplingOptions(), &p);

    context->abandonContext();
}
