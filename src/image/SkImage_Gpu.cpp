/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Gpu.h"

#include "include/core/SkCanvas.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrYUVABackendTextures.h"
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
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/GrYUVATextureProxies.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/gl/GrGLTexture.h"

#include <cstddef>
#include <cstring>
#include <type_traits>

inline SkImage_Gpu::ProxyChooser::ProxyChooser(sk_sp<GrSurfaceProxy> stableProxy)
        : fStableProxy(std::move(stableProxy)) {
    SkASSERT(fStableProxy);
}

inline SkImage_Gpu::ProxyChooser::ProxyChooser(sk_sp<GrSurfaceProxy> stableProxy,
                                               sk_sp<GrSurfaceProxy> volatileProxy,
                                               sk_sp<GrRenderTask> copyTask,
                                               int volatileProxyTargetCount)
        : fStableProxy(std::move(stableProxy))
        , fVolatileProxy(std::move(volatileProxy))
        , fVolatileToStableCopyTask(std::move(copyTask))
        , fVolatileProxyTargetCount(volatileProxyTargetCount) {
    SkASSERT(fStableProxy);
    SkASSERT(fVolatileProxy);
    SkASSERT(fVolatileToStableCopyTask);
}

inline SkImage_Gpu::ProxyChooser::~ProxyChooser() {
    // The image is being destroyed. If there is a stable copy proxy but we've been able to use
    // the volatile proxy for all requests then we can skip the copy.
    if (fVolatileToStableCopyTask) {
        fVolatileToStableCopyTask->makeSkippable();
    }
}

inline sk_sp<GrSurfaceProxy> SkImage_Gpu::ProxyChooser::chooseProxy(GrRecordingContext* context) {
    SkAutoSpinlock hold(fLock);
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxyTargetCount <= fVolatileProxy->getTaskTargetCount());
        // If this image is used off the direct context it originated on, i.e. on a recording-only
        // context, we don't know how the recording context's actions are ordered WRT direct context
        // actions until the recording context's DAG is imported into the direct context.
        if (context->asDirectContext() &&
            fVolatileProxyTargetCount == fVolatileProxy->getTaskTargetCount()) {
            return fVolatileProxy;
        }
        fVolatileProxy.reset();
        fVolatileToStableCopyTask.reset();
        return fStableProxy;
    }
    return fStableProxy;
}

inline sk_sp<GrSurfaceProxy> SkImage_Gpu::ProxyChooser::switchToStableProxy() {
    SkAutoSpinlock hold(fLock);
    fVolatileProxy.reset();
    fVolatileToStableCopyTask.reset();
    return fStableProxy;
}

inline sk_sp<GrSurfaceProxy> SkImage_Gpu::ProxyChooser::makeVolatileProxyStable() {
    SkAutoSpinlock hold(fLock);
    if (fVolatileProxy) {
        fStableProxy = std::move(fVolatileProxy);
        fVolatileToStableCopyTask->makeSkippable();
        fVolatileToStableCopyTask.reset();
    }
    return fStableProxy;
}

inline bool SkImage_Gpu::ProxyChooser::surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const {
    SkAutoSpinlock hold(fLock);
    return surfaceProxy->underlyingUniqueID() == fStableProxy->underlyingUniqueID();
}

inline size_t SkImage_Gpu::ProxyChooser::gpuMemorySize() const {
    SkAutoSpinlock hold(fLock);
    size_t size = fStableProxy->gpuMemorySize();
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxy->gpuMemorySize() == size);
    }
    return size;
}

inline GrMipmapped SkImage_Gpu::ProxyChooser::mipmapped() const {
    SkAutoSpinlock hold(fLock);
    GrMipmapped mipmapped = fStableProxy->asTextureProxy()->mipmapped();
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxy->asTextureProxy()->mipmapped() == mipmapped);
    }
    return mipmapped;
}

#ifdef SK_DEBUG
inline GrBackendFormat SkImage_Gpu::ProxyChooser::backendFormat() {
    SkAutoSpinlock hold(fLock);
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxy->backendFormat() == fStableProxy->backendFormat());
    }
    return fStableProxy->backendFormat();
}
#endif

