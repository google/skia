/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMipmapAccessor_DEFINED
#define SkMipmapAccessor_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "src/core/SkMipmap.h"
#include <tuple>

class SkImage_Base;

class SkMipmapAccessor : ::SkNoncopyable {
public:
    // Returns null on failure
    static SkMipmapAccessor* Make(SkArenaAlloc*, const SkImage*, const SkMatrix& inv, SkMipmapMode);

    std::pair<SkPixmap, SkMatrix> level() const {
        SkASSERT(fUpper.addr() != nullptr);
        return std::make_pair(fUpper, fUpperInv);
    }

    std::pair<SkPixmap, SkMatrix> lowerLevel() const {
        SkASSERT(fLower.addr() != nullptr);
        return std::make_pair(fLower, fLowerInv);
    }

    // 0....1. Will be 0 if there is no lowerLevel
    float lowerWeight() const { return fLowerWeight; }

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

public:
    // Don't call publicly -- this is only public for SkArenaAlloc to access it inside Make()
    SkMipmapAccessor(const SkImage_Base*, const SkMatrix& inv, SkMipmapMode requestedMode);
};

#endif
