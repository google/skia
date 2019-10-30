/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_drawable_DEFINED
#define sk_drawable_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef struct sk_drawable_t sk_drawable_t;

SK_C_API void sk_drawable_unref (sk_drawable_t*);
SK_C_API uint32_t sk_drawable_get_generation_id (sk_drawable_t*);
SK_C_API void sk_drawable_get_bounds (sk_drawable_t*, sk_rect_t*);
SK_C_API void sk_drawable_draw (sk_drawable_t*, sk_canvas_t*, const sk_matrix_t*);
SK_C_API sk_picture_t* sk_drawable_new_picture_snapshot(sk_drawable_t*);
SK_C_API void sk_drawable_notify_drawing_changed (sk_drawable_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
