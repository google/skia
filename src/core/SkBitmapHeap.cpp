/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapHeap.h"

SkBitmapHeap::SkBitmapHeap(int maxCount, size_t maxBytes)
    : fMaxBytes(maxBytes)
    , fCurrBytes(0)
    , fMaxCount(maxCount)
{
    SkASSERT(maxCount >= 0);
}

SkBitmapHeap::~SkBitmapHeap() {
    fImages.unrefAll();
}

static size_t approx_image_size(SkImage* img) {
    return img->width() * img->height() << 2;
}

sk_sp<SkImage> SkBitmapHeap::bitmapToImage(const SkBitmap& bitmap) {
    this->validate();

    int64_t size64 = bitmap.computeSafeSize64();
    if (size64 > (int64_t)fMaxBytes) {
        return SkImage::MakeFromBitmap(bitmap);
    }

    const uint32_t genID = bitmap.getGenerationID();
    int index = fImages.select([genID](SkImage* elem) {
        return elem->uniqueID() == genID;
    });
    if (index >= 0) {
        return sk_ref_sp(fImages[index]);
    }

    this->makeRoomFor(SkToSizeT(size64));

    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bitmap);
    // our LRU is most-recent is at head of the list [index 0]
    *fImages.insert(0) = SkRef(img.get());
    this->validate();
    return img;
}

void SkBitmapHeap::removeTail() {
    this->validate();
    SkImage* img = fImages[fImages.count() - 1];
    size_t size = approx_image_size(img);
    img->unref();
    SkASSERT(fCurrBytes >= size);
    fCurrBytes -= size;
    fImages.setCount(fImages.count() - 1);
    this->validate();
}

void SkBitmapHeap::makeRoomFor(size_t newBitmapSize) {
    SkASSERT(newBitmapSize <= fMaxBytes);
    fCurrBytes += newBitmapSize;

    // purge bottom-up as needed
    while (fImages.count() >= fMaxCount || fCurrBytes > fMaxBytes) {
        this->removeTail();
    }
}

#ifdef SK_DEBUG
void SkBitmapHeap::validate() const {
    SkASSERT(fImages.count() <= fMaxCount);

    size_t bytes = 0;
    (void)fImages.select([&bytes](SkImage* elem) {
        bytes += approx_image_size(elem);
        return false;
    });
    SkASSERT(bytes <= fMaxBytes);
}
#endif
