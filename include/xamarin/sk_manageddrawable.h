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

typedef void          (*sk_manageddrawable_draw_proc)               (sk_manageddrawable_t* d, void* context, sk_canvas_t* ccanvas);
typedef void          (*sk_manageddrawable_getBounds_proc)          (sk_manageddrawable_t* d, void* context, sk_rect_t* rect);
typedef sk_picture_t* (*sk_manageddrawable_newPictureSnapshot_proc) (sk_manageddrawable_t* d, void* context);
typedef void          (*sk_manageddrawable_destroy_proc)            (sk_manageddrawable_t* d, void* context);

typedef struct {
    sk_manageddrawable_draw_proc fDraw;
    sk_manageddrawable_getBounds_proc fGetBounds;
    sk_manageddrawable_newPictureSnapshot_proc fNewPictureSnapshot;
    sk_manageddrawable_destroy_proc fDestroy;
} sk_manageddrawable_procs_t;

SK_X_API sk_manageddrawable_t* sk_manageddrawable_new(void* context);
SK_X_API void sk_manageddrawable_unref(sk_manageddrawable_t*);
SK_X_API void sk_manageddrawable_set_procs(sk_manageddrawable_procs_t procs);

SK_C_PLUS_PLUS_END_GUARD

#endif
