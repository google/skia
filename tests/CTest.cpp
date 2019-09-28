/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/c/sk_canvas.h"
#include "include/c/sk_imageinfo.h"
#include "include/c/sk_paint.h"
#include "include/c/sk_shader.h"
#include "include/c/sk_surface.h"
#include "include/c/sk_types.h"
#include "tests/Test.h"

#include <stddef.h>
#include <stdint.h>

#include "../../src/c/sk_types_priv.h"

static void shader_test(skiatest::Reporter* reporter) {
    sk_imageinfo_t* info = sk_imageinfo_new(64, 64, RGBA_8888_SK_COLORTYPE, PREMUL_SK_ALPHATYPE,
                                            NULL);
    sk_surface_t* surface  = sk_surface_new_raster(info, nullptr);
    sk_canvas_t* canvas = sk_surface_get_canvas(surface);
    sk_paint_t* paint = sk_paint_new();

    sk_point_t point = { 0.0f, 0.0f };
    sk_point_t point2 = { 30.0f, 40.0f };
    sk_color_t colors[] = {
        (sk_color_t)sk_color_set_argb(0xFF, 0x00, 0x00, 0xFF),
        (sk_color_t)sk_color_set_argb(0xFF, 0x00, 0xFF, 0x00)
    };
    sk_shader_t* shader;

    shader = sk_shader_new_radial_gradient(
        &point, 1.0f, colors, nullptr, 2, CLAMP_SK_SHADER_TILEMODE, nullptr);
    REPORTER_ASSERT(reporter, shader != nullptr);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_paint(canvas, paint);

    shader = sk_shader_new_sweep_gradient(
        &point, colors, nullptr, 2, CLAMP_SK_SHADER_TILEMODE, 0, 360, nullptr);
    REPORTER_ASSERT(reporter, shader != nullptr);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_paint(canvas, paint);

    shader = sk_shader_new_two_point_conical_gradient(
        &point, 10.0f,  &point2, 50.0f, colors, nullptr, 2, CLAMP_SK_SHADER_TILEMODE, nullptr);
    REPORTER_ASSERT(reporter, shader != nullptr);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_paint(canvas, paint);

    sk_paint_delete(paint);
    sk_surface_unref(surface);
    sk_imageinfo_delete(info);
}

static void test_c(skiatest::Reporter* reporter) {
    sk_imageinfo_t* info = sk_imageinfo_new(1, 1, RGBA_8888_SK_COLORTYPE, PREMUL_SK_ALPHATYPE,
                                            NULL);
    uint32_t pixel[1] = { 0 };
    sk_surfaceprops_t* props = sk_surfaceprops_new((sk_surfaceprops_flags_t)0, UNKNOWN_SK_PIXELGEOMETRY);

    sk_surface_t* surface = sk_surface_new_raster_direct(info, pixel, sizeof(uint32_t),
                                                         &surfaceProps);
    sk_paint_t* paint = sk_paint_new();

    sk_canvas_t* canvas = sk_surface_get_canvas(surface);
    REPORTER_ASSERT(reporter, canvas != nullptr);

    sk_paint_t* paint = sk_paint_new();
    REPORTER_ASSERT(reporter, paint != nullptr);

    sk_canvas_draw_paint(canvas, paint);
    REPORTER_ASSERT(reporter, 0xFF000000 == pixel[0]);

    sk_paint_set_color(paint, sk_color_set_argb(0xFF, 0xFF, 0xFF, 0xFF));
    sk_canvas_draw_paint(canvas, paint);
    REPORTER_ASSERT(reporter, 0xFFFFFFFF == pixel[0]);

    sk_paint_set_blendmode(paint, SRC_SK_BLENDMODE);
    sk_paint_set_color(paint, sk_color_set_argb(0x80, 0x80, 0x80, 0x80));
    sk_canvas_draw_paint(canvas, paint);
    REPORTER_ASSERT(reporter, 0x80404040 == pixel[0]);

    sk_paint_delete(paint);
    sk_surface_unref(surface);
    sk_imageinfo_delete(info);
}

static void null_imageinfo_test(skiatest::Reporter* reporter) {
    auto cppinfo = SkImageInfo::Make(0, 0, (SkColorType)0, (SkAlphaType)0, nullptr);
    auto cinfo = ToImageInfo(&cppinfo);

    REPORTER_ASSERT(reporter, cinfo->width == 0);
    REPORTER_ASSERT(reporter, cinfo->height == 0);
    REPORTER_ASSERT(reporter, cinfo->colorType == (sk_colortype_t)0);
    REPORTER_ASSERT(reporter, cinfo->alphaType == (sk_alphatype_t)0);
    REPORTER_ASSERT(reporter, cinfo->colorspace == nullptr);
}

static void nonnull_imageinfo_test(skiatest::Reporter* reporter) {
    sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB();
    sk_colorspace_t* csptr = ToColorSpace(cs.get());

    SkImageInfo cppinfo = SkImageInfo::Make(1, 2, (SkColorType)3, (SkAlphaType)4, cs);
    sk_imageinfo_t* cinfo = ToImageInfo(&cppinfo);

    REPORTER_ASSERT(reporter, cinfo->width == 1);
    REPORTER_ASSERT(reporter, cinfo->height == 2);
    REPORTER_ASSERT(reporter, cinfo->colorType == (sk_colortype_t)3);
    REPORTER_ASSERT(reporter, cinfo->alphaType == (sk_alphatype_t)4);
    REPORTER_ASSERT(reporter, cinfo->colorspace != nullptr);
    REPORTER_ASSERT(reporter, cinfo->colorspace == ToColorSpace(cs.get()));

    sk_imageinfo_t newcinfo = {
        csptr,
        1,
        2,
        (sk_colortype_t)3,
        (sk_alphatype_t)4
    };
    SkImageInfo* newcppinfo = AsImageInfo(&newcinfo);

    REPORTER_ASSERT(reporter, newcppinfo->width() == 1);
    REPORTER_ASSERT(reporter, newcppinfo->height() == 2);
    REPORTER_ASSERT(reporter, newcppinfo->colorType() == (SkColorType)3);
    REPORTER_ASSERT(reporter, newcppinfo->alphaType() == (SkAlphaType)4);
    REPORTER_ASSERT(reporter, newcppinfo->colorSpace() != nullptr);
    REPORTER_ASSERT(reporter, newcppinfo->colorSpace() == AsColorSpace(csptr));
}

DEF_TEST(C_API, reporter) {
    test_c(reporter);
    shader_test(reporter);
    null_imageinfo_test(reporter);
    nonnull_imageinfo_test(reporter);
}
