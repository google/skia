/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapCache.h"
#include "SkBitmapController.h"
#include "SkBitmapProvider.h"
#include "SkMatrix.h"
#include "SkMipMap.h"
#include "SkTemplates.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBitmapController::State* SkBitmapController::requestBitmap(const SkBitmapProvider& provider,
                                                             const SkMatrix& inv,
                                                             SkFilterQuality quality,
                                                             void* storage, size_t storageSize) {
    State* state = this->onRequestBitmap(provider, inv, quality, storage, storageSize);
    if (state) {
        if (nullptr == state->fPixmap.addr()) {
            SkInPlaceDeleteCheck(state, storage);
            state = nullptr;
        }
    }
    return state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkDefaultBitmapControllerState : public SkBitmapController::State {
public:
    SkDefaultBitmapControllerState(const SkBitmapProvider&, const SkMatrix& inv, SkFilterQuality);

private:
    SkBitmap                fResultBitmap;
    sk_sp<const SkMipMap>   fCurrMip;

    bool processHighRequest(const SkBitmapProvider&);
    bool processMediumRequest(const SkBitmapProvider&);
};

bool SkDefaultBitmapControllerState::processHighRequest(const SkBitmapProvider& provider) {
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
    (void)provider.asBitmap(&fResultBitmap);
    return true;
}

/*
 *  Modulo internal errors, this should always succeed *if* the matrix is downscaling
 *  (in this case, we have the inverse, so it succeeds if fInvMatrix is upscaling)
 */
bool SkDefaultBitmapControllerState::processMediumRequest(const SkBitmapProvider& provider) {
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

    SkDestinationSurfaceColorMode colorMode = provider.dstColorSpace()
        ? SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware
        : SkDestinationSurfaceColorMode::kLegacy;
    if (invScaleSize.width() > SK_Scalar1 || invScaleSize.height() > SK_Scalar1) {
        fCurrMip.reset(SkMipMapCache::FindAndRef(provider.makeCacheDesc(), colorMode));
        if (nullptr == fCurrMip.get()) {
            SkBitmap orig;
            if (!provider.asBitmap(&orig)) {
                return false;
            }
            fCurrMip.reset(SkMipMapCache::AddAndRef(orig, colorMode));
            if (nullptr == fCurrMip.get()) {
                return false;
            }
        }
        // diagnostic for a crasher...
        if (nullptr == fCurrMip->data()) {
            sk_throw();
        }

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

SkDefaultBitmapControllerState::SkDefaultBitmapControllerState(const SkBitmapProvider& provider,
                                                               const SkMatrix& inv,
                                                               SkFilterQuality qual) {
    fInvMatrix = inv;
    fQuality = qual;

    if (this->processHighRequest(provider) || this->processMediumRequest(provider)) {
        SkASSERT(fResultBitmap.getPixels());
    } else {
        (void)provider.asBitmap(&fResultBitmap);
    }

    // fResultBitmap.getPixels() may be null, but our caller knows to check fPixmap.addr()
    // and will destroy us if it is nullptr.
    fPixmap.reset(fResultBitmap.info(), fResultBitmap.getPixels(), fResultBitmap.rowBytes());
}

SkBitmapController::State* SkDefaultBitmapController::onRequestBitmap(const SkBitmapProvider& bm,
                                                                      const SkMatrix& inverse,
                                                                      SkFilterQuality quality,
                                                                      void* storage, size_t size) {
    return SkInPlaceNewCheck<SkDefaultBitmapControllerState>(storage, size, bm, inverse, quality);
}
