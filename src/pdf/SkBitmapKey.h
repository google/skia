/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBitmapKey_DEFINED
#define SkBitmapKey_DEFINED

#include "SkBitmap.h"
#include "SkImage.h"
#include "SkCanvas.h"

class SkBitmapKey {
public:
    SkBitmapKey() : fSubset(SkIRect::MakeEmpty()), fID(0) {}
    explicit SkBitmapKey(const SkBitmap& bm)
        : fSubset(bm.getSubset()), fID(bm.getGenerationID()) {}
    explicit SkBitmapKey(const SkImage* img)
        : fSubset(img ? img->bounds() : SkIRect::MakeEmpty())
        , fID(img ? img->uniqueID() : 0) {}
    explicit SkBitmapKey(const sk_sp<SkImage> img)
        : fSubset(img->bounds()), fID(img->uniqueID()) {}
    bool operator==(const SkBitmapKey& rhs) const {
        return fID == rhs.fID && fSubset == rhs.fSubset;
    }
    bool operator!=(const SkBitmapKey& rhs) const { return !(*this == rhs); }
    uint32_t id() const { return fID; }

private:
    SkIRect fSubset;
    uint32_t fID;
};

/**
   This wraps a thing that could either be a bitmap or a image and
   abstracts out some common tasks.
 */
class SkImageBitmap {
public:
    explicit SkImageBitmap(const SkBitmap& b) : fBitmap(b), fImage(nullptr) {}
    explicit SkImageBitmap(SkImage* i) : fImage(i) { SkASSERT(fImage); }
    SkIRect bounds() const { return fImage ? fImage->bounds() : fBitmap.bounds(); }
    SkISize dimensions() const {
        return fImage ? fImage->dimensions() : fBitmap.dimensions();
    }
    sk_sp<SkImage> makeImage() const {
        return fImage ? sk_ref_sp(fImage) : SkImage::MakeFromBitmap(fBitmap);
    }
    SkBitmapKey getKey() const {
        return fImage ? SkBitmapKey(fImage) : SkBitmapKey(fBitmap);
    }
    void draw(SkCanvas* canvas, SkPaint* paint) const {
        if (fImage) {
            canvas->drawImage(fImage, 0, 0, paint);
        } else {
            canvas->drawBitmap(fBitmap, 0, 0, paint);
        }
    }

private:
    SkBitmap fBitmap;
    SkImage* fImage; // non-owning; when drawImage starts passing a sk_sp<>,
                     // we can take a const ref to that sk_sp<>.
};

#endif  // SkBitmapKey_DEFINED
