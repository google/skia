/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProvider.h"
#include "SkImage_Base.h"
#include "SkPixelRef.h"

int SkBitmapProvider::width() const {
    return fImage->width();
}

int SkBitmapProvider::height() const {
    return fImage->height();
}

uint32_t SkBitmapProvider::getID() const {
    return fImage->uniqueID();
}

SkImageInfo SkBitmapProvider::info() const {
    return as_IB(fImage)->onImageInfo();
}

bool SkBitmapProvider::isVolatile() const {
    // add flag to images?
    const SkBitmap* bm = as_IB(fImage)->onPeekBitmap();
    return bm ? bm->isVolatile() : false;
}

SkBitmapCacheDesc SkBitmapProvider::makeCacheDesc() const {
    // TODO: This is ugly. Perhaps we should add SkImage_Base::getCacheDesc(colorType, colorSpace),
    // so that we can ask the image what cache entry it's going to use when we call getROPixels?
    if (fImage->isLazyGenerated()) {
        return SkBitmapCacheDesc::Make(fImage->uniqueID(), fLazyColorType, fLazyColorSpace,
                                       SkIRect::MakeWH(fImage->width(), fImage->height()));
    } else {
        return SkBitmapCacheDesc::Make(fImage);
    }
}

void SkBitmapProvider::notifyAddedToCache() const {
    as_IB(fImage)->notifyAddedToRasterCache();
}

bool SkBitmapProvider::asBitmap(SkBitmap* bm) const {
    return as_IB(fImage)->getROPixels(bm, fLazyColorType, fLazyColorSpace);
}
