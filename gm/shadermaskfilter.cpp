/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkShaderMaskFilter.h"

#include "SkGradientShader.h"
static sk_sp<SkShader> make_shader(const SkRect& r) {
    const SkPoint pts[] = {
        { r.fLeft, r.fTop }, { r.fRight, r.fBottom },
    };
    const SkColor colors[] = { 0, SK_ColorWHITE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kRepeat_TileMode);
}

DEF_SIMPLE_GM(shadermaskfilter, canvas, 512, 512) {
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
