/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapController.h"
#include "SkBitmapProvider.h"
#include "SkMatrix.h"
#include "SkPixelRef.h"
#include "SkTemplates.h"

// RESIZE_LANCZOS3 is another good option, but chrome prefers mitchell at the moment
#define kHQ_RESIZE_METHOD   SkBitmapScaler::RESIZE_MITCHELL

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

#include "SkBitmapCache.h"
#include "SkBitmapScaler.h"
#include "SkMipMap.h"
#include "SkResourceCache.h"

class SkDefaultBitmapControllerState : public SkBitmapController::State {
public:
    SkDefaultBitmapControllerState(const SkBitmapProvider&,
                                   const SkMatrix& inv,
                                   SkFilterQuality,
                                   bool canShadeHQ);

private:
    SkBitmap                      fResultBitmap;
    sk_sp<const SkMipMap>         fCurrMip;
    bool                          fCanShadeHQ;

    bool processHQRequest(const SkBitmapProvider&);
    bool processMediumRequest(const SkBitmapProvider&);
};

// Check to see that the size of the bitmap that would be produced by
// scaling by the given inverted matrix is less than the maximum allowed.
static inline bool cache_size_okay(const SkBitmapProvider& provider, const SkMatrix& invMat) {
    size_t maximumAllocation = SkResourceCache::GetEffectiveSingleAllocationByteLimit();
    if (0 == maximumAllocation) {
        return true;
    }
    // float matrixScaleFactor = 1.0 / (invMat.scaleX * invMat.scaleY);
    // return ((origBitmapSize * matrixScaleFactor) < maximumAllocationSize);
    // Skip the division step:
    const size_t size = provider.info().getSafeSize(provider.info().minRowBytes());
    SkScalar invScaleSqr = invMat.getScaleX() * invMat.getScaleY();
    return size < (maximumAllocation * SkScalarAbs(invScaleSqr));
}

/*
 *  High quality is implemented by performing up-right scale-only filtering and then
 *  using bilerp for any remaining transformations.
 */
bool SkDefaultBitmapControllerState::processHQRequest(const SkBitmapProvider& provider) {
    if (fQuality != kHigh_SkFilterQuality) {
        return false;
    }

    // Our default return state is to downgrade the request to Medium, w/ or w/o setting fBitmap
    // to a valid bitmap. If we succeed, we will set this to Low instead.
    fQuality = kMedium_SkFilterQuality;
#ifdef SK_USE_MIP_FOR_DOWNSCALE_HQ
    return false;
#endif

    if (kN32_SkColorType != provider.info().colorType() || !cache_size_okay(provider, fInvMatrix) ||
        fInvMatrix.hasPerspective())
    {
        return false; // can't handle the reqeust
    }

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

    if (SkScalarNearlyEqual(invScaleX, 1) && SkScalarNearlyEqual(invScaleY, 1)) {
        return false; // no need for HQ
    }

    if (invScaleX > 1 || invScaleY > 1) {
        return false; // only use HQ when upsampling
    }

    // If the shader can natively handle HQ filtering, let it do it.
    if (fCanShadeHQ) {
        fQuality = kHigh_SkFilterQuality;
        SkAssertResult(provider.asBitmap(&fResultBitmap));
        fResultBitmap.lockPixels();
        return true;
    }

    const int dstW = SkScalarRoundToScalar(provider.width() / invScaleX);
    const int dstH = SkScalarRoundToScalar(provider.height() / invScaleY);
    const SkBitmapCacheDesc desc = provider.makeCacheDesc(dstW, dstH);

    if (!SkBitmapCache::Find(desc, &fResultBitmap)) {
        SkBitmap orig;
        if (!provider.asBitmap(&orig)) {
            return false;
        }
        SkAutoPixmapUnlock src;
        if (!orig.requestLock(&src)) {
            return false;
        }

        SkPixmap dst;
        SkBitmapCache::RecPtr rec;
        const SkImageInfo info = SkImageInfo::MakeN32(desc.fScaledWidth, desc.fScaledHeight,
                                                      src.pixmap().alphaType());
        if (provider.isVolatile()) {
            if (!fResultBitmap.tryAllocPixels(info)) {
                return false;
            }
            SkASSERT(fResultBitmap.getPixels());
            fResultBitmap.peekPixels(&dst);
            fResultBitmap.setImmutable();   // a little cheat, as we haven't resized yet, but ok
        } else {
            rec = SkBitmapCache::Alloc(desc, info, &dst);
            if (!rec) {
                return false;
            }
        }
        if (!SkBitmapScaler::Resize(dst, src.pixmap(), kHQ_RESIZE_METHOD)) {
            return false; // we failed to create fScaledBitmap
        }
        if (rec) {
            SkBitmapCache::Add(std::move(rec), &fResultBitmap);
            SkASSERT(fResultBitmap.getPixels());
            provider.notifyAddedToCache();
        }
    }

    SkASSERT(fResultBitmap.getPixels());
    SkASSERT(fResultBitmap.isImmutable());

    fInvMatrix.postScale(SkIntToScalar(dstW) / provider.width(),
                         SkIntToScalar(dstH) / provider.height());
    fQuality = kLow_SkFilterQuality;
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
                                                               SkFilterQuality qual,
                                                               bool canShadeHQ) {
    fInvMatrix = inv;
    fQuality = qual;
    fCanShadeHQ = canShadeHQ;

    bool processed = this->processHQRequest(provider) || this->processMediumRequest(provider);

    if (processed) {
        SkASSERT(fResultBitmap.getPixels());
    } else {
        (void)provider.asBitmap(&fResultBitmap);
        fResultBitmap.lockPixels();
        // lock may fail to give us pixels
    }
    SkASSERT(fCanShadeHQ || fQuality <= kLow_SkFilterQuality);

    // fResultBitmap.getPixels() may be null, but our caller knows to check fPixmap.addr()
    // and will destroy us if it is nullptr.
    fPixmap.reset(fResultBitmap.info(), fResultBitmap.getPixels(), fResultBitmap.rowBytes(),
                  fResultBitmap.getColorTable());
}

SkBitmapController::State* SkDefaultBitmapController::onRequestBitmap(const SkBitmapProvider& bm,
                                                                      const SkMatrix& inverse,
                                                                      SkFilterQuality quality,
                                                                      void* storage, size_t size) {
    return SkInPlaceNewCheck<SkDefaultBitmapControllerState>(storage, size,
                                                             bm, inverse, quality, fCanShadeHQ);
}
