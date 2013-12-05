/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"

static SkImage_Base* as_IB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

static const SkImage_Base* as_IB(const SkImage* image) {
    return static_cast<const SkImage_Base*>(image);
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
    as_IB(this)->onDraw(canvas, x, y, paint);
}

void SkImage::draw(SkCanvas* canvas, const SkRect* src, const SkRect& dst,
                   const SkPaint* paint) {
    as_IB(this)->onDrawRectToRect(canvas, src, dst, paint);
}

GrTexture* SkImage::getTexture() {
    return as_IB(this)->onGetTexture();
}

SkData* SkImage::encode(SkImageEncoder::Type type, int quality) const {
    SkBitmap bm;
    if (as_IB(this)->getROPixels(&bm)) {
        return SkImageEncoder::EncodeData(bm, type, quality);
    }
    return NULL;
}
