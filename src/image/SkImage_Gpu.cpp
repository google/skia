/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "GrAHardwareBufferImageGenerator.h"
#include "GrAHardwareBufferUtils.h"
#include "GrBackendSurface.h"
#include "GrBackendTextureImageGenerator.h"
#include "GrBitmapTextureMaker.h"
#include "GrCaps.h"
#include "GrClip.h"
#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrGpu.h"
#include "GrImageTextureMaker.h"
#include "GrProxyProvider.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrResourceProviderPriv.h"
#include "GrSemaphore.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "GrTextureContext.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "SkAutoPixmapStorage.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkImageInfoPriv.h"
#include "SkImage_Gpu.h"
#include "SkMipMap.h"
#include "SkScopeExit.h"
#include "SkTraceEvent.h"
#include "effects/GrYUVtoRGBEffect.h"
#include "gl/GrGLTexture.h"

static SkColorType proxy_color_type(GrTextureProxy* proxy) {
    SkColorType colorType;
    if (!GrPixelConfigToColorType(proxy->config(), &colorType)) {
        colorType = kUnknown_SkColorType;
    }
    return colorType;
}

SkImage_Gpu::SkImage_Gpu(sk_sp<GrContext> context, uint32_t uniqueID, SkAlphaType at,
                         sk_sp<GrTextureProxy> proxy, sk_sp<SkColorSpace> colorSpace)
        : INHERITED(std::move(context), proxy->worstCaseWidth(), proxy->worstCaseHeight(), uniqueID,
                    proxy_color_type(proxy.get()), at, colorSpace)
        , fProxy(std::move(proxy)) {}

SkImage_Gpu::~SkImage_Gpu() {}

sk_sp<SkImage> SkImage_Gpu::onMakeColorTypeAndColorSpace(GrRecordingContext* context,
                                                         SkColorType targetCT,
                                                         sk_sp<SkColorSpace> targetCS) const {
    if (!context || !fContext->priv().matches(context)) {
        return nullptr;
    }

    auto xform = GrColorSpaceXformEffect::Make(this->colorSpace(), this->alphaType(),
                                               targetCS.get(), this->alphaType());
    SkASSERT(xform || targetCT != this->colorType());

    sk_sp<GrTextureProxy> proxy = this->asTextureProxyRef(context);

    GrBackendFormat format = proxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        context->priv().makeDeferredRenderTargetContextWithFallback(
            format, SkBackingFit::kExact, this->width(), this->height(),
            SkColorType2GrPixelConfig(targetCT), nullptr));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(std::move(proxy), SkMatrix::I());
    if (xform) {
        paint.addColorFragmentProcessor(std::move(xform));
    }

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                                  SkRect::MakeIWH(this->width(), this->height()));
    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, this->alphaType(),
                                   renderTargetContext->asTextureProxyRef(), std::move(targetCS));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx,
                                                 const GrBackendTexture& backendTex,
                                                 GrSurfaceOrigin origin,
                                                 SkAlphaType at, sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 SkImage::TextureReleaseProc releaseProc,
                                                 SkImage::ReleaseContext releaseCtx) {
    if (!backendTex.isValid() || backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy =
            proxyProvider->wrapBackendTexture(backendTex, origin, ownership, GrWrapCacheable::kNo,
                                              kRead_GrIOType, releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID, at, std::move(proxy),
                                   std::move(colorSpace));
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    if (!ctx) {
        return nullptr;
    }
    GrBackendTexture texCopy = tex;
    if (!SkImage_GpuBase::ValidateBackendTexture(ctx, texCopy, &texCopy.fConfig, ct, at, cs)) {
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, texCopy, origin, at, std::move(cs),
                                      kBorrow_GrWrapOwnership, releaseP, releaseC);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx,
                                               const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                               SkColorType ct, SkAlphaType at,
                                               sk_sp<SkColorSpace> cs) {
    if (!ctx || !ctx->priv().resourceProvider()) {
        // We have a DDL context and we don't support adopted textures for them.
        return nullptr;
    }
    GrBackendTexture texCopy = tex;
    if (!SkImage_GpuBase::ValidateBackendTexture(ctx, texCopy, &texCopy.fConfig, ct, at, cs)) {
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, texCopy, origin, at, std::move(cs),
                                      kAdopt_GrWrapOwnership, nullptr, nullptr);
}

sk_sp<SkImage> SkImage::MakeFromCompressed(GrContext* context, sk_sp<SkData> data,
                                           int width, int height, CompressionType type) {
    // create the backing texture
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = width;
    desc.fHeight = height;
    switch (type) {
        case kETC1_CompressionType:
            desc.fConfig = kRGB_ETC1_GrPixelConfig;
            break;
        default:
            desc.fConfig = kUnknown_GrPixelConfig;
            break;
    }
    desc.fSampleCnt = 1;

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(std::move(data), desc);

    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, kOpaque_SkAlphaType,
                                   std::move(proxy), nullptr);
}

