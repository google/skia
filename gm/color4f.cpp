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

static SkColorFilter* make_cf1() {
    SkColorMatrix cm;
    cm.setSaturation(0.75f);
    SkAutoTUnref<SkColorFilter> a(SkColorMatrixFilter::Create(cm));
    // CreateComposedFilter will try to concat these two matrices, resulting in a single
    // filter (which is good for speed). For this test, we want to force a real compose of
    // these two, so our inner filter has a scale-up, which disables the optimization of
    // combining the two matrices.
    cm.setScale(1.1f, 0.9f, 1);
    SkAutoTUnref<SkColorFilter> b(SkColorMatrixFilter::Create(cm));
    return SkColorFilter::CreateComposeFilter(a, b);
}

static SkColorFilter* make_cf2() {
    return SkColorFilter::CreateModeFilter(0x8044CC88, SkXfermode::kSrcATop_Mode);
}

static void draw_into_canvas(SkCanvas* canvas) {
    const SkRect r = SkRect::MakeWH(50, 100);
    SkShader* (*shaders[])() { make_opaque_color, make_alpha_color };
    SkColorFilter* (*filters[])() { make_cf_null, make_cf0, make_cf1, make_cf2 };
    
    SkPaint paint;
    for (auto shProc : shaders) {
        paint.setShader(shProc())->unref();
        for (auto cfProc : filters) {
            SkSafeUnref(paint.setColorFilter(cfProc()));
            canvas->drawRect(r, paint);
            canvas->translate(60, 0);
        }
    }
}

DEF_SIMPLE_GM(color4f, canvas, 1024, 260) {
    canvas->translate(10, 10);

    SkPaint bg;
    // need the target to be opaque, so we can draw it to the screen
    // even if it holds sRGB values.
    bg.setColor(0xFFFFFFFF);

    SkColorProfileType const profiles[] { kLinear_SkColorProfileType, kSRGB_SkColorProfileType };
    for (auto profile : profiles) {
        const SkImageInfo info = SkImageInfo::Make(1024, 100, kN32_SkColorType, kPremul_SkAlphaType,
                                                   profile);
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
        surface->getCanvas()->drawPaint(bg);
        draw_into_canvas(surface->getCanvas());
        surface->draw(canvas, 0, 0, nullptr);
        canvas->translate(0, 120);
    }
}
