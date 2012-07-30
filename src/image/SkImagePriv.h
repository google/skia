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
extern SkImage* SkNewImageFromPixelRef(const SkImage::Info&, SkPixelRef*,
                                       size_t rowBytes);

/**
 *  Examines the bitmap to decide if it can share the existing pixelRef, or
 *  if it needs to make a deep-copy of the pixels
 */
extern SkImage* SkNewImageFromBitmap(const SkBitmap&);

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

#endif
