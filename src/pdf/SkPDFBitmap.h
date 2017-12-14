/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFBitmap_DEFINED
#define SkPDFBitmap_DEFINED

#include "SkRefCnt.h"

class SkImage;
#ifdef SK_SUPPORT_LEGACY_PDF_PIXELSERIALIZER
class SkPixelSerializer;
#endif
class SkPDFObject;

/**
 * SkPDFBitmap wraps a SkImage and serializes it as an image Xobject.
 * It is designed to use a minimal amout of memory, aside from refing
 * the image, and its emitObject() does not cache any data.
 */
sk_sp<SkPDFObject> SkPDFCreateBitmapObject(sk_sp<SkImage>,
#ifdef SK_SUPPORT_LEGACY_PDF_PIXELSERIALIZER
                                           SkPixelSerializer*,
#endif
                                           int encodingQuality);

#endif  // SkPDFBitmap_DEFINED