//////////////////////////////////////////////////////////////////////////////

SkImage_Gpu::SkImage_Gpu(sk_sp<GrImageContext> context,
                         uint32_t uniqueID,
                         GrSurfaceProxyView view,
                         SkColorInfo info)
        : INHERITED(std::move(context),
                    SkImageInfo::Make(view.proxy()->backingStoreDimensions(), std::move(info)),
                    uniqueID)
        , fChooser(view.detachProxy())
        , fSwizzle(view.swizzle())
        , fOrigin(view.origin()) {
#ifdef SK_DEBUG
    const GrBackendFormat& format = fChooser.backendFormat();
    const GrCaps* caps = this->context()->priv().caps();
    GrColorType grCT = SkColorTypeToGrColorType(this->colorType());
    SkASSERT(caps->isFormatCompressed(format) ||
             caps->areColorTypeAndFormatCompatible(grCT, format));
#endif
}

SkImage_Gpu::SkImage_Gpu(sk_sp<GrDirectContext> dContext,
                         GrSurfaceProxyView volatileSrc,
                         sk_sp<GrSurfaceProxy> stableCopy,
                         sk_sp<GrRenderTask> copyTask,
                         int volatileSrcTargetCount,
                         SkColorInfo info)
        : INHERITED(std::move(dContext),
                    SkImageInfo::Make(volatileSrc.proxy()->backingStoreDimensions(),
                                      std::move(info)),
                    kNeedNewImageUniqueID)
        , fChooser(std::move(stableCopy),
                   volatileSrc.detachProxy(),
                   std::move(copyTask),
                   volatileSrcTargetCount)
        , fSwizzle(volatileSrc.swizzle())
        , fOrigin(volatileSrc.origin()) {
#ifdef SK_DEBUG
    const GrBackendFormat& format = fChooser.backendFormat();
    const GrCaps* caps = this->context()->priv().caps();
    GrColorType grCT = SkColorTypeToGrColorType(this->colorType());
    SkASSERT(caps->isFormatCompressed(format) ||
             caps->areColorTypeAndFormatCompatible(grCT, format));
#endif
}

sk_sp<SkImage> SkImage_Gpu::MakeWithVolatileSrc(sk_sp<GrRecordingContext> rContext,
                                                GrSurfaceProxyView volatileSrc,
                                                SkColorInfo colorInfo) {
    SkASSERT(rContext);
    SkASSERT(volatileSrc);
    SkASSERT(volatileSrc.proxy()->asTextureProxy());
    GrMipmapped mm = volatileSrc.proxy()->asTextureProxy()->mipmapped();
    sk_sp<GrRenderTask> copyTask;
    auto copy = GrSurfaceProxy::Copy(rContext.get(),
                                     volatileSrc.refProxy(),
                                     volatileSrc.origin(),
                                     mm,
                                     SkBackingFit::kExact,
                                     SkBudgeted::kYes,
                                     &copyTask);
    if (!copy) {
        return nullptr;
    }
    // We only attempt to make a dual-proxy image on a direct context. This optimziation requires
    // knowing how things are ordered and recording-only contexts are not well ordered WRT other
    // recording contexts.
    if (auto direct = sk_ref_sp(rContext->asDirectContext())) {
        int targetCount = volatileSrc.proxy()->getTaskTargetCount();
        return sk_sp<SkImage>(new SkImage_Gpu(std::move(direct),
                                              std::move(volatileSrc),
                                              std::move(copy),
                                              std::move(copyTask),
                                              targetCount,
                                              std::move(colorInfo)));
    }
    GrSurfaceProxyView copyView(std::move(copy), volatileSrc.origin(), volatileSrc.swizzle());
    return sk_make_sp<SkImage_Gpu>(std::move(rContext),
                                   kNeedNewImageUniqueID,
                                   std::move(copyView),
                                   std::move(colorInfo));
}

SkImage_Gpu::~SkImage_Gpu() = default;

bool SkImage_Gpu::surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const {
    return fChooser.surfaceMustCopyOnWrite(surfaceProxy);
}

