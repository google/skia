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
    SkColorType ct;
    if (!GrPixelConfigToColorType(fProxy->config(), &ct)) {
        ct = kUnknown_SkColorType;
    }
    return SkImageInfo::Make(fProxy->width(), fProxy->height(), ct, fAlphaType, fColorSpace);
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

    GrTextureAdjuster adjuster(fContext, fProxy, this->alphaType(), this->uniqueID(),
                               this->fColorSpace.get());
    return adjuster.refTextureProxyForParams(params, dstColorSpace, texColorSpace, scaleAdjust);
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
    desc.fOrigin = fProxy->origin();
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = fProxy->config();

    sk_sp<GrSurfaceContext> sContext(fContext->contextPriv().makeDeferredSurfaceContext(
                                                                        desc,
                                                                        GrMipMapped::kNo,
                                                                        SkBackingFit::kExact,
                                                                        fBudgeted));
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
    sk_sp<GrTextureProxy> proxy = proxyProvider->createWrappedTextureProxy(
                                        backendTex, origin, ownership, releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(ctx, kNeedNewImageUniqueID,
                                   at, std::move(proxy), std::move(colorSpace), SkBudgeted::kNo);
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    if (!ctx) {
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, tex, origin, at, std::move(cs), kBorrow_GrWrapOwnership,
                                      releaseP, releaseC);
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
    return MakeFromTexture(ctx, texCopy, origin, at, cs, releaseP, releaseC);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx,
                                               const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                               SkAlphaType at, sk_sp<SkColorSpace> cs) {
    if (!ctx->contextPriv().resourceProvider()) {
        // We have a DDL context and we don't support adopted textures for them.
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, tex, origin, at, std::move(cs), kAdopt_GrWrapOwnership,
                                      nullptr, nullptr);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx,
                                               const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                               SkColorType ct, SkAlphaType at,
                                               sk_sp<SkColorSpace> cs) {
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(ctx, texCopy, &texCopy.fConfig, ct, at, cs)) {
        return nullptr;
    }
    return MakeFromAdoptedTexture(ctx, texCopy, origin, at, cs);
}

static GrBackendTexture make_backend_texture_from_handle(GrBackend backend,
                                                         int width, int height,
                                                         GrPixelConfig config,
                                                         GrBackendObject handle) {
    switch (backend) {
        case kOpenGL_GrBackend: {
            const GrGLTextureInfo* glInfo = (const GrGLTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *glInfo);
        }
#ifdef SK_VULKAN
        case kVulkan_GrBackend: {
            const GrVkImageInfo* vkInfo = (const GrVkImageInfo*)(handle);
            return GrBackendTexture(width, height, *vkInfo);
        }
#endif
        case kMock_GrBackend: {
            const GrMockTextureInfo* mockInfo = (const GrMockTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *mockInfo);
        }
        default:
            return GrBackendTexture();
    }
}

static bool are_yuv_sizes_valid(const SkISize yuvSizes[], bool nv12) {
    if (yuvSizes[0].fWidth <= 0 || yuvSizes[0].fHeight <= 0 ||
        yuvSizes[1].fWidth <= 0 || yuvSizes[1].fHeight <= 0) {
        return false;
    }
    if (!nv12 && (yuvSizes[2].fWidth <= 0 || yuvSizes[2].fHeight <= 0)) {
        return false;
    }

    return true;
}

static sk_sp<SkImage> make_from_yuv_textures_copy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                  bool nv12,
                                                  const GrBackendTexture yuvBackendTextures[],
                                                  const SkISize yuvSizes[],
                                                  GrSurfaceOrigin origin,
                                                  sk_sp<SkColorSpace> imageColorSpace) {
    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();

    if (!are_yuv_sizes_valid(yuvSizes, nv12)) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> yProxy = proxyProvider->createWrappedTextureProxy(yuvBackendTextures[0],
                                                                            origin);
    sk_sp<GrTextureProxy> uProxy = proxyProvider->createWrappedTextureProxy(yuvBackendTextures[1],
                                                                            origin);
    sk_sp<GrTextureProxy> vProxy;

    if (nv12) {
        vProxy = uProxy;
    } else {
        vProxy = proxyProvider->createWrappedTextureProxy(yuvBackendTextures[2], origin);
    }
    if (!yProxy || !uProxy || !vProxy) {
        return nullptr;
    }

    const int width = yuvSizes[0].fWidth;
    const int height = yuvSizes[0].fHeight;

    // Needs to be a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(ctx->makeDeferredRenderTargetContext(
            SkBackingFit::kExact, width, height, kRGBA_8888_GrPixelConfig,
            std::move(imageColorSpace), 1, GrMipMapped::kNo, origin));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorFragmentProcessor(GrYUVtoRGBEffect::Make(yProxy, uProxy, vProxy,
                                                           yuvSizes, colorSpace, nv12));

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

