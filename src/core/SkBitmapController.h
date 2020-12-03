/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapController_DEFINED
#define SkBitmapController_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "src/core/SkMipmap.h"
#include <tuple>

class SkImage_Base;

class SkMipmapAccessor : ::SkNoncopyable {
public:
    SkMipmapAccessor(const SkImage_Base*, const SkMatrix& inv, SkMipmapMode requestedMode);

    std::pair<SkPixmap, SkMatrix> level() const { return std::make_pair(fUpper, fUpperInv); }

    std::pair<SkPixmap, SkMatrix> lowerLevel() const {
        SkASSERT(this->mode() == SkMipmapMode::kLinear);
        return std::make_pair(fLower, fLowerInv);
    }

    // 0....1. Will be 0 if there is no lowerLevel
    float lowerWeight() const { return fLowerWeight; }

    SkMipmapMode mode() const { return fResolvedMode; }

private:
    SkPixmap     fUpper,
                 fLower; // only valid for mip_linear
    float        fLowerWeight;   // lower * weight + upper * (1 - weight)
    SkMatrix     fUpperInv,
                 fLowerInv;
    SkMipmapMode fResolvedMode;

    // these manage lifetime for the buffers
    SkBitmap              fBaseStorage;
    sk_sp<const SkMipmap> fCurrMip;
};

#endif
