/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkKeyedImage.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "src/image/SkImage_Base.h"

#include <utility>

SkBitmapKey SkBitmapKeyFromImage(const SkImage* image) {
    if (!image) {
        return {{0, 0, 0, 0}, 0};
    }
    if (const SkBitmap* bm = as_IB(image)->onPeekBitmap()) {
        SkIPoint o = bm->pixelRefOrigin();
        return {image->bounds().makeOffset(o), bm->getGenerationID()};
    }
    return {image->bounds(), image->uniqueID()};
}

SkKeyedImage::SkKeyedImage(sk_sp<SkImage> i) : fImage(std::move(i)) {
    fKey = SkBitmapKeyFromImage(fImage.get());
}

SkKeyedImage::SkKeyedImage(const SkBitmap& bm) : fImage(bm.asImage()) {
    if (fImage) {
        fKey = {bm.getSubset(), bm.getGenerationID()};
    }
}

SkKeyedImage SkKeyedImage::subset(SkIRect subset) const {
    SkKeyedImage img;
    if (fImage && subset.intersect(fImage->bounds())) {
        img.fImage = fImage->makeSubset(nullptr, subset);
        if (img.fImage) {
            img.fKey = {subset.makeOffset(fKey.fSubset.topLeft()), fKey.fID};
        }
    }
    return img;
}

sk_sp<SkImage> SkKeyedImage::release() {
    sk_sp<SkImage> image = std::move(fImage);
    SkASSERT(nullptr == fImage);
    fKey = {{0, 0, 0, 0}, 0};
    return image;
}
