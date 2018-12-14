/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkManagedDrawable.h"

#include "sk_manageddrawable.h"
#include "sk_types_priv.h"


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

void dDraw(SkManagedDrawable* managedDrawable, sk_canvas_t canvas)
{
    return gDraw(ToManagedDrawable(managedDrawable), canvas);
}

sk_rect_t dGetBounds(SkManagedDrawable* managedDrawable) 
{
    gGetBounds(ToManagedDrawable(managedDrawableDrawable));
}

sk_picture_t dNewPictureSnapshot(const SkManagedDrawable* managedDrawable) 
{
    return gNewPictureSnapshot(ToManagedDrawable(managedDrawable));
}

sk_manageddrawable_t* sk_manageddrawable_new()
{ 
	return ToManagedDrawable(new SkManagedDrawable());
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
