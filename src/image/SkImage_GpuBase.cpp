/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "effects/GrYUVtoRGBEffect.h"
#include "SkBitmapCache.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuBase.h"
#include "SkReadPixelsRec.h"

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkAlphaType at, sk_sp<SkColorSpace> cs)
        : INHERITED(width, height, uniqueID)
        , fContext(std::move(context))
        , fAlphaType(at)
        , fColorSpace(std::move(cs)) {}

SkImage_GpuBase::~SkImage_GpuBase() {}

//////////////////////////////////////////////////////////////////////////////////////////////////

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
    *config = ctx->contextPriv().caps()->getConfigFromBackendFormat(backendFormat, ct);
    return *config != kUnknown_GrPixelConfig;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuBase::getROPixels(SkBitmap* dst, CachingHint chint) const {
    if (!fContext->contextPriv().resourceProvider()) {
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

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
        this->asTextureProxyRef(),
        fColorSpace);
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

sk_sp<SkImage> SkImage_GpuBase::onMakeSubset(const SkIRect& subset) const {
    sk_sp<GrSurfaceProxy> proxy = this->asTextureProxyRef();

    GrSurfaceDesc desc;
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = proxy->config();

    GrBackendFormat format = proxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    // TODO: Should this inherit our proxy's budgeted status?
    sk_sp<GrSurfaceContext> sContext(fContext->contextPriv().makeDeferredSurfaceContext(
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
                                   sContext->asTextureProxyRef(), fColorSpace);
}

static void apply_premul(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    switch (info.colorType()) {
    case kRGBA_8888_SkColorType:
    case kBGRA_8888_SkColorType:
        break;
    default:
        return; // nothing to do
    }

    // SkColor is not necesarily RGBA or BGRA, but it is one of them on little-endian,
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
    if (!fContext->contextPriv().resourceProvider()) {
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

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
        this->asTextureProxyRef(), fColorSpace);
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

sk_sp<GrTextureProxy> SkImage_GpuBase::asTextureProxyRef(GrContext* context,
                                                         const GrSamplerState& params,
                                                         SkScalar scaleAdjust[2]) const {
    if (context->uniqueID() != fContext->uniqueID()) {
        SkASSERT(0);
        return nullptr;
    }

    GrTextureAdjuster adjuster(fContext.get(), this->asTextureProxyRef(), fAlphaType,
                               this->uniqueID(), fColorSpace.get());
    return adjuster.refTextureProxyForParams(params, scaleAdjust);
}

GrBackendTexture SkImage_GpuBase::onGetBackendTexture(bool flushPendingGrContextIO,
                                                      GrSurfaceOrigin* origin) const {
    sk_sp<GrTextureProxy> proxy = this->asTextureProxyRef();
    SkASSERT(proxy);

    if (!fContext->contextPriv().resourceProvider() && !proxy->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return GrBackendTexture();
}

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return GrBackendTexture(); // invalid
    }

    GrTexture* texture = proxy->peekTexture();

    if (texture) {
        if (flushPendingGrContextIO) {
            fContext->contextPriv().prepareSurfaceForExternalIO(proxy.get());
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
    if (!proxy) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxyRef = this->asTextureProxyRef();
    if (!fContext->contextPriv().resourceProvider() && !proxyRef->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return nullptr;
    }

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return nullptr;
    }

    return proxy->peekTexture();
}

sk_sp<SkImage> SkImage_GpuBase::onMakeColorSpace(sk_sp<SkColorSpace> target) const {
    auto xform = GrColorSpaceXformEffect::Make(fColorSpace.get(), fAlphaType,
                                               target.get(),      fAlphaType);
    SkASSERT(xform);

    sk_sp<GrTextureProxy> proxy = this->asTextureProxyRef();

    GrBackendFormat format = proxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fContext->contextPriv().makeDeferredRenderTargetContextWithFallback(
            format, SkBackingFit::kExact, this->width(), this->height(),
            proxy->config(), nullptr));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(std::move(proxy), SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(xform));

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                                  SkRect::MakeIWH(this->width(), this->height()));
    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, fAlphaType,
                                   renderTargetContext->asTextureProxyRef(), std::move(target));
}

bool SkImage_GpuBase::onIsValid(GrContext* context) const {
    // The base class has already checked that context isn't abandoned (if it's not nullptr)
    if (fContext->abandoned()) {
        return false;
    }

    if (context && context != fContext.get()) {
        return false;
    }

    return true;
}

