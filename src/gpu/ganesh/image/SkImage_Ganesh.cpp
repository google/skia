/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/SkImage_Ganesh.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <utility>

class SkMatrix;
enum SkColorType : int;
enum class SkTileMode;

inline SkImage_Ganesh::ProxyChooser::ProxyChooser(sk_sp<GrSurfaceProxy> stableProxy)
        : fStableProxy(std::move(stableProxy)) {
    SkASSERT(fStableProxy);
}

inline SkImage_Ganesh::ProxyChooser::ProxyChooser(sk_sp<GrSurfaceProxy> stableProxy,
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

inline SkImage_Ganesh::ProxyChooser::~ProxyChooser() {
    // The image is being destroyed. If there is a stable copy proxy but we've been able to use
    // the volatile proxy for all requests then we can skip the copy.
    if (fVolatileToStableCopyTask) {
        fVolatileToStableCopyTask->makeSkippable();
    }
}

inline sk_sp<GrSurfaceProxy> SkImage_Ganesh::ProxyChooser::chooseProxy(
        GrRecordingContext* context) {
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

inline sk_sp<GrSurfaceProxy> SkImage_Ganesh::ProxyChooser::switchToStableProxy() {
    SkAutoSpinlock hold(fLock);
    fVolatileProxy.reset();
    fVolatileToStableCopyTask.reset();
    return fStableProxy;
}

inline sk_sp<GrSurfaceProxy> SkImage_Ganesh::ProxyChooser::makeVolatileProxyStable() {
    SkAutoSpinlock hold(fLock);
    if (fVolatileProxy) {
        fStableProxy = std::move(fVolatileProxy);
        fVolatileToStableCopyTask->makeSkippable();
        fVolatileToStableCopyTask.reset();
    }
    return fStableProxy;
}

inline bool SkImage_Ganesh::ProxyChooser::surfaceMustCopyOnWrite(
        GrSurfaceProxy* surfaceProxy) const {
    SkAutoSpinlock hold(fLock);
    return surfaceProxy->underlyingUniqueID() == fStableProxy->underlyingUniqueID();
}

inline size_t SkImage_Ganesh::ProxyChooser::gpuMemorySize() const {
    SkAutoSpinlock hold(fLock);
    size_t size = fStableProxy->gpuMemorySize();
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxy->gpuMemorySize() == size);
    }
    return size;
}

inline skgpu::Mipmapped SkImage_Ganesh::ProxyChooser::mipmapped() const {
    SkAutoSpinlock hold(fLock);
    skgpu::Mipmapped mipmapped = fStableProxy->asTextureProxy()->mipmapped();
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxy->asTextureProxy()->mipmapped() == mipmapped);
    }
    return mipmapped;
}

inline skgpu::Protected SkImage_Ganesh::ProxyChooser::isProtected() const {
    SkAutoSpinlock hold(fLock);
    skgpu::Protected isProtected = fStableProxy->asTextureProxy()->isProtected();
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxy->asTextureProxy()->isProtected() == isProtected);
    }
    return isProtected;
}

#ifdef SK_DEBUG
inline const GrBackendFormat& SkImage_Ganesh::ProxyChooser::backendFormat() {
    SkAutoSpinlock hold(fLock);
    if (fVolatileProxy) {
        SkASSERT(fVolatileProxy->backendFormat() == fStableProxy->backendFormat());
    }
    return fStableProxy->backendFormat();
}
#endif

//////////////////////////////////////////////////////////////////////////////

