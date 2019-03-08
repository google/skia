/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_GpuBase.h"
#include "GrBackendSurface.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "SkBitmapCache.h"
#include "SkImage_Gpu.h"
#include "SkPromiseImageTexture.h"
#include "SkReadPixelsRec.h"
#include "SkTLList.h"
#include "effects/GrYUVtoRGBEffect.h"

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkAlphaType at, sk_sp<SkColorSpace> cs)
        : INHERITED(width, height, uniqueID)
        , fContext(std::move(context))
        , fAlphaType(at)
        , fColorSpace(std::move(cs)) {}

SkImage_GpuBase::~SkImage_GpuBase() {}

//////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS
void SkImage_GpuBase::resetContext(sk_sp<GrContext> newContext) {
    SkASSERT(fContext->priv().matches(newContext.get()));
    fContext = newContext;
}
#endif

bool SkImage_GpuBase::ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
                                             GrPixelConfig* config, SkColorType ct, SkAlphaType at,
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
    *config = ctx->priv().caps()->getConfigFromBackendFormat(backendFormat, ct);
    return *config != kUnknown_GrPixelConfig;
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
        rec = SkBitmapCache::Alloc(desc, this->onImageInfo(), &pmap);
        if (!rec) {
            return false;
        }
    } else {
        if (!dst->tryAllocPixels(this->onImageInfo()) || !dst->peekPixels(&pmap)) {
            return false;
        }
    }

    sk_sp<GrSurfaceContext> sContext = direct->priv().makeWrappedSurfaceContext(
        this->asTextureProxyRef(direct), fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), 0, 0)) {
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

    GrSurfaceDesc desc;
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = proxy->config();

    GrBackendFormat format = proxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    // TODO: Should this inherit our proxy's budgeted status?
    sk_sp<GrSurfaceContext> sContext(context->priv().makeDeferredSurfaceContext(
            format, desc, proxy->origin(), GrMipMapped::kNo, SkBackingFit::kExact,
            proxy->isBudgeted()));
    if (!sContext) {
        return nullptr;
    }

    if (!sContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, fAlphaType,
                                   sContext->asTextureProxyRef(), this->refColorSpace());
}

static void apply_premul(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    switch (info.colorType()) {
    case kRGBA_8888_SkColorType:
    case kBGRA_8888_SkColorType:
        break;
    default:
        return; // nothing to do
    }

    // SkColor is not necessarily RGBA or BGRA, but it is one of them on little-endian,
    // and in either case, the alpha-byte is always in the same place, so we can safely call
    // SkPreMultiplyColor()
    //
    SkColor* row = (SkColor*)pixels;
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            row[x] = SkPreMultiplyColor(row[x]);
        }
        row = (SkColor*)((char*)(row)+rowBytes);
    }
}

