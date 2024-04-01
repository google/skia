/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/SkImageGanesh.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/GrExternalTextureGenerator.h"
#include "include/private/base/SkAssert.h"
#include "include/private/chromium/SkImageChromium.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "include/private/gpu/ganesh/GrTextureGenerator.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrBackendTextureImageGenerator.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/image/SkImage_Base.h"

#include <algorithm>
#include <memory>
#include <utility>

enum SkColorType : int;
enum class SkTextureCompressionType;

namespace SkImages {

bool MakeBackendTextureFromImage(GrDirectContext* direct,
                                 sk_sp<SkImage> image,
                                 GrBackendTexture* backendTexture,
                                 BackendTextureReleaseProc* releaseProc) {
    if (!image || !backendTexture || !releaseProc) {
        return false;
    }

    auto [view, ct] = skgpu::ganesh::AsView(direct, image, skgpu::Mipmapped::kNo);
    if (!view) {
        return false;
    }

    // Flush any pending IO on the texture.
    direct->priv().flushSurface(view.proxy());

    GrTexture* texture = view.asTextureProxy()->peekTexture();
    if (!texture) {
        return false;
    }
    // We must make a copy of the image if the image is not unique, if the GrTexture owned by the
    // image is not unique, or if the texture wraps an external object.
    if (!image->unique() || !texture->unique() || texture->resourcePriv().refsWrappedObjects()) {
        // onMakeSubset will always copy the image.
        image = as_IB(image)->onMakeSubset(direct, image->bounds());
        if (!image) {
            return false;
        }
        return MakeBackendTextureFromImage(direct, std::move(image), backendTexture, releaseProc);
    }

    SkASSERT(!texture->resourcePriv().refsWrappedObjects());
    SkASSERT(texture->unique());
    SkASSERT(image->unique());

    // Take a reference to the GrTexture and release the image.
    sk_sp<GrTexture> textureRef = sk_ref_sp(texture);
    view.reset();
    image = nullptr;
    SkASSERT(textureRef->unique());

    // Steal the backend texture from the GrTexture, releasing the GrTexture in the process.
    return GrTexture::StealBackendTexture(std::move(textureRef), backendTexture, releaseProc);
}

bool GetBackendTextureFromImage(const SkImage* img,
                                GrBackendTexture* outTexture,
                                bool flushPendingGrContextIO,
                                GrSurfaceOrigin* origin) {
    if (!img) {
        return false;
    }
    auto ib = as_IB(img);
    if (ib->type() != SkImage_Base::Type::kGanesh) {
        return false;
    }
    auto ig = static_cast<const SkImage_Ganesh*>(img);
    return ig->getExistingBackendTexture(outTexture, flushPendingGrContextIO, origin);
}

sk_sp<SkImage> TextureFromCompressedTexture(GrRecordingContext* context,
                                            const GrBackendTexture& backendTexture,
                                            GrSurfaceOrigin origin,
                                            SkAlphaType alphaType,
                                            sk_sp<SkColorSpace> colorSpace,
                                            TextureReleaseProc textureReleaseProc,
                                            ReleaseContext releaseContext) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(textureReleaseProc, releaseContext);

    if (!context) {
        return nullptr;
    }

    const GrCaps* caps = context->priv().caps();