SkImage_Ganesh::SkImage_Ganesh(sk_sp<GrImageContext> context,
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

SkImage_Ganesh::SkImage_Ganesh(sk_sp<GrDirectContext> dContext,
                               GrSurfaceProxyView volatileSrc,
                               sk_sp<GrSurfaceProxy> stableCopy,
                               sk_sp<GrRenderTask> copyTask,
                               int volatileSrcTargetCount,
                               SkColorInfo info)
        : INHERITED(
                  std::move(dContext),
                  SkImageInfo::Make(volatileSrc.proxy()->backingStoreDimensions(), std::move(info)),
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

sk_sp<SkImage> SkImage_Ganesh::MakeWithVolatileSrc(sk_sp<GrRecordingContext> rContext,
                                                   GrSurfaceProxyView volatileSrc,
                                                   SkColorInfo colorInfo) {
    SkASSERT(rContext);
    SkASSERT(volatileSrc);
    SkASSERT(volatileSrc.proxy()->asTextureProxy());
    skgpu::Mipmapped mm = volatileSrc.proxy()->asTextureProxy()->mipmapped();
    sk_sp<GrRenderTask> copyTask;
    auto copy = GrSurfaceProxy::Copy(rContext.get(),
                                     volatileSrc.refProxy(),
                                     volatileSrc.origin(),
                                     mm,
                                     SkBackingFit::kExact,
                                     skgpu::Budgeted::kYes,
                                     /*label=*/"ImageGpu_MakeWithVolatileSrc",
                                     &copyTask);
    if (!copy) {
        return nullptr;
    }
    // We only attempt to make a dual-proxy image on a direct context. This optimziation requires
    // knowing how things are ordered and recording-only contexts are not well ordered WRT other
    // recording contexts.
    if (auto direct = sk_ref_sp(rContext->asDirectContext())) {
        int targetCount = volatileSrc.proxy()->getTaskTargetCount();
        return sk_sp<SkImage>(new SkImage_Ganesh(std::move(direct),
                                                 std::move(volatileSrc),
                                                 std::move(copy),
                                                 std::move(copyTask),
                                                 targetCount,
                                                 std::move(colorInfo)));
    }
    GrSurfaceProxyView copyView(std::move(copy), volatileSrc.origin(), volatileSrc.swizzle());
    return sk_make_sp<SkImage_Ganesh>(
            std::move(rContext), kNeedNewImageUniqueID, std::move(copyView), std::move(colorInfo));
}

SkImage_Ganesh::~SkImage_Ganesh() = default;

bool SkImage_Ganesh::surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const {
    return fChooser.surfaceMustCopyOnWrite(surfaceProxy);
}

bool SkImage_Ganesh::onHasMipmaps() const { return fChooser.mipmapped() == skgpu::Mipmapped::kYes; }

bool SkImage_Ganesh::onIsProtected() const {
    return fChooser.isProtected() == skgpu::Protected::kYes;
}

GrSemaphoresSubmitted SkImage_Ganesh::flush(GrDirectContext* dContext,
                                            const GrFlushInfo& info) const {
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
    return dContext->priv().flushSurface(
            proxy.get(), SkSurfaces::BackendSurfaceAccess::kNoAccess, info);
}

bool SkImage_Ganesh::getExistingBackendTexture(GrBackendTexture* outTexture,
                                               bool flushPendingGrContextIO,
                                               GrSurfaceOrigin* origin) const {
    auto direct = fContext->asDirectContext();
    if (!direct) {
        // This image was created with a DDL context and cannot be instantiated.
        return false;
    }
    if (direct->abandoned()) {
        return false;
    }

    // We don't know how client's use of the texture will be ordered WRT Skia's. Ensure the
    // texture seen by the client won't be mutated by a SkSurface.
    sk_sp<GrSurfaceProxy> proxy = fChooser.switchToStableProxy();

    if (!proxy->isInstantiated()) {
        auto resourceProvider = direct->priv().resourceProvider();

        if (!proxy->instantiate(resourceProvider)) {
            return false;
        }
    }

    GrTexture* texture = proxy->peekTexture();
    if (!texture) {
        return false;
    }
    if (flushPendingGrContextIO) {
        direct->priv().flushSurface(proxy.get());
    }
    if (origin) {
        *origin = fOrigin;
    }
    if (outTexture) {
        *outTexture = texture->getBackendTexture();
    }
    return true;
}

size_t SkImage_Ganesh::textureSize() const { return fChooser.gpuMemorySize(); }

sk_sp<SkImage> SkImage_Ganesh::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                            sk_sp<SkColorSpace> targetCS,
                                                            GrDirectContext* dContext) const {
    SkColorInfo info(targetCT, this->alphaType(), std::move(targetCS));
    if (!fContext->priv().matches(dContext)) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy = fChooser.chooseProxy(dContext);

    auto sfc = dContext->priv().makeSFCWithFallback(GrImageInfo(info, this->dimensions()),
                                                    SkBackingFit::kExact,
                                                    /* sampleCount= */ 1,
                                                    skgpu::Mipmapped::kNo,
                                                    proxy->isProtected());
    if (!sfc) {
        return nullptr;
    }
    // We respecify info's CT because we called MakeWithFallback.
    auto ct = GrColorTypeToSkColorType(sfc->colorInfo().colorType());
    info = info.makeColorType(ct);

    // Draw this image's texture into the SFC.
    auto [view, _] = skgpu::ganesh::AsView(dContext, this, skgpu::Mipmapped(this->hasMipmaps()));
    auto texFP = GrTextureEffect::Make(std::move(view), this->alphaType());
    auto colorFP =
            GrColorSpaceXformEffect::Make(std::move(texFP), this->imageInfo().colorInfo(), info);
    sfc->fillWithFP(std::move(colorFP));

    return sk_make_sp<SkImage_Ganesh>(
            sk_ref_sp(dContext), kNeedNewImageUniqueID, sfc->readSurfaceView(), std::move(info));
}

sk_sp<SkImage> SkImage_Ganesh::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // It doesn't seem worth the complexity of trying to share the ProxyChooser among multiple
    // images. Just fall back to the stable copy.
    GrSurfaceProxyView view(fChooser.switchToStableProxy(), fOrigin, fSwizzle);
    return sk_make_sp<SkImage_Ganesh>(
            fContext,
            kNeedNewImageUniqueID,
            std::move(view),
            this->imageInfo().colorInfo().makeColorSpace(std::move(newCS)));
}

