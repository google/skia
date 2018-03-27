/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "SkAutoPixmapStorage.h"
#include "GrBackendSurface.h"
#include "GrBackendTextureImageGenerator.h"
#include "GrAHardwareBufferImageGenerator.h"
#include "GrBitmapTextureMaker.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrImageTextureMaker.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrSurfacePriv.h"
#include "GrTextureAdjuster.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "gl/GrGLDefines.h"
#include "effects/GrNonlinearColorSpaceXformEffect.h"
#include "effects/GrYUVtoRGBEffect.h"
#include "SkCanvas.h"
#include "SkBitmapCache.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkImageCacherator.h"
#include "SkImageInfoPriv.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkReadPixelsRec.h"
#include "SkTraceEvent.h"

SkImage_Gpu::SkImage_Gpu(GrContext* context, uint32_t uniqueID, SkAlphaType at,
                         sk_sp<GrTextureProxy> proxy,
                         sk_sp<SkColorSpace> colorSpace, SkBudgeted budgeted)
    : INHERITED(proxy->worstCaseWidth(), proxy->worstCaseHeight(), uniqueID)
    , fContext(context)
    , fProxy(std::move(proxy))
    , fAlphaType(at)
    , fBudgeted(budgeted)
    , fColorSpace(std::move(colorSpace))
    , fAddedRasterVersionToCache(false) {
}

SkImage_Gpu::~SkImage_Gpu() {
    if (fAddedRasterVersionToCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

SkImageInfo SkImage_Gpu::onImageInfo() const {
    return SkImageInfo::Make(fProxy->width(), fProxy->height(), this->onColorType(), fAlphaType,
                             fColorSpace);
}

SkColorType SkImage_Gpu::onColorType() const {
    SkColorType ct;
    if (!GrPixelConfigToColorType(fProxy->config(), &ct)) {
        ct = kUnknown_SkColorType;
    }
    return ct;
}

bool SkImage_Gpu::getROPixels(SkBitmap* dst, SkColorSpace*, CachingHint chint) const {
    // The SkColorSpace parameter "dstColorSpace" is really just a hint about how/where the bitmap
    // will be used. The client doesn't expect that we convert to that color space, it's intended
    // for codec-backed images, to drive our decoding heuristic. In theory we *could* read directly
    // into that color space (to save the client some effort in whatever they're about to do), but
    // that would make our use of the bitmap cache incorrect (or much less efficient, assuming we
    // rolled the dstColorSpace into the key).
    const auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, dst)) {
        SkASSERT(dst->getGenerationID() == this->uniqueID());
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
                                                                                    fProxy,
                                                                                    fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), 0, 0)) {
        return false;
    }

    if (rec) {
        SkBitmapCache::Add(std::move(rec), dst);
        fAddedRasterVersionToCache.store(true);
    }
    return true;
}

sk_sp<GrTextureProxy> SkImage_Gpu::asTextureProxyRef(GrContext* context,
                                                     const GrSamplerState& params,
                                                     SkColorSpace* dstColorSpace,
                                                     sk_sp<SkColorSpace>* texColorSpace,
                                                     SkScalar scaleAdjust[2]) const {
    if (context != fContext) {
        SkASSERT(0);
        return nullptr;
    }

    if (texColorSpace) {
        *texColorSpace = this->fColorSpace;
    }

    GrTextureAdjuster adjuster(fContext, fProxy, this->alphaType(), this->uniqueID(),
                               this->fColorSpace.get());
    return adjuster.refTextureProxySafeForParams(params, scaleAdjust);
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
        row = (SkColor*)((char*)(row) + rowBytes);
    }
}