static sk_sp<SkImage> make_from_yuv_objects_copy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                 bool nv12,
                                                 const GrBackendObject yuvTextureHandles[],
                                                 const SkISize yuvSizes[],
                                                 GrSurfaceOrigin origin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    if (!are_yuv_sizes_valid(yuvSizes, nv12)) {
        return nullptr;
    }

    GrBackendTexture backendTextures[3];

    const GrPixelConfig kConfig = nv12 ? kRGBA_8888_GrPixelConfig : kAlpha_8_GrPixelConfig;

    GrBackend backend = ctx->contextPriv().getBackend();
    backendTextures[0] = make_backend_texture_from_handle(backend,
                                                          yuvSizes[0].fWidth,
                                                          yuvSizes[0].fHeight,
                                                          kConfig,
                                                          yuvTextureHandles[0]);
    backendTextures[1] = make_backend_texture_from_handle(backend,
                                                          yuvSizes[1].fWidth,
                                                          yuvSizes[1].fHeight,
                                                          kConfig,
                                                          yuvTextureHandles[1]);

    if (!nv12) {
        backendTextures[2] = make_backend_texture_from_handle(backend,
                                                              yuvSizes[2].fWidth,
                                                              yuvSizes[2].fHeight,
                                                              kConfig,
                                                              yuvTextureHandles[2]);
    }

    return make_from_yuv_textures_copy(ctx, colorSpace, nv12,
                                       backendTextures, yuvSizes, origin,
                                       std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                const GrBackendObject yuvTextureHandles[3],
                                                const SkISize yuvSizes[3], GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> imageColorSpace) {
    return make_from_yuv_objects_copy(ctx, colorSpace, false, yuvTextureHandles, yuvSizes, origin,
                                      std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromNV12TexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                 const GrBackendObject yuvTextureHandles[2],
                                                 const SkISize yuvSizes[2],
                                                 GrSurfaceOrigin origin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    return make_from_yuv_objects_copy(ctx, colorSpace, true, yuvTextureHandles, yuvSizes, origin,
                                      std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                const GrBackendTexture yuvBackendTextures[3],
                                                const SkISize yuvSizes[3], GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> imageColorSpace) {
    return make_from_yuv_textures_copy(ctx, colorSpace, false, yuvBackendTextures, yuvSizes, origin,
                                       std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromNV12TexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                 const GrBackendTexture yuvBackendTextures[2],
                                                 const SkISize yuvSizes[2],
                                                 GrSurfaceOrigin origin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    return make_from_yuv_textures_copy(ctx, colorSpace, true, yuvBackendTextures, yuvSizes, origin,
                                       std::move(imageColorSpace));
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
                                                    std::move(sema), codecImage->alphaType(),
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
            GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(pixmap.info(), *proxyProvider->caps());
            proxy = proxyProvider->createTextureProxy(desc, SkBudgeted::kYes, pixmap.addr(),
                                                      pixmap.rowBytes());
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
                                                    std::move(sema), pixmap.alphaType(),
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

    sk_sp<GrRenderTargetContext> renderTargetContext(fContext->makeDeferredRenderTargetContext(
        SkBackingFit::kExact, this->width(), this->height(), kRGBA_8888_GrPixelConfig, nullptr));
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
    if (fContext->abandoned()) {
        return false;
    }

    if (context && context != fContext) {
        return false;
    }

    return true;
}
