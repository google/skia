/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImagePriv_DEFINED
#define SkImagePriv_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"

class SkPixelRef;

enum SkCopyPixelsMode {
    kIfMutable_SkCopyPixelsMode,  //!< only copy src pixels if they are marked mutable
    kAlways_SkCopyPixelsMode,     //!< always copy src pixels (even if they are marked immutable)
    kNever_SkCopyPixelsMode,      //!< never copy src pixels (even if they are marked mutable)
};

// Convenience function to return a shader that implements the shader+image behavior defined for
// drawImage/Bitmap where the paint's shader is ignored when the bitmap is a color image, but
// properly compose them together when it is an alpha image. This allows the returned paint to
// be assigned to a paint clone without discarding the original behavior.
sk_sp<SkShader> SkMakeBitmapShaderForPaint(const SkPaint& paint, const SkBitmap& src,
                                           SkTileMode, SkTileMode, const SkSamplingOptions&,
                                           const SkMatrix* localMatrix, SkCopyPixelsMode);

// Given arguments for a call to SkCanvas::drawImageRect, modify the SkPaint and return a
// possibly adjusted 'dst' SkRect such that calling SkCanvas::drawRect(newDst, *paint) produces
// visually equivalent results to the original drawImageRect() call.
SkRect SkModifyPaintAndDstForDrawImageRect(const SkImage* image,
                                           const SkSamplingOptions&,
                                           SkRect src,
                                           SkRect dst,
                                           bool strictSrcSubset,
                                           SkPaint* paint);

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
extern SK_SPI sk_sp<SkImage> SkMakeImageFromRasterBitmap(const SkBitmap&, SkCopyPixelsMode);

// Given an image created from SkNewImageFromBitmap, return its pixelref. This
// may be called to see if the surface and the image share the same pixelref,
// in which case the surface may need to perform a copy-on-write.
extern const SkPixelRef* SkBitmapImageGetPixelRef(const SkImage* rasterImage);

#endif
