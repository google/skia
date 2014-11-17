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

void sk_canvas_draw_paint(sk_canvas_t*, const sk_paint_t*);
void sk_canvas_draw_rect(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_oval(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_path(sk_canvas_t*, const sk_path_t*, const sk_paint_t*);
void sk_canvas_draw_image(sk_canvas_t*, const sk_image_t*, float x, float y, const sk_paint_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
