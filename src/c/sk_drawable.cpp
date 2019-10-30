/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDrawable.h"

#include "include/c/sk_drawable.h"
#include "include/c/sk_picture.h"
#include "src/c/sk_types_priv.h"


void sk_drawable_unref(sk_drawable_t* drawable)
{
    SkSafeUnref(AsDrawable(drawable));
}

uint32_t sk_drawable_get_generation_id(sk_drawable_t* d)
{
    return AsDrawable(d)->getGenerationID();
}

void sk_drawable_get_bounds(sk_drawable_t* d, sk_rect_t* rect)
{
    *rect = ToRect(AsDrawable(d)->getBounds());
}

void sk_drawable_draw(sk_drawable_t* d, sk_canvas_t* c, const sk_matrix_t* matrix)
{
    SkMatrix m;
    if (matrix) {
        m = AsMatrix(matrix);
    }
    AsDrawable(d)->draw(AsCanvas(c), matrix ? &m : nullptr);
}

sk_picture_t* sk_drawable_new_picture_snapshot(sk_drawable_t* d)
{
    return ToPicture(AsDrawable(d)->newPictureSnapshot());
}

void sk_drawable_notify_drawing_changed(sk_drawable_t* d)
{
    AsDrawable(d)->notifyDrawingChanged();
}
