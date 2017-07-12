/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkKeyedImage.h"

#include "SkImage_Base.h"

SkKeyedImage::SkKeyedImage(sk_sp<SkImage> i) : fImage(std::move(i)) {
    if (fImage) {
        if (const SkBitmap* bm = as_IB(fImage.get())->onPeekBitmap()) {
            SkIPoint o = bm->pixelRefOrigin();
            fKey = {fImage->bounds().makeOffset(o.fX, o.fY), bm->getGenerationID()};
        } else {
            fKey = {fImage->bounds(), fImage->uniqueID()};
        }
    }
}

SkKeyedImage::SkKeyedImage(const SkBitmap& bm) : fImage(SkImage::MakeFromBitmap(bm)) {
    if (fImage) {
        fKey = {bm.getSubset(), bm.getGenerationID()};
    }
}

SkKeyedImage SkKeyedImage::subset(SkIRect subset) const {
    SkKeyedImage img;
    if (fImage && subset.intersect(fImage->bounds())) {
        img.fImage = fImage->makeSubset(subset);
        if (img.fImage) {
            img.fKey = {subset.makeOffset(fKey.fSubset.x(), fKey.fSubset.y()), fKey.fID};
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
