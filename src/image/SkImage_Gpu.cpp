/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "include/core/SkCanvas.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkScopeExit.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAHardwareBufferImageGenerator.h"
#include "src/gpu/GrAHardwareBufferUtils.h"
#include "src/gpu/GrBackendTextureImageGenerator.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrImageTextureMaker.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/gl/GrGLTexture.h"
#include "src/image/SkImage_Gpu.h"

SkImage_Gpu::SkImage_Gpu(sk_sp<GrContext> context, uint32_t uniqueID, GrSurfaceProxyView view,
                         SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> colorSpace)
        : INHERITED(std::move(context), view.proxy()->backingStoreDimensions(), uniqueID,
                    ct, at, colorSpace)
        , fView(std::move(view)) {
#ifdef SK_DEBUG
    const GrBackendFormat& format = fView.proxy()->backendFormat();
    GrColorType grCT = SkColorTypeToGrColorType(ct);
    const GrCaps* caps = this->context()->priv().caps();
    if (caps->isFormatSRGB(format)) {
        SkASSERT(grCT == GrColorType::kRGBA_8888);
        grCT = GrColorType::kRGBA_8888_SRGB;
    }
    SkASSERT(caps->isFormatCompressed(format) ||
             caps->areColorTypeAndFormatCompatible(grCT, format));
#endif
}

SkImage_Gpu::~SkImage_Gpu() {}

GrSemaphoresSubmitted SkImage_Gpu::onFlush(GrContext* context, const GrFlushInfo& info) {
    if (!context || !fContext->priv().matches(context) || fContext->abandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }

    GrSurfaceProxy* p[1] = {fView.proxy()};
    return context->priv().flushSurfaces(p, 1, info);
}

sk_sp<SkImage> SkImage_Gpu::onMakeColorTypeAndColorSpace(GrRecordingContext* context,
                                                         SkColorType targetCT,
                                                         sk_sp<SkColorSpace> targetCS) const {
    if (!context || !fContext->priv().matches(context)) {
        return nullptr;
    }

    auto xform = GrColorSpaceXformEffect::Make(this->colorSpace(), this->alphaType(),
                                               targetCS.get(), this->alphaType());
    SkASSERT(xform || targetCT != this->colorType());

    auto renderTargetContext = GrRenderTargetContext::MakeWithFallback(
            context, SkColorTypeToGrColorType(targetCT), nullptr, SkBackingFit::kExact,
            this->dimensions());
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorFragmentProcessor(GrTextureEffect::Make(*this->view(context), this->alphaType()));
    if (xform) {
        paint.addColorFragmentProcessor(std::move(xform));
    }

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                                  SkRect::MakeIWH(this->width(), this->height()));
    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    targetCT = GrColorTypeToSkColorType(renderTargetContext->colorInfo().colorType());
    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   renderTargetContext->readSurfaceView(), targetCT,
                                   this->alphaType(), std::move(targetCS));
}

sk_sp<SkImage> SkImage_Gpu::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, fView, this->colorType(),
                                   this->alphaType(), std::move(newCS));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx,
                                                 const GrBackendTexture& backendTex,
                                                 GrColorType colorType, GrSurfaceOrigin origin,
                                                 SkAlphaType at, sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 SkImage::TextureReleaseProc releaseProc,
                                                 SkImage::ReleaseContext releaseCtx) {
    if (!backendTex.isValid() || backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTex, ownership, GrWrapCacheable::kNo, kRead_GrIOType, releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }

    GrSwizzle swizzle = ctx->priv().caps()->getReadSwizzle(proxy->backendFormat(), colorType);
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID, std::move(view),
                                   GrColorTypeToSkColorType(colorType), at, std::move(colorSpace));
}

