/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImagePriv_DEFINED
#define SkImagePriv_DEFINED

#include "SkImage.h"

// Call this if you explicitly want to use/share this pixelRef in the image
extern SkImage* SkNewImageFromPixelRef(const SkImageInfo&, SkPixelRef*,
                                       size_t rowBytes);

/**
 *  Examines the bitmap to decide if it can share the existing pixelRef, or
 *  if it needs to make a deep-copy of the pixels. The bitmap's pixelref will
 *  be shared if either the bitmap is marked as immutable, or canSharePixelRef
 *  is true.
 *
 *  If the bitmap's colortype cannot be converted into a corresponding
 *  SkImageInfo, or the bitmap's pixels cannot be accessed, this will return
 *  NULL.
 */
extern SkImage* SkNewImageFromBitmap(const SkBitmap&, bool canSharePixelRef);

static inline size_t SkImageMinRowBytes(const SkImageInfo& info) {
    return SkAlign4(info.minRowBytes());
}

// Given an image created from SkNewImageFromBitmap, return its pixelref. This
// may be called to see if the surface and the image share the same pixelref,
// in which case the surface may need to perform a copy-on-write.
extern SkPixelRef* SkBitmapImageGetPixelRef(SkImage* rasterImage);

// Given an image created with NewTexture, return its GrTexture. This
// may be called to see if the surface and the image share the same GrTexture,
// in which case the surface may need to perform a copy-on-write.
extern GrTexture* SkTextureImageGetTexture(SkImage* textureImage);

// Update the texture wrapped by an image created with NewTexture. This
// is called when a surface and image share the same GrTexture and the
// surface needs to perform a copy-on-write
extern void SkTextureImageSetTexture(SkImage* image, GrTexture* texture);

#endif
