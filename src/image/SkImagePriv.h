/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImagePriv_DEFINED
#define SkImagePriv_DEFINED

#include "SkImage.h"
#include "SkSurface.h"

// Call this if you explicitly want to use/share this pixelRef in the image
extern SkImage* SkNewImageFromPixelRef(const SkImageInfo&, SkPixelRef*,
                                       const SkIPoint& pixelRefOrigin,
                                       size_t rowBytes);

/**
 *  Examines the bitmap to decide if it can share the existing pixelRef, or
 *  if it needs to make a deep-copy of the pixels.
 *
 *  The bitmap's pixelref will be shared if either the bitmap is marked as
 *  immutable, or forceSharePixelRef is true.  Shared pixel refs are also
 *  locked when kLocked_SharedPixelRefMode is specified.
 *
 *  Passing kLocked_SharedPixelRefMode allows the image's peekPixels() method
 *  to succeed, but it will force any lazy decodes/generators to execute if
 *  they exist on the pixelref.
 *
 *  It is illegal to call this with a texture-backed bitmap.
 *
 *  If the bitmap's colortype cannot be converted into a corresponding
 *  SkImageInfo, or the bitmap's pixels cannot be accessed, this will return
 *  nullptr.
 */
enum ForceCopyMode {
    kNo_ForceCopyMode,
    kYes_ForceCopyMode, // must copy the pixels even if the bitmap is immutable
};
extern SkImage* SkNewImageFromRasterBitmap(const SkBitmap&, ForceCopyMode = kNo_ForceCopyMode);

static inline size_t SkImageMinRowBytes(const SkImageInfo& info) {
    size_t minRB = info.minRowBytes();
    if (kIndex_8_SkColorType != info.colorType()) {
        minRB = SkAlign4(minRB);
    }
    return minRB;
}

// Given an image created from SkNewImageFromBitmap, return its pixelref. This
// may be called to see if the surface and the image share the same pixelref,
// in which case the surface may need to perform a copy-on-write.
extern const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* rasterImage);

// When a texture is shared by a surface and an image its budgeted status is that of the
// surface. This function is used when the surface makes a new texture for itself in order
// for the orphaned image to determine whether the original texture counts against the
// budget or not.
extern void SkTextureImageApplyBudgetedDecision(SkImage* textureImage);

// Update the texture wrapped by an image created with NewTexture. This
// is called when a surface and image share the same GrTexture and the
// surface needs to perform a copy-on-write
extern void SkTextureImageSetTexture(SkImage* image, GrTexture* texture);

GrTexture* GrDeepCopyTexture(GrTexture* src, bool isBudgeted);

#endif
