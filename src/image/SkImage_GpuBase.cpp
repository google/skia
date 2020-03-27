/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_GpuBase.h"

#include "include/core/SkPromiseImageTexture.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkTLList.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkReadPixelsRec.h"

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrContext> context, SkISize size, uint32_t uniqueID,
                                 SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs)
        : INHERITED(SkImageInfo::Make(size, ct, at, std::move(cs)), uniqueID)
        , fContext(std::move(context)) {}

//////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS
void SkImage_GpuBase::resetContext(sk_sp<GrContext> newContext) {
    SkASSERT(fContext->priv().matches(newContext.get()));
    fContext = newContext;
}
#endif

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

bool SkImage_GpuBase::getROPixels(SkBitmap* dst, CachingHint chint) const {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
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

    const GrSurfaceProxyView* view = this->view(direct);
    SkASSERT(view);
    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(
            fContext->priv().caps(), this->colorType(), view->proxy()->backendFormat());

    auto sContext = GrSurfaceContext::Make(direct, *view, grColorType, this->alphaType(),
                                           this->refColorSpace());
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), {0, 0})) {
        return false;
    }

    if (rec) {
        SkBitmapCache::Add(std::move(rec), dst);
        this->notifyAddedToRasterCache();
    }
    return true;
}

sk_sp<SkImage> SkImage_GpuBase::onMakeSubset(GrRecordingContext* context,
                                             const SkIRect& subset) const {
    if (!context || !fContext->priv().matches(context)) {
        return nullptr;
    }

    const GrSurfaceProxyView* view = this->view(context);
    SkASSERT(view && view->proxy());

    GrColorType grColorType = SkColorTypeToGrColorType(this->colorType());

    GrSurfaceProxyView copyView = GrSurfaceProxy::Copy(
            context, view->proxy(), view->origin(), grColorType, GrMipMapped::kNo, subset,
            SkBackingFit::kExact, view->proxy()->isBudgeted());

    if (!copyView.proxy()) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, std::move(copyView),
                                   this->colorType(), this->alphaType(), this->refColorSpace());
}

bool SkImage_GpuBase::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                   int srcX, int srcY, CachingHint) const {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        return false;
    }

    if (!SkImageInfoValidConversion(dstInfo, this->imageInfo())) {
        return false;
    }

    const GrSurfaceProxyView* view = this->view(direct);
    SkASSERT(view);
    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(
            fContext->priv().caps(), this->colorType(), view->proxy()->backendFormat());

    auto sContext = GrSurfaceContext::Make(direct, *view, grColorType, this->alphaType(),
                                           this->refColorSpace());
    if (!sContext) {
        return false;
    }

    return sContext->readPixels(dstInfo, dstPixels, dstRB, {srcX, srcY});
}

GrSurfaceProxyView SkImage_GpuBase::refView(GrRecordingContext* context,
                                            GrMipMapped mipMapped) const {
    if (!context || !fContext->priv().matches(context)) {
        SkASSERT(0);
        return {};
    }

    GrTextureAdjuster adjuster(fContext.get(), *this->view(context), this->imageInfo().colorInfo(),
                               this->uniqueID());
    return adjuster.view(mipMapped);
}

GrBackendTexture SkImage_GpuBase::onGetBackendTexture(bool flushPendingGrContextIO,
                                                      GrSurfaceOrigin* origin) const {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        // This image was created with a DDL context and cannot be instantiated.
        return GrBackendTexture();  // invalid
    }

    const GrSurfaceProxyView* view = this->view(direct);
    SkASSERT(view && *view);
    GrSurfaceProxy* proxy = view->proxy();

    if (!proxy->isInstantiated()) {
        auto resourceProvider = direct->priv().resourceProvider();

        if (!proxy->instantiate(resourceProvider)) {
            return GrBackendTexture();  // invalid
        }
    }

    GrTexture* texture = proxy->peekTexture();
    if (texture) {
        if (flushPendingGrContextIO) {
            direct->priv().flushSurface(proxy);
        }
        if (origin) {
            *origin = view->origin();
        }
        return texture->getBackendTexture();
    }
    return GrBackendTexture();  // invalid
}

GrTexture* SkImage_GpuBase::getTexture() const {
    GrTextureProxy* proxy = this->peekProxy();
    if (proxy && proxy->isInstantiated()) {
        return proxy->peekTexture();
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        // This image was created with a DDL context and cannot be instantiated.
        return nullptr;
    }

    const GrSurfaceProxyView* view = this->view(direct);
    SkASSERT(view && *view && !view->proxy()->isInstantiated());

    if (!view->proxy()->instantiate(direct->priv().resourceProvider())) {
        return nullptr;
    }

    return view->proxy()->peekTexture();
}