void SkImage_Ganesh::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                                 SkIRect srcRect,
                                                 RescaleGamma rescaleGamma,
                                                 RescaleMode rescaleMode,
                                                 ReadPixelsCallback callback,
                                                 ReadPixelsContext context) const {
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
    ctx->asyncRescaleAndReadPixels(
            dContext, info, srcRect, rescaleGamma, rescaleMode, callback, context);
}

void SkImage_Ganesh::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                       bool readAlpha,
                                                       sk_sp<SkColorSpace> dstColorSpace,
                                                       SkIRect srcRect,
                                                       SkISize dstSize,
                                                       RescaleGamma rescaleGamma,
                                                       RescaleMode rescaleMode,
                                                       ReadPixelsCallback callback,
                                                       ReadPixelsContext context) const {
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
                                         readAlpha,
                                         std::move(dstColorSpace),
                                         srcRect,
                                         dstSize,
                                         rescaleGamma,
                                         rescaleMode,
                                         callback,
                                         context);
}

void SkImage_Ganesh::generatingSurfaceIsDeleted() { fChooser.makeVolatileProxyStable(); }

std::tuple<GrSurfaceProxyView, GrColorType> SkImage_Ganesh::asView(
        GrRecordingContext* recordingContext,
        skgpu::Mipmapped mipmapped,
        GrImageTexGenPolicy policy) const {
    if (!fContext->priv().matches(recordingContext)) {
        return {};
    }
    if (policy != GrImageTexGenPolicy::kDraw) {
        return {skgpu::ganesh::CopyView(recordingContext,
                                        this->makeView(recordingContext),
                                        mipmapped,
                                        policy,
                                        /*label=*/"SkImageGpu_AsView"),
                SkColorTypeToGrColorType(this->colorType())};
    }
    GrSurfaceProxyView view = this->makeView(recordingContext);
    GrColorType ct = SkColorTypeToGrColorType(this->colorType());
    if (mipmapped == skgpu::Mipmapped::kYes) {
        view = skgpu::ganesh::FindOrMakeCachedMipmappedView(recordingContext, std::move(view),
                                                            this->uniqueID());
    }
    return {std::move(view), ct};
}

std::unique_ptr<GrFragmentProcessor> SkImage_Ganesh::asFragmentProcessor(
        skgpu::ganesh::SurfaceDrawContext* sdc,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    GrRecordingContext* rContext = sdc->recordingContext();
    if (!fContext->priv().matches(rContext)) {
        return {};
    }
    auto mm =
            sampling.mipmap == SkMipmapMode::kNone ? skgpu::Mipmapped::kNo : skgpu::Mipmapped::kYes;
    return skgpu::ganesh::MakeFragmentProcessorFromView(
            rContext,
            std::get<0>(skgpu::ganesh::AsView(rContext, this, mm)),
            this->alphaType(),
            sampling,
            tileModes,
            m,
            subset,
            domain);
}

GrSurfaceProxyView SkImage_Ganesh::makeView(GrRecordingContext* rContext) const {
    return {fChooser.chooseProxy(rContext), fOrigin, fSwizzle};
}