bool SkImage_Gpu::onHasMipmaps() const { return fChooser.mipmapped() == GrMipmapped::kYes; }

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

    sk_sp<GrSurfaceProxy> proxy = fChooser.chooseProxy(dContext);
    return dContext->priv().flushSurface(proxy.get(),
                                         SkSurface::BackendSurfaceAccess::kNoAccess,
                                         info);
}

GrBackendTexture SkImage_Gpu::onGetBackendTexture(bool flushPendingGrContextIO,
                                                  GrSurfaceOrigin* origin) const {
    auto direct = fContext->asDirectContext();
    if (!direct) {
        // This image was created with a DDL context and cannot be instantiated.
        return GrBackendTexture();  // invalid
    }
    if (direct->abandoned()) {
        return GrBackendTexture();  // invalid;
    }

    // We don't know how client's use of the texture will be ordered WRT Skia's. Ensure the
    // texture seen by the client won't be mutated by a SkSurface.
    sk_sp<GrSurfaceProxy> proxy = fChooser.switchToStableProxy();

    if (!proxy->isInstantiated()) {
        auto resourceProvider = direct->priv().resourceProvider();

        if (!proxy->instantiate(resourceProvider)) {
            return GrBackendTexture();  // invalid
        }
    }

    GrTexture* texture = proxy->peekTexture();
    if (texture) {
        if (flushPendingGrContextIO) {
            direct->priv().flushSurface(proxy.get());
        }
        if (origin) {
            *origin = fOrigin;
        }
        return texture->getBackendTexture();
    }
    return GrBackendTexture();  // invalid
}

size_t SkImage_Gpu::onTextureSize() const { return fChooser.gpuMemorySize(); }

sk_sp<SkImage> SkImage_Gpu::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                         sk_sp<SkColorSpace> targetCS,
                                                         GrDirectContext* dContext) const {
    SkColorInfo info(targetCT, this->alphaType(), std::move(targetCS));
    if (!fContext->priv().matches(dContext)) {
        return nullptr;
    }

    auto sfc = dContext->priv().makeSFCWithFallback(GrImageInfo(info, this->dimensions()),
                                                    SkBackingFit::kExact);
    if (!sfc) {
        return nullptr;
    }
    // We respecify info's CT because we called MakeWithFallback.
    auto ct = GrColorTypeToSkColorType(sfc->colorInfo().colorType());
    info = info.makeColorType(ct);

    // Draw this image's texture into the SFC.
    auto [view, _] = this->asView(dContext, GrMipmapped(this->hasMipmaps()));
    auto texFP = GrTextureEffect::Make(std::move(view), this->alphaType());
    auto colorFP = GrColorSpaceXformEffect::Make(std::move(texFP),
                                                 this->imageInfo().colorInfo(),
                                                 info);
    sfc->fillWithFP(std::move(colorFP));

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(dContext),
                                   kNeedNewImageUniqueID,
                                   sfc->readSurfaceView(),
                                   std::move(info));
}

sk_sp<SkImage> SkImage_Gpu::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // It doesn't seem worth the complexity of trying to share the ProxyChooser among multiple
    // images. Just fall back to the stable copy.
    GrSurfaceProxyView view(fChooser.switchToStableProxy(), fOrigin, fSwizzle);
    return sk_make_sp<SkImage_Gpu>(fContext,
                                   kNeedNewImageUniqueID,
                                   std::move(view),
                                   this->imageInfo().colorInfo().makeColorSpace(std::move(newCS)));
}

void SkImage_Gpu::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                              const SkIRect& srcRect,
                                              RescaleGamma rescaleGamma,
                                              RescaleMode rescaleMode,
                                              ReadPixelsCallback callback,
                                              ReadPixelsContext context) {
    auto dContext = fContext->asDirectContext();
    if (!dContext) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        callback(context, nullptr);
        return;
    }
    auto ctx = dContext->priv().makeSC(this->makeView(dContext), this->imageInfo().colorInfo());
    if (!ctx) {
        callback(context, nullptr);
        return;
    }
    ctx->asyncRescaleAndReadPixels(dContext, info, srcRect, rescaleGamma, rescaleMode,
                                   callback, context);
}