bool SkImage_GpuBase::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                   int srcX, int srcY, CachingHint) const {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        return false;
    }

    if (!SkImageInfoValidConversion(dstInfo, this->onImageInfo())) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    // TODO: this seems to duplicate code in GrTextureContext::onReadPixels and
    // GrRenderTargetContext::onReadPixels
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == rec.fInfo.alphaType() && kPremul_SkAlphaType == fAlphaType) {
        // let the GPU perform this transformation for us
        flags = GrContextPriv::kUnpremul_PixelOpsFlag;
    }

    sk_sp<GrSurfaceContext> sContext = direct->priv().makeWrappedSurfaceContext(
        this->asTextureProxyRef(direct), this->refColorSpace());
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, flags)) {
        return false;
    }

    // do we have to manually fix-up the alpha channel?
    //      src         dst
    //      unpremul    premul      fix manually
    //      premul      unpremul    done by kUnpremul_PixelOpsFlag
    // all other combos need to change.
    //
    // Should this be handled by Ganesh? todo:?
    //
    if (kPremul_SkAlphaType == rec.fInfo.alphaType() && kUnpremul_SkAlphaType == fAlphaType) {
        apply_premul(rec.fInfo, rec.fPixels, rec.fRowBytes);
    }
    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::asTextureProxyRef(GrRecordingContext* context,
                                                         const GrSamplerState& params,
                                                         SkScalar scaleAdjust[2]) const {
    if (!context || !fContext->priv().matches(context)) {
        SkASSERT(0);
        return nullptr;
    }

    GrTextureAdjuster adjuster(fContext.get(), this->asTextureProxyRef(context), fAlphaType,
                               this->uniqueID(), fColorSpace.get());
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
            direct->priv().prepareSurfaceForExternalIO(proxy.get());
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

    // We need to make a copy of the input backend textures because we need to preserve the result
    // of validate_backend_texture.
    GrBackendTexture yuvaTexturesCopy[4];
    for (int textureIndex = 0; textureIndex < numTextures; ++textureIndex) {
        yuvaTexturesCopy[textureIndex] = yuvaTextures[textureIndex];
        GrBackendFormat backendFormat = yuvaTexturesCopy[textureIndex].getBackendFormat();
        if (!backendFormat.isValid()) {
            return false;
        }
        yuvaTexturesCopy[textureIndex].fConfig =
                ctx->priv().caps()->getYUVAConfigFromBackendFormat(backendFormat);
        if (yuvaTexturesCopy[textureIndex].fConfig == kUnknown_GrPixelConfig) {
            return false;
        }
        SkASSERT(yuvaTexturesCopy[textureIndex].isValid());

        tempTextureProxies[textureIndex] = proxyProvider->wrapBackendTexture(
                yuvaTexturesCopy[textureIndex], imageOrigin, kBorrow_GrWrapOwnership,
                GrWrapCacheable::kNo, kRead_GrIOType);
        if (!tempTextureProxies[textureIndex]) {
            return false;
        }

        // Check that each texture contains the channel data for the corresponding YUVA index
        GrPixelConfig config = yuvaTexturesCopy[textureIndex].fConfig;
        for (int yuvaIndex = 0; yuvaIndex < SkYUVAIndex::kIndexCount; ++yuvaIndex) {
            if (yuvaIndices[yuvaIndex].fIndex == textureIndex) {
                switch (yuvaIndices[yuvaIndex].fChannel) {
                    case SkColorChannel::kR:
                        if (kAlpha_8_as_Alpha_GrPixelConfig == config) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kG:
                    case SkColorChannel::kB:
                        if (kAlpha_8_as_Alpha_GrPixelConfig == config ||
                            kAlpha_8_as_Red_GrPixelConfig == config) {
                            return false;
                        }
                        break;
                    case SkColorChannel::kA:
                    default:
                        if (kRGB_888_GrPixelConfig == config) {
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

    // DDL TODO: in the promise image version we must not flush here
    ctx->priv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());

    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::MakePromiseImageLazyProxy(
        GrContext* context, int width, int height, GrSurfaceOrigin origin, GrPixelConfig config,
        GrBackendFormat backendFormat, GrMipMapped mipMapped,
        PromiseImageTextureFulfillProc fulfillProc,
        PromiseImageTextureReleaseProc releaseProc,
        PromiseImageTextureDoneProc doneProc,
        PromiseImageTextureContext textureContext) {
    SkASSERT(context);
    SkASSERT(width > 0 && height > 0);
    SkASSERT(doneProc);
    SkASSERT(config != kUnknown_GrPixelConfig);

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
                                       GrPixelConfig config)
                : fFulfillProc(fulfillProc), fConfig(config) {
            auto doneHelper = sk_make_sp<GrRefCntedCallback>(doneProc, context);
            fIdleCallback = sk_make_sp<GrRefCntedCallback>(releaseProc, context);
            fIdleCallback->addChild(std::move(doneHelper));
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
            if (fIdleCallback) {
                SkASSERT(!fTexture);
                // We were never fulfilled. Pass false so done proc is still called.
                fIdleCallback->abandon();
            }
            // Our destructor can run on any thread. We trigger the unref of fTexture by message.
            if (fTexture) {
                SkASSERT(!fIdleCallback);
                SkMessageBus<GrGpuResourceFreedMessage>::Post({fTexture, fTextureContextID});
            }
        }

        sk_sp<GrSurface> operator()(GrResourceProvider* resourceProvider) {
            // Our proxy is getting instantiated for the second+ time. We are only allowed to call
            // Fulfill once. So return our cached result.
            if (fTexture) {
                return sk_ref_sp(fTexture);
            }
            SkASSERT(fIdleCallback);
            PromiseImageTextureContext textureContext = fIdleCallback->context();
            sk_sp<SkPromiseImageTexture> promiseTexture = fFulfillProc(textureContext);
            // From here on out our contract is that the release proc must be called, even if
            // the return from fulfill was invalid or we fail for some other reason.
            if (!promiseTexture) {
                // Make sure we explicitly reset this because our destructor assumes a non-null
                // fIdleCallback means fulfill was never called.
                fIdleCallback.reset();
                return sk_sp<GrTexture>();
            }

            auto backendTexture = promiseTexture->backendTexture();
            backendTexture.fConfig = fConfig;
            if (!backendTexture.isValid()) {
                fIdleCallback.reset();
                return sk_sp<GrTexture>();
            }

            sk_sp<GrTexture> tex;
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey key;
            GrUniqueKey::Builder builder(&key, kDomain, 2, "promise");
            builder[0] = promiseTexture->uniqueID();
            builder[1] = fConfig;
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
                    fIdleCallback.reset();
                    return sk_sp<GrTexture>();
                }
            }
            tex->addIdleProc(std::move(fIdleCallback));
            promiseTexture->addKeyToInvalidate(tex->getContext()->priv().contextID(), key);
            fTexture = tex.get();
            // We need to hold on to the GrTexture in case our proxy gets reinstantiated. However,
            // we can't unref in our destructor because we may be on another thread then. So we
            // let the cache know it is waiting on an unref message. We will send that message from
            // our destructor.
            GrContext* context = fTexture->getContext();
            context->priv().getResourceCache()->insertDelayedResourceUnref(fTexture);
            fTextureContextID = context->priv().contextID();
            return std::move(tex);
        }

    private:
        GrTexture* fTexture = nullptr;
        uint32_t fTextureContextID = SK_InvalidUniqueID;
        sk_sp<GrRefCntedCallback> fIdleCallback;
        PromiseImageTextureFulfillProc fFulfillProc;
        GrPixelConfig fConfig;
    } callback(fulfillProc, releaseProc, doneProc, textureContext, config);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    // We pass kReadOnly here since we should treat content of the client's texture as immutable.
    return proxyProvider->createLazyProxy(std::move(callback), backendFormat, desc, origin,
                                          mipMapped, GrInternalSurfaceFlags::kReadOnly,
                                          SkBackingFit::kExact, SkBudgeted::kNo,
                                          GrSurfaceProxy::LazyInstantiationType::kDeinstantiate);
}
