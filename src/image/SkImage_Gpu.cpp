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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkScopeExit.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAHardwareBufferImageGenerator.h"
#include "src/gpu/GrAHardwareBufferUtils.h"
#include "src/gpu/GrBackendTextureImageGenerator.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
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
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureAdjuster.h"
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

GrSemaphoresSubmitted SkImage_Gpu::onFlush(GrDirectContext* dContext, const GrFlushInfo& info) {
    if (!fContext->priv().matches(dContext) || dContext->abandoned()) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo;
    }

    GrSurfaceProxy* p[1] = {fView.proxy()};
    return dContext->priv().flushSurfaces(p, 1, info);
}

sk_sp<SkImage> SkImage_Gpu::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                         sk_sp<SkColorSpace> targetCS,
                                                         GrDirectContext* direct) const {
    if (!fContext->priv().matches(direct)) {
        return nullptr;
    }

    auto renderTargetContext = GrRenderTargetContext::MakeWithFallback(
            direct, SkColorTypeToGrColorType(targetCT), nullptr, SkBackingFit::kExact,
            this->dimensions());
    if (!renderTargetContext) {
        return nullptr;
    }

    auto texFP = GrTextureEffect::Make(*this->view(direct), this->alphaType());
    auto colorFP = GrColorSpaceXformEffect::Make(std::move(texFP),
                                                 this->colorSpace(), this->alphaType(),
                                                 targetCS.get(), this->alphaType());
    SkASSERT(colorFP);

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.setColorFragmentProcessor(std::move(colorFP));

    renderTargetContext->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                  SkRect::MakeIWH(this->width(), this->height()));
    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    targetCT = GrColorTypeToSkColorType(renderTargetContext->colorInfo().colorType());
    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(direct), kNeedNewImageUniqueID,
                                   renderTargetContext->readSurfaceView(), targetCT,
                                   this->alphaType(), std::move(targetCS));
}

sk_sp<SkImage> SkImage_Gpu::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID, fView, this->colorType(),
                                   this->alphaType(), std::move(newCS));
}

void SkImage_Gpu::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                              const SkIRect& srcRect,
                                              RescaleGamma rescaleGamma,
                                              SkFilterQuality rescaleQuality,
                                              ReadPixelsCallback callback,
                                              ReadPixelsContext context) {
    GrColorType ct = SkColorTypeToGrColorType(this->colorType());
    auto ctx = GrSurfaceContext::Make(fContext.get(), fView, ct, this->alphaType(),
                                      this->refColorSpace());
    if (!ctx) {
        callback(context, nullptr);
        return;
    }
    ctx->asyncRescaleAndReadPixels(info, srcRect, rescaleGamma, rescaleQuality, callback, context);
}

void SkImage_Gpu::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                    sk_sp<SkColorSpace> dstColorSpace,
                                                    const SkIRect& srcRect,
                                                    const SkISize& dstSize,
                                                    RescaleGamma rescaleGamma,
                                                    SkFilterQuality rescaleQuality,
                                                    ReadPixelsCallback callback,
                                                    ReadPixelsContext context) {
    GrColorType ct = SkColorTypeToGrColorType(this->colorType());
    auto ctx = GrSurfaceContext::Make(fContext.get(), fView, ct, this->alphaType(),
                                      this->refColorSpace());
    if (!ctx) {
        callback(context, nullptr);
        return;
    }
    ctx->asyncRescaleAndReadPixelsYUV420(yuvColorSpace,
                                         std::move(dstColorSpace),
                                         srcRect,
                                         dstSize,
                                         rescaleGamma,
                                         rescaleQuality,
                                         callback,
                                         context);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx,
                                                 const GrBackendTexture& backendTex,
                                                 GrColorType colorType, GrSurfaceOrigin origin,
                                                 SkAlphaType at, sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 sk_sp<GrRefCntedCallback> releaseHelper) {
    if (!backendTex.isValid() || backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTex, ownership, GrWrapCacheable::kNo, kRead_GrIOType, std::move(releaseHelper));
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
    sk_sp<GrRefCntedCallback> releaseHelper;
    if (releaseP) {
        releaseHelper.reset(new GrRefCntedCallback(releaseP, releaseC));
    }

    if (!ctx) {
        return nullptr;
    }

    const GrCaps* caps = ctx->priv().caps();

    if (!SkImage_GpuBase::ValidateCompressedBackendTexture(caps, tex, at)) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapCompressedBackendTexture(
            tex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    CompressionType type = GrBackendFormatToCompressionType(tex.getBackendFormat());
    SkColorType ct = GrCompressionTypeToSkColorType(type);

    GrSurfaceProxyView view(std::move(proxy), origin, GrSwizzle::RGBA());
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID,  std::move(view), ct, at,
                                   std::move(cs));
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    sk_sp<GrRefCntedCallback> releaseHelper;
    if (releaseP) {
        releaseHelper.reset(new GrRefCntedCallback(releaseP, releaseC));
    }

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
                                      kBorrow_GrWrapOwnership, std::move(releaseHelper));
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrRecordingContext* rContext,
                                               const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                               SkColorType ct, SkAlphaType at,
                                               sk_sp<SkColorSpace> cs) {
    auto dContext = GrAsDirectContext(rContext);
    if (!dContext) {
        // We have a DDL context and we don't support adopted textures for them.
        return nullptr;
    }

    const GrCaps* caps = dContext->priv().caps();

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(caps, ct, tex.getBackendFormat());
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    if (!SkImage_GpuBase::ValidateBackendTexture(caps, tex, grColorType, ct, at, cs)) {
        return nullptr;
    }

    return new_wrapped_texture_common(dContext, tex, grColorType, origin, at, std::move(cs),
                                      kAdopt_GrWrapOwnership, nullptr);
}

