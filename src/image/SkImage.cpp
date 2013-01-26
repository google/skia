/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkBitmap.h"
#include "SkCanvas.h"

SK_DEFINE_INST_COUNT(SkImage)

static SkImage_Base* asIB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

uint32_t SkImage::NextUniqueID() {
    static int32_t gUniqueID;

    // never return 0;
    uint32_t id;
    do {
        id = sk_atomic_inc(&gUniqueID) + 1;
    } while (0 == id);
    return id;
}

void SkImage::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                   const SkPaint* paint) {
    asIB(this)->onDraw(canvas, x, y, paint);
}
