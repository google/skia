/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapHeap_DEFINED
#define SkBitmapHeap_DEFINED

#include "SkBitmap.h"
#include "SkImage.h"
#include "../../private/SkTDArray.h"

class SkBitmapHeap {
public:
    SkBitmapHeap(int maxEntries, size_t maxBytes);
    ~SkBitmapHeap();

    sk_sp<SkImage> bitmapToImage(const SkBitmap&);

private:
    SkTDArray<SkImage*> fImages;
    const size_t        fMaxBytes;
    size_t              fCurrBytes;
    const int           fMaxCount;

    void removeTail();
    void makeRoomFor(size_t);

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};

#endif
