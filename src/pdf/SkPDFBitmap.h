/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFBitmap_DEFINED
#define SkPDFBitmap_DEFINED

#include "SkPDFTypes.h"
#include "SkBitmap.h"

class SkPDFCanon;

/**
 * SkPDFBitmap wraps a SkBitmap and serializes it as an image Xobject.
 * It is designed to use a minimal amout of memory, aside from refing
 * the bitmap's pixels, and its emitObject() does not cache any data.
 *
 * If !bitmap.isImmutable(), then a copy of the bitmap must be made;
 * there is no way around this.
 *
 * The SkPDFBitmap::Create function will check the canon for duplicates.
 */
class SkPDFBitmap : public SkPDFObject {
public:
    // Returns NULL on unsupported bitmap;
    static SkPDFBitmap* Create(SkPDFCanon*, const SkBitmap&);
    bool equals(const SkBitmap& other) const {
        return fBitmap.getGenerationID() == other.getGenerationID() &&
               fBitmap.pixelRefOrigin() == other.pixelRefOrigin() &&
               fBitmap.dimensions() == other.dimensions();
    }

protected:
    const SkBitmap fBitmap;
    SkPDFBitmap(const SkBitmap& bm) : fBitmap(bm) {}
};

#endif  // SkPDFBitmap_DEFINED
