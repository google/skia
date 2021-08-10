/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_GpuBase.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPromiseImageTexture.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrYUVABackendTextures.h"
#include "src/core/SkBitmapCache.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrYUVATextureProxies.h"
#include "src/gpu/SurfaceContext.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkReadPixelsRec.h"

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrImageContext> context, SkImageInfo info, uint32_t uniqueID)
        : INHERITED(std::move(info), uniqueID)
        , fContext(std::move(context)) {}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuBase::ValidateBackendTexture(const GrCaps* caps, const GrBackendTexture& tex,
                                             GrColorType grCT, SkColorType ct, SkAlphaType at,
                                             sk_sp<SkColorSpace> cs) {
    if (!tex.isValid()) {
        return false;
    }
    SkColorInfo info(ct, at, cs);
    if (!SkColorInfoIsValid(info)) {
        return false;
    }
    GrBackendFormat backendFormat = tex.getBackendFormat();
    if (!backendFormat.isValid()) {
        return false;
    }

    return caps->areColorTypeAndFormatCompatible(grCT, backendFormat);
}

bool SkImage_GpuBase::ValidateCompressedBackendTexture(const GrCaps* caps,
                                                       const GrBackendTexture& tex,
                                                       SkAlphaType at) {
    if (!tex.isValid() || tex.width() <= 0 || tex.height() <= 0) {
        return false;
    }

    if (tex.width() > caps->maxTextureSize() || tex.height() > caps->maxTextureSize()) {
        return false;
    }

    if (at == kUnknown_SkAlphaType) {
        return false;
    }

    GrBackendFormat backendFormat = tex.getBackendFormat();
    if (!backendFormat.isValid()) {
        return false;
    }

    if (!caps->isFormatCompressed(backendFormat)) {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuBase::getROPixels(GrDirectContext* dContext,
                                  SkBitmap* dst,
                                  CachingHint chint) const {
    if (!fContext->priv().matches(dContext)) {
        return false;
    }

    const auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, dst)) {
        SkASSERT(dst->isImmutable());
        SkASSERT(dst->getPixels());
        return true;
    }

    SkBitmapCache::RecPtr rec = nullptr;
    SkPixmap pmap;
    if (kAllow_CachingHint == chint) {
        rec = SkBitmapCache::Alloc(desc, this->imageInfo(), &pmap);
        if (!rec) {
            return false;
        }
    } else {
        if (!dst->tryAllocPixels(this->imageInfo()) || !dst->peekPixels(&pmap)) {
            return false;
        }
    }

    auto [view, ct] = this->asView(dContext, GrMipmapped::kNo);
    if (!view) {
        return false;
    }

    GrColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    auto sContext = dContext->priv().makeSC(std::move(view), std::move(colorInfo));
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(dContext, pmap, {0, 0})) {
        return false;
    }

    if (rec) {
        SkBitmapCache::Add(std::move(rec), dst);
        this->notifyAddedToRasterCache();
    }
    return true;
}

sk_sp<SkImage> SkImage_GpuBase::onMakeSubset(const SkIRect& subset,
                                             GrDirectContext* direct) const {
    if (!fContext->priv().matches(direct)) {
        return nullptr;
    }

    auto [view, ct] = this->asView(direct, GrMipmapped::kNo);
    SkASSERT(view);
    SkASSERT(ct == SkColorTypeToGrColorType(this->colorType()));

    SkBudgeted isBudgeted = view.proxy()->isBudgeted();
    auto copyView = GrSurfaceProxyView::Copy(direct,
                                             std::move(view),
                                             GrMipmapped::kNo,
                                             subset,
                                             SkBackingFit::kExact,
                                             isBudgeted);

    if (!copyView) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(direct),
                                   kNeedNewImageUniqueID,
                                   std::move(copyView),
                                   this->imageInfo().colorInfo());
}

bool SkImage_GpuBase::onReadPixels(GrDirectContext* dContext,
                                   const SkImageInfo& dstInfo,
                                   void* dstPixels,
                                   size_t dstRB,
                                   int srcX,
                                   int srcY,
                                   CachingHint) const {
    if (!fContext->priv().matches(dContext) ||
        !SkImageInfoValidConversion(dstInfo, this->imageInfo())) {
        return false;
    }

    auto [view, ct] = this->asView(dContext, GrMipmapped::kNo);
    SkASSERT(view);

    GrColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    auto sContext = dContext->priv().makeSC(std::move(view), colorInfo);
    if (!sContext) {
        return false;
    }

    return sContext->readPixels(dContext, {dstInfo, dstPixels, dstRB}, {srcX, srcY});
}

