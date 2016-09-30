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
    SkImageSubset(sk_sp<SkImage> i, SkIRect subset = {0, 0, 0, 0})
        : fImage(std::move(i)) {
        if (!fImage) {
            fSubset = {0, 0, 0, 0};
            fID = 0;
            return;
        }
        fID = fImage->uniqueID();
        if (subset.isEmpty()) {
            fSubset = fImage->bounds();
            // SkImage always has a non-zero dimensions.
            SkASSERT(!fSubset.isEmpty());
        } else {
            fSubset = subset;
            if (!fSubset.intersect(fImage->bounds())) {
                fImage = nullptr;
                fSubset = {0, 0, 0, 0};
                fID = 0;
            }
        }
    }

    void setID(uint32_t id) { fID = id; }

    bool isValid() const { return fImage != nullptr; }

    SkIRect bounds() const { return SkIRect::MakeSize(this->dimensions()); }

    SkISize dimensions() const { return fSubset.size(); }

    sk_sp<SkImage> makeImage() const {
        return fSubset == fImage->bounds() ? fImage : fImage->makeSubset(fSubset);
    }

    SkBitmapKey getKey() const { return SkBitmapKey{fSubset, fID}; }

    void draw(SkCanvas* canvas, SkPaint* paint) const {
        SkASSERT(this->isValid());
        SkRect src = SkRect::Make(fSubset),
               dst = SkRect::Make(this->bounds());
        canvas->drawImageRect(fImage.get(), src, dst, paint);
    }

private:
    SkIRect fSubset;
    sk_sp<SkImage> fImage;
    uint32_t fID;
};

#endif  // SkBitmapKey_DEFINED