bool SkImage_GpuBase::MakeTempTextureProxies(GrContext* ctx, const GrBackendTexture yuvaTextures[],
                                             int numTextures, const SkYUVAIndex yuvaIndices[4],
                                             GrSurfaceOrigin imageOrigin,
                                             sk_sp<GrTextureProxy> tempTextureProxies[4]) {
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();

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
                ctx->contextPriv().caps()->getYUVAConfigFromBackendFormat(backendFormat);
        if (yuvaTexturesCopy[textureIndex].fConfig == kUnknown_GrPixelConfig) {
            return false;
        }
        SkASSERT(yuvaTexturesCopy[textureIndex].isValid());

        tempTextureProxies[textureIndex] =
                proxyProvider->wrapBackendTexture(yuvaTexturesCopy[textureIndex], imageOrigin,
                                                  kBorrow_GrWrapOwnership, kRead_GrIOType);
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
                                       const sk_sp<GrTextureProxy> proxies[4],
                                       const SkYUVAIndex yuvaIndices[4]) {
    SkASSERT(renderTargetContext);
    if (!renderTargetContext->asSurfaceProxy()) {
        return false;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    paint.addColorFragmentProcessor(GrYUVtoRGBEffect::Make(proxies, yuvaIndices,
                                                           yuvColorSpace,
                                                           GrSamplerState::Filter::kNearest));

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    // DDL TODO: in the promise image version we must not flush here
    ctx->contextPriv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());

    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuBase::MakePromiseImageLazyProxy(
        GrContext* context, int width, int height, GrSurfaceOrigin origin, GrPixelConfig config,
        GrBackendFormat backendFormat, GrMipMapped mipMapped,
        SkImage_GpuBase::TextureFulfillProc fulfillProc,
        SkImage_GpuBase::TextureReleaseProc releaseProc, SkImage_GpuBase::PromiseDoneProc doneProc,
        SkImage_GpuBase::TextureContext textureContext) {
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
     * This helper class manages the ref counting for the the ReleaseProc and DoneProc for promise
     * images. It holds a weak ref on the ReleaseProc (hard refs are owned by GrTextures). The weak
     * ref allows us to reuse an outstanding ReleaseProc (because we dropped our GrTexture but the
     * GrTexture isn't done on the GPU) without needing to call FulfillProc again. It also holds a
     * hard ref on the DoneProc. The idea is that after every flush we may call the ReleaseProc so
     * that the client can free up their GPU memory if they want to. The life time of the DoneProc
     * matches that of any outstanding ReleaseProc as well as the PromiseLazyInstantiateCallback.
     * Thus we won't call the DoneProc until all ReleaseProcs are finished and we are finished with
     * the PromiseImageHelper (i.e. won't call FulfillProc again).
     */
    class PromiseLazyInstantiateCallback {
    public:
        PromiseLazyInstantiateCallback(SkImage_GpuBase::TextureFulfillProc fulfillProc,
                                       SkImage_GpuBase::TextureReleaseProc releaseProc,
                                       SkImage_GpuBase::PromiseDoneProc doneProc,
                                       SkImage_GpuBase::TextureContext context,
                                       GrPixelConfig config)
                : fFulfillProc(fulfillProc)
                , fReleaseProc(releaseProc)
                , fContext(context)
                , fConfig(config) {
            fDoneHelper.reset(new GrReleaseProcHelper(doneProc, context));
        }

        sk_sp<GrSurface> operator()(GrResourceProvider* resourceProvider) {
            if (!resourceProvider) {
                this->reset();
                return sk_sp<GrTexture>();
            }

            // Releases the promise helper if there are no outstanding hard refs. This means that we
            // don't have any ReleaseProcs waiting to be called so we will need to do a fulfill.
            if (fReleaseHelper && fReleaseHelper->weak_expired()) {
                this->resetReleaseHelper();
            }

            sk_sp<GrTexture> tex;
            if (!fReleaseHelper) {
                fFulfillProc(fContext, &fBackendTex);
                fBackendTex.fConfig = fConfig;
                if (!fBackendTex.isValid()) {
                    // Even though the GrBackendTexture is not valid, we must call the release
                    // proc to keep our contract of always calling Fulfill and Release in pairs.
                    fReleaseProc(fContext);
                    return sk_sp<GrTexture>();
                }

                tex = resourceProvider->wrapBackendTexture(fBackendTex, kBorrow_GrWrapOwnership,
                                                           kRead_GrIOType);
                if (!tex) {
                    // Even though the GrBackendTexture is not valid, we must call the release
                    // proc to keep our contract of always calling Fulfill and Release in pairs.
                    fReleaseProc(fContext);
                    return sk_sp<GrTexture>();
                }
                fReleaseHelper =
                        new SkPromiseReleaseProcHelper(fReleaseProc, fContext, fDoneHelper);
                // Take a weak ref
                fReleaseHelper->weak_ref();
            } else {
                SkASSERT(fBackendTex.isValid());
                tex = resourceProvider->wrapBackendTexture(fBackendTex, kBorrow_GrWrapOwnership,
                                                           kRead_GrIOType);
                if (!tex) {
                    // We weren't able to make a texture here, but since we are in this branch
                    // of the calls (promiseHelper.fReleaseHelper is valid) there is already a
                    // texture out there which will call the release proc so we don't need to
                    // call it here.
                    return sk_sp<GrTexture>();
                }

                SkAssertResult(fReleaseHelper->try_ref());
            }
            SkASSERT(tex);
            // Pass the hard ref off to the texture
            tex->setRelease(sk_sp<GrReleaseProcHelper>(fReleaseHelper));
            return std::move(tex);
        }

    private:
        void reset() {
            this->resetReleaseHelper();
            fDoneHelper.reset();
        }

        // Weak unrefs fReleaseHelper and sets it to null
        void resetReleaseHelper() {
            if (fReleaseHelper) {
                fReleaseHelper->weak_unref();
                fReleaseHelper = nullptr;
            }
        }

        SkImage_GpuBase::TextureFulfillProc fFulfillProc;
        SkImage_GpuBase::TextureReleaseProc fReleaseProc;
        SkImage_GpuBase::TextureContext fContext;

        GrPixelConfig fConfig;
        // We cache the GrBackendTexture so that if we deleted the GrTexture but the the release
        // proc has yet not been called (this can happen on Vulkan), then we can create a new
        // texture without needing to call the fulfill proc again.
        GrBackendTexture fBackendTex;
        // The fReleaseHelper is used to track a weak ref on the release proc. This helps us make
        // sure we are always pairing fulfill and release proc calls correctly.
        SkPromiseReleaseProcHelper* fReleaseHelper = nullptr;
        // We don't want to call the fDoneHelper until we are done with the PromiseImageHelper and
        // all ReleaseHelpers are finished. Thus we hold a hard ref here and we will pass a hard ref
        // to each fReleaseHelper we make.
        sk_sp<GrReleaseProcHelper> fDoneHelper;
    } callback(fulfillProc, releaseProc, doneProc, textureContext, config);

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

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
