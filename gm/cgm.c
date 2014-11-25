/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#include "sk_canvas.h"
#include "sk_paint.h"
#include "sk_shader.h"
#include "sk_surface.h"

extern void sk_test_c_api(sk_canvas_t*);

#define W   256
#define H   256

static sk_shader_t* make_shader() {
    sk_point_t pts[] = { { 0, 0 }, { W, H } };
    sk_color_t colors[] = { 0xFF00FF00, 0xFF0000FF };
    return sk_shader_new_linear_gradient(pts, colors, NULL, 2, CLAMP_SK_SHADER_TILEMODE, NULL);
}

void sk_test_c_api(sk_canvas_t* canvas) {
    sk_paint_t* paint = sk_paint_new();
    sk_paint_set_antialias(paint, true);

    sk_paint_set_color(paint, 0xFFFFFFFF);
    sk_canvas_draw_paint(canvas, paint);

    sk_rect_t r = { 10, 10, W - 10, H - 10 };

    sk_paint_set_color(paint, 0xFFFF0000);
    sk_canvas_draw_rect(canvas, &r, paint);

    sk_shader_t* shader = make_shader();
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);

    sk_canvas_draw_oval(canvas, &r, paint);

    sk_paint_delete(paint);
}


