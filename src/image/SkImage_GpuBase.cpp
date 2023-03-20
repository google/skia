/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_GpuBase.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPromiseImageTexture.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/image/SkImage_Gpu.h"

#include <functional>
#include <memory>
#include <utility>

class GrContextThreadSafeProxy;
class SkImage;
enum SkColorType : int;
struct SkIRect;

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Log.h"
#endif

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrImageContext> context, SkImageInfo info, uint32_t uniqueID)
        : SkImage_Base(std::move(info), uniqueID)
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

    auto [view, ct] = this->asView(dContext, skgpu::Mipmapped::kNo);
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

    auto [view, ct] = this->asView(direct, skgpu::Mipmapped::kNo);
    SkASSERT(view);
    SkASSERT(ct == SkColorTypeToGrColorType(this->colorType()));

    skgpu::Budgeted isBudgeted = view.proxy()->isBudgeted();
    auto copyView = GrSurfaceProxyView::Copy(direct,
                                             std::move(view),
                                             skgpu::Mipmapped::kNo,
                                             subset,
                                             SkBackingFit::kExact,
                                             isBudgeted,
                                             /*label=*/"ImageGpuBase_MakeSubset");

    if (!copyView) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(direct),
                                   kNeedNewImageUniqueID,
                                   std::move(copyView),
                                   this->imageInfo().colorInfo());
}

#if defined(SK_GRAPHITE)
sk_sp<SkImage> SkImage_GpuBase::onMakeTextureImage(skgpu::graphite::Recorder*,
                                                   SkImage::RequiredImageProperties) const {
    SKGPU_LOG_W("Cannot convert Ganesh-backed image to Graphite");
    return nullptr;
}

sk_sp<SkImage> SkImage_GpuBase::onMakeSubset(const SkIRect&,
                                             skgpu::graphite::Recorder*,
                                             RequiredImageProperties) const {
    SKGPU_LOG_W("Cannot convert Ganesh-backed image to Graphite");
    return nullptr;
}

sk_sp<SkImage> SkImage_GpuBase::onMakeColorTypeAndColorSpace(SkColorType,
                                                             sk_sp<SkColorSpace>,
                                                             skgpu::graphite::Recorder*,
                                                             RequiredImageProperties) const {
    SKGPU_LOG_W("Cannot convert Ganesh-backed image to Graphite");
    return nullptr;
}
#endif

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

    auto [view, ct] = this->asView(dContext, skgpu::Mipmapped::kNo);
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
        skgpu::Mipmapped mipmapped,
        PromiseImageTextureFulfillProc fulfillProc,
        sk_sp<skgpu::RefCntedCallback> releaseHelper) {
    SkASSERT(tsp);
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(releaseHelper);

    if (!fulfillProc) {
        return nullptr;
    }

    if (mipmapped == skgpu::Mipmapped::kYes &&
        GrTextureTypeHasRestrictedSampling(backendFormat.textureType())) {
        // It is invalid to have a GL_TEXTURE_EXTERNAL or GL_TEXTURE_RECTANGLE and have mips as
        // well.
        return nullptr;
    }

    /**
     * This class is the lazy instantiation callback for promise images. It manages calling the
     * client's Fulfill and Release procs. It attempts to reuse a GrTexture instance in
     * cases where the client provides the same SkPromiseImageTexture as Fulfill results for
     * multiple SkImages. The created GrTexture is given a key based on a unique ID associated with
     * the SkPromiseImageTexture.
     *
     * A key invalidation message is installed on the SkPromiseImageTexture so that the GrTexture
     * is deleted once it can no longer be used to instantiate a proxy.
     */
    class PromiseLazyInstantiateCallback {
    public:
        PromiseLazyInstantiateCallback(PromiseImageTextureFulfillProc fulfillProc,
                                       sk_sp<skgpu::RefCntedCallback> releaseHelper)
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
            if (fTexture) {
                GrResourceCache::ReturnResourceFromThread(std::move(fTexture), fTextureContextID);
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
                return {fTexture, kReleaseCallbackOnInstantiation, kKeySyncMode};
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

            fTexture = resourceProvider->wrapBackendTexture(backendTexture,
                                                            kBorrow_GrWrapOwnership,
                                                            GrWrapCacheable::kNo,
                                                            kRead_GrIOType);
            if (!fTexture) {
                return {};
            }
            fTexture->setRelease(fReleaseHelper);
            auto dContext = fTexture->getContext();
            fTextureContextID = dContext->directContextID();
            return {fTexture, kReleaseCallbackOnInstantiation, kKeySyncMode};
        }

    private:
        PromiseImageTextureFulfillProc fFulfillProc;
        sk_sp<skgpu::RefCntedCallback> fReleaseHelper;
        sk_sp<GrTexture> fTexture;
        GrDirectContext::DirectContextID fTextureContextID;
        bool fFulfillProcFailed = false;
    } callback(fulfillProc, std::move(releaseHelper));

    return GrProxyProvider::CreatePromiseProxy(tsp, std::move(callback), backendFormat, dimensions,
                                               mipmapped);
}