GrBackendObject SkImage_Gpu::onGetTextureHandle(bool flushPendingGrContextIO,
                                                GrSurfaceOrigin* origin) const {
    SkASSERT(fProxy);

    if (!fContext->contextPriv().resourceProvider() && !fProxy->priv().isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated. Thus we return 0
        // here which is considered invalid for all backends.
        return 0;
    }

    if (GrSurfaceProxy::LazyState::kNot != fProxy->lazyInstantiationState()) {
        SkASSERT(fContext->contextPriv().resourceProvider());
        fProxy->priv().doLazyInstantiation(fContext->contextPriv().resourceProvider());
        if (!fProxy->priv().isInstantiated()) {
            // We failed to instantiate the lazy proxy. Thus we return 0 here which is considered
            // invalid for all backends.
            return 0;
        }
    }

    if (!fProxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return 0;
    }

    GrTexture* texture = fProxy->priv().peekTexture();

    if (texture) {
        if (flushPendingGrContextIO) {
            fContext->contextPriv().prepareSurfaceForExternalIO(fProxy.get());
        }
        if (origin) {
            *origin = fProxy->origin();
        }
        return texture->getTextureHandle();
    }
    return 0;
}

GrTexture* SkImage_Gpu::onGetTexture() const {
    GrTextureProxy* proxy = this->peekProxy();
    if (!proxy) {
        return nullptr;
    }

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return nullptr;
    }

    return proxy->priv().peekTexture();
}

