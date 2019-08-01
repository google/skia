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
#include "src/core/SkMipMap.h"
#include "src/image/SkImage_Base.h"

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

    fQuality = kMedium_SkFilterQuality;

    SkScalar invScaleX = fInvMatrix.getScaleX();
    SkScalar invScaleY = fInvMatrix.getScaleY();
    if (fInvMatrix.getType() & SkMatrix::kAffine_Mask) {
        SkSize scale;
        if (!fInvMatrix.decomposeScale(&scale)) {
            return false;
        }
        invScaleX = scale.width();
        invScaleY = scale.height();
    }
    invScaleX = SkScalarAbs(invScaleX);
    invScaleY = SkScalarAbs(invScaleY);

    if (invScaleX >= 1 - SK_ScalarNearlyZero || invScaleY >= 1 - SK_ScalarNearlyZero) {
        // we're down-scaling so abort HQ
        return false;
    }

    // Confirmed that we can use HQ (w/ rasterpipeline)
    fQuality = kHigh_SkFilterQuality;
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
        fCurrMip.reset(SkMipMapCache::FindAndRef(SkBitmapCacheDesc::Make(image)));
        if (nullptr == fCurrMip.get()) {
            fCurrMip.reset(SkMipMapCache::AddAndRef(image));
            if (nullptr == fCurrMip.get()) {
                return false;
            }
        }
        // diagnostic for a crasher...
        SkASSERT_RELEASE(fCurrMip->data());

        const SkSize scale = SkSize::Make(SkScalarInvert(invScaleSize.width()),
                                          SkScalarInvert(invScaleSize.height()));
        SkMipMap::Level level;
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
