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
class SkPDFObject;

/**
 * SkPDFBitmap wraps a SkImage and serializes it as an image Xobject.
 * It is designed to use a minimal amout of memory, aside from refing
 * the image, and its emitObject() does not cache any data.
 *
 *  quality > 100 means lossless
 */
sk_sp<SkPDFObject> SkPDFCreateBitmapObject(sk_sp<SkImage>, int encodingQuality = 101);

#endif  // SkPDFBitmap_DEFINED
