/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_paint_DEFINED
#define sk_paint_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

sk_paint_t* sk_paint_new();
void sk_paint_delete(sk_paint_t*);

bool sk_paint_is_antialias(const sk_paint_t*);
void sk_paint_set_antialias(sk_paint_t*, bool);

sk_color_t sk_paint_get_color(const sk_paint_t*);
void sk_paint_set_color(sk_paint_t*, sk_color_t);

/* stroke settings */

bool sk_paint_is_stroke(const sk_paint_t*);
void sk_paint_set_stroke(sk_paint_t*, bool);

float sk_paint_get_stroke_width(const sk_paint_t*);
void sk_paint_set_stroke_width(sk_paint_t*, float width);

float sk_paint_get_stroke_miter(const sk_paint_t*);
void sk_paint_set_stroke_miter(sk_paint_t*, float miter);

typedef enum {
    BUTT_SK_STROKE_CAP,
    ROUND_SK_STROKE_CAP,
    SQUARE_SK_STROKE_CAP
} sk_stroke_cap_t;

sk_stroke_cap_t sk_paint_get_stroke_cap(const sk_paint_t*);
void sk_paint_set_stroke_cap(sk_paint_t*, sk_stroke_cap_t);

typedef enum {
    MITER_SK_STROKE_JOIN,
    ROUND_SK_STROKE_JOIN,
    BEVEL_SK_STROKE_JOIN
} sk_stroke_join_t;

sk_stroke_join_t sk_paint_get_stroke_join(const sk_paint_t*);
void sk_paint_set_stroke_join(sk_paint_t*, sk_stroke_join_t);

/**
 *  Set the paint's shader to the specified parameter. This will automatically call unref() on
 *  any previous value, and call ref() on the new value.
 */
void sk_paint_set_shader(sk_paint_t*, sk_shader_t*);

/**
 *  Set the paint's maskfilter to the specified parameter. This will automatically call unref() on
 *  any previous value, and call ref() on the new value.
 */
void sk_paint_set_maskfilter(sk_paint_t*, sk_maskfilter_t*);

/**
 *  Set the paint's xfermode to the specified parameter.
 */
void sk_paint_set_xfermode_mode(sk_paint_t*, sk_xfermode_mode_t);

SK_C_PLUS_PLUS_END_GUARD

#endif
