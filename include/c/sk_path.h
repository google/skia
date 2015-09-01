/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_path_DEFINED
#define sk_path_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef enum {
    CW_SK_PATH_DIRECTION,
    CCW_SK_PATH_DIRECTION,
} sk_path_direction_t;

SK_API sk_path_t* sk_path_new();
SK_API void sk_path_delete(sk_path_t*);

SK_API void sk_path_move_to(sk_path_t*, float x, float y);
SK_API void sk_path_line_to(sk_path_t*, float x, float y);
SK_API void sk_path_quad_to(sk_path_t*, float x0, float y0, float x1, float y1);
SK_API void sk_path_conic_to(sk_path_t*, float x0, float y0, float x1, float y1, float w);
SK_API void sk_path_cubic_to(sk_path_t*,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2);
SK_API void sk_path_close(sk_path_t*);

SK_API void sk_path_add_rect(sk_path_t*, const sk_rect_t*, sk_path_direction_t);
SK_API void sk_path_add_oval(sk_path_t*, const sk_rect_t*, sk_path_direction_t);

/**
 *  If the path is empty, return false and set the rect parameter to [0, 0, 0, 0].
 *  else return true and set the rect parameter to the bounds of the control-points
 *  of the path.
 */
SK_API bool sk_path_get_bounds(const sk_path_t*, sk_rect_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