bool SkImage_GpuBase::onIsValid(GrContext* context) const {
    // The base class has already checked that context isn't abandoned (if it's not nullptr)
    if (fContext->priv().abandoned()) {
        return false;
    }

    if (context && !fContext->priv().matches(context)) {
        return false;
    }

    return true;
}

bool SkImage_GpuBase::MakeTempTextureProxies(GrContext* ctx, const GrBackendTexture yuvaTextures[],
                                             int numTextures, const SkYUVAIndex yuvaIndices[4],
                                             GrSurfaceOrigin imageOrigin,
                                             GrSurfaceProxyView tempViews[4]) {
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    const GrCaps* caps = ctx->priv().caps();

    for (int textureIndex = 0; textureIndex < numTextures; ++textureIndex) {
        GrBackendFormat backendFormat = yuvaTextures[textureIndex].getBackendFormat();
        if (!backendFormat.isValid()) {
            return false;
        }

        GrColorType grColorType = caps->getYUVAColorTypeFromBackendFormat(
                backendFormat, yuvaIndices[3].fIndex == textureIndex);
        if (GrColorType::kUnknown == grColorType) {
            return false;
        }

        SkASSERT(yuvaTextures[textureIndex].isValid());

        auto proxy = proxyProvider->wrapBackendTexture(yuvaTextures[textureIndex],
                                                       kBorrow_GrWrapOwnership,
                                                       GrWrapCacheable::kNo, kRead_GrIOType);
        if (!proxy) {
            return false;
        }
        GrSwizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), grColorType);
        tempViews[textureIndex] = GrSurfaceProxyView(std::move(proxy), imageOrigin, swizzle);

        // Check that each texture contains the channel data for the corresponding YUVA index
        auto channelFlags = GrColorTypeChannelFlags(grColorType);
        for (int yuvaIndex = 0; yuvaIndex < SkYUVAIndex::kIndexCount; ++yuvaIndex) {
            if (yuvaIndices[yuvaIndex].fIndex == textureIndex) {
                switch (yuvaIndices[yuvaIndex].fChannel) {
                    case SkColorChannel::kR:
                        // TODO: Chrome needs to be patched before this can be
                        // enforced.
                        // if (!(kRed_SkColorChannelFlag & channelFlags)) {
                        //     return false;
                        // }
                        break;
                    case SkColorChannel::kG:
                        if (!(kGreen_SkColorChannelFlag & channelFlags)) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kB:
                        if (!(kBlue_SkColorChannelFlag & channelFlags)) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kA:
                        if (!(kAlpha_SkColorChannelFlag & channelFlags)) {
                            return false;
                        }
                        break;
                }
            }
        }
    }

    return true;
}

