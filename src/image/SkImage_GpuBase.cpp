/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPromiseImageTexture.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkTLList.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkImage_GpuBase.h"
#include "src/image/SkReadPixelsRec.h"

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs)
        : INHERITED(SkImageInfo::Make(width, height, ct, at, std::move(cs)), uniqueID)
        , fContext(std::move(context)) {}

SkImage_GpuBase::~SkImage_GpuBase() {}

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
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, at, cs);
    if (!SkImageInfoIsValid(info)) {
        return false;
    }
    GrBackendFormat backendFormat = tex.getBackendFormat();
    if (!backendFormat.isValid()) {
        return false;
    }

    return caps->areColorTypeAndFormatCompatible(grCT, backendFormat);
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

    sk_sp<GrTextureProxy> texProxy = this->asTextureProxyRef(direct);
    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(fContext->priv().caps(),
                                                                this->colorType(),
                                                                texProxy->backendFormat());

    auto sContext = direct->priv().makeWrappedSurfaceContext(
            std::move(texProxy), grColorType, this->alphaType(), this->refColorSpace());
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

    sk_sp<GrSurfaceProxy> proxy = this->asTextureProxyRef(context);

    sk_sp<GrTextureProxy> copyProxy = GrSurfaceProxy::Copy(
            context, proxy.get(), GrMipMapped::kNo, subset, SkBackingFit::kExact,
            proxy->isBudgeted());

    if (!copyProxy) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, this->alphaType(),
                                   std::move(copyProxy), this->refColorSpace());
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

    sk_sp<GrTextureProxy> texProxy = this->asTextureProxyRef(direct);
    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(fContext->priv().caps(),
                                                                this->colorType(),
                                                                texProxy->backendFormat());

    auto sContext = direct->priv().makeWrappedSurfaceContext(
            std::move(texProxy), grColorType, this->alphaType(), this->refColorSpace());
    if (!sContext) {
        return false;
    }

    return sContext->readPixels(dstInfo, dstPixels, dstRB, {srcX, srcY});
}

sk_sp<GrTextureProxy> SkImage_GpuBase::asTextureProxyRef(GrRecordingContext* context,
                                                         const GrSamplerState& params,
                                                         SkScalar scaleAdjust[2]) const {
    if (!context || !fContext->priv().matches(context)) {
        SkASSERT(0);
        return nullptr;
    }

    GrTextureAdjuster adjuster(fContext.get(), this->asTextureProxyRef(context),
                               SkColorTypeToGrColorType(this->colorType()), this->alphaType(),
                               this->uniqueID(), this->colorSpace());
    return adjuster.refTextureProxyForParams(params, scaleAdjust);
}

GrBackendTexture SkImage_GpuBase::onGetBackendTexture(bool flushPendingGrContextIO,
                                                      GrSurfaceOrigin* origin) const {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        // This image was created with a DDL context and cannot be instantiated.
        return GrBackendTexture(); // invalid
    }

    sk_sp<GrTextureProxy> proxy = this->asTextureProxyRef(direct);
    SkASSERT(proxy);

    if (!proxy->isInstantiated()) {
        auto resourceProvider = direct->priv().resourceProvider();

        if (!proxy->instantiate(resourceProvider)) {
            return GrBackendTexture(); // invalid
        }
    }

    GrTexture* texture = proxy->peekTexture();
    if (texture) {
        if (flushPendingGrContextIO) {
            direct->priv().flushSurface(proxy.get());
        }
        if (origin) {
            *origin = proxy->origin();
        }
        return texture->getBackendTexture();
    }
    return GrBackendTexture(); // invalid
}

