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

static inline SkManagedDrawable* AsManagedDrawable(sk_manageddrawable_t* d) {
    return reinterpret_cast<SkManagedDrawable*>(d);
}
static inline sk_manageddrawable_t* ToManagedDrawable(SkManagedDrawable* d) {
    return reinterpret_cast<sk_manageddrawable_t*>(d);
}

static sk_manageddrawable_procs_t gProcs;

void dDraw(SkManagedDrawable* d, void* context, SkCanvas* canvas) {
    if (!gProcs.fDraw) return;
    gProcs.fDraw(ToManagedDrawable(d), context, ToCanvas(canvas));
}
void dGetBounds(SkManagedDrawable* d, void* context, SkRect* rect) {
    if (!gProcs.fGetBounds) return;
    gProcs.fGetBounds(ToManagedDrawable(d), context, ToRect(rect));
}
SkPicture* dNewPictureSnapshot(SkManagedDrawable* d, void* context) {
    if (!gProcs.fNewPictureSnapshot) return nullptr;
    return AsPicture(gProcs.fNewPictureSnapshot(ToManagedDrawable(d), context));
}
void dDestroy(SkManagedDrawable* d, void* context) {
    if (!gProcs.fDestroy) return;
    gProcs.fDestroy(ToManagedDrawable(d), context);
}

sk_manageddrawable_t* sk_manageddrawable_new(void* context) {
    return ToManagedDrawable(new SkManagedDrawable(context));
}
void sk_manageddrawable_unref(sk_manageddrawable_t* drawable) {
    SkSafeUnref(AsManagedDrawable(drawable));
}
void sk_manageddrawable_set_procs(sk_manageddrawable_procs_t procs) {
    gProcs = procs;

    SkManagedDrawable::Procs p;
    p.fDraw = dDraw;
    p.fGetBounds = dGetBounds;
    p.fNewPictureSnapshot = dNewPictureSnapshot;
    p.fDestroy = dDestroy;

    SkManagedDrawable::setProcs(p);
}