void SkImage_Gpu::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                    sk_sp<SkColorSpace> dstColorSpace,
                                                    const SkIRect& srcRect,
                                                    const SkISize& dstSize,
                                                    RescaleGamma rescaleGamma,
                                                    RescaleMode rescaleMode,
                                                    ReadPixelsCallback callback,
                                                    ReadPixelsContext context) {
    auto dContext = fContext->asDirectContext();
    if (!dContext) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        callback(context, nullptr);
        return;
    }
    auto ctx = dContext->priv().makeSC(this->makeView(dContext), this->imageInfo().colorInfo());
    if (!ctx) {
        callback(context, nullptr);
        return;
    }
    ctx->asyncRescaleAndReadPixelsYUV420(dContext,
                                         yuvColorSpace,
                                         std::move(dstColorSpace),
                                         srcRect,
                                         dstSize,
                                         rescaleGamma,
                                         rescaleMode,
                                         callback,
                                         context);
}

void SkImage_Gpu::generatingSurfaceIsDeleted() { fChooser.makeVolatileProxyStable(); }

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrRecordingContext* rContext,
                                                 const GrBackendTexture& backendTex,
                                                 GrColorType colorType,
                                                 GrSurfaceOrigin origin,
                                                 SkAlphaType at,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 sk_sp<GrRefCntedCallback> releaseHelper) {
    if (!backendTex.isValid() || backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTex, ownership, GrWrapCacheable::kNo, kRead_GrIOType, std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    GrSwizzle swizzle = rContext->priv().caps()->getReadSwizzle(proxy->backendFormat(), colorType);
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    SkColorInfo info(GrColorTypeToSkColorType(colorType), at, std::move(colorSpace));
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(rContext),
                                   kNeedNewImageUniqueID,
                                   std::move(view),
                                   std::move(info));
}

sk_sp<SkImage> SkImage::MakeFromCompressedTexture(GrRecordingContext* rContext,
                                                  const GrBackendTexture& tex,
                                                  GrSurfaceOrigin origin,
                                                  SkAlphaType at,
                                                  sk_sp<SkColorSpace> cs,
                                                  TextureReleaseProc releaseP,
                                                  ReleaseContext releaseC) {
    auto releaseHelper = GrRefCntedCallback::Make(releaseP, releaseC);

    if (!rContext) {
        return nullptr;
    }

    const GrCaps* caps = rContext->priv().caps();

    if (!SkImage_GpuBase::ValidateCompressedBackendTexture(caps, tex, at)) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapCompressedBackendTexture(
            tex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    CompressionType type = GrBackendFormatToCompressionType(tex.getBackendFormat());
    SkColorType ct = GrCompressionTypeToSkColorType(type);

    GrSurfaceProxyView view(std::move(proxy), origin, GrSwizzle::RGBA());
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(rContext),
                                   kNeedNewImageUniqueID,
                                   std::move(view),
                                   SkColorInfo(ct, at, std::move(cs)));
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrRecordingContext* rContext,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    auto releaseHelper = GrRefCntedCallback::Make(releaseP, releaseC);

    if (!rContext) {
        return nullptr;
    }

    const GrCaps* caps = rContext->priv().caps();

    GrColorType grColorType = SkColorTypeToGrColorType(ct);
    if (GrColorType::kUnknown == grColorType) {
        return nullptr;
    }

    if (!SkImage_GpuBase::ValidateBackendTexture(caps, tex, grColorType, ct, at, cs)) {
        return nullptr;
    }

    return new_wrapped_texture_common(rContext, tex, grColorType, origin, at, std::move(cs),
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

    GrColorType grColorType = SkColorTypeToGrColorType(ct);
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

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(direct),
                                   kNeedNewImageUniqueID,
                                   std::move(view),
                                   SkColorInfo(colorType, kOpaque_SkAlphaType, nullptr));
}