sk_sp<SkImage> SkImage_Gpu::ConvertYUVATexturesToRGB(GrContext* ctx, SkYUVColorSpace yuvColorSpace,
                                                     const GrBackendTexture yuvaTextures[],
                                                     const SkYUVAIndex yuvaIndices[4], SkISize size,
                                                     GrSurfaceOrigin origin,
                                                     GrRenderTargetContext* renderTargetContext) {
    SkASSERT(renderTargetContext);

    int numTextures;
    if (!SkYUVAIndex::AreValidIndices(yuvaIndices, &numTextures)) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> tempTextureProxies[4];
    if (!SkImage_GpuBase::MakeTempTextureProxies(ctx, yuvaTextures, numTextures, yuvaIndices,
                                                 origin, tempTextureProxies)) {
        return nullptr;
    }

    const SkRect rect = SkRect::MakeIWH(size.width(), size.height());
    if (!RenderYUVAToRGBA(ctx, renderTargetContext, rect, yuvColorSpace, nullptr,
                          tempTextureProxies, yuvaIndices)) {
        return nullptr;
    }

    SkAlphaType at = GetAlphaTypeFromYUVAIndices(yuvaIndices);
    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID, at,
                                   renderTargetContext->asTextureProxyRef(),
                                   renderTargetContext->colorSpaceInfo().refColorSpace());
}

sk_sp<SkImage> SkImage::MakeFromYUVATexturesCopy(GrContext* ctx,
                                                 SkYUVColorSpace yuvColorSpace,
                                                 const GrBackendTexture yuvaTextures[],
                                                 const SkYUVAIndex yuvaIndices[4],
                                                 SkISize imageSize,
                                                 GrSurfaceOrigin imageOrigin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    const int width = imageSize.width();
    const int height = imageSize.height();

    const GrBackendFormat format =
            ctx->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    // Needs to create a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(
            ctx->priv().makeDeferredRenderTargetContext(
                    format, SkBackingFit::kExact, width, height, kRGBA_8888_GrPixelConfig,
                    std::move(imageColorSpace), 1, GrMipMapped::kNo, imageOrigin));
    if (!renderTargetContext) {
        return nullptr;
    }

    return SkImage_Gpu::ConvertYUVATexturesToRGB(ctx, yuvColorSpace, yuvaTextures, yuvaIndices,
                                                 imageSize, imageOrigin, renderTargetContext.get());
}

