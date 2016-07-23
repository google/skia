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

static sk_sp<SkShader> make_opaque_color() {
    return SkShader::MakeColorShader(0xFFFF0000);
}

static sk_sp<SkShader> make_alpha_color() {
    return SkShader::MakeColorShader(0x80FF0000);
}

static sk_sp<SkColorFilter> make_cf_null() {
    return nullptr;
}

static sk_sp<SkColorFilter> make_cf0() {
    SkColorMatrix cm;
    cm.setSaturation(0.75f);
    return SkColorFilter::MakeMatrixFilterRowMajor255(cm.fMat);
}

static sk_sp<SkColorFilter> make_cf1() {
    SkColorMatrix cm;
    cm.setSaturation(0.75f);
    auto a(SkColorFilter::MakeMatrixFilterRowMajor255(cm.fMat));
    // CreateComposedFilter will try to concat these two matrices, resulting in a single
    // filter (which is good for speed). For this test, we want to force a real compose of
    // these two, so our inner filter has a scale-up, which disables the optimization of
    // combining the two matrices.
    cm.setScale(1.1f, 0.9f, 1);
    auto b(SkColorFilter::MakeMatrixFilterRowMajor255(cm.fMat));
    return SkColorFilter::MakeComposeFilter(a, b);
}

static sk_sp<SkColorFilter> make_cf2() {
    return SkColorFilter::MakeModeFilter(0x8044CC88, SkXfermode::kSrcATop_Mode);
}

static void draw_into_canvas(SkCanvas* canvas) {
    const SkRect r = SkRect::MakeWH(50, 100);
    sk_sp<SkShader> (*shaders[])() { make_opaque_color, make_alpha_color };
    sk_sp<SkColorFilter> (*filters[])() { make_cf_null, make_cf0, make_cf1, make_cf2 };

    SkPaint paint;
    for (auto shProc : shaders) {
        paint.setShader(shProc());
        for (auto cfProc : filters) {
            paint.setColorFilter(cfProc());
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

    sk_sp<SkColorSpace> colorSpaces[]{
        nullptr,
        SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named)
    };
    for (auto colorSpace : colorSpaces) {
        const SkImageInfo info = SkImageInfo::Make(1024, 100, kN32_SkColorType, kPremul_SkAlphaType,
                                                   colorSpace);
        auto surface(SkSurface::MakeRaster(info));
        surface->getCanvas()->drawPaint(bg);
        draw_into_canvas(surface->getCanvas());
        surface->draw(canvas, 0, 0, nullptr);
        canvas->translate(0, 120);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkColorSpace.h"

DEF_SIMPLE_GM(color4shader, canvas, 1024, 260) {
    canvas->translate(10, 10);

    SkMatrix44 mat(SkMatrix44::kUninitialized_Constructor);
    // red -> blue, green -> red, blue -> green
    mat.set3x3(0, 1, 0, 0, 0, 1, 1, 0, 0);

    const SkColor4f colors[] {
        { 1, 0, 0, 1 },
        { 0, 1, 0, 1 },
        { 0, 0, 1, 1 },
        { 0.5, 0.5, 0.5, 1 },
    };

    SkPaint paint;
    SkRect r = SkRect::MakeWH(100, 100);

    for (const auto& c4 : colors) {
        sk_sp<SkShader> shaders[] {
            SkShader::MakeColorShader(c4, nullptr),
            SkShader::MakeColorShader(c4, SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named)),
            SkShader::MakeColorShader(c4, SkColorSpace::NewRGB(SkColorSpace::kLinear_GammaNamed,
                                                               mat)),
        };

        canvas->save();
        for (const auto& s : shaders) {
            paint.setShader(s);
            canvas->drawRect(r, paint);
            canvas->translate(r.width() * 6 / 5, 0);
        }
        canvas->restore();
        canvas->translate(0, r.height() * 6 / 5);
    }
}
