/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "sk_canvas.h"
#include "sk_paint.h"
#include "sk_surface.h"
#include "sk_shader.h"

static void shader_test(skiatest::Reporter* reporter) {
    sk_imageinfo_t info =
        {64, 64, sk_colortype_get_default_8888(), PREMUL_SK_ALPHATYPE};
    sk_surface_t* surface  = sk_surface_new_raster(&info, nullptr);
    sk_canvas_t* canvas = sk_surface_get_canvas(surface);
    sk_paint_t* paint = sk_paint_new();

    sk_shader_tilemode_t tilemode = CLAMP_SK_SHADER_TILEMODE;
    sk_point_t point = {0.0f, 0.0f};
    sk_point_t point2 = {30.0f, 40.0f};
    sk_color_t colors[] = {
        (sk_color_t)sk_color_set_argb(0xFF, 0x00, 0x00, 0xFF),
        (sk_color_t)sk_color_set_argb(0xFF, 0x00, 0xFF, 0x00)
    };
    sk_shader_t* shader;

    shader = sk_shader_new_radial_gradient(
            &point, 1.0f, colors, nullptr, 2, tilemode, nullptr);
    REPORTER_ASSERT(reporter, shader != nullptr);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_paint(canvas, paint);

    shader = sk_shader_new_sweep_gradient(&point, colors, nullptr, 2, nullptr);
    REPORTER_ASSERT(reporter, shader != nullptr);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_paint(canvas, paint);

    shader = sk_shader_new_two_point_conical_gradient(
            &point, 10.0f,  &point2, 50.0f, colors, nullptr, 2, tilemode, nullptr);
    REPORTER_ASSERT(reporter, shader != nullptr);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_paint(canvas, paint);

    sk_paint_delete(paint);
    sk_surface_unref(surface);
}

static void test_c(skiatest::Reporter* reporter) {
    sk_colortype_t ct = sk_colortype_get_default_8888();

    sk_imageinfo_t info = {
        1, 1, ct, PREMUL_SK_ALPHATYPE
    };
    uint32_t pixel[1] = { 0 };
    sk_surfaceprops_t surfaceProps = { UNKNOWN_SK_PIXELGEOMETRY };

    sk_surface_t* surface = sk_surface_new_raster_direct(&info, pixel, sizeof(uint32_t),
                                                         &surfaceProps);
    sk_paint_t* paint = sk_paint_new();

    sk_canvas_t* canvas = sk_surface_get_canvas(surface);
    sk_canvas_draw_paint(canvas, paint);
    REPORTER_ASSERT(reporter, 0xFF000000 == pixel[0]);

    sk_paint_set_color(paint, sk_color_set_argb(0xFF, 0xFF, 0xFF, 0xFF));
    sk_canvas_draw_paint(canvas, paint);
    REPORTER_ASSERT(reporter, 0xFFFFFFFF == pixel[0]);

    sk_paint_set_xfermode_mode(paint, SRC_SK_XFERMODE_MODE);
    sk_paint_set_color(paint, sk_color_set_argb(0x80, 0x80, 0x80, 0x80));
    sk_canvas_draw_paint(canvas, paint);
    REPORTER_ASSERT(reporter, 0x80404040 == pixel[0]);

    sk_paint_delete(paint);
    sk_surface_unref(surface);
}

DEF_TEST(C_API, reporter) {
    test_c(reporter);
    shader_test(reporter);
}