sk_sp<SkImage> SkImage::MakeTextureFromCompressed(GrDirectContext* direct, sk_sp<SkData> data,
                                                  int width, int height, CompressionType type,
                                                  GrMipmapped mipMapped,
                                                  GrProtected isProtected) {
    if (!direct || !data) {
        return nullptr;
    }

    GrBackendFormat beFormat = direct->compressedBackendFormat(type);
    if (!beFormat.isValid()) {
        sk_sp<SkImage> tmp = MakeRasterFromCompressed(std::move(data), width, height, type);
        if (!tmp) {
            return nullptr;
        }
        return tmp->makeTextureImage(direct, mipMapped);
    }

    GrProxyProvider* proxyProvider = direct->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createCompressedTextureProxy(
            {width, height}, SkBudgeted::kYes, mipMapped, isProtected, type, std::move(data));
    if (!proxy) {
        return nullptr;
    }
    GrSurfaceProxyView view(std::move(proxy));

    SkColorType colorType = GrCompressionTypeToSkColorType(type);

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(direct), kNeedNewImageUniqueID, std::move(view),
                                   colorType, kOpaque_SkAlphaType, nullptr);
}

sk_sp<SkImage> SkImage_Gpu::ConvertYUVATexturesToRGB(GrRecordingContext* rContext,
                                                     SkYUVColorSpace yuvColorSpace,
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
    if (!SkImage_GpuBase::MakeTempTextureProxies(rContext, yuvaTextures, numTextures, yuvaIndices,
                                                 origin, tempViews, nullptr)) {
        return nullptr;
    }

    const SkRect rect = SkRect::MakeIWH(size.width(), size.height());
    const GrCaps& caps = *rContext->priv().caps();
    if (!RenderYUVAToRGBA(caps, renderTargetContext, rect, yuvColorSpace, nullptr,
                          tempViews, yuvaIndices)) {
        return nullptr;
    }

    SkColorType ct = GrColorTypeToSkColorType(renderTargetContext->colorInfo().colorType());
    SkAlphaType at = GetAlphaTypeFromYUVAIndices(yuvaIndices);
    // TODO: Remove this use of backdoor.
    GrContext* backdoorGrCtx = rContext->priv().backdoor();
    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(backdoorGrCtx), kNeedNewImageUniqueID,
                                   renderTargetContext->readSurfaceView(), ct, at,
                                   renderTargetContext->colorInfo().refColorSpace());
}

sk_sp<SkImage> SkImage::MakeFromYUVATexturesCopy(GrRecordingContext* rContext,
                                                 SkYUVColorSpace yuvColorSpace,
                                                 const GrBackendTexture yuvaTextures[],
                                                 const SkYUVAIndex yuvaIndices[4],
                                                 SkISize imageSize,
                                                 GrSurfaceOrigin imageOrigin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    auto renderTargetContext = GrRenderTargetContext::Make(
            rContext, GrColorType::kRGBA_8888, std::move(imageColorSpace), SkBackingFit::kExact,
            imageSize, 1, GrMipmapped::kNo, GrProtected::kNo, imageOrigin);
    if (!renderTargetContext) {
        return nullptr;
    }

    return SkImage_Gpu::ConvertYUVATexturesToRGB(rContext, yuvColorSpace, yuvaTextures, yuvaIndices,
                                                 imageSize, imageOrigin, renderTargetContext.get());
}

