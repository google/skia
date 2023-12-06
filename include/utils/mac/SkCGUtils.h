
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCGUtils_DEFINED
#define SkCGUtils_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#endif

class SkBitmap;
class SkColorSpace;
class SkData;
class SkPixmap;
class SkStreamRewindable;

SK_API CGContextRef SkCreateCGContext(const SkPixmap&);

/**
 *  Given a CGImage, allocate an SkBitmap and copy the image's pixels into it. If scaleToFit is not
 *  null, use it to determine the size of the bitmap, and scale the image to fill the bitmap.
 *  Otherwise use the image's width/height.
 *
 *  On failure, return false, and leave bitmap unchanged.
 */
SK_API bool SkCreateBitmapFromCGImage(SkBitmap* dst, CGImageRef src);

SK_API sk_sp<SkImage> SkMakeImageFromCGImage(CGImageRef);

/**
 *  Given a CGColorSpace, return the closest matching SkColorSpace. If no conversion is possible
 *  or if the input CGColorSpace is nullptr then return nullptr.
 */
SK_API sk_sp<SkColorSpace> SkMakeColorSpaceFromCGColorSpace(CGColorSpaceRef);

/**
 *  Copy the pixels from src into the memory specified by info/rowBytes/dstPixels. On failure,
 *  return false (e.g. ImageInfo incompatible with src).
 */
SK_API bool SkCopyPixelsFromCGImage(const SkImageInfo& info, size_t rowBytes, void* dstPixels,
                                    CGImageRef src);
static inline bool SkCopyPixelsFromCGImage(const SkPixmap& dst, CGImageRef src) {
    return SkCopyPixelsFromCGImage(dst.info(), dst.rowBytes(), dst.writable_addr(), src);
}

/**
 *  Create an imageref from the specified bitmap. The color space parameter is ignored.
 */
SK_API CGImageRef SkCreateCGImageRefWithColorspace(const SkBitmap& bm,
                                                   CGColorSpaceRef space);

/**
 *  Create an imageref from the specified bitmap.
 */
SK_API CGImageRef SkCreateCGImageRef(const SkBitmap& bm);

/**
 *  Given an SkColorSpace, create a CGColorSpace. This will return sRGB if the specified
 *  SkColorSpace is nullptr or on failure. This will not retain the specified SkColorSpace.
 */
SK_API CGColorSpaceRef SkCreateCGColorSpace(const SkColorSpace*);

/**
 *  Given an SkData, create a CGDataProviderRef that refers to the and retains the specified data.
 */
SK_API CGDataProviderRef SkCreateCGDataProvider(sk_sp<SkData>);

/**
 *  Draw the bitmap into the specified CG context. (x,y) specifies the position of the top-left
 *  corner of the bitmap.
 */
void SkCGDrawBitmap(CGContextRef, const SkBitmap&, float x, float y);

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#endif  // SkCGUtils_DEFINED
