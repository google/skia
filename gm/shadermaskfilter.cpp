/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkShaderMaskFilter.h"

static void draw_masked_image(SkCanvas* canvas, const SkImage* image, SkScalar x, SkScalar y,
                              const SkImage* mask) {
    SkMatrix matrix = SkMatrix::MakeScale(SkIntToScalar(image->width()) / mask->width(),
                                          SkIntToScalar(image->height() / mask->height()));
    SkPaint paint;
    paint.setMaskFilter(SkShaderMaskFilter::Make(mask->makeShader(&matrix)));
    canvas->drawImage(image, x, y, &paint);
}

#include "SkGradientShader.h"
static sk_sp<SkShader> make_shader(const SkRect& r) {
    const SkPoint pts[] = {
        { r.fLeft, r.fTop }, { r.fRight, r.fBottom },
    };
    const SkColor colors[] = { 0, SK_ColorWHITE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kRepeat_TileMode);
}

DEF_SIMPLE_GM(shadermaskfilter_gradient, canvas, 512, 512) {
    SkRect r = { 0, 0, 100, 150 };
    auto shader = make_shader(r);
    auto mf = SkShaderMaskFilter::Make(shader);

    canvas->translate(20, 20);
    canvas->scale(2, 2);

    SkPaint paint;
    paint.setMaskFilter(mf);
    paint.setColor(SK_ColorRED);
    canvas->drawOval(r, paint);
}

#include "Resources.h"
DEF_SIMPLE_GM(shadermaskfilter_image, canvas, 512, 512) {
    canvas->scale(1.25f, 1.25f);

    auto image = GetResourceAsImage("images/mandrill_128.png");
    auto mask = GetResourceAsImage("images/color_wheel.png");

    canvas->drawImage(image, 10, 10, nullptr);
    canvas->drawImage(mask, 10 + image->width() + 10.f, 10, nullptr);

    draw_masked_image(canvas, image.get(), 10, 10 + image->height() + 10.f, mask.get());
}
