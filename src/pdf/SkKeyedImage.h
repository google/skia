/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkKeyedImage_DEFINED
#define SkKeyedImage_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "src/pdf/SkBitmapKey.h"

/**
   This class has all the advantages of SkBitmaps and SkImages.

   The SkImage holds on to encoded data.  The SkBitmapKey properly de-dups subsets.
 */
class SkKeyedImage {
public:
    SkKeyedImage() {}
    SkKeyedImage(sk_sp<SkImage>);
    SkKeyedImage(const SkBitmap&);
    SkKeyedImage(SkKeyedImage&&) = default;
    SkKeyedImage(const SkKeyedImage&) = default;

    SkKeyedImage& operator=(SkKeyedImage&&) = default;
    SkKeyedImage& operator=(const SkKeyedImage&) = default;

    explicit operator bool() const { return fImage != nullptr; }
    const SkBitmapKey& key() const { return fKey; }
    const sk_sp<SkImage>& image() const { return fImage; }
    sk_sp<SkImage> release();
    SkKeyedImage subset(SkIRect subset) const;

private:
    sk_sp<SkImage> fImage;
    SkBitmapKey fKey = {{0, 0, 0, 0}, 0};
};

/**
 *  Given an Image, return the Bitmap Key that corresponds to it.  If the Image
 *  wraps a Bitmap, use that Bitmap's key.
 */
SkBitmapKey SkBitmapKeyFromImage(const SkImage*);
#endif  // SkKeyedImage_DEFINED
