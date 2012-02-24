
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCGUtils_DEFINED
#define SkCGUtils_DEFINED

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#endif

class SkBitmap;
class SkStream;

/**
 *  Create an imageref from the specified bitmap using the specified colorspace.
 *  If space is NULL, then CGColorSpaceCreateDeviceRGB() is used.
 */
SK_API CGImageRef SkCreateCGImageRefWithColorspace(const SkBitmap& bm,
                                                   CGColorSpaceRef space);

/**
 *  Create an imageref from the specified bitmap using the colorspace returned
 *  by CGColorSpaceCreateDeviceRGB()
 */
static inline CGImageRef SkCreateCGImageRef(const SkBitmap& bm) {
    return SkCreateCGImageRefWithColorspace(bm, NULL);
}

/**
 *  Draw the bitmap into the specified CG context. The bitmap will be converted
 *  to a CGImage using the generic RGB colorspace. (x,y) specifies the position
 *  of the top-left corner of the bitmap. The bitmap is converted using the
 *  colorspace returned by CGColorSpaceCreateDeviceRGB()
 */
void SkCGDrawBitmap(CGContextRef, const SkBitmap&, float x, float y);

bool SkPDFDocumentToBitmap(SkStream* stream, SkBitmap* output);

/**
 *  Return a provider that wraps the specified stream. It will become an
 *  owner of the stream, so the caller must still manage its ownership.
 *
 *  To hand-off ownership of the stream to the provider, the caller must do
 *  something like the following:
 *
 *  SkStream* stream = new ...;
 *  CGDataProviderRef provider = SkStreamToDataProvider(stream);
 *  stream->unref();
 *
 *  Now when the provider is finally deleted, it will delete the stream.
 */
CGDataProviderRef SkCreateDataProviderFromStream(SkStream*);

#endif
