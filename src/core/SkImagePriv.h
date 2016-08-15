/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImagePriv_DEFINED
#define SkImagePriv_DEFINED

#include "SkImage.h"
#include "SkSmallAllocator.h"
#include "SkSurface.h"

enum SkCopyPixelsMode {
    kIfMutable_SkCopyPixelsMode,  //!< only copy src pixels if they are marked mutable
    kAlways_SkCopyPixelsMode,     //!< always copy src pixels (even if they are marked immutable)
    kNever_SkCopyPixelsMode,      //!< never copy src pixels (even if they are marked mutable)
};

enum {kSkBlitterContextSize = 5300};  // minimum required for compose shader blitter

// Commonly used allocator. It currently is only used to allocate up to 3 objects. The total
// bytes requested is calculated using one of our large shaders, its context size plus the size of
// an Sk3DBlitter in SkDraw.cpp
// Note that some contexts may contain other contexts (e.g. for compose shaders), but we've not
// yet found a situation where the size below isn't big enough.
typedef SkSmallAllocator<3, kSkBlitterContextSize> SkTBlitterAllocator;

// If alloc is non-nullptr, it will be used to allocate the returned SkShader, and MUST outlive
// the SkShader.
sk_sp<SkShader> SkMakeBitmapShader(const SkBitmap& src, SkShader::TileMode, SkShader::TileMode,
                                   const SkMatrix* localMatrix, SkCopyPixelsMode,
                                   SkTBlitterAllocator* alloc);

// Call this if you explicitly want to use/share this pixelRef in the image
extern sk_sp<SkImage> SkMakeImageFromPixelRef(const SkImageInfo&, SkPixelRef*,
                                              const SkIPoint& pixelRefOrigin,
                                              size_t rowBytes);

/**
 *  Examines the bitmap to decide if it can share the existing pixelRef, or
 *  if it needs to make a deep-copy of the pixels.
 *
 *  The bitmap's pixelref will be shared if either the bitmap is marked as
 *  immutable, or CopyPixelsMode allows it. Shared pixel refs are also
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
extern sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap&, SkCopyPixelsMode,
                                                  SkTBlitterAllocator* = nullptr);

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

GrTexture* GrDeepCopyTexture(GrTexture* src, SkBudgeted);

#endif
