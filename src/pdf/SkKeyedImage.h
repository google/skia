/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkKeyedImage_DEFINED
#define SkKeyedImage_DEFINED

#include "SkBitmap.h"
#include "SkBitmapKey.h"
#include "SkImage.h"

/**
   This class has all the advantages of SkBitmaps and SkImages.

   The SkImage holds on to encoded data.  The SkBitmapKey properly de-dups subsets.
 */
class SkKeyedImage {
public:
    SkKeyedImage() {}
    SkKeyedImage(sk_sp<const SkImage> img);
    SkKeyedImage(const SkBitmap& bitmap);
    SkKeyedImage(SkKeyedImage&&) = default;
    SkKeyedImage(const SkKeyedImage&) = default;

    SkKeyedImage& operator=(SkKeyedImage&&) = default;
    SkKeyedImage& operator=(const SkKeyedImage&) = default;

    explicit operator bool() const { return fImage; }
    const SkBitmapKey& key() const { return fKey; }
    const SkImage* image() const { return fImage.get(); }
    const sk_sp<const SkImage>& ref() const { return fImage; }
    SkKeyedImage subset(SkIRect subset) const;

private:
    sk_sp<const SkImage> fImage;
    SkBitmapKey fKey = {{0, 0, 0, 0}, 0};
};
#endif  // SkKeyedImage_DEFINED