sk_sp<SkImage> SkImage::MakeFromCompressedTexture(GrContext* ctx,
                                                  const GrBackendTexture& tex,
                                                  GrSurfaceOrigin origin,
                                                  SkAlphaType at,
                                                  sk_sp<SkColorSpace> cs,
                                                  TextureReleaseProc releaseP,
                                                  ReleaseContext releaseC) {
    if (!ctx) {
        return nullptr;
    }

    const GrCaps* caps = ctx->priv().caps();

    if (!SkImage_GpuBase::ValidateCompressedBackendTexture(caps, tex, at)) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapCompressedBackendTexture(
            tex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, releaseP, releaseC);
    if (!proxy) {
        return nullptr;
    }

    CompressionType type = caps->compressionType(tex.getBackendFormat());
    SkColorType ct = GrCompressionTypeToSkColorType(type);

    GrSurfaceProxyView view(std::move(proxy), origin, GrSwizzle::RGBA());
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID,  std::move(view), ct, at,
                                   std::move(cs));
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    if (!ctx) {
        return nullptr;
    }

    const GrCaps* caps = ctx->priv().caps();

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(caps, ct, tex.getBackendFormat());
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    if (!SkImage_GpuBase::ValidateBackendTexture(caps, tex, grColorType, ct, at, cs)) {
        return nullptr;
    }

    return new_wrapped_texture_common(ctx, tex, grColorType, origin, at, std::move(cs),
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

    const GrCaps* caps = ctx->priv().caps();

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(caps, ct, tex.getBackendFormat());
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    if (!SkImage_GpuBase::ValidateBackendTexture(caps, tex, grColorType, ct, at, cs)) {
        return nullptr;
    }

    return new_wrapped_texture_common(ctx, tex, grColorType, origin, at, std::move(cs),
                                      kAdopt_GrWrapOwnership, nullptr, nullptr);
}

sk_sp<SkImage> SkImage::MakeTextureFromCompressed(GrContext* context, sk_sp<SkData> data,
                                                  int width, int height, CompressionType type,
                                                  GrMipMapped mipMapped,
                                                  GrProtected isProtected) {
    if (!context || !data) {
        return nullptr;
    }

    GrBackendFormat beFormat = context->compressedBackendFormat(type);
    if (!beFormat.isValid()) {
        sk_sp<SkImage> tmp = MakeRasterFromCompressed(std::move(data), width, height, type);
        if (!tmp) {
            return nullptr;
        }
        return tmp->makeTextureImage(context, mipMapped);
    }

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createCompressedTextureProxy(
            {width, height}, SkBudgeted::kYes, mipMapped, isProtected, type, std::move(data));
    if (!proxy) {
        return nullptr;
    }
    GrSurfaceProxyView view(std::move(proxy));

    SkColorType colorType = GrCompressionTypeToSkColorType(type);

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, std::move(view),
                                   colorType, kOpaque_SkAlphaType, nullptr);
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

    GrSurfaceProxyView tempViews[4];
    if (!SkImage_GpuBase::MakeTempTextureProxies(ctx, yuvaTextures, numTextures, yuvaIndices,
                                                 origin, tempViews)) {
        return nullptr;
    }

    const SkRect rect = SkRect::MakeIWH(size.width(), size.height());
    if (!RenderYUVAToRGBA(ctx, renderTargetContext, rect, yuvColorSpace, nullptr,
                          tempViews, yuvaIndices)) {
        return nullptr;
    }

    SkColorType ct = GrColorTypeToSkColorType(renderTargetContext->colorInfo().colorType());
    SkAlphaType at = GetAlphaTypeFromYUVAIndices(yuvaIndices);
    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID,
                                   renderTargetContext->readSurfaceView(), ct, at,
                                   renderTargetContext->colorInfo().refColorSpace());
}

sk_sp<SkImage> SkImage::MakeFromYUVATexturesCopy(GrContext* ctx,
                                                 SkYUVColorSpace yuvColorSpace,
                                                 const GrBackendTexture yuvaTextures[],
                                                 const SkYUVAIndex yuvaIndices[4],
                                                 SkISize imageSize,
                                                 GrSurfaceOrigin imageOrigin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    auto renderTargetContext = GrRenderTargetContext::Make(
            ctx, GrColorType::kRGBA_8888, std::move(imageColorSpace), SkBackingFit::kExact,
            imageSize, 1, GrMipMapped::kNo, GrProtected::kNo, imageOrigin);
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
        sk_sp<SkColorSpace> imageColorSpace,
        TextureReleaseProc textureReleaseProc,
        ReleaseContext releaseContext) {
    const GrCaps* caps = ctx->priv().caps();

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(caps, kRGBA_8888_SkColorType,
                                                                backendTexture.getBackendFormat());
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    SkAlphaType at = SkImage_GpuBase::GetAlphaTypeFromYUVAIndices(yuvaIndices);
    if (!SkImage_Gpu::ValidateBackendTexture(caps, backendTexture, grColorType,
                                             kRGBA_8888_SkColorType, at, nullptr)) {
        return nullptr;
    }

    // Needs to create a render target with external texture
    // in order to draw to it for the yuv->rgb conversion.
    auto renderTargetContext = GrRenderTargetContext::MakeFromBackendTexture(
            ctx, grColorType, std::move(imageColorSpace), backendTexture, 1, imageOrigin,
            nullptr, textureReleaseProc, releaseContext);
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
            std::move(imageColorSpace), nullptr, nullptr);
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
        sk_sp<SkColorSpace> imageColorSpace,
        TextureReleaseProc textureReleaseProc,
        ReleaseContext releaseContext) {
    SkYUVAIndex yuvaIndices[4] = {
            SkYUVAIndex{0, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kG},
            SkYUVAIndex{-1, SkColorChannel::kA}};
    SkISize size{nv12Textures[0].width(), nv12Textures[0].height()};
    return SkImage_Gpu::MakeFromYUVATexturesCopyWithExternalBackend(
            ctx, yuvColorSpace, nv12Textures, yuvaIndices, size, imageOrigin, backendTexture,
            std::move(imageColorSpace), textureReleaseProc, releaseContext);
}

