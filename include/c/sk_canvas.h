/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_canvas_DEFINED
#define sk_canvas_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

void sk_canvas_save(sk_canvas_t*);
void sk_canvas_save_layer(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_restore(sk_canvas_t*);

void sk_canvas_translate(sk_canvas_t*, float dx, float dy);
void sk_canvas_scale(sk_canvas_t*, float sx, float sy);
void sk_canvas_rotate_degrees(sk_canvas_t*, float degrees);
void sk_canvas_rotate_radians(sk_canvas_t*, float radians);
void sk_canvas_skew(sk_canvas_t*, float sx, float sy);
void sk_canvas_concat(sk_canvas_t*, const sk_matrix_t*);

void sk_canvas_clip_rect(sk_canvas_t*, const sk_rect_t*);
void sk_canvas_clip_path(sk_canvas_t*, const sk_path_t*);

void sk_canvas_draw_paint(sk_canvas_t*, const sk_paint_t*);
void sk_canvas_draw_rect(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_oval(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_path(sk_canvas_t*, const sk_path_t*, const sk_paint_t*);
void sk_canvas_draw_image(sk_canvas_t*, const sk_image_t*, float x, float y, const sk_paint_t*);
void sk_canvas_draw_image_rect(sk_canvas_t*, const sk_image_t*, const sk_rect_t* src,
                               const sk_rect_t* dst, const sk_paint_t*);
void sk_canvas_draw_picture(sk_canvas_t*, const sk_picture_t*, const sk_matrix_t*, const sk_paint_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
