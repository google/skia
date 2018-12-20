/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkManagedDrawable.h"

#include "sk_manageddrawable.h"
#include "sk_picture.h"
#include "sk_types_priv.h"

/* This is a C wrapper araound the C++ class, so that it is possible
   to access the C++ from C#.                                        */
   
static sk_manageddrawable_draw_delegate               gDraw;
static sk_manageddrawable_getBounds_delegate          gGetBounds;
static sk_manageddrawable_newPictureSnapshot_delegate gNewPictureSnapshot;

static inline SkManagedDrawable* AsManagedDrawable(sk_manageddrawable_t* cdrawable) 
{
    return reinterpret_cast<SkManagedDrawable*>(cdrawable);
}

static inline sk_manageddrawable_t* ToManagedDrawable(SkManagedDrawable* drawable) 
{
    return reinterpret_cast<sk_manageddrawable_t*>(drawable);
}

static inline const sk_manageddrawable_t* ToManagedDrawable(const SkManagedDrawable* drawable) 
{
    return reinterpret_cast<const sk_manageddrawable_t*>(drawable);
}

void dDraw(SkManagedDrawable* managedDrawable, SkCanvas* canvas)
{
    gDraw(ToManagedDrawable(managedDrawable), ToCanvas(canvas));
}

void dGetBounds(SkManagedDrawable* managedDrawable, SkRect* rect) 
{
    gGetBounds(ToManagedDrawable(managedDrawable), ToRect(rect));
}

SkPicture* dNewPictureSnapshot(SkManagedDrawable* managedDrawable) 
{
    return AsPicture(gNewPictureSnapshot(ToManagedDrawable(managedDrawable)));
}

sk_manageddrawable_t* sk_manageddrawable_new()
{ 
	return ToManagedDrawable(new SkManagedDrawable());
}

void sk_manageddrawable_destroy(sk_manageddrawable_t* drawable)
{ 
    delete AsManagedDrawable(drawable);
}

void sk_manageddrawable_set_delegates(const sk_manageddrawable_draw_delegate pDraw,
                                      const sk_manageddrawable_getBounds_delegate pGetBounds,
                                      const sk_manageddrawable_newPictureSnapshot_delegate pNewPictureSnapshot)
{
    gDraw = pDraw;
    gGetBounds = pGetBounds;
    gNewPictureSnapshot = pNewPictureSnapshot;

    SkManagedDrawable::setDelegates(dDraw, dGetBounds, dNewPictureSnapshot);
}

uint32_t sk_manageddrawable_get_generation_id(sk_manageddrawable_t* d)
{ 
	return AsManagedDrawable(d)->getGenerationID();
}

void sk_manageddrawable_get_bounds(sk_manageddrawable_t* d, sk_rect_t* rect)
{ 
    *rect = ToRect(AsManagedDrawable(d)->getBounds()); 
}

void sk_manageddrawable_draw(sk_manageddrawable_t* d, sk_canvas_t* c, const sk_matrix_t* matrix)
{
    AsManagedDrawable(d)->draw(AsCanvas(c), &AsMatrix(matrix));
}

sk_picture_t* sk_manageddrawable_new_picture_snapshot(sk_manageddrawable_t* d)
{ 
	return ToPicture(AsManagedDrawable(d)->newPictureSnapshot());
}

void sk_manageddrawable_notify_drawing_changed(sk_manageddrawable_t* d)
{
    AsManagedDrawable(d)->notifyDrawingChanged();
}
