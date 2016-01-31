/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkShader.h"
#include "SkSurface.h"

#include "SkColorMatrixFilter.h"
#include "SkGradientShader.h"

static SkShader* make_opaque_color() {
    return SkShader::CreateColorShader(0xFFFF0000);
}

static SkShader* make_alpha_color() {
    return SkShader::CreateColorShader(0x80FF0000);
}

static SkColorFilter* make_cf_null() {
    return nullptr;
}

static SkColorFilter* make_cf0() {
    SkColorMatrix cm;
    cm.setSaturation(0.75f);
    return SkColorMatrixFilter::Create(cm);
}

static void draw_into_canvas(SkCanvas* canvas) {
    const SkRect r = SkRect::MakeWH(100, 100);
    SkShader* (*shaders[])() { make_opaque_color, make_alpha_color };
    SkColorFilter* (*filters[])() { make_cf_null, make_cf0 };
    
    SkPaint paint;
    for (auto shProc : shaders) {
        paint.setShader(shProc())->unref();
        for (auto cfProc : filters) {
            SkSafeUnref(paint.setColorFilter(cfProc()));
            canvas->drawRect(r, paint);
            canvas->translate(120, 0);
        }
    }
}

DEF_SIMPLE_GM(color4f, canvas, 510, 250) {
    canvas->translate(20, 20);

    SkPaint bg;
    // need the target to be opaque, so we can draw it to the screen
    // even if it holds sRGB values.
    bg.setColor(0xFFFFFFFF);

    SkColorProfileType const profiles[] { kLinear_SkColorProfileType, kSRGB_SkColorProfileType };
    for (auto profile : profiles) {
        const SkImageInfo info = SkImageInfo::Make(500, 100, kN32_SkColorType, kPremul_SkAlphaType,
                                                   profile);
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
        surface->getCanvas()->drawPaint(bg);
        draw_into_canvas(surface->getCanvas());
        surface->draw(canvas, 0, 0, nullptr);
        canvas->translate(0, 120);
    }
}
