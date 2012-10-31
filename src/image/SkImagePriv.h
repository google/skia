/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImagePriv_DEFINED
#define SkImagePriv_DEFINED

#include "SkBitmap.h"
#include "SkImage.h"

class SkPicture;

extern SkBitmap::Config SkImageInfoToBitmapConfig(const SkImage::Info&,
                                                  bool* isOpaque);

extern int SkImageBytesPerPixel(SkImage::ColorType);

extern bool SkBitmapToImageInfo(const SkBitmap&, SkImage::Info*);

// Call this if you explicitly want to use/share this pixelRef in the image
extern SkImage* SkNewImageFromPixelRef(const SkImage::Info&, SkPixelRef*,
                                       size_t rowBytes);

/**
 *  Examines the bitmap to decide if it can share the existing pixelRef, or
 *  if it needs to make a deep-copy of the pixels. The bitmap's pixelref will
 *  be shared if either the bitmap is marked as immutable, or canSharePixelRef
 *  is true.
 *
 *  If the bitmap's config cannot be converted into a corresponding
 *  SkImage::Info, or the bitmap's pixels cannot be accessed, this will return
 *  NULL.
 */
extern SkImage* SkNewImageFromBitmap(const SkBitmap&, bool canSharePixelRef);

extern void SkImagePrivDrawPicture(SkCanvas*, SkPicture*,
                                   SkScalar x, SkScalar y, const SkPaint*);

/**
 *  Return an SkImage whose contents are those of the specified picture. Note:
 *  The picture itself is unmodified, and may continue to be used for recording
 */
extern SkImage* SkNewImageFromPicture(const SkPicture*);

static inline size_t SkImageMinRowBytes(const SkImage::Info& info) {
    size_t rb = info.fWidth * SkImageBytesPerPixel(info.fColorType);
    return SkAlign4(rb);
}

// Given an image created from SkNewImageFromBitmap, return its pixelref. This
// may be called to see if the surface and the image share the same pixelref,
// in which case the surface may need to perform a copy-on-write.
extern SkPixelRef* SkBitmapImageGetPixelRef(SkImage* rasterImage);

// Given an image created with NewTexture, return its GrTexture. This
// may be called to see if the surface and the image share the same GrTexture,
// in which case the surface may need to perform a copy-on-write.
extern GrTexture* SkTextureImageGetTexture(SkImage* rasterImage);

// Update the texture wrapped by an image created with NewTexture. This
// is called when a surface and image share the same GrTexture and the
// surface needs to perform a copy-on-write
extern void SkTextureImageSetTexture(SkImage* image, GrTexture* texture);

#endif