sk_sp<SkImage> SkImage::MakeFromYUVATexturesCopyWithExternalBackend(
        GrContext* ctx,
        SkYUVColorSpace yuvColorSpace,
        const GrBackendTexture yuvaTextures[],
        const SkYUVAIndex yuvaIndices[4],
        SkISize imageSize,
        GrSurfaceOrigin imageOrigin,
        const GrBackendTexture& backendTexture,
        sk_sp<SkColorSpace> imageColorSpace) {
    GrBackendTexture backendTextureCopy = backendTexture;

    SkAlphaType at = SkImage_GpuBase::GetAlphaTypeFromYUVAIndices(yuvaIndices);
    if (!SkImage_Gpu::ValidateBackendTexture(ctx, backendTextureCopy, &backendTextureCopy.fConfig,
                                             kRGBA_8888_SkColorType, at, nullptr)) {
        return nullptr;
    }

    // Needs to create a render target with external texture
    // in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(
            ctx->priv().makeBackendTextureRenderTargetContext(backendTextureCopy,
                                                              imageOrigin, 1,
                                                              std::move(imageColorSpace)));

    if (!renderTargetContext) {
        return nullptr;
    }

    return SkImage_Gpu::ConvertYUVATexturesToRGB(ctx, yuvColorSpace, yuvaTextures, yuvaIndices,
                                                 imageSize, imageOrigin, renderTargetContext.get());
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopy(GrContext* ctx, SkYUVColorSpace yuvColorSpace,
                                                const GrBackendTexture yuvTextures[3],
                                                GrSurfaceOrigin imageOrigin,
                                                sk_sp<SkColorSpace> imageColorSpace) {
    // TODO: SkImageSourceChannel input is being ingored right now. Setup correctly in the future.
    SkYUVAIndex yuvaIndices[4] = {
            SkYUVAIndex{0, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kR},
            SkYUVAIndex{2, SkColorChannel::kR},
            SkYUVAIndex{-1, SkColorChannel::kA}};
    SkISize size{yuvTextures[0].width(), yuvTextures[0].height()};
    return SkImage_Gpu::MakeFromYUVATexturesCopy(ctx, yuvColorSpace, yuvTextures, yuvaIndices,
                                                 size, imageOrigin, std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopyWithExternalBackend(
        GrContext* ctx, SkYUVColorSpace yuvColorSpace, const GrBackendTexture yuvTextures[3],
        GrSurfaceOrigin imageOrigin, const GrBackendTexture& backendTexture,
        sk_sp<SkColorSpace> imageColorSpace) {
    SkYUVAIndex yuvaIndices[4] = {
            SkYUVAIndex{0, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kR},
            SkYUVAIndex{2, SkColorChannel::kR},
            SkYUVAIndex{-1, SkColorChannel::kA}};
    SkISize size{yuvTextures[0].width(), yuvTextures[0].height()};
    return SkImage_Gpu::MakeFromYUVATexturesCopyWithExternalBackend(
            ctx, yuvColorSpace, yuvTextures, yuvaIndices, size, imageOrigin, backendTexture,
            std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromNV12TexturesCopy(GrContext* ctx, SkYUVColorSpace yuvColorSpace,
                                                 const GrBackendTexture nv12Textures[2],
                                                 GrSurfaceOrigin imageOrigin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    // TODO: SkImageSourceChannel input is being ingored right now. Setup correctly in the future.
    SkYUVAIndex yuvaIndices[4] = {
            SkYUVAIndex{0, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kG},
            SkYUVAIndex{-1, SkColorChannel::kA}};
    SkISize size{nv12Textures[0].width(), nv12Textures[0].height()};
    return SkImage_Gpu::MakeFromYUVATexturesCopy(ctx, yuvColorSpace, nv12Textures, yuvaIndices,
                                                 size, imageOrigin, std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromNV12TexturesCopyWithExternalBackend(
        GrContext* ctx,
        SkYUVColorSpace yuvColorSpace,
        const GrBackendTexture nv12Textures[2],
        GrSurfaceOrigin imageOrigin,
        const GrBackendTexture& backendTexture,
        sk_sp<SkColorSpace> imageColorSpace) {
    SkYUVAIndex yuvaIndices[4] = {
            SkYUVAIndex{0, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kG},
            SkYUVAIndex{-1, SkColorChannel::kA}};
    SkISize size{nv12Textures[0].width(), nv12Textures[0].height()};
    return SkImage_Gpu::MakeFromYUVATexturesCopyWithExternalBackend(
            ctx, yuvColorSpace, nv12Textures, yuvaIndices, size, imageOrigin, backendTexture,
            std::move(imageColorSpace));
}

static sk_sp<SkImage> create_image_from_producer(GrContext* context, GrTextureProducer* producer,
                                                 SkAlphaType at, uint32_t id,
                                                 GrMipMapped mipMapped) {
    sk_sp<GrTextureProxy> proxy(producer->refTextureProxy(mipMapped));
    if (!proxy) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), id, at, std::move(proxy),
                                   sk_ref_sp(producer->colorSpace()));
}

sk_sp<SkImage> SkImage::makeTextureImage(GrContext* context, SkColorSpace* dstColorSpace,
                                         GrMipMapped mipMapped) const {
    if (!context) {
        return nullptr;
    }

    if (this->isTextureBacked()) {
        if (!as_IB(this)->context()->priv().matches(context)) {
            return nullptr;
        }

        sk_sp<GrTextureProxy> proxy = as_IB(this)->asTextureProxyRef(context);
        SkASSERT(proxy);
        if (GrMipMapped::kNo == mipMapped || proxy->mipMapped() == mipMapped) {
            return sk_ref_sp(const_cast<SkImage*>(this));
        }
        GrTextureAdjuster adjuster(context, std::move(proxy), this->alphaType(),
                                   this->uniqueID(), this->colorSpace());
        return create_image_from_producer(context, &adjuster, this->alphaType(),
                                          this->uniqueID(), mipMapped);
    }

    if (this->isLazyGenerated()) {
        GrImageTextureMaker maker(context, this, kDisallow_CachingHint);
        return create_image_from_producer(context, &maker, this->alphaType(),
                                          this->uniqueID(), mipMapped);
    }

    if (const SkBitmap* bmp = as_IB(this)->onPeekBitmap()) {
        GrBitmapTextureMaker maker(context, *bmp);
        return create_image_from_producer(context, &maker, this->alphaType(),
                                          this->uniqueID(), mipMapped);
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Gpu::MakePromiseTexture(GrContext* context,
                                               const GrBackendFormat& backendFormat,
                                               int width,
                                               int height,
                                               GrMipMapped mipMapped,
                                               GrSurfaceOrigin origin,
                                               SkColorType colorType,
                                               SkAlphaType alphaType,
                                               sk_sp<SkColorSpace> colorSpace,
                                               PromiseImageTextureFulfillProc textureFulfillProc,
                                               PromiseImageTextureReleaseProc textureReleaseProc,
                                               PromiseImageTextureDoneProc textureDoneProc,
                                               PromiseImageTextureContext textureContext,
                                               PromiseImageApiVersion version) {
    // The contract here is that if 'promiseDoneProc' is passed in it should always be called,
    // even if creation of the SkImage fails. Once we call MakePromiseImageLazyProxy it takes
    // responsibility for calling the done proc.
    if (!textureDoneProc) {
        return nullptr;
    }
    SkScopeExit callDone([textureDoneProc, textureContext]() { textureDoneProc(textureContext); });

    SkImageInfo info = SkImageInfo::Make(width, height, colorType, alphaType, colorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    if (!context) {
        return nullptr;
    }

    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    GrPixelConfig config =
            context->priv().caps()->getConfigFromBackendFormat(backendFormat, colorType);
    if (config == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    callDone.clear();
    auto proxy = MakePromiseImageLazyProxy(context, width, height, origin, config, backendFormat,
                                           mipMapped, textureFulfillProc, textureReleaseProc,
                                           textureDoneProc, textureContext, version);
    if (!proxy) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, alphaType,
                                   std::move(proxy), std::move(colorSpace));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeCrossContextFromEncoded(GrContext* context, sk_sp<SkData> encoded,
                                                    bool buildMips, SkColorSpace* dstColorSpace,
                                                    bool limitToMaxTextureSize) {
    sk_sp<SkImage> codecImage = SkImage::MakeFromEncoded(std::move(encoded));
    if (!codecImage) {
        return nullptr;
    }

    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!context || !context->priv().caps()->crossContextTextureSupport()) {
        return codecImage;
    }

    // If non-power-of-two mipmapping isn't supported, ignore the client's request
    if (!context->priv().caps()->mipMapSupport()) {
        buildMips = false;
    }

    auto maxTextureSize = context->priv().caps()->maxTextureSize();
    if (limitToMaxTextureSize &&
        (codecImage->width() > maxTextureSize || codecImage->height() > maxTextureSize)) {
        SkAutoPixmapStorage pmap;
        SkImageInfo info = codecImage->imageInfo();
        if (!dstColorSpace) {
            info = info.makeColorSpace(nullptr);
        }
        if (!pmap.tryAlloc(info) || !codecImage->readPixels(pmap, 0, 0, kDisallow_CachingHint)) {
            return nullptr;
        }
        return MakeCrossContextFromPixmap(context, pmap, buildMips, dstColorSpace, true);
    }

    // Turn the codec image into a GrTextureProxy
    GrImageTextureMaker maker(context, codecImage.get(), kDisallow_CachingHint);
    GrSamplerState samplerState(
            GrSamplerState::WrapMode::kClamp,
            buildMips ? GrSamplerState::Filter::kMipMap : GrSamplerState::Filter::kBilerp);
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(maker.refTextureProxyForParams(samplerState, scaleAdjust));
    // Given that we disable mipmaps if non-power-of-two mipmapping isn't supported, we always
    // expect the created texture to be unscaled.
    SkASSERT(scaleAdjust[0] == 1.0f && scaleAdjust[1] == 1.0f);
    if (!proxy) {
        return codecImage;
    }

    // Flush any writes or uploads
    context->priv().prepareSurfaceForExternalIO(proxy.get());
    if (!proxy->isInstantiated()) {
        return codecImage;
    }

    sk_sp<GrTexture> texture = sk_ref_sp(proxy->peekTexture());

    GrGpu* gpu = context->priv().getGpu();
    sk_sp<GrSemaphore> sema = gpu->prepareTextureForCrossContextUsage(texture.get());

    auto gen = GrBackendTextureImageGenerator::Make(
            std::move(texture), proxy->origin(), std::move(sema), codecImage->colorType(),
            codecImage->alphaType(), codecImage->refColorSpace());
    return SkImage::MakeFromGenerator(std::move(gen));
}

sk_sp<SkImage> SkImage::MakeCrossContextFromPixmap(GrContext* context,
                                                   const SkPixmap& originalPixmap, bool buildMips,
                                                   SkColorSpace* dstColorSpace,
                                                   bool limitToMaxTextureSize) {
    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!context || !context->priv().caps()->crossContextTextureSupport()) {
        return SkImage::MakeRasterCopy(originalPixmap);
    }

    // If we don't have access to the resource provider and gpu (i.e. in a DDL context) we will not
    // be able to make everything needed for a GPU CrossContext image. Thus return a raster copy
    // instead.
    if (!context->priv().resourceProvider()) {
        return SkImage::MakeRasterCopy(originalPixmap);
    }

    // If non-power-of-two mipmapping isn't supported, ignore the client's request
    if (!context->priv().caps()->mipMapSupport()) {
        buildMips = false;
    }

    const SkPixmap* pixmap = &originalPixmap;
    SkAutoPixmapStorage resized;
    int maxTextureSize = context->priv().caps()->maxTextureSize();
    int maxDim = SkTMax(originalPixmap.width(), originalPixmap.height());
    if (limitToMaxTextureSize && maxDim > maxTextureSize) {
        float scale = static_cast<float>(maxTextureSize) / maxDim;
        int newWidth = SkTMin(static_cast<int>(originalPixmap.width() * scale), maxTextureSize);
        int newHeight = SkTMin(static_cast<int>(originalPixmap.height() * scale), maxTextureSize);
        SkImageInfo info = originalPixmap.info().makeWH(newWidth, newHeight);
        if (!resized.tryAlloc(info) || !originalPixmap.scalePixels(resized, kLow_SkFilterQuality)) {
            return nullptr;
        }
        pixmap = &resized;
    }
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    // Turn the pixmap into a GrTextureProxy
    SkBitmap bmp;
    bmp.installPixels(*pixmap);
    GrMipMapped mipMapped = buildMips ? GrMipMapped::kYes : GrMipMapped::kNo;
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxyFromBitmap(bmp, mipMapped);
    if (!proxy) {
        return SkImage::MakeRasterCopy(*pixmap);
    }

    sk_sp<GrTexture> texture = sk_ref_sp(proxy->peekTexture());

    // Flush any writes or uploads
    context->priv().prepareSurfaceForExternalIO(proxy.get());
    GrGpu* gpu = context->priv().getGpu();

    sk_sp<GrSemaphore> sema = gpu->prepareTextureForCrossContextUsage(texture.get());

    auto gen = GrBackendTextureImageGenerator::Make(std::move(texture), proxy->origin(),
                                                    std::move(sema), pixmap->colorType(),
                                                    pixmap->alphaType(),
                                                    pixmap->info().refColorSpace());
    return SkImage::MakeFromGenerator(std::move(gen));
}

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
sk_sp<SkImage> SkImage::MakeFromAHardwareBuffer(AHardwareBuffer* graphicBuffer, SkAlphaType at,
                                                sk_sp<SkColorSpace> cs,
                                                GrSurfaceOrigin surfaceOrigin) {
    auto gen = GrAHardwareBufferImageGenerator::Make(graphicBuffer, at, cs, surfaceOrigin);
    return SkImage::MakeFromGenerator(std::move(gen));
}

sk_sp<SkImage> SkImage::MakeFromAHardwareBufferWithData(GrContext* context,
                                                        const SkPixmap& pixmap,
                                                        AHardwareBuffer* hardwareBuffer,
                                                        GrSurfaceOrigin surfaceOrigin) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE)) {
        return nullptr;
    }

    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(context,
                                                                             hardwareBuffer,
                                                                             bufferDesc.format,
                                                                             true);

    if (!backendFormat.isValid()) {
        return nullptr;
    }

    GrAHardwareBufferUtils::DeleteImageProc deleteImageProc = nullptr;
    GrAHardwareBufferUtils::DeleteImageCtx deleteImageCtx = nullptr;

    GrBackendTexture backendTexture =
            GrAHardwareBufferUtils::MakeBackendTexture(context, hardwareBuffer,
                                                       bufferDesc.width, bufferDesc.height,
                                                       &deleteImageProc, &deleteImageCtx,
                                                       false, backendFormat, true);
    if (!backendTexture.isValid()) {
        return nullptr;
    }
    SkASSERT(deleteImageProc);

    SkColorType colorType =
            GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

    backendTexture.fConfig = context->priv().caps()->getConfigFromBackendFormat(backendFormat,
                                                                                colorType);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    if (!proxyProvider) {
        deleteImageProc(deleteImageCtx);
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy =
            proxyProvider->wrapBackendTexture(backendTexture, surfaceOrigin,
                                              kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                                              kRW_GrIOType, deleteImageProc, deleteImageCtx);
    if (!proxy) {
        deleteImageProc(deleteImageCtx);
        return nullptr;
    }

    sk_sp<SkColorSpace> cs = pixmap.refColorSpace();
    SkAlphaType at =  pixmap.alphaType();

    sk_sp<SkImage> image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, at,
                                                   proxy, cs);
    if (!image) {
        return nullptr;
    }

    GrDrawingManager* drawingManager = context->priv().drawingManager();
    if (!drawingManager) {
        return nullptr;
    }

    sk_sp<GrTextureContext> texContext = drawingManager->makeTextureContext(proxy, cs);
    if (!texContext) {
        return nullptr;
    }

    SkImageInfo srcInfo = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType, at,
                                            std::move(cs));
    texContext->writePixels(srcInfo, pixmap.addr(0, 0), pixmap.rowBytes(), 0, 0);

    drawingManager->flush(proxy.get(), SkSurface::BackendSurfaceAccess::kNoAccess,
                          kSyncCpu_GrFlushFlag, 0, nullptr, nullptr, nullptr);

    return image;
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
    ctx->priv().prepareSurfaceForExternalIO(as_IB(image)->peekProxy());
    SkASSERT(!texture->surfacePriv().hasPendingIO());

    // We must make a copy of the image if the image is not unique, if the GrTexture owned by the
    // image is not unique, or if the texture wraps an external object.
    if (!image->unique() || !texture->surfacePriv().hasUniqueRef() ||
        texture->resourcePriv().refsWrappedObjects()) {
        // onMakeSubset will always copy the image.
        image = as_IB(image)->onMakeSubset(ctx, image->bounds());
        if (!image) {
            return false;
        }

        texture = image->getTexture();
        if (!texture) {
            return false;
        }

        // Flush to ensure that the copy is completed before we return the texture.
        ctx->priv().prepareSurfaceForExternalIO(as_IB(image)->peekProxy());
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