static sk_sp<SkImage> create_image_from_producer(GrContext* context, GrTextureProducer* producer,
                                                 uint32_t id, GrMipMapped mipMapped) {
    auto view = producer->view(mipMapped);
    if (!view) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), id, std::move(view),
                                   GrColorTypeToSkColorType(producer->colorType()),
                                   producer->alphaType(), sk_ref_sp(producer->colorSpace()));
}

sk_sp<SkImage> SkImage::makeTextureImage(GrContext* context,
                                         GrMipMapped mipMapped,
                                         SkBudgeted budgeted) const {
    if (!context) {
        return nullptr;
    }

    if (this->isTextureBacked()) {
        if (!as_IB(this)->context()->priv().matches(context)) {
            return nullptr;
        }

        // TODO: Don't flatten YUVA images here.
        const GrSurfaceProxyView* view = as_IB(this)->view(context);
        SkASSERT(view && view->asTextureProxy());

        if (mipMapped == GrMipMapped::kNo || view->asTextureProxy()->mipMapped() == mipMapped ||
            !context->priv().caps()->mipMapSupport()) {
            return sk_ref_sp(const_cast<SkImage*>(this));
        }
        auto copy = GrCopyBaseMipMapToTextureProxy(context->priv().asRecordingContext(),
                                                   view->proxy(),
                                                   view->origin(),
                                                   SkColorTypeToGrColorType(this->colorType()),
                                                   budgeted);
        if (!copy) {
            return nullptr;
        }
        return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), this->uniqueID(), std::move(copy),
                                       this->colorType(), this->alphaType(), this->refColorSpace());
    }

    auto policy = budgeted == SkBudgeted::kYes ? GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                               : GrImageTexGenPolicy::kNew_Uncached_Unbudgeted;
    if (this->isLazyGenerated()) {
        GrImageTextureMaker maker(context, this, policy);
        return create_image_from_producer(context, &maker, this->uniqueID(), mipMapped);
    }

    if (const SkBitmap* bmp = as_IB(this)->onPeekBitmap()) {
        GrBitmapTextureMaker maker(context, *bmp, policy);
        return create_image_from_producer(context, &maker, this->uniqueID(), mipMapped);
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

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(context->priv().caps(),
                                                                colorType,
                                                                backendFormat);
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    callDone.clear();
    auto proxy = MakePromiseImageLazyProxy(context, width, height, grColorType, backendFormat,
                                           mipMapped, textureFulfillProc, textureReleaseProc,
                                           textureDoneProc, textureContext, version);
    if (!proxy) {
        return nullptr;
    }
    GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, std::move(view),
                                   colorType, alphaType, std::move(colorSpace));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeCrossContextFromPixmap(GrContext* context,
                                                   const SkPixmap& originalPixmap, bool buildMips,
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
    int maxDim = std::max(originalPixmap.width(), originalPixmap.height());
    if (limitToMaxTextureSize && maxDim > maxTextureSize) {
        float scale = static_cast<float>(maxTextureSize) / maxDim;
        int newWidth = std::min(static_cast<int>(originalPixmap.width() * scale), maxTextureSize);
        int newHeight = std::min(static_cast<int>(originalPixmap.height() * scale), maxTextureSize);
        SkImageInfo info = originalPixmap.info().makeWH(newWidth, newHeight);
        if (!resized.tryAlloc(info) || !originalPixmap.scalePixels(resized, kLow_SkFilterQuality)) {
            return nullptr;
        }
        pixmap = &resized;
    }
    // Turn the pixmap into a GrTextureProxy
    SkBitmap bmp;
    bmp.installPixels(*pixmap);
    GrBitmapTextureMaker bitmapMaker(context, bmp, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
    GrMipMapped mipMapped = buildMips ? GrMipMapped::kYes : GrMipMapped::kNo;
    auto view = bitmapMaker.view(mipMapped);
    if (!view) {
        return SkImage::MakeRasterCopy(*pixmap);
    }

    sk_sp<GrTexture> texture = sk_ref_sp(view.proxy()->peekTexture());

    // Flush any writes or uploads
    context->priv().flushSurface(view.proxy());
    GrGpu* gpu = context->priv().getGpu();

    std::unique_ptr<GrSemaphore> sema = gpu->prepareTextureForCrossContextUsage(texture.get());

    SkColorType skCT = GrColorTypeToSkColorType(bitmapMaker.colorType());
    auto gen = GrBackendTextureImageGenerator::Make(std::move(texture), view.origin(),
                                                    std::move(sema), skCT,
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
    GrAHardwareBufferUtils::UpdateImageProc updateImageProc = nullptr;
    GrAHardwareBufferUtils::TexImageCtx deleteImageCtx = nullptr;

    GrBackendTexture backendTexture =
            GrAHardwareBufferUtils::MakeBackendTexture(context, hardwareBuffer,
                                                       bufferDesc.width, bufferDesc.height,
                                                       &deleteImageProc, &updateImageProc,
                                                       &deleteImageCtx, false, backendFormat, true);
    if (!backendTexture.isValid()) {
        return nullptr;
    }
    SkASSERT(deleteImageProc);

    SkColorType colorType =
            GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    if (!proxyProvider) {
        deleteImageProc(deleteImageCtx);
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTexture, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType,
            deleteImageProc, deleteImageCtx);
    if (!proxy) {
        deleteImageProc(deleteImageCtx);
        return nullptr;
    }

    sk_sp<SkColorSpace> cs = pixmap.refColorSpace();
    SkAlphaType at =  pixmap.alphaType();

    GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView view(std::move(proxy), surfaceOrigin, swizzle);
    sk_sp<SkImage> image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, view,
                                                   colorType, at, cs);
    if (!image) {
        return nullptr;
    }

    GrDrawingManager* drawingManager = context->priv().drawingManager();
    if (!drawingManager) {
        return nullptr;
    }

    GrSurfaceContext surfaceContext(context, std::move(view),
                                    SkColorTypeToGrColorType(pixmap.colorType()),
                                    pixmap.alphaType(), cs);

    SkImageInfo srcInfo = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType, at,
                                            std::move(cs));
    surfaceContext.writePixels(srcInfo, pixmap.addr(0, 0), pixmap.rowBytes(), {0, 0});

    GrFlushInfo info;
    info.fFlags = kSyncCpu_GrFlushFlag;
    GrSurfaceProxy* p[1] = {surfaceContext.asSurfaceProxy()};
    drawingManager->flush(p, 1, SkSurface::BackendSurfaceAccess::kNoAccess, info,
                          GrPrepareForExternalIORequests());

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
        image = image->makeTextureImage(ctx);
        if (!image) {
            return false;
        }
    }
    SkImage_GpuBase* gpuImage = static_cast<SkImage_GpuBase*>(as_IB(image));
    GrTexture* texture = gpuImage->getTexture();
    if (!texture) {
        // In context-loss cases, we may not have a texture.
        return false;
    }

    // If the image's context doesn't match the provided context, fail.
    if (texture->getContext() != ctx) {
        return false;
    }

    // Flush any pending IO on the texture.
    ctx->priv().flushSurface(as_IB(image)->peekProxy());

    // We must make a copy of the image if the image is not unique, if the GrTexture owned by the
    // image is not unique, or if the texture wraps an external object.
    if (!image->unique() || !texture->unique() ||
        texture->resourcePriv().refsWrappedObjects()) {
        // onMakeSubset will always copy the image.
        image = as_IB(image)->onMakeSubset(ctx, image->bounds());
        if (!image) {
            return false;
        }

        texture = gpuImage->getTexture();
        if (!texture) {
            return false;
        }

        // Flush to ensure that the copy is completed before we return the texture.
        ctx->priv().flushSurface(as_IB(image)->peekProxy());
    }

    SkASSERT(!texture->resourcePriv().refsWrappedObjects());
    SkASSERT(texture->unique());
    SkASSERT(image->unique());

    // Take a reference to the GrTexture and release the image.
    sk_sp<GrTexture> textureRef(SkSafeRef(texture));
    image = nullptr;

    // Steal the backend texture from the GrTexture, releasing the GrTexture in the process.
    return GrTexture::StealBackendTexture(std::move(textureRef), backendTexture, releaseProc);
}