sk_sp<SkImage> SkImage::MakeFromYUVATexturesCopyWithExternalBackend(
        GrRecordingContext* rContext,
        SkYUVColorSpace yuvColorSpace,
        const GrBackendTexture yuvaTextures[],
        const SkYUVAIndex yuvaIndices[4],
        SkISize imageSize,
        GrSurfaceOrigin imageOrigin,
        const GrBackendTexture& backendTexture,
        sk_sp<SkColorSpace> imageColorSpace,
        TextureReleaseProc textureReleaseProc,
        ReleaseContext releaseContext) {
    const GrCaps* caps = rContext->priv().caps();

    sk_sp<GrRefCntedCallback> releaseHelper;
    if (textureReleaseProc) {
        releaseHelper.reset(new GrRefCntedCallback(textureReleaseProc, releaseContext));
    }

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
            rContext, grColorType, std::move(imageColorSpace), backendTexture, 1, imageOrigin,
            nullptr, std::move(releaseHelper));
    if (!renderTargetContext) {
        return nullptr;
    }

    return SkImage_Gpu::ConvertYUVATexturesToRGB(rContext, yuvColorSpace, yuvaTextures, yuvaIndices,
                                                 imageSize, imageOrigin, renderTargetContext.get());
}

// Some YUVA factories infer the YUVAIndices. This helper identifies the channel to use for single
// channel textures.
static SkColorChannel get_single_channel(const GrBackendTexture& tex) {
    switch (tex.getBackendFormat().channelMask()) {
        case kGray_SkColorChannelFlag:  // Gray can be read as any of kR, kG, kB.
        case kRed_SkColorChannelFlag:
            return SkColorChannel::kR;
        case kAlpha_SkColorChannelFlag:
            return SkColorChannel::kA;
        default: // multiple channels in the texture. Guess kR.
            return SkColorChannel::kR;
    }
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopyWithExternalBackend(
        GrContext* ctx, SkYUVColorSpace yuvColorSpace, const GrBackendTexture yuvTextures[3],
        GrSurfaceOrigin imageOrigin, const GrBackendTexture& backendTexture,
        sk_sp<SkColorSpace> imageColorSpace) {
    SkYUVAIndex yuvaIndices[4] = {
            SkYUVAIndex{0, get_single_channel(yuvTextures[0])},
            SkYUVAIndex{1, get_single_channel(yuvTextures[1])},
            SkYUVAIndex{2, get_single_channel(yuvTextures[2])},
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
            SkYUVAIndex{0, get_single_channel(nv12Textures[0])},
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
            SkYUVAIndex{0, get_single_channel(nv12Textures[0])},
            SkYUVAIndex{1, SkColorChannel::kR},
            SkYUVAIndex{1, SkColorChannel::kG},
            SkYUVAIndex{-1, SkColorChannel::kA}};
    SkISize size{nv12Textures[0].width(), nv12Textures[0].height()};
    return SkImage_Gpu::MakeFromYUVATexturesCopyWithExternalBackend(
            ctx, yuvColorSpace, nv12Textures, yuvaIndices, size, imageOrigin, backendTexture,
            std::move(imageColorSpace), textureReleaseProc, releaseContext);
}

static sk_sp<SkImage> create_image_from_producer(GrContext* context, GrTextureProducer* producer,
                                                 uint32_t id, GrMipmapped mipMapped) {
    auto view = producer->view(mipMapped);
    if (!view) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), id, std::move(view),
                                   GrColorTypeToSkColorType(producer->colorType()),
                                   producer->alphaType(), sk_ref_sp(producer->colorSpace()));
}