bool SkImage_GpuBase::RenderYUVAToRGBA(GrContext* ctx, GrRenderTargetContext* renderTargetContext,
                                       const SkRect& rect, SkYUVColorSpace yuvColorSpace,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       GrSurfaceProxyView views[4],
                                       const SkYUVAIndex yuvaIndices[4]) {
    SkASSERT(renderTargetContext);
    if (!renderTargetContext->asSurfaceProxy()) {
        return false;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    const auto& caps = *ctx->priv().caps();
    auto fp = GrYUVtoRGBEffect::Make(views, yuvaIndices, yuvColorSpace,
                                     GrSamplerState::Filter::kNearest, caps);
    if (colorSpaceXform) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(colorSpaceXform));
    }
    paint.addColorFragmentProcessor(std::move(fp));

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);
    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::MakePromiseImageLazyProxy(
        GrContext* context, int width, int height, GrColorType colorType,
        GrBackendFormat backendFormat, GrMipMapped mipMapped,
        PromiseImageTextureFulfillProc fulfillProc, PromiseImageTextureReleaseProc releaseProc,
        PromiseImageTextureDoneProc doneProc, PromiseImageTextureContext textureContext,
        PromiseImageApiVersion version) {
    SkASSERT(context);
    SkASSERT(width > 0 && height > 0);
    SkASSERT(doneProc);
    SkASSERT(colorType != GrColorType::kUnknown);

    if (!fulfillProc || !releaseProc) {
        doneProc(textureContext);
        return nullptr;
    }

    if (mipMapped == GrMipMapped::kYes &&
        GrTextureTypeHasRestrictedSampling(backendFormat.textureType())) {
        // It is invalid to have a GL_TEXTURE_EXTERNAL or GL_TEXTURE_RECTANGLE and have mips as
        // well.
        doneProc(textureContext);
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
                                       PromiseImageTextureReleaseProc releaseProc,
                                       PromiseImageTextureDoneProc doneProc,
                                       PromiseImageTextureContext context,
                                       GrColorType colorType,
                                       PromiseImageApiVersion version)
                : fFulfillProc(fulfillProc)
                , fReleaseProc(releaseProc)
                , fColorType(colorType)
                , fVersion(version) {
            fDoneCallback = sk_make_sp<GrRefCntedCallback>(doneProc, context);
        }
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
                SkMessageBus<GrTextureFreedMessage>::Post({fTexture, fTextureContextID});
            }
        }

        GrSurfaceProxy::LazyCallbackResult operator()(GrResourceProvider* resourceProvider) {
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
            } else if (fColorType == GrColorType::kUnknown) {
                // We've already called fulfill and it failed. Our contract says that we should only
                // call each callback once.
                return {};
            }
            SkASSERT(fDoneCallback);
            PromiseImageTextureContext textureContext = fDoneCallback->context();
            sk_sp<SkPromiseImageTexture> promiseTexture = fFulfillProc(textureContext);
            // From here on out our contract is that the release proc must be called, even if
            // the return from fulfill was invalid or we fail for some other reason.
            auto releaseCallback = sk_make_sp<GrRefCntedCallback>(fReleaseProc, textureContext);
            if (!promiseTexture) {
                // This records that we have failed.
                fColorType = GrColorType::kUnknown;
                return {};
            }

            const GrBackendTexture& backendTexture = promiseTexture->backendTexture();
            if (!backendTexture.isValid()) {
                return {};
            }

            sk_sp<GrTexture> tex;
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey key;
            GrUniqueKey::Builder builder(&key, kDomain, 2, "promise");
            builder[0] = promiseTexture->uniqueID();
            builder[1] = (uint32_t)fColorType;
            builder.finish();
            // A texture with this key may already exist from a different instance of this lazy
            // callback. This could happen if the client fulfills a promise image with a texture
            // that was previously used to fulfill a different promise image.
            if (auto surf = resourceProvider->findByUniqueKey<GrSurface>(key)) {
                tex = sk_ref_sp(surf->asTexture());
                SkASSERT(tex);
            } else {
                if ((tex = resourceProvider->wrapBackendTexture(
                             backendTexture, kBorrow_GrWrapOwnership, GrWrapCacheable::kYes,
                             kRead_GrIOType))) {
                    tex->resourcePriv().setUniqueKey(key);
                } else {
                    return {};
                }
            }
            auto releaseIdleState = fVersion == PromiseImageApiVersion::kLegacy
                                            ? GrTexture::IdleState::kFinished
                                            : GrTexture::IdleState::kFlushed;
            tex->addIdleProc(std::move(releaseCallback), releaseIdleState);
            tex->addIdleProc(std::move(fDoneCallback), GrTexture::IdleState::kFinished);
            promiseTexture->addKeyToInvalidate(tex->getContext()->priv().contextID(), key);
            fTexture = tex.get();
            // We need to hold on to the GrTexture in case our proxy gets reinstantiated. However,
            // we can't unref in our destructor because we may be on another thread then. So we
            // let the cache know it is waiting on an unref message. We will send that message from
            // our destructor.
            GrContext* context = fTexture->getContext();
            context->priv().getResourceCache()->insertDelayedTextureUnref(fTexture);
            fTextureContextID = context->priv().contextID();
            return {std::move(tex), kReleaseCallbackOnInstantiation, kKeySyncMode};
        }

    private:
        PromiseImageTextureFulfillProc fFulfillProc;
        PromiseImageTextureReleaseProc fReleaseProc;
        sk_sp<GrRefCntedCallback> fDoneCallback;
        GrTexture* fTexture = nullptr;
        uint32_t fTextureContextID = SK_InvalidUniqueID;
        GrColorType fColorType;
        PromiseImageApiVersion fVersion;
    } callback(fulfillProc, releaseProc, doneProc, textureContext, colorType, version);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    // Ganesh assumes that, when wrapping a mipmapped backend texture from a client, that its
    // mipmaps are fully fleshed out.
    GrMipMapsStatus mipMapsStatus = (GrMipMapped::kYes == mipMapped)
            ? GrMipMapsStatus::kValid : GrMipMapsStatus::kNotAllocated;

    // We pass kReadOnly here since we should treat content of the client's texture as immutable.
    // The promise API provides no way for the client to indicated that the texture is protected.
    return proxyProvider->createLazyProxy(
            std::move(callback), backendFormat, {width, height}, GrRenderable::kNo, 1, mipMapped,
            mipMapsStatus, GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact, SkBudgeted::kNo,
            GrProtected::kNo, GrSurfaceProxy::UseAllocator::kYes);
}
