/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMipmapAccessor.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkMipmap.h"
#include "src/image/SkImage_Base.h"

class SkImage;

// Try to load from the base image, or from the cache
static sk_sp<const SkMipmap> try_load_mips(const SkImage_Base* image) {
    sk_sp<const SkMipmap> mips = image->refMips();
    if (!mips) {
        mips.reset(SkMipmapCache::FindAndRef(SkBitmapCacheDesc::Make(image)));
    }
    if (!mips) {
        mips.reset(SkMipmapCache::AddAndRef(image));
    }
    return mips;
}

SkMipmapAccessor::SkMipmapAccessor(const SkImage_Base* image, const SkMatrix& inv,
                                   SkMipmapMode requestedMode) {
    SkMipmapMode resolvedMode = requestedMode;
    fLowerWeight = 0;

    auto load_upper_from_base = [&]() {
        // only do this once
        if (fBaseStorage.getPixels() == nullptr) {
            auto dContext = as_IB(image)->directContext();
            (void)image->getROPixels(dContext, &fBaseStorage);
            fUpper.reset(fBaseStorage.info(), fBaseStorage.getPixels(), fBaseStorage.rowBytes());
        }
    };

    float level = 0;
    if (requestedMode != SkMipmapMode::kNone) {
        SkSize scale;
        if (!inv.decomposeScale(&scale, nullptr)) {
            resolvedMode = SkMipmapMode::kNone;
        } else {
            level = SkMipmap::ComputeLevel({1/scale.width(), 1/scale.height()});
            if (level <= 0) {
                resolvedMode = SkMipmapMode::kNone;
                level = 0;
            }
        }
    }

    auto scale = [image](const SkPixmap& pm) {
        return SkMatrix::Scale(SkIntToScalar(pm.width())  / image->width(),
                               SkIntToScalar(pm.height()) / image->height());
    };

    // Nearest mode uses this level, so we round to pick the nearest. In linear mode we use this
    // level as the lower of the two to interpolate between, so we take the floor.
    int levelNum = resolvedMode == SkMipmapMode::kNearest ? sk_float_round2int(level)
                                                          : sk_float_floor2int(level);
    float lowerWeight = level - levelNum;   // fract(level)
    SkASSERT(levelNum >= 0);

    if (levelNum == 0) {
        load_upper_from_base();
    }
    // load fCurrMip if needed
    if (levelNum > 0 || (resolvedMode == SkMipmapMode::kLinear && lowerWeight > 0)) {
        fCurrMip = try_load_mips(image);
        if (!fCurrMip) {
            load_upper_from_base();
            resolvedMode = SkMipmapMode::kNone;
        } else {
            SkMipmap::Level levelRec;

            SkASSERT(resolvedMode != SkMipmapMode::kNone);
            if (levelNum > 0) {
                if (fCurrMip->getLevel(levelNum - 1, &levelRec)) {
                    fUpper = levelRec.fPixmap;
                } else {
                    load_upper_from_base();
                    resolvedMode = SkMipmapMode::kNone;
                }
            }

            if (resolvedMode == SkMipmapMode::kLinear) {
                if (fCurrMip->getLevel(levelNum, &levelRec)) {
                    fLower = levelRec.fPixmap;
                    fLowerWeight = lowerWeight;
                    fLowerInv = scale(fLower);
                } else {
                    resolvedMode = SkMipmapMode::kNearest;
                }
            }
        }
    }
    fUpperInv = scale(fUpper);
}

SkMipmapAccessor* SkMipmapAccessor::Make(SkArenaAlloc* alloc, const SkImage* image,
                                         const SkMatrix& inv, SkMipmapMode mipmap) {
    auto* access = alloc->make<SkMipmapAccessor>(as_IB(image), inv, mipmap);
    // return null if we failed to get the level (so the caller won't try to use it)
    return access->fUpper.addr() ? access : nullptr;
}