bool SkImage_GpuBase::onIsValid(GrRecordingContext* context) const {
    // The base class has already checked that 'context' isn't abandoned (if it's not nullptr)
    if (fContext->priv().abandoned()) {
        return false;
    }

    if (context && !fContext->priv().matches(context)) {
        return false;
    }

    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::MakePromiseImageLazyProxy(
        GrContextThreadSafeProxy* tsp,
        SkISize dimensions,
        GrBackendFormat backendFormat,
        GrMipmapped mipMapped,
        PromiseImageTextureFulfillProc fulfillProc,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(tsp);
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(releaseHelper);

    if (!fulfillProc) {
        return nullptr;
    }

    if (mipMapped == GrMipmapped::kYes &&
        GrTextureTypeHasRestrictedSampling(backendFormat.textureType())) {
        // It is invalid to have a GL_TEXTURE_EXTERNAL or GL_TEXTURE_RECTANGLE and have mips as
        // well.
        return nullptr;
    }

    /**
     * This class is the lazy instantiation callback for promise images. It manages calling the
     * client's Fulfill, Release, and Done procs. It attempts to reuse a GrTexture instance in
     * cases where the client provides the same SkPromiseImageTexture as Fulfill results for
     * multiple SkImages. The created GrTexture is given a key based on a unique ID associated with
     * the SkPromiseImageTexture.
     *
     * The GrTexutre idle proc mechanism is used to call the Release and Done procs. We use this
     * instead of the GrSurface release proc because the GrTexture is cached and therefore may
     * outlive the proxy into which this callback is installed.
     *
     * A key invalidation message is installed on the SkPromiseImageTexture so that the GrTexture
     * is deleted once it can no longer be used to instantiate a proxy.
     */
    class PromiseLazyInstantiateCallback {
    public:
        PromiseLazyInstantiateCallback(PromiseImageTextureFulfillProc fulfillProc,
                                       sk_sp<GrRefCntedCallback> releaseHelper)
                : fFulfillProc(fulfillProc), fReleaseHelper(std::move(releaseHelper)) {}
        PromiseLazyInstantiateCallback(PromiseLazyInstantiateCallback&&) = default;
        PromiseLazyInstantiateCallback(const PromiseLazyInstantiateCallback&) {
            // Because we get wrapped in std::function we must be copyable. But we should never
            // be copied.
            SkASSERT(false);
        }
        PromiseLazyInstantiateCallback& operator=(PromiseLazyInstantiateCallback&&) = default;
        PromiseLazyInstantiateCallback& operator=(const PromiseLazyInstantiateCallback&) {
            SkASSERT(false);
            return *this;
        }

        ~PromiseLazyInstantiateCallback() {
            // Our destructor can run on any thread. We trigger the unref of fTexture by message.
            // This unreffed texture pointer is a real problem! When the context has been
            // abandoned, the GrTexture pointed to by this pointer is deleted! Due to virtual
            // inheritance any manipulation of this pointer at that point will cause a crash.
            // For now we "work around" the problem by just passing it, untouched, into the
            // message bus but this very fragile.
            // In the future the GrSurface class hierarchy refactoring should eliminate this
            // difficulty by removing the virtual inheritance.
            if (fTexture) {
                GrTextureFreedMessage msg { fTexture, fTextureContextID };
                SkMessageBus<GrTextureFreedMessage, GrDirectContext::DirectContextID>::Post(msg);
            }
        }

        GrSurfaceProxy::LazyCallbackResult operator()(GrResourceProvider* resourceProvider,
                                                      const GrSurfaceProxy::LazySurfaceDesc&) {
            // We use the unique key in a way that is unrelated to the SkImage-based key that the
            // proxy may receive, hence kUnsynced.
            static constexpr auto kKeySyncMode =
                    GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced;

            // In order to make the SkImage "thread safe" we rely on holding an extra ref to the
            // texture in the callback and signalling the unref via a message to the resource cache.
            // We need to extend the callback's lifetime to that of the proxy.
            static constexpr auto kReleaseCallbackOnInstantiation = false;

            // Our proxy is getting instantiated for the second+ time. We are only allowed to call
            // Fulfill once. So return our cached result.
            if (fTexture) {
                return {sk_ref_sp(fTexture), kReleaseCallbackOnInstantiation, kKeySyncMode};
            } else if (fFulfillProcFailed) {
                // We've already called fulfill and it failed. Our contract says that we should only
                // call each callback once.
                return {};
            }

            PromiseImageTextureContext textureContext = fReleaseHelper->context();
            sk_sp<SkPromiseImageTexture> promiseTexture = fFulfillProc(textureContext);

            if (!promiseTexture) {
                fFulfillProcFailed = true;
                return {};
            }

            const GrBackendTexture& backendTexture = promiseTexture->backendTexture();
            if (!backendTexture.isValid()) {
                return {};
            }

            sk_sp<GrTexture> tex = resourceProvider->wrapBackendTexture(backendTexture,
                                                                        kBorrow_GrWrapOwnership,
                                                                        GrWrapCacheable::kNo,
                                                                        kRead_GrIOType);
            if (!tex) {
                return {};
            }
            tex->setRelease(fReleaseHelper);
            fTexture = tex.get();
            // We need to hold on to the GrTexture in case our proxy gets reinstantiated. However,
            // we can't unref in our destructor because we may be on another thread then. So we
            // let the cache know it is waiting on an unref message. We will send that message from
            // our destructor.
            auto dContext = fTexture->getContext();
            dContext->priv().getResourceCache()->insertDelayedTextureUnref(fTexture);
            fTextureContextID = dContext->directContextID();
            return {std::move(tex), kReleaseCallbackOnInstantiation, kKeySyncMode};
        }

    private:
        PromiseImageTextureFulfillProc fFulfillProc;
        sk_sp<GrRefCntedCallback> fReleaseHelper;
        GrTexture* fTexture = nullptr;
        GrDirectContext::DirectContextID fTextureContextID;
        bool fFulfillProcFailed = false;
    } callback(fulfillProc, std::move(releaseHelper));

    return GrProxyProvider::CreatePromiseProxy(tsp, std::move(callback), backendFormat, dimensions,
                                               mipMapped);
}
