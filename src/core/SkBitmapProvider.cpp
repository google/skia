/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProvider.h"
#include "SkImage_Base.h"

SkBitmapCacheDesc SkBitmapProvider::makeCacheDesc() const {
    return SkBitmapCacheDesc::Make(fImage);
}

void SkBitmapProvider::notifyAddedToCache() const {
    as_IB(fImage)->notifyAddedToRasterCache();
}

bool SkBitmapProvider::asBitmap(SkBitmap* bm) const {
    return as_IB(fImage)->getROPixels(bm);
}
