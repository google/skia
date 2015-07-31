/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_canvas.h"
#include "sk_paint.h"
#include "sk_surface.h"

#include "Test.h"

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
}
