/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkManagedDrawable.h"


// delegates
static draw_delegate fDraw = nullptr;
static getBounds_delegate fGetBounds = nullptr;
static newPictureSnapshot_delegate fNewPictureSnapshot = nullptr;


SkManagedDrawable::SkManagedDrawable() {
}

SkManagedDrawable::~SkManagedDrawable() {
}

void SkManagedDrawable::setDelegates(const draw_delegate pDraw,
                                     const getBounds_delegate pGetBounds,
                                     const newPictureSnapshot_delegate pNewPictureSnapshot)
{
    ::fDraw = (pDraw);
    ::fGetBounds = (pGetBounds);
    ::fNewPictureSnapshot = (pNewPictureSnapshot);
}

void SkManagedDrawable::onDraw(SkCanvas* canvas) {
    ::fDraw(this, canvas);
}

SkRect SkManagedDrawable::onGetBounds() {
    return ::fGetBounds(this);
}

SkPicture* SkManagedDrawable::onNewPictureSnapshot() {
    return ::fNewPictureSnapshot(this);
}