sk_sp<SkImage> SkImage::makeTextureImage(GrDirectContext* dContext,
                                         GrMipmapped mipMapped,
                                         SkBudgeted budgeted) const {
    if (!dContext) {
        return nullptr;
    }

    if (this->isTextureBacked()) {
        if (!as_IB(this)->context()->priv().matches(dContext)) {
            return nullptr;
        }

        // TODO: Don't flatten YUVA images here.
        const GrSurfaceProxyView* view = as_IB(this)->view(dContext);
        SkASSERT(view && view->asTextureProxy());

        if (mipMapped == GrMipmapped::kNo || view->asTextureProxy()->mipmapped() == mipMapped ||
            !dContext->priv().caps()->mipmapSupport()) {
            return sk_ref_sp(const_cast<SkImage*>(this));
        }
        auto copy = GrCopyBaseMipMapToView(dContext, *view, budgeted);
        if (!copy) {
            return nullptr;
        }
        return sk_make_sp<SkImage_Gpu>(sk_ref_sp(dContext), this->uniqueID(), copy,
                                       this->colorType(), this->alphaType(), this->refColorSpace());
    }

    auto policy = budgeted == SkBudgeted::kYes ? GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                               : GrImageTexGenPolicy::kNew_Uncached_Unbudgeted;
    if (this->isLazyGenerated()) {
        GrImageTextureMaker maker(dContext, this, policy);
        return create_image_from_producer(dContext, &maker, this->uniqueID(), mipMapped);
    }

    if (const SkBitmap* bmp = as_IB(this)->onPeekBitmap()) {
        GrBitmapTextureMaker maker(dContext, *bmp, policy);
        return create_image_from_producer(dContext, &maker, this->uniqueID(), mipMapped);
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Gpu::MakePromiseTexture(GrRecordingContext* context,
                                               const GrBackendFormat& backendFormat,
                                               int width,
                                               int height,
                                               GrMipmapped mipMapped,
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

    if (!context->priv().caps()->areColorTypeAndFormatCompatible(grColorType, backendFormat)) {
        return nullptr;
    }

    callDone.clear();
    auto proxy = MakePromiseImageLazyProxy(context, width, height, backendFormat,
                                           mipMapped, textureFulfillProc, textureReleaseProc,
                                           textureDoneProc, textureContext, version);
    if (!proxy) {
        return nullptr;
    }
    GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    // CONTEXT TODO: rm this usage of the 'backdoor' to create an image
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context->priv().backdoor()), kNeedNewImageUniqueID,
                                   std::move(view), colorType, alphaType, std::move(colorSpace));
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
    if (!context->priv().caps()->mipmapSupport()) {
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
    GrMipmapped mipMapped = buildMips ? GrMipmapped::kYes : GrMipmapped::kNo;
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

    auto direct = GrAsDirectContext(context);
    if (!direct) {
        SkDebugf("Direct context required\n");
        return nullptr;
    }

    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(direct,
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
            GrAHardwareBufferUtils::MakeBackendTexture(direct, hardwareBuffer,
                                                       bufferDesc.width, bufferDesc.height,
                                                       &deleteImageProc, &updateImageProc,
                                                       &deleteImageCtx, false, backendFormat, true);
    if (!backendTexture.isValid()) {
        return nullptr;
    }
    SkASSERT(deleteImageProc);

    sk_sp<GrRefCntedCallback> releaseHelper(new GrRefCntedCallback(deleteImageProc,
                                                                   deleteImageCtx));

    SkColorType colorType =
            GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    GrProxyProvider* proxyProvider = direct->priv().proxyProvider();
    if (!proxyProvider) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTexture, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType,
            std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    sk_sp<SkColorSpace> cs = pixmap.refColorSpace();
    SkAlphaType at =  pixmap.alphaType();

    GrSwizzle swizzle = direct->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView view(std::move(proxy), surfaceOrigin, swizzle);
    sk_sp<SkImage> image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(direct), kNeedNewImageUniqueID, view,
                                                   colorType, at, cs);
    if (!image) {
        return nullptr;
    }

    GrDrawingManager* drawingManager = direct->priv().drawingManager();
    if (!drawingManager) {
        return nullptr;
    }

    GrSurfaceContext surfaceContext(direct, std::move(view),
                                    SkColorTypeToGrColorType(pixmap.colorType()),
                                    pixmap.alphaType(), cs);

    SkImageInfo srcInfo = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType, at,
                                            std::move(cs));
    surfaceContext.writePixels(srcInfo, pixmap.addr(0, 0), pixmap.rowBytes(), {0, 0});

    GrSurfaceProxy* p[1] = {surfaceContext.asSurfaceProxy()};
    drawingManager->flush(p, 1, SkSurface::BackendSurfaceAccess::kNoAccess, {}, nullptr);

    return image;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage::MakeBackendTextureFromSkImage(GrContext* ctx,
                                            sk_sp<SkImage> image,
                                            GrBackendTexture* backendTexture,
                                            BackendTextureReleaseProc* releaseProc) {
    // TODO: Elevate direct context requirement to public API.
    auto direct = GrAsDirectContext(ctx);
    if (!image || !direct || !backendTexture || !releaseProc) {
        return false;
    }

    // Ensure we have a texture backed image.
    if (!image->isTextureBacked()) {
        image = image->makeTextureImage(direct);
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
    if (texture->getContext() != direct) {
        return false;
    }

    // Flush any pending IO on the texture.
    direct->priv().flushSurface(as_IB(image)->peekProxy());

    // We must make a copy of the image if the image is not unique, if the GrTexture owned by the
    // image is not unique, or if the texture wraps an external object.
    if (!image->unique() || !texture->unique() ||
        texture->resourcePriv().refsWrappedObjects()) {
        // onMakeSubset will always copy the image.
        image = as_IB(image)->onMakeSubset(image->bounds(), direct);
        if (!image) {
            return false;
        }

        texture = gpuImage->getTexture();
        if (!texture) {
            return false;
        }

        // Flush to ensure that the copy is completed before we return the texture.
        direct->priv().flushSurface(as_IB(image)->peekProxy());
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