sk_sp<SkImage> SkImage::makeTextureImage(GrDirectContext* dContext,
                                         GrMipmapped mipmapped,
                                         SkBudgeted budgeted) const {
    if (!dContext) {
        return nullptr;
    }
    if (!dContext->priv().caps()->mipmapSupport() || this->dimensions().area() <= 1) {
        mipmapped = GrMipmapped::kNo;
    }

    if (this->isTextureBacked()) {
        if (!as_IB(this)->context()->priv().matches(dContext)) {
            return nullptr;
        }

        if (this->isTextureBacked() && (mipmapped == GrMipmapped::kNo || this->hasMipmaps())) {
            return sk_ref_sp(const_cast<SkImage*>(this));
        }
    }
    GrImageTexGenPolicy policy = budgeted == SkBudgeted::kYes
                                         ? GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                         : GrImageTexGenPolicy::kNew_Uncached_Unbudgeted;
    // TODO: Don't flatten YUVA images here. Add mips to the planes instead.
    auto [view, ct] = as_IB(this)->asView(dContext, mipmapped, policy);
    if (!view) {
        return nullptr;
    }
    SkASSERT(view.asTextureProxy());
    SkASSERT(mipmapped == GrMipmapped::kNo ||
             view.asTextureProxy()->mipmapped() == GrMipmapped::kYes);
    SkColorInfo colorInfo(GrColorTypeToSkColorType(ct), this->alphaType(), this->refColorSpace());
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(dContext),
                                   this->uniqueID(),
                                   std::move(view),
                                   std::move(colorInfo));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakePromiseTexture(sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                                           const GrBackendFormat& backendFormat,
                                           SkISize dimensions,
                                           GrMipmapped mipMapped,
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
    auto releaseHelper = GrRefCntedCallback::Make(textureReleaseProc, textureContext);
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

    auto proxy = SkImage_GpuBase::MakePromiseImageLazyProxy(threadSafeProxy.get(),
                                                            dimensions,
                                                            backendFormat,
                                                            mipMapped,
                                                            textureFulfillProc,
                                                            std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }
    GrSwizzle swizzle = threadSafeProxy->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    sk_sp<GrImageContext> ctx(GrImageContextPriv::MakeForPromiseImage(std::move(threadSafeProxy)));
    return sk_make_sp<SkImage_Gpu>(std::move(ctx),
                                   kNeedNewImageUniqueID,
                                   std::move(view),
                                   SkColorInfo(colorType, alphaType, std::move(colorSpace)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeCrossContextFromPixmap(GrDirectContext* dContext,
                                                   const SkPixmap& originalPixmap, bool buildMips,
                                                   bool limitToMaxTextureSize) {
    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!dContext || !dContext->priv().caps()->crossContextTextureSupport()) {
        return SkImage::MakeRasterCopy(originalPixmap);
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
    GrMipmapped mipmapped = buildMips ? GrMipmapped::kYes : GrMipmapped::kNo;
    auto [view, ct] = GrMakeUncachedBitmapProxyView(dContext, bmp, mipmapped);
    if (!view) {
        return SkImage::MakeRasterCopy(*pixmap);
    }

    sk_sp<GrTexture> texture = sk_ref_sp(view.proxy()->peekTexture());

    // Flush any writes or uploads
    dContext->priv().flushSurface(view.proxy());
    GrGpu* gpu = dContext->priv().getGpu();

    std::unique_ptr<GrSemaphore> sema = gpu->prepareTextureForCrossContextUsage(texture.get());

    SkColorType skCT = GrColorTypeToSkColorType(ct);
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

sk_sp<SkImage> SkImage::MakeFromAHardwareBufferWithData(GrDirectContext* dContext,
                                                        const SkPixmap& pixmap,
                                                        AHardwareBuffer* hardwareBuffer,
                                                        GrSurfaceOrigin surfaceOrigin) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE)) {
        return nullptr;
    }


    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(dContext,
                                                                             hardwareBuffer,
                                                                             bufferDesc.format,
                                                                             true);

    if (!backendFormat.isValid()) {
        return nullptr;
    }

    GrAHardwareBufferUtils::DeleteImageProc deleteImageProc = nullptr;
    GrAHardwareBufferUtils::UpdateImageProc updateImageProc = nullptr;
    GrAHardwareBufferUtils::TexImageCtx deleteImageCtx = nullptr;

    const bool isRenderable = SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER);

    GrBackendTexture backendTexture =
            GrAHardwareBufferUtils::MakeBackendTexture(dContext, hardwareBuffer,
                                                       bufferDesc.width, bufferDesc.height,
                                                       &deleteImageProc, &updateImageProc,
                                                       &deleteImageCtx, false, backendFormat,
                                                       isRenderable);
    if (!backendTexture.isValid()) {
        return nullptr;
    }
    SkASSERT(deleteImageProc);

    auto releaseHelper = GrRefCntedCallback::Make(deleteImageProc, deleteImageCtx);

    SkColorType colorType =
            GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    if (!proxyProvider) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTexture, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType,
            std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView framebufferView(std::move(proxy), surfaceOrigin, swizzle);
    SkColorInfo colorInfo = pixmap.info().colorInfo().makeColorType(colorType);
    sk_sp<SkImage> image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(dContext),
                                                   kNeedNewImageUniqueID,
                                                   framebufferView,
                                                   std::move(colorInfo));
    if (!image) {
        return nullptr;
    }

    GrDrawingManager* drawingManager = dContext->priv().drawingManager();
    if (!drawingManager) {
        return nullptr;
    }

    skgpu::SurfaceContext surfaceContext(
            dContext, std::move(framebufferView),image->imageInfo().colorInfo());

    surfaceContext.writePixels(dContext, pixmap, {0, 0});

    GrSurfaceProxy* p[1] = {surfaceContext.asSurfaceProxy()};
    drawingManager->flush(SkMakeSpan(p), SkSurface::BackendSurfaceAccess::kNoAccess, {}, nullptr);

    return image;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage::MakeBackendTextureFromSkImage(GrDirectContext* direct,
                                            sk_sp<SkImage> image,
                                            GrBackendTexture* backendTexture,
                                            BackendTextureReleaseProc* releaseProc) {
    if (!image || !backendTexture || !releaseProc) {
        return false;
    }

    auto [view, ct] = as_IB(image)->asView(direct, GrMipmapped::kNo);

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
    if (!image->unique() || !texture->unique() ||
        texture->resourcePriv().refsWrappedObjects()) {
        // onMakeSubset will always copy the image.
        image = as_IB(image)->onMakeSubset(image->bounds(), direct);
        if (!image) {
            return false;
        }
        return MakeBackendTextureFromSkImage(direct, std::move(image), backendTexture, releaseProc);
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

std::tuple<GrSurfaceProxyView, GrColorType> SkImage_Gpu::onAsView(
        GrRecordingContext* recordingContext,
        GrMipmapped mipmapped,
        GrImageTexGenPolicy policy) const {
    if (!fContext->priv().matches(recordingContext)) {
        return {};
    }
    if (policy != GrImageTexGenPolicy::kDraw) {
        return {CopyView(recordingContext, this->makeView(recordingContext), mipmapped, policy),
                SkColorTypeToGrColorType(this->colorType())};
    }
    GrSurfaceProxyView view = this->makeView(recordingContext);
    GrColorType ct = SkColorTypeToGrColorType(this->colorType());
    if (mipmapped == GrMipmapped::kYes) {
        view = FindOrMakeCachedMipmappedView(recordingContext, std::move(view), this->uniqueID());
    }
    return {std::move(view), ct};
}

std::unique_ptr<GrFragmentProcessor> SkImage_Gpu::onAsFragmentProcessor(
        GrRecordingContext* rContext,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    if (!fContext->priv().matches(rContext)) {
        return {};
    }
    auto mm = sampling.mipmap == SkMipmapMode::kNone ? GrMipmapped::kNo : GrMipmapped::kYes;
    return MakeFragmentProcessorFromView(rContext,
                                         std::get<0>(this->asView(rContext, mm)),
                                         this->alphaType(),
                                         sampling,
                                         tileModes,
                                         m,
                                         subset,
                                         domain);
}

GrSurfaceProxyView SkImage_Gpu::makeView(GrRecordingContext* rContext) const {
    return {fChooser.chooseProxy(rContext), fOrigin, fSwizzle};
}