    if (!SkImage_GaneshBase::ValidateCompressedBackendTexture(caps, backendTexture, alphaType)) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy =
            proxyProvider->wrapCompressedBackendTexture(backendTexture,
                                                        kBorrow_GrWrapOwnership,
                                                        GrWrapCacheable::kNo,
                                                        std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    SkTextureCompressionType type =
            GrBackendFormatToCompressionType(backendTexture.getBackendFormat());
    SkColorType ct = skgpu::CompressionTypeToSkColorType(type);

    GrSurfaceProxyView view(std::move(proxy), origin, skgpu::Swizzle::RGBA());
    return sk_make_sp<SkImage_Ganesh>(sk_ref_sp(context),
                                      kNeedNewImageUniqueID,
                                      std::move(view),
                                      SkColorInfo(ct, alphaType, std::move(colorSpace)));
}

static sk_sp<SkImage> new_wrapped_texture_common(GrRecordingContext* rContext,
                                                 const GrBackendTexture& backendTex,
                                                 GrColorType colorType,
                                                 GrSurfaceOrigin origin,
                                                 SkAlphaType at,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 sk_sp<skgpu::RefCntedCallback> releaseHelper) {
    if (!backendTex.isValid() || backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTex, ownership, GrWrapCacheable::kNo, kRead_GrIOType, std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    skgpu::Swizzle swizzle =
            rContext->priv().caps()->getReadSwizzle(proxy->backendFormat(), colorType);
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    SkColorInfo info(GrColorTypeToSkColorType(colorType), at, std::move(colorSpace));
    return sk_make_sp<SkImage_Ganesh>(
            sk_ref_sp(rContext), kNeedNewImageUniqueID, std::move(view), std::move(info));
}

sk_sp<SkImage> BorrowTextureFrom(GrRecordingContext* context,
                                 const GrBackendTexture& backendTexture,
                                 GrSurfaceOrigin origin,
                                 SkColorType colorType,
                                 SkAlphaType alphaType,
                                 sk_sp<SkColorSpace> colorSpace,
                                 TextureReleaseProc textureReleaseProc,
                                 ReleaseContext releaseContext) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(textureReleaseProc, releaseContext);

    if (!context) {
        return nullptr;
    }

    const GrCaps* caps = context->priv().caps();

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    if (!SkImage_GaneshBase::ValidateBackendTexture(
                caps, backendTexture, grColorType, colorType, alphaType, colorSpace)) {
        return nullptr;
    }

    return new_wrapped_texture_common(context,
                                      backendTexture,
                                      grColorType,
                                      origin,
                                      alphaType,
                                      std::move(colorSpace),
                                      kBorrow_GrWrapOwnership,
                                      std::move(releaseHelper));
}

sk_sp<SkImage> AdoptTextureFrom(GrRecordingContext* context,
                                const GrBackendTexture& backendTexture,
                                GrSurfaceOrigin textureOrigin,
                                SkColorType colorType) {
    return AdoptTextureFrom(
            context, backendTexture, textureOrigin, colorType, kPremul_SkAlphaType, nullptr);
}

sk_sp<SkImage> AdoptTextureFrom(GrRecordingContext* context,
                                const GrBackendTexture& backendTexture,
                                GrSurfaceOrigin textureOrigin,
                                SkColorType colorType,
                                SkAlphaType alphaType) {
    return AdoptTextureFrom(context, backendTexture, textureOrigin, colorType, alphaType, nullptr);
}

sk_sp<SkImage> AdoptTextureFrom(GrRecordingContext* context,
                                const GrBackendTexture& backendTexture,
                                GrSurfaceOrigin origin,
                                SkColorType colorType,
                                SkAlphaType alphaType,
                                sk_sp<SkColorSpace> colorSpace) {
    auto dContext = GrAsDirectContext(context);
    if (!dContext) {
        // We have a DDL context and we don't support adopted textures for them.
        return nullptr;
    }

    const GrCaps* caps = dContext->priv().caps();

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    if (!SkImage_GaneshBase::ValidateBackendTexture(
                caps, backendTexture, grColorType, colorType, alphaType, colorSpace)) {
        return nullptr;
    }

    return new_wrapped_texture_common(dContext,
                                      backendTexture,
                                      grColorType,
                                      origin,
                                      alphaType,
                                      std::move(colorSpace),
                                      kAdopt_GrWrapOwnership,
                                      nullptr);
}

sk_sp<SkImage> TextureFromCompressedTextureData(GrDirectContext* direct,
                                                sk_sp<SkData> data,
                                                int width,
                                                int height,
                                                SkTextureCompressionType type,
                                                skgpu::Mipmapped mipmapped,
                                                GrProtected isProtected) {
    if (!direct || !data) {
        return nullptr;
    }

    GrBackendFormat beFormat = direct->compressedBackendFormat(type);
    if (!beFormat.isValid()) {
        sk_sp<SkImage> tmp = RasterFromCompressedTextureData(std::move(data), width, height, type);
        if (!tmp) {
            return nullptr;
        }
        return TextureFromImage(direct, tmp, mipmapped);
    }

    GrProxyProvider* proxyProvider = direct->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->createCompressedTextureProxy(
            {width, height}, skgpu::Budgeted::kYes, mipmapped, isProtected, type, std::move(data));
    if (!proxy) {
        return nullptr;
    }
    GrSurfaceProxyView view(std::move(proxy));

    SkColorType colorType = skgpu::CompressionTypeToSkColorType(type);

    return sk_make_sp<SkImage_Ganesh>(sk_ref_sp(direct),
                                      kNeedNewImageUniqueID,
                                      std::move(view),
                                      SkColorInfo(colorType, kOpaque_SkAlphaType, nullptr));
}

sk_sp<SkImage> PromiseTextureFrom(sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                                  const GrBackendFormat& backendFormat,
                                  SkISize dimensions,
                                  skgpu::Mipmapped mipmapped,
                                  GrSurfaceOrigin origin,
                                  SkColorType colorType,
                                  SkAlphaType alphaType,
                                  sk_sp<SkColorSpace> colorSpace,
                                  PromiseImageTextureFulfillProc textureFulfillProc,
                                  PromiseImageTextureReleaseProc textureReleaseProc,
                                  PromiseImageTextureContext textureContext) {
    // Our contract is that we will always call the release proc even on failure.
    // We use the helper to convey the context, so we need to ensure make doesn't fail.
    textureReleaseProc = textureReleaseProc ? textureReleaseProc : [](void*) {};
    auto releaseHelper = skgpu::RefCntedCallback::Make(textureReleaseProc, textureContext);
    SkImageInfo info = SkImageInfo::Make(dimensions, colorType, alphaType, colorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    if (!threadSafeProxy) {
        return nullptr;
    }

    if (dimensions.isEmpty()) {
        return nullptr;
    }

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    if (!threadSafeProxy->priv().caps()->areColorTypeAndFormatCompatible(grColorType,
                                                                         backendFormat)) {
        return nullptr;
    }

    auto proxy = SkImage_GaneshBase::MakePromiseImageLazyProxy(threadSafeProxy.get(),
                                                               dimensions,
                                                               backendFormat,
                                                               mipmapped,
                                                               textureFulfillProc,
                                                               std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }
    skgpu::Swizzle swizzle =
            threadSafeProxy->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    sk_sp<GrImageContext> ctx(GrImageContextPriv::MakeForPromiseImage(std::move(threadSafeProxy)));
    return sk_make_sp<SkImage_Ganesh>(std::move(ctx),
                                      kNeedNewImageUniqueID,
                                      std::move(view),
                                      SkColorInfo(colorType, alphaType, std::move(colorSpace)));
}

sk_sp<SkImage> CrossContextTextureFromPixmap(GrDirectContext* dContext,
                                             const SkPixmap& originalPixmap,
                                             bool buildMips,
                                             bool limitToMaxTextureSize) {
    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!dContext || !dContext->priv().caps()->crossContextTextureSupport()) {
        return RasterFromPixmapCopy(originalPixmap);
    }

    // If non-power-of-two mipmapping isn't supported, ignore the client's request
    if (!dContext->priv().caps()->mipmapSupport()) {
        buildMips = false;
    }

    const SkPixmap* pixmap = &originalPixmap;
    SkAutoPixmapStorage resized;
    int maxTextureSize = dContext->priv().caps()->maxTextureSize();
    int maxDim = std::max(originalPixmap.width(), originalPixmap.height());
    if (limitToMaxTextureSize && maxDim > maxTextureSize) {
        float scale = static_cast<float>(maxTextureSize) / maxDim;
        int newWidth = std::min(static_cast<int>(originalPixmap.width() * scale), maxTextureSize);
        int newHeight = std::min(static_cast<int>(originalPixmap.height() * scale), maxTextureSize);
        SkImageInfo info = originalPixmap.info().makeWH(newWidth, newHeight);
        SkSamplingOptions sampling(SkFilterMode::kLinear);
        if (!resized.tryAlloc(info) || !originalPixmap.scalePixels(resized, sampling)) {
            return nullptr;
        }
        pixmap = &resized;
    }
    // Turn the pixmap into a GrTextureProxy
    SkBitmap bmp;
    bmp.installPixels(*pixmap);
    skgpu::Mipmapped mipmapped = buildMips ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    auto [view, ct] = GrMakeUncachedBitmapProxyView(dContext, bmp, mipmapped);
    if (!view) {
        return RasterFromPixmapCopy(*pixmap);
    }

    sk_sp<GrTexture> texture = sk_ref_sp(view.proxy()->peekTexture());

    // Flush any writes or uploads
    dContext->priv().flushSurface(view.proxy());
    GrGpu* gpu = dContext->priv().getGpu();

    std::unique_ptr<GrSemaphore> sema = gpu->prepareTextureForCrossContextUsage(texture.get());

    SkColorType skCT = GrColorTypeToSkColorType(ct);
    auto gen = GrBackendTextureImageGenerator::Make(std::move(texture),
                                                    view.origin(),
                                                    std::move(sema),
                                                    skCT,
                                                    pixmap->alphaType(),
                                                    pixmap->info().refColorSpace());
    return DeferredFromTextureGenerator(std::move(gen));
}

sk_sp<SkImage> TextureFromImage(GrDirectContext* dContext,
                                const SkImage* img,
                                skgpu::Mipmapped mipmapped,
                                skgpu::Budgeted budgeted) {
    if (!dContext || !img) {
        return nullptr;
    }
    auto ib = as_IB(img);
    if (!dContext->priv().caps()->mipmapSupport() || ib->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (ib->isGaneshBacked()) {
        if (!ib->context()->priv().matches(dContext)) {
            return nullptr;
        }

        if (mipmapped == skgpu::Mipmapped::kNo || ib->hasMipmaps()) {
            return sk_ref_sp(const_cast<SkImage_Base*>(ib));
        }
    }
    GrImageTexGenPolicy policy = budgeted == skgpu::Budgeted::kYes
                                         ? GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                         : GrImageTexGenPolicy::kNew_Uncached_Unbudgeted;
    // TODO: Don't flatten YUVA images here. Add mips to the planes instead.
    auto [view, ct] = skgpu::ganesh::AsView(dContext, ib, mipmapped, policy);
    if (!view) {
        return nullptr;
    }
    SkASSERT(view.asTextureProxy());
    SkASSERT(mipmapped == skgpu::Mipmapped::kNo ||
             view.asTextureProxy()->mipmapped() == skgpu::Mipmapped::kYes);
    SkColorInfo colorInfo(GrColorTypeToSkColorType(ct), ib->alphaType(), ib->refColorSpace());
    return sk_make_sp<SkImage_Ganesh>(
            sk_ref_sp(dContext), ib->uniqueID(), std::move(view), std::move(colorInfo));
}

}  // namespace SkImages
