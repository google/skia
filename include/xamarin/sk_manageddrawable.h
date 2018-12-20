/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_manageddrawable_DEFINED
#define sk_manageddrawable_DEFINED

#include "sk_xamarin.h"

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD


typedef struct sk_manageddrawable_t sk_manageddrawable_t;


typedef void          (*sk_manageddrawable_draw_delegate)               (sk_manageddrawable_t* cmanagedDrawable, sk_canvas_t* ccanvas);
typedef void          (*sk_manageddrawable_getBounds_delegate)          (sk_manageddrawable_t* cmanagedDrawable, sk_rect_t* rect);
typedef sk_picture_t* (*sk_manageddrawable_newPictureSnapshot_delegate) (sk_manageddrawable_t* cmanagedDrawable);


SK_X_API sk_manageddrawable_t* sk_manageddrawable_new (void);
SK_X_API void sk_manageddrawable_destroy (sk_manageddrawable_t*);
SK_X_API void sk_manageddrawable_set_delegates (const sk_manageddrawable_draw_delegate pDraw,
                                                const sk_manageddrawable_getBounds_delegate pGetBounds,
                                                const sk_manageddrawable_newPictureSnapshot_delegate pNewPictureSnapshot);
SK_X_API uint32_t sk_manageddrawable_get_generation_id (sk_manageddrawable_t*);
SK_X_API void sk_manageddrawable_get_bounds (sk_manageddrawable_t*, sk_rect_t*);
SK_X_API void sk_manageddrawable_draw (sk_manageddrawable_t*, sk_canvas_t*, const sk_matrix_t*);
SK_X_API sk_picture_t* sk_manageddrawable_new_picture_snapshot(sk_manageddrawable_t*);
SK_X_API void sk_manageddrawable_notify_drawing_changed (sk_manageddrawable_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
