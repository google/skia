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
#include "SkPixelRef.h"

struct SkBitmapKey {
    SkIRect fSubset;
    uint32_t fID;
    bool operator==(const SkBitmapKey& rhs) const {
        return fID == rhs.fID && fSubset == rhs.fSubset;
    }
    bool operator!=(const SkBitmapKey& rhs) const { return !(*this == rhs); }
};

/**
   This class has all the advantages of SkBitmaps and SkImages.
 */
class SkImageSubset {
public:
    explicit SkImageSubset(const SkBitmap& b) {
        SkASSERT(!b.drawsNothing());
        fSubset = b.getSubset();
        SkAutoLockPixels autoLockPixels(b);
        SkASSERT(b.pixelRef());
        SkBitmap tmp;
        tmp.setInfo(b.pixelRef()->info(), b.rowBytes());
        tmp.setPixelRef(b.pixelRef());
        tmp.lockPixels();
        fImage = SkImage::MakeFromBitmap(tmp);
        if (fImage) {
            SkASSERT(!b.isImmutable() || fImage->uniqueID() == b.getGenerationID());
            SkASSERT(fImage->bounds().contains(fSubset));
        }
    }
    explicit SkImageSubset(sk_sp<SkImage> i) : fImage(std::move(i)) {
        SkASSERT(fImage);
        fSubset = fImage->bounds();
        SkASSERT(fImage->bounds().contains(fSubset));
    }
    SkImageSubset(sk_sp<SkImage> i, SkIRect subset)
        : fSubset(subset), fImage(std::move(i)) {
        SkASSERT(fImage);
        if (!fSubset.intersect(fImage->bounds())) {
            fImage = nullptr;
            fSubset = {0, 0, 0, 0};
        }
    }

    bool good() const { return fImage != nullptr; }
    
    SkIRect bounds() const { return SkIRect::MakeSize(this->dimensions()); }

    SkISize dimensions() const { return fSubset.size(); }

    sk_sp<SkImage> makeImage() const {
        return fSubset == fImage->bounds() ? fImage : fImage->makeSubset(fSubset);
    }

    SkBitmapKey getKey() const { return SkBitmapKey{fSubset, fImage->uniqueID()}; }

    void draw(SkCanvas* canvas, SkPaint* paint) const {
        SkRect src = SkRect::Make(fSubset),
               dst = SkRect::Make(this->bounds());
        canvas->drawImageRect(fImage.get(), src, dst, paint);
    }

private:
    SkIRect fSubset;
    sk_sp<SkImage> fImage;
};

#endif  // SkBitmapKey_DEFINED