bool SkImage_Gpu::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                               int srcX, int srcY, CachingHint) const {
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

    // This hack allows us to call makeNonTextureImage on images with arbitrary color spaces.
    // Otherwise, we'll be unable to create a render target context.
    // TODO: This shouldn't be necessary - we need more robust support for images (and surfaces)
    // with arbitrary color spaces. Unfortunately, this is one spot where we go from image to
    // surface (rather than the opposite), and our lenient image rules break our (currently) more
    // strict surface rules.
    // GrSurfaceContext::readPixels does not make use of the context's color space. However, we
    // don't allow creating a surface context for a sRGB GrPixelConfig unless the color space has
    // sRGB gamma. So we choose null for non-SRGB GrPixelConfigs and sRGB for sRGB GrPixelConfigs.
    sk_sp<SkColorSpace> surfaceColorSpace = fColorSpace;
    if (!flags) {
        if (!dstInfo.colorSpace() ||
            SkColorSpace::Equals(fColorSpace.get(), dstInfo.colorSpace())) {
            if (GrPixelConfigIsSRGB(fProxy->config())) {
                surfaceColorSpace = SkColorSpace::MakeSRGB();
            } else {
                surfaceColorSpace = nullptr;
            }
        }
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
            fProxy, surfaceColorSpace);
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

sk_sp<SkImage> SkImage_Gpu::onMakeSubset(const SkIRect& subset) const {
    GrSurfaceDesc desc;
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = fProxy->config();

    sk_sp<GrSurfaceContext> sContext(fContext->contextPriv().makeDeferredSurfaceContext(
            desc, fProxy->origin(), GrMipMapped::kNo, SkBackingFit::kExact, fBudgeted));
    if (!sContext) {
        return nullptr;
    }

    if (!sContext->copy(fProxy.get(), subset, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   fAlphaType, sContext->asTextureProxyRef(),
                                   fColorSpace, fBudgeted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx,
                                                 const GrBackendTexture& backendTex,
                                                 GrSurfaceOrigin origin,
                                                 SkAlphaType at, sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 SkImage::TextureReleaseProc releaseProc,
                                                 SkImage::ReleaseContext releaseCtx) {
    if (backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(backendTex, origin, ownership,
                                                                    releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(ctx, kNeedNewImageUniqueID,
                                   at, std::move(proxy), std::move(colorSpace), SkBudgeted::kNo);
}

bool validate_backend_texture(GrContext* ctx, const GrBackendTexture& tex, GrPixelConfig* config,
                              SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs) {
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, at, cs);
    if (!SkImageInfoIsValidAllowNumericalCS(info)) {
        return false;
    }

    return ctx->caps()->validateBackendTexture(tex, ct, config);
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    if (!ctx) {
        return nullptr;
    }
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(ctx, texCopy, &texCopy.fConfig, ct, at, cs)) {
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, texCopy, origin, at, std::move(cs),
                                      kBorrow_GrWrapOwnership, releaseP, releaseC);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx,
                                               const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                               SkColorType ct, SkAlphaType at,
                                               sk_sp<SkColorSpace> cs) {
    if (!ctx || !ctx->contextPriv().resourceProvider()) {
        // We have a DDL context and we don't support adopted textures for them.
        return nullptr;
    }
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(ctx, texCopy, &texCopy.fConfig, ct, at, cs)) {
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, texCopy, origin, at, std::move(cs),
                                      kAdopt_GrWrapOwnership, nullptr, nullptr);
}

sk_sp<SkImage> SkImage_Gpu::MakeFromYUVTexturesCopyImpl(
        GrContext* ctx, SkYUVColorSpace colorSpace, bool nv12,
        const GrBackendTexture yuvBackendTexturesOrig[], GrSurfaceOrigin origin,
        sk_sp<SkColorSpace> imageColorSpace) {
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();

    GrBackendTexture yuvBackendTextures[]{
            yuvBackendTexturesOrig[0],
            yuvBackendTexturesOrig[1],
            yuvBackendTexturesOrig[2],
    };
    auto ct = nv12 ? kRGBA_8888_SkColorType : kAlpha_8_SkColorType;
    for (int i = 0; i < (nv12 ? 2 : 3); ++i) {
        if (!validate_backend_texture(ctx, yuvBackendTextures[i], &yuvBackendTextures[i].fConfig,
                                      ct, kPremul_SkAlphaType, nullptr)) {
            return nullptr;
        }
    }
    sk_sp<GrTextureProxy> yProxy = proxyProvider->wrapBackendTexture(yuvBackendTextures[0], origin);
    sk_sp<GrTextureProxy> uProxy = proxyProvider->wrapBackendTexture(yuvBackendTextures[1], origin);
    sk_sp<GrTextureProxy> vProxy;

    if (nv12) {
        vProxy = uProxy;
    } else {
        vProxy = proxyProvider->wrapBackendTexture(yuvBackendTextures[2], origin);
    }
    if (!yProxy || !uProxy || !vProxy) {
        return nullptr;
    }

    const int width = yuvBackendTextures[0].width();
    const int height = yuvBackendTextures[0].height();

    // Needs to be a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(
            ctx->contextPriv().makeDeferredRenderTargetContext(
                                SkBackingFit::kExact, width, height, kRGBA_8888_GrPixelConfig,
                                std::move(imageColorSpace), 1, GrMipMapped::kNo, origin));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    SkISize sizes[] = {{yuvBackendTextures[0].width(), yuvBackendTextures[0].height()},
                       {yuvBackendTextures[1].width(), yuvBackendTextures[1].height()},
                       {yuvBackendTextures[2].width(), yuvBackendTextures[2].height()}};
    paint.addColorFragmentProcessor(
            GrYUVtoRGBEffect::Make(yProxy, uProxy, vProxy, sizes, colorSpace, nv12));

    const SkRect rect = SkRect::MakeIWH(width, height);

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->asSurfaceProxy()) {
        return nullptr;
    }
    ctx->contextPriv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(ctx, kNeedNewImageUniqueID, kOpaque_SkAlphaType,
                                   renderTargetContext->asTextureProxyRef(),
                                   renderTargetContext->colorSpaceInfo().refColorSpace(),
                                   SkBudgeted::kYes);
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                const GrBackendTexture yuvTextures[3],
                                                GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> imageColorSpace) {
    return SkImage_Gpu::MakeFromYUVTexturesCopyImpl(ctx, colorSpace, false, yuvTextures,
                                                    origin, std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromNV12TexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                 const GrBackendTexture nv12Textures[2],
                                                 GrSurfaceOrigin origin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    return SkImage_Gpu::MakeFromYUVTexturesCopyImpl(ctx, colorSpace, true, nv12Textures,
                                                    origin, std::move(imageColorSpace));
}

static sk_sp<SkImage> create_image_from_maker(GrContext* context, GrTextureMaker* maker,
                                              SkAlphaType at, uint32_t id,
                                              SkColorSpace* dstColorSpace) {
    sk_sp<SkColorSpace> texColorSpace;
    sk_sp<GrTextureProxy> proxy(maker->refTextureProxyForParams(
            GrSamplerState::ClampNearest(), dstColorSpace, &texColorSpace, nullptr));
    if (!proxy) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(context, id, at,
                                   std::move(proxy), std::move(texColorSpace), SkBudgeted::kNo);
}

sk_sp<SkImage> SkImage::makeTextureImage(GrContext* context, SkColorSpace* dstColorSpace) const {
    if (!context) {
        return nullptr;
    }
    if (GrContext* incumbent = as_IB(this)->context()) {
        return incumbent == context ? sk_ref_sp(const_cast<SkImage*>(this)) : nullptr;
    }

    if (this->isLazyGenerated()) {
        GrImageTextureMaker maker(context, this, kDisallow_CachingHint);
        return create_image_from_maker(context, &maker, this->alphaType(),
                                       this->uniqueID(), dstColorSpace);
    }

    if (const SkBitmap* bmp = as_IB(this)->onPeekBitmap()) {
        GrBitmapTextureMaker maker(context, *bmp);
        return create_image_from_maker(context, &maker, this->alphaType(),
                                       this->uniqueID(), dstColorSpace);
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This helper holds the normal hard ref for the Release proc as well as a hard ref on the DoneProc.
 * Thus when a GrTexture is being released, it will unref both the ReleaseProc and DoneProc.
 */
class PromiseReleaseProcHelper : public GrReleaseProcHelper {
public:
    PromiseReleaseProcHelper(SkImage_Gpu::TextureReleaseProc releaseProc,
                             SkImage_Gpu::TextureContext context,
                             sk_sp<GrReleaseProcHelper> doneHelper)
            : INHERITED(releaseProc, context)
            , fDoneProcHelper(std::move(doneHelper)) {}

    void weak_dispose() const override {
        // Call the inherited weak_dispose first so that we call the ReleaseProc before the DoneProc
        // if we hold the last ref to the DoneProc.
        INHERITED::weak_dispose();
        fDoneProcHelper.reset();
    }

private:
    mutable sk_sp<GrReleaseProcHelper> fDoneProcHelper;

    typedef GrReleaseProcHelper INHERITED;
};

/**
 * This helper class manages the ref counting for the the ReleaseProc and DoneProc for promise
 * images. It holds a weak ref on the ReleaseProc (hard refs are owned by GrTextures). The weak ref
 * allows us to reuse an outstanding ReleaseProc (because we dropped our GrTexture but the GrTexture
 * isn't done on the GPU) without needing to call FulfillProc again. It also holds a hard ref on the
 * DoneProc. The idea is that after every flush we may call the ReleaseProc so that the client can
 * free up their GPU memory if they want to. The life time of the DoneProc matches that of any
 * outstanding ReleaseProc as well as the PromiseImageHelper. Thus we won't call the DoneProc until
 * all ReleaseProcs are finished and we are finished with the PromiseImageHelper (i.e. won't call
 * FulfillProc again).
 */
class PromiseImageHelper {
public:
    PromiseImageHelper(SkImage_Gpu::TextureFulfillProc fulFillProc,
                       SkImage_Gpu::TextureReleaseProc releaseProc,
                       SkImage_Gpu::PromiseDoneProc doneProc,
                       SkImage_Gpu::TextureContext context)
            : fFulfillProc(fulFillProc)
            , fReleaseProc(releaseProc)
            , fContext(context)
            , fDoneHelper(new GrReleaseProcHelper(doneProc, context)) {}

    void reset() {
        this->resetReleaseHelper();
        fDoneHelper.reset();
    }

    sk_sp<GrTexture> getTexture(GrResourceProvider* resourceProvider, GrPixelConfig config) {
        // Releases the promise helper if there are no outstanding hard refs. This means that we
        // don't have any ReleaseProcs waiting to be called so we will need to do a fulfill.
        if (fReleaseHelper && fReleaseHelper->weak_expired()) {
            this->resetReleaseHelper();
        }

        sk_sp<GrTexture> tex;
        if (!fReleaseHelper) {
            fFulfillProc(fContext, &fBackendTex);
            fBackendTex.fConfig = config;
            if (!fBackendTex.isValid()) {
                // Even though the GrBackendTexture is not valid, we must call the release
                // proc to keep our contract of always calling Fulfill and Release in pairs.
                fReleaseProc(fContext);
                return sk_sp<GrTexture>();
            }

            tex = resourceProvider->wrapBackendTexture(fBackendTex, kBorrow_GrWrapOwnership);
            if (!tex) {
                // Even though the GrBackendTexture is not valid, we must call the release
                // proc to keep our contract of always calling Fulfill and Release in pairs.
                fReleaseProc(fContext);
                return sk_sp<GrTexture>();
            }
            fReleaseHelper = new PromiseReleaseProcHelper(fReleaseProc, fContext, fDoneHelper);
            // Take a weak ref
            fReleaseHelper->weak_ref();
        } else {
            SkASSERT(fBackendTex.isValid());
            tex = resourceProvider->wrapBackendTexture(fBackendTex, kBorrow_GrWrapOwnership);
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
        return tex;
    }

private:
    // Weak unrefs fReleaseHelper and sets it to null
    void resetReleaseHelper() {
        if (fReleaseHelper) {
            fReleaseHelper->weak_unref();
            fReleaseHelper = nullptr;
        }
    }

    SkImage_Gpu::TextureFulfillProc fFulfillProc;
    SkImage_Gpu::TextureReleaseProc fReleaseProc;
    SkImage_Gpu::TextureContext     fContext;

    // We cache the GrBackendTexture so that if we deleted the GrTexture but the the release proc
    // has yet not been called (this can happen on Vulkan), then we can create a new texture without
    // needing to call the fulfill proc again.
    GrBackendTexture fBackendTex;
    // The fReleaseHelper is used to track a weak ref on the release proc. This helps us make sure
    // we are always pairing fulfill and release proc calls correctly.
    PromiseReleaseProcHelper* fReleaseHelper = nullptr;
    // We don't want to call the fDoneHelper until we are done with the PromiseImageHelper and all
    // ReleaseHelpers are finished. Thus we hold a hard ref here and we will pass a hard ref to each
    // fReleaseHelper we make.
    sk_sp<GrReleaseProcHelper> fDoneHelper;
};

static GrInternalSurfaceFlags get_flags_from_format(const GrBackendFormat& backendFormat) {
    if (const GrGLenum* target = backendFormat.getGLTarget()) {
        if (GR_GL_TEXTURE_RECTANGLE == *target || GR_GL_TEXTURE_EXTERNAL == *target) {
            return GrInternalSurfaceFlags::kDoesNotSupportMipMaps |
                   GrInternalSurfaceFlags::kIsClampOnly;
        }
    }

    return GrInternalSurfaceFlags::kNone;
}

sk_sp<SkImage> SkImage_Gpu::MakePromiseTexture(GrContext* context,
                                               const GrBackendFormat& backendFormat,
                                               int width,
                                               int height,
                                               GrMipMapped mipMapped,
                                               GrSurfaceOrigin origin,
                                               SkColorType colorType,
                                               SkAlphaType alphaType,
                                               sk_sp<SkColorSpace> colorSpace,
                                               TextureFulfillProc textureFulfillProc,
                                               TextureReleaseProc textureReleaseProc,
                                               PromiseDoneProc promiseDoneProc,
                                               TextureContext textureContext) {
    if (!context) {
        return nullptr;
    }

    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    if (!textureFulfillProc || !textureReleaseProc || !promiseDoneProc) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(width, height, colorType, alphaType, colorSpace);
    if (!SkImageInfoIsValidAllowNumericalCS(info)) {
        return nullptr;
    }
    GrPixelConfig config = kUnknown_GrPixelConfig;
    if (!context->caps()->getConfigFromBackendFormat(backendFormat, colorType, &config)) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    PromiseImageHelper promiseHelper(textureFulfillProc, textureReleaseProc, promiseDoneProc,
                                     textureContext);

    GrInternalSurfaceFlags formatFlags = get_flags_from_format(backendFormat);

    sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
            [promiseHelper, config] (GrResourceProvider* resourceProvider) mutable {
                if (!resourceProvider) {
                    promiseHelper.reset();
                    return sk_sp<GrTexture>();
                }

                return promiseHelper.getTexture(resourceProvider, config);
            }, desc, origin, mipMapped, formatFlags, SkBackingFit::kExact,
               SkBudgeted::kNo, GrSurfaceProxy::LazyInstantiationType::kUninstantiate);

    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(context, kNeedNewImageUniqueID, alphaType, std::move(proxy),
                                   std::move(colorSpace), SkBudgeted::kNo);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeCrossContextFromEncoded(GrContext* context, sk_sp<SkData> encoded,
                                                    bool buildMips, SkColorSpace* dstColorSpace) {
    sk_sp<SkImage> codecImage = SkImage::MakeFromEncoded(std::move(encoded));
    if (!codecImage) {
        return nullptr;
    }

    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!context || !context->caps()->crossContextTextureSupport()) {
        return codecImage;
    }

    // Turn the codec image into a GrTextureProxy
    GrImageTextureMaker maker(context, codecImage.get(), kDisallow_CachingHint);
    sk_sp<SkColorSpace> texColorSpace;
    GrSamplerState samplerState(
            GrSamplerState::WrapMode::kClamp,
            buildMips ? GrSamplerState::Filter::kMipMap : GrSamplerState::Filter::kBilerp);
    sk_sp<GrTextureProxy> proxy(
            maker.refTextureProxyForParams(samplerState, dstColorSpace, &texColorSpace, nullptr));
    if (!proxy) {
        return codecImage;
    }

    if (!proxy->instantiate(context->contextPriv().resourceProvider())) {
        return codecImage;
    }
    sk_sp<GrTexture> texture = sk_ref_sp(proxy->priv().peekTexture());

    // Flush any writes or uploads
    context->contextPriv().prepareSurfaceForExternalIO(proxy.get());

    GrGpu* gpu = context->contextPriv().getGpu();
    sk_sp<GrSemaphore> sema = gpu->prepareTextureForCrossContextUsage(texture.get());

    auto gen = GrBackendTextureImageGenerator::Make(std::move(texture), proxy->origin(),
                                                    std::move(sema),
                                                    as_IB(codecImage)->onImageInfo().colorType(),
                                                    codecImage->alphaType(),
                                                    std::move(texColorSpace));
    return SkImage::MakeFromGenerator(std::move(gen));
}

sk_sp<SkImage> SkImage::MakeCrossContextFromPixmap(GrContext* context, const SkPixmap& pixmap,
                                                   bool buildMips, SkColorSpace* dstColorSpace) {
    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!context || !context->caps()->crossContextTextureSupport()) {
        return SkImage::MakeRasterCopy(pixmap);
    }

    // If we don't have access to the resource provider and gpu (i.e. in a DDL context) we will not
    // be able to make everything needed for a GPU CrossContext image. Thus return a raster copy
    // instead.
    if (!context->contextPriv().resourceProvider()) {
        return SkImage::MakeRasterCopy(pixmap);
    }

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    // Turn the pixmap into a GrTextureProxy
    sk_sp<GrTextureProxy> proxy;
    if (buildMips) {
        SkBitmap bmp;
        bmp.installPixels(pixmap);
        proxy = proxyProvider->createMipMapProxyFromBitmap(bmp, dstColorSpace);
    } else {
        SkDestinationSurfaceColorMode colorMode = dstColorSpace
                ? SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware
                : SkDestinationSurfaceColorMode::kLegacy;

        if (SkImageInfoIsValid(pixmap.info(), colorMode)) {
            ATRACE_ANDROID_FRAMEWORK("Upload Texture [%ux%u]", pixmap.width(), pixmap.height());
            // We don't need a release proc on the data in pixmap since we know we are in a
            // GrContext that has a resource provider. Thus the createTextureProxy call will
            // immediately upload the data.
            sk_sp<SkImage> image = SkImage::MakeFromRaster(pixmap, nullptr, nullptr);
            proxy = proxyProvider->createTextureProxy(std::move(image), kNone_GrSurfaceFlags, 1,
                                                      SkBudgeted::kYes, SkBackingFit::kExact);
        }
    }

    if (!proxy) {
        return SkImage::MakeRasterCopy(pixmap);
    }

    sk_sp<GrTexture> texture = sk_ref_sp(proxy->priv().peekTexture());

    // Flush any writes or uploads
    context->contextPriv().prepareSurfaceForExternalIO(proxy.get());
    GrGpu* gpu = context->contextPriv().getGpu();

    sk_sp<GrSemaphore> sema = gpu->prepareTextureForCrossContextUsage(texture.get());

    auto gen = GrBackendTextureImageGenerator::Make(std::move(texture), proxy->origin(),
                                                    std::move(sema), pixmap.colorType(),
                                                    pixmap.alphaType(),
                                                    pixmap.info().refColorSpace());
    return SkImage::MakeFromGenerator(std::move(gen));
}

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
sk_sp<SkImage> SkImage::MakeFromAHardwareBuffer(AHardwareBuffer* graphicBuffer, SkAlphaType at,
                                               sk_sp<SkColorSpace> cs) {
    auto gen = GrAHardwareBufferImageGenerator::Make(graphicBuffer, at, cs);
    return SkImage::MakeFromGenerator(std::move(gen));
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage::MakeBackendTextureFromSkImage(GrContext* ctx,
                                            sk_sp<SkImage> image,
                                            GrBackendTexture* backendTexture,
                                            BackendTextureReleaseProc* releaseProc) {
    if (!image || !ctx || !backendTexture || !releaseProc) {
        return false;
    }

    // Ensure we have a texture backed image.
    if (!image->isTextureBacked()) {
        image = image->makeTextureImage(ctx, nullptr);
        if (!image) {
            return false;
        }
    }
    GrTexture* texture = image->getTexture();
    if (!texture) {
        // In context-loss cases, we may not have a texture.
        return false;
    }

    // If the image's context doesn't match the provided context, fail.
    if (texture->getContext() != ctx) {
        return false;
    }

    // Flush any pending IO on the texture.
    ctx->contextPriv().prepareSurfaceForExternalIO(as_IB(image)->peekProxy());
    SkASSERT(!texture->surfacePriv().hasPendingIO());

    // We must make a copy of the image if the image is not unique, if the GrTexture owned by the
    // image is not unique, or if the texture wraps an external object.
    if (!image->unique() || !texture->surfacePriv().hasUniqueRef() ||
        texture->resourcePriv().refsWrappedObjects()) {
        // onMakeSubset will always copy the image.
        image = as_IB(image)->onMakeSubset(image->bounds());
        if (!image) {
            return false;
        }

        texture = image->getTexture();
        if (!texture) {
            return false;
        }

        // Flush to ensure that the copy is completed before we return the texture.
        ctx->contextPriv().prepareSurfaceForExternalIO(as_IB(image)->peekProxy());
        SkASSERT(!texture->surfacePriv().hasPendingIO());
    }

    SkASSERT(!texture->resourcePriv().refsWrappedObjects());
    SkASSERT(texture->surfacePriv().hasUniqueRef());
    SkASSERT(image->unique());

    // Take a reference to the GrTexture and release the image.
    sk_sp<GrTexture> textureRef(SkSafeRef(texture));
    image = nullptr;

    // Steal the backend texture from the GrTexture, releasing the GrTexture in the process.
    return GrTexture::StealBackendTexture(std::move(textureRef), backendTexture, releaseProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Gpu::onMakeColorSpace(sk_sp<SkColorSpace> target, SkColorType,
                                             SkTransferFunctionBehavior premulBehavior) const {
    if (SkTransferFunctionBehavior::kRespect == premulBehavior) {
        // TODO: Implement this.
        return nullptr;
    }

    sk_sp<SkColorSpace> srcSpace = fColorSpace;
    if (!fColorSpace) {
        if (target->isSRGB()) {
            return sk_ref_sp(const_cast<SkImage*>((SkImage*)this));
        }

        srcSpace = SkColorSpace::MakeSRGB();
    }

    auto xform = GrNonlinearColorSpaceXformEffect::Make(srcSpace.get(), target.get());
    if (!xform) {
        return sk_ref_sp(const_cast<SkImage_Gpu*>(this));
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
            fContext->contextPriv().makeDeferredRenderTargetContext(
                                            SkBackingFit::kExact, this->width(), this->height(),
                                            kRGBA_8888_GrPixelConfig, nullptr));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(fProxy, SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   fAlphaType, renderTargetContext->asTextureProxyRef(),
                                   std::move(target), fBudgeted);

}

bool SkImage_Gpu::onIsValid(GrContext* context) const {
    // The base class has already checked that context isn't abandoned (if it's not nullptr)
    if (fContext->contextPriv().abandoned()) {
        return false;
    }

    if (context && context != fContext) {
        return false;
    }

    return true;
}
