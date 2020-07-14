/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkBitmapController.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMipmap.h"
#include "src/image/SkImage_Base.h"

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

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBitmapController::State* SkBitmapController::RequestBitmap(const SkImage_Base* image,
                                                             const SkMatrix& inv,
                                                             SkFilterQuality quality,
                                                             SkArenaAlloc* alloc) {
    auto* state = alloc->make<SkBitmapController::State>(image, inv, quality);

    return state->pixmap().addr() ? state : nullptr;
}

bool SkBitmapController::State::processHighRequest(const SkImage_Base* image) {
    if (fQuality != kHigh_SkFilterQuality) {
        return false;
    }

    if (SkMatrixPriv::AdjustHighQualityFilterLevel(fInvMatrix, true) != kHigh_SkFilterQuality) {
        fQuality = kMedium_SkFilterQuality;
        return false;
    }

    (void)image->getROPixels(&fResultBitmap);
    return true;
}

/*
 *  Modulo internal errors, this should always succeed *if* the matrix is downscaling
 *  (in this case, we have the inverse, so it succeeds if fInvMatrix is upscaling)
 */
bool SkBitmapController::State::processMediumRequest(const SkImage_Base* image) {
    SkASSERT(fQuality <= kMedium_SkFilterQuality);
    if (fQuality != kMedium_SkFilterQuality) {
        return false;
    }

    // Our default return state is to downgrade the request to Low, w/ or w/o setting fBitmap
    // to a valid bitmap.
    fQuality = kLow_SkFilterQuality;

    SkSize invScaleSize;
    if (!fInvMatrix.decomposeScale(&invScaleSize, nullptr)) {
        return false;
    }

    if (invScaleSize.width() > SK_Scalar1 || invScaleSize.height() > SK_Scalar1) {
        fCurrMip = try_load_mips(image);
        if (!fCurrMip) {
            return false;
        }
        // diagnostic for a crasher...
        SkASSERT_RELEASE(fCurrMip->data());

        const SkSize scale = SkSize::Make(SkScalarInvert(invScaleSize.width()),
                                          SkScalarInvert(invScaleSize.height()));
        SkMipmap::Level level;
        if (fCurrMip->extractLevel(scale, &level)) {
            const SkSize& invScaleFixup = level.fScale;
            fInvMatrix.postScale(invScaleFixup.width(), invScaleFixup.height());

            // todo: if we could wrap the fCurrMip in a pixelref, then we could just install
            //       that here, and not need to explicitly track it ourselves.
            return fResultBitmap.installPixels(level.fPixmap);
        } else {
            // failed to extract, so release the mipmap
            fCurrMip.reset(nullptr);
        }
    }
    return false;
}

SkBitmapController::State::State(const SkImage_Base* image,
                                 const SkMatrix& inv,
                                 SkFilterQuality qual) {
    fInvMatrix = inv;
    fQuality = qual;

    if (this->processHighRequest(image) || this->processMediumRequest(image)) {
        SkASSERT(fResultBitmap.getPixels());
    } else {
        (void)image->getROPixels(&fResultBitmap);
    }

    // fResultBitmap.getPixels() may be null, but our caller knows to check fPixmap.addr()
    // and will destroy us if it is nullptr.
    fPixmap.reset(fResultBitmap.info(), fResultBitmap.getPixels(), fResultBitmap.rowBytes());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkMipmapAccessor::SkMipmapAccessor(const SkImage_Base* image, const SkMatrix& inv,
                                   SkMipmapMode requestedMode) {
    fResolvedMode = requestedMode;
    fLowerWeight = 0;

    auto load_upper_from_base = [&]() {
        // only do this once
        if (fBaseStorage.getPixels() == nullptr) {
            (void)image->getROPixels(&fBaseStorage);
            fUpper.reset(fBaseStorage.info(), fBaseStorage.getPixels(), fBaseStorage.rowBytes());
        }
    };

    float  level = 0;
    if (requestedMode != SkMipmapMode::kNone) {
        SkSize scale;
        if (!inv.decomposeScale(&scale, nullptr)) {
            fResolvedMode = SkMipmapMode::kNone;
        } else {
            level = SkMipmap::ComputeLevel({1/scale.width(), 1/scale.height()});
            if (level <= 0) {
                fResolvedMode = SkMipmapMode::kNone;
                level = 0;
            }
        }
    }

    int levelNum = sk_float_floor2int(level);
    float lowerWeight = level - levelNum;   // fract(level)
    SkASSERT(levelNum >= 0);

    if (levelNum == 0) {
        load_upper_from_base();
    }
    // load fCurrMip if needed
    if (levelNum > 0 || (fResolvedMode == SkMipmapMode::kLinear && lowerWeight > 0)) {
        fCurrMip = try_load_mips(image);
        if (!fCurrMip) {
            load_upper_from_base();
            fResolvedMode = SkMipmapMode::kNone;
        } else {
            SkMipmap::Level levelRec;

            SkASSERT(fResolvedMode != SkMipmapMode::kNone);
            if (levelNum > 0) {
                if (fCurrMip->getLevel(levelNum - 1, &levelRec)) {
                    fUpper = levelRec.fPixmap;
                } else {
                    load_upper_from_base();
                    fResolvedMode = SkMipmapMode::kNone;
                }
            }

            if (fResolvedMode == SkMipmapMode::kLinear) {
                if (fCurrMip->getLevel(levelNum, &levelRec)) {
                    fLower = levelRec.fPixmap;
                    fLowerWeight = lowerWeight;
                } else {
                    fResolvedMode = SkMipmapMode::kNearest;
                }
            }
        }
    }
}