GrTexture* SkImage_GpuBase::onGetTexture() const {
    GrTextureProxy* proxy = this->peekProxy();
    if (proxy && proxy->isInstantiated()) {
        return proxy->peekTexture();
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        // This image was created with a DDL context and cannot be instantiated.
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxyRef = this->asTextureProxyRef(direct);
    SkASSERT(proxyRef && !proxyRef->isInstantiated());

    if (!proxyRef->instantiate(direct->priv().resourceProvider())) {
        return nullptr;
    }

    return proxyRef->peekTexture();
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
                                             sk_sp<GrTextureProxy> tempTextureProxies[4]) {
    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    const GrCaps* caps = ctx->priv().caps();

    for (int textureIndex = 0; textureIndex < numTextures; ++textureIndex) {
        GrBackendFormat backendFormat = yuvaTextures[textureIndex].getBackendFormat();
        if (!backendFormat.isValid()) {
            return false;
        }

        GrColorType grColorType = caps->getYUVAColorTypeFromBackendFormat(
                                                        backendFormat,
                                                        yuvaIndices[3].fIndex == textureIndex);
        if (GrColorType::kUnknown == grColorType) {
            return false;
        }

        SkASSERT(yuvaTextures[textureIndex].isValid());

        tempTextureProxies[textureIndex] = proxyProvider->wrapBackendTexture(
                yuvaTextures[textureIndex], grColorType, imageOrigin, kBorrow_GrWrapOwnership,
                GrWrapCacheable::kNo, kRead_GrIOType);
        if (!tempTextureProxies[textureIndex]) {
            return false;
        }

        // Check that each texture contains the channel data for the corresponding YUVA index
        auto componentFlags = GrColorTypeComponentFlags(grColorType);
        for (int yuvaIndex = 0; yuvaIndex < SkYUVAIndex::kIndexCount; ++yuvaIndex) {
            if (yuvaIndices[yuvaIndex].fIndex == textureIndex) {
                switch (yuvaIndices[yuvaIndex].fChannel) {
                    case SkColorChannel::kR:
                          // TODO: Chrome needs to be patched before this can be
                          // enforced.
//                        if (!(kRed_SkColorTypeComponentFlag & componentFlags)) {
//                            return false;
//                        }
                        break;
                    case SkColorChannel::kG:
                        if (!(kGreen_SkColorTypeComponentFlag & componentFlags)) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kB:
                        if (!(kBlue_SkColorTypeComponentFlag & componentFlags)) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kA:
                        if (!(kAlpha_SkColorTypeComponentFlag & componentFlags)) {
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
                                       const sk_sp<GrTextureProxy> proxies[4],
                                       const SkYUVAIndex yuvaIndices[4]) {
    SkASSERT(renderTargetContext);
    if (!renderTargetContext->asSurfaceProxy()) {
        return false;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    auto fp = GrYUVtoRGBEffect::Make(proxies, yuvaIndices, yuvColorSpace,
                                     GrSamplerState::Filter::kNearest);
    if (colorSpaceXform) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(colorSpaceXform));
    }
    paint.addColorFragmentProcessor(std::move(fp));

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);
    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::MakePromiseImageLazyProxy(
        GrContext* context, int width, int height, GrSurfaceOrigin origin, GrColorType colorType,
        GrBackendFormat backendFormat, GrMipMapped mipMapped,
        PromiseImageTextureFulfillProc fulfillProc,
        PromiseImageTextureReleaseProc releaseProc,
        PromiseImageTextureDoneProc doneProc,
        PromiseImageTextureContext textureContext,
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
            if (fTexture) {
                SkMessageBus<GrGpuResourceFreedMessage>::Post({fTexture, fTextureContextID});
            }
        }

        GrSurfaceProxy::LazyInstantiationResult operator()(GrResourceProvider* resourceProvider) {
            // We use the unique key in a way that is unrelated to the SkImage-based key that the
            // proxy may receive, hence kUnsynced.
            static constexpr auto kKeySyncMode =
                    GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced;

            // Our proxy is getting instantiated for the second+ time. We are only allowed to call
            // Fulfill once. So return our cached result.
            if (fTexture) {
                return {sk_ref_sp(fTexture), kKeySyncMode};
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
            builder[1] = (uint32_t) fColorType;
            builder.finish();
            // A texture with this key may already exist from a different instance of this lazy
            // callback. This could happen if the client fulfills a promise image with a texture
            // that was previously used to fulfill a different promise image.
            if (auto surf = resourceProvider->findByUniqueKey<GrSurface>(key)) {
                tex = sk_ref_sp(surf->asTexture());
                SkASSERT(tex);
            } else {
                if ((tex = resourceProvider->wrapBackendTexture(
                             backendTexture, fColorType, kBorrow_GrWrapOwnership,
                             GrWrapCacheable::kYes, kRead_GrIOType))) {
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
            context->priv().getResourceCache()->insertDelayedResourceUnref(fTexture);
            fTextureContextID = context->priv().contextID();
            return {std::move(tex), kKeySyncMode};
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

    GrPixelConfig config = context->priv().caps()->getConfigFromBackendFormat(
                                                                     backendFormat,
                                                                     colorType);

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    // Ganesh assumes that, when wrapping a mipmapped backend texture from a client, that its
    // mipmaps are fully fleshed out.
    GrMipMapsStatus mipMapsStatus = (GrMipMapped::kYes == mipMapped)
            ? GrMipMapsStatus::kValid : GrMipMapsStatus::kNotAllocated;

    // We pass kReadOnly here since we should treat content of the client's texture as immutable.
    // The promise API provides no way for the client to indicated that the texture is protected.
    return proxyProvider->createLazyProxy(
            std::move(callback), backendFormat, desc, GrRenderable::kNo, 1, origin, mipMapped,
            mipMapsStatus, GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact, SkBudgeted::kNo,
            GrProtected::kNo, GrSurfaceProxy::LazyInstantiationType::kDeinstantiate);
}
