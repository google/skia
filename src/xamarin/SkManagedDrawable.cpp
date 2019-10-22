/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkManagedDrawable.h"

SkManagedDrawable::Procs SkManagedDrawable::fProcs;

void SkManagedDrawable::setProcs(SkManagedDrawable::Procs procs) {
    fProcs = procs;
}

SkManagedDrawable::SkManagedDrawable(void* context) {
    fContext = context;
}
SkManagedDrawable::~SkManagedDrawable() {
    if (!fProcs.fDestroy) return;
    fProcs.fDestroy(this, fContext);
}

void SkManagedDrawable::onDraw(SkCanvas* canvas) {
    if (!fProcs.fDraw) return;
    fProcs.fDraw(this, fContext, canvas);
}
SkRect SkManagedDrawable::onGetBounds() {
    SkRect rect;
    if (fProcs.fGetBounds)
        fProcs.fGetBounds(this, fContext, &rect);
    return rect;
}
SkPicture* SkManagedDrawable::onNewPictureSnapshot() {
    if (!fProcs.fNewPictureSnapshot) return nullptr;
    return fProcs.fNewPictureSnapshot(this, fContext);
}
