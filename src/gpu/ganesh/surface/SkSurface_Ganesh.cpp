/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/SkRenderEngineAbortf.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/image/SkImage_Base.h"

#include <algorithm>
#include <cstddef>
#include <utility>

class GrBackendSemaphore;
class SkCapabilities;
class SkPaint;
class SkPixmap;

SkSurface_Ganesh::SkSurface_Ganesh(sk_sp<skgpu::ganesh::Device> device)
        : INHERITED(device->width(), device->height(), &device->surfaceProps())
        , fDevice(std::move(device)) {
    SkASSERT(fDevice->targetProxy()->priv().isExact());
}

SkSurface_Ganesh::~SkSurface_Ganesh() {
    if (this->hasCachedImage()) {
        as_IB(this->refCachedImage())->generatingSurfaceIsDeleted();
    }
}

GrRecordingContext* SkSurface_Ganesh::onGetRecordingContext() const {
    return fDevice->recordingContext();
}

skgpu::ganesh::Device* SkSurface_Ganesh::getDevice() { return fDevice.get(); }

SkImageInfo SkSurface_Ganesh::imageInfo() const { return fDevice->imageInfo(); }

static GrRenderTarget* prepare_rt_for_external_access(SkSurface_Ganesh* surface,
                                                      SkSurfaces::BackendHandleAccess access) {
    auto dContext = surface->recordingContext()->asDirectContext();
    if (!dContext) {
        return nullptr;
    }
    if (dContext->abandoned()) {
        return nullptr;
    }

    switch (access) {
        case SkSurfaces::BackendHandleAccess::kFlushRead:
            break;
        case SkSurfaces::BackendHandleAccess::kFlushWrite:
        case SkSurfaces::BackendHandleAccess::kDiscardWrite:
            // for now we don't special-case on Discard, but we may in the future.
            surface->notifyContentWillChange(SkSurface::kRetain_ContentChangeMode);
            break;
    }

    dContext->priv().flushSurface(surface->getDevice()->targetProxy());

    // Grab the render target *after* firing notifications, as it may get switched if CoW kicks in.
    return surface->getDevice()->targetProxy()->peekRenderTarget();
}

GrBackendTexture SkSurface_Ganesh::getBackendTexture(BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    if (!rt) {
        return GrBackendTexture();  // invalid
    }
    GrTexture* texture = rt->asTexture();
    if (texture) {
        return texture->getBackendTexture();
    }
    return GrBackendTexture();  // invalid
}

GrBackendRenderTarget SkSurface_Ganesh::getBackendRenderTarget(BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    if (!rt) {
        return GrBackendRenderTarget();  // invalid
    }

    return rt->getBackendRenderTarget();
}

SkCanvas* SkSurface_Ganesh::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> SkSurface_Ganesh::onNewSurface(const SkImageInfo& info) {
    GrSurfaceProxyView targetView = fDevice->readSurfaceView();
    int sampleCount = targetView.asRenderTargetProxy()->numSamples();
    GrSurfaceOrigin origin = targetView.origin();
    // TODO: Make caller specify this (change virtual signature of onNewSurface).
    static const skgpu::Budgeted kBudgeted = skgpu::Budgeted::kNo;

    bool isProtected = targetView.asRenderTargetProxy()->isProtected() == GrProtected::kYes;
    return SkSurfaces::RenderTarget(
            fDevice->recordingContext(), kBudgeted, info, sampleCount, origin, &this->props(),
            /* shouldCreateWithMips= */ false, isProtected);
}

sk_sp<SkImage> SkSurface_Ganesh::onNewImageSnapshot(const SkIRect* subset) {
    GrRenderTargetProxy* rtp = fDevice->targetProxy();
    if (!rtp) {
        return nullptr;
    }

    auto rContext = fDevice->recordingContext();

    GrSurfaceProxyView srcView = fDevice->readSurfaceView();

    skgpu::Budgeted budgeted = rtp->isBudgeted();

    if (subset || !srcView.asTextureProxy() || rtp->refsWrappedObjects()) {
        // If the original render target is a buffer originally created by the client, then we don't
        // want to ever retarget the SkSurface at another buffer we create. If the source is a
        // texture (and the image is not subsetted) we make a dual-proxied SkImage that will
        // attempt to share the backing store until the surface writes to the shared backing store
        // at which point it uses a copy.
        if (!subset && srcView.asTextureProxy()) {
            return SkImage_Ganesh::MakeWithVolatileSrc(
                    sk_ref_sp(rContext), srcView, fDevice->imageInfo().colorInfo());
        }
        auto rect = subset ? *subset : SkIRect::MakeSize(srcView.dimensions());
        skgpu::Mipmapped mipmapped = srcView.mipmapped();
        srcView = GrSurfaceProxyView::Copy(rContext,
                                           std::move(srcView),
                                           mipmapped,
                                           rect,
                                           SkBackingFit::kExact,
                                           budgeted,
                                           /*label=*/"SurfaceGpu_NewImageSnapshot");
    }

    const SkImageInfo info = fDevice->imageInfo();
    if (!srcView.asTextureProxy()) {
        return nullptr;
    }
    // The surfaceDrawContext coming out of SkGpuDevice should always be exact and the
    // above copy creates a kExact surfaceContext.
    SkASSERT(srcView.proxy()->priv().isExact());
    return sk_make_sp<SkImage_Ganesh>(
            sk_ref_sp(rContext), kNeedNewImageUniqueID, std::move(srcView), info.colorInfo());
}

void SkSurface_Ganesh::onWritePixels(const SkPixmap& src, int x, int y) {
    fDevice->writePixels(src, x, y);
}

void SkSurface_Ganesh::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                                   SkIRect srcRect,
                                                   RescaleGamma rescaleGamma,
                                                   RescaleMode rescaleMode,
                                                   ReadPixelsCallback callback,
                                                   ReadPixelsContext context) {
    fDevice->asyncRescaleAndReadPixels(info, srcRect, rescaleGamma, rescaleMode, callback, context);
}

void SkSurface_Ganesh::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                         bool readAlpha,
                                                         sk_sp<SkColorSpace> dstColorSpace,
                                                         SkIRect srcRect,
                                                         SkISize dstSize,
                                                         RescaleGamma rescaleGamma,
                                                         RescaleMode rescaleMode,
                                                         ReadPixelsCallback callback,
                                                         ReadPixelsContext context) {
    fDevice->asyncRescaleAndReadPixelsYUV420(yuvColorSpace,
                                             readAlpha,
                                             std::move(dstColorSpace),
                                             srcRect,
                                             dstSize,
                                             rescaleGamma,
                                             rescaleMode,
                                             callback,
                                             context);
}

// Create a new render target and, if necessary, copy the contents of the old
// render target into it. Note that this flushes the SkGpuDevice but
// doesn't force an OpenGL flush.
bool SkSurface_Ganesh::onCopyOnWrite(ContentChangeMode mode) {
    GrSurfaceProxyView readSurfaceView = fDevice->readSurfaceView();

    // are we sharing our backing proxy with the image? Note this call should never create a new
    // image because onCopyOnWrite is only called when there is a cached image.
    sk_sp<SkImage> image = this->refCachedImage();
    SkASSERT(image);

    if (static_cast<SkImage_Ganesh*>(image.get())
                ->surfaceMustCopyOnWrite(readSurfaceView.proxy())) {
        if (!fDevice->replaceBackingProxy(mode)) {
            return false;
        }
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Ganesh::onDiscard();
    }
    return true;
}

void SkSurface_Ganesh::onDiscard() { fDevice->discard(); }

void SkSurface_Ganesh::resolveMSAA() { fDevice->resolveMSAA(); }

bool SkSurface_Ganesh::onWait(int numSemaphores,
                              const GrBackendSemaphore* waitSemaphores,
                              bool deleteSemaphoresAfterWait) {
    return fDevice->wait(numSemaphores, waitSemaphores, deleteSemaphoresAfterWait);
}

bool SkSurface_Ganesh::onCharacterize(GrSurfaceCharacterization* characterization) const {
    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    SkImageInfo ii = fDevice->imageInfo();
    if (ii.colorType() == kUnknown_SkColorType) {
        return false;
    }

    GrSurfaceProxyView readSurfaceView = fDevice->readSurfaceView();
    size_t maxResourceBytes = direct->getResourceCacheLimit();

    skgpu::Mipmapped mipmapped = readSurfaceView.asTextureProxy()
                                         ? readSurfaceView.asTextureProxy()->mipmapped()
                                         : skgpu::Mipmapped::kNo;

    bool usesGLFBO0 = readSurfaceView.asRenderTargetProxy()->glRTFBOIDIs0();
    // We should never get in the situation where we have a texture render target that is also
    // backend by FBO 0.
    SkASSERT(!usesGLFBO0 || !SkToBool(readSurfaceView.asTextureProxy()));

    bool vkRTSupportsInputAttachment =
            readSurfaceView.asRenderTargetProxy()->supportsVkInputAttachment();

    GrBackendFormat format = readSurfaceView.proxy()->backendFormat();
    int numSamples = readSurfaceView.asRenderTargetProxy()->numSamples();
    GrProtected isProtected = readSurfaceView.asRenderTargetProxy()->isProtected();

    characterization->set(
            direct->threadSafeProxy(),
            maxResourceBytes,
            ii,
            format,
            readSurfaceView.origin(),
            numSamples,
            GrSurfaceCharacterization::Textureable(SkToBool(readSurfaceView.asTextureProxy())),
            mipmapped,
            GrSurfaceCharacterization::UsesGLFBO0(usesGLFBO0),
            GrSurfaceCharacterization::VkRTSupportsInputAttachment(vkRTSupportsInputAttachment),
            GrSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
            isProtected,
            this->props());
    return true;
}

void SkSurface_Ganesh::onDraw(SkCanvas* canvas,
                              SkScalar x,
                              SkScalar y,
                              const SkSamplingOptions& sampling,
                              const SkPaint* paint) {
    // If the dst is also GPU we try to not force a new image snapshot (by calling the base class
    // onDraw) since that may not always perform the copy-on-write optimization.
    auto tryDraw = [&] {
        auto surfaceContext = fDevice->recordingContext();
        auto canvasContext = GrAsDirectContext(canvas->recordingContext());
        if (!canvasContext) {
            return false;
        }
        if (canvasContext->priv().contextID() != surfaceContext->priv().contextID()) {
            return false;
        }
        GrSurfaceProxyView srcView = fDevice->readSurfaceView();
        if (!srcView.asTextureProxyRef()) {
            return false;
        }
        // Possibly we could skip making an image here if SkGpuDevice exposed a lower level way
        // of drawing a texture proxy.
        const SkImageInfo info = fDevice->imageInfo();
        sk_sp<SkImage> image = sk_make_sp<SkImage_Ganesh>(sk_ref_sp(canvasContext),
                                                          kNeedNewImageUniqueID,
                                                          std::move(srcView),
                                                          info.colorInfo());
        canvas->drawImage(image.get(), x, y, sampling, paint);
        return true;
    };
    if (!tryDraw()) {
        INHERITED::onDraw(canvas, x, y, sampling, paint);
    }
}

bool SkSurface_Ganesh::onIsCompatible(const GrSurfaceCharacterization& characterization) const {
    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    if (!characterization.isValid()) {
        return false;
    }

    if (characterization.vulkanSecondaryCBCompatible()) {
        return false;
    }

    SkImageInfo ii = fDevice->imageInfo();
    if (ii.colorType() == kUnknown_SkColorType) {
        return false;
    }

    GrSurfaceProxyView targetView = fDevice->readSurfaceView();
    // As long as the current state if the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    // DDL TODO: should we just remove the resource check and ignore the cache limits on playback?
    size_t maxResourceBytes = direct->getResourceCacheLimit();

    if (characterization.isTextureable()) {
        if (!targetView.asTextureProxy()) {
            // If the characterization was textureable we require the replay dest to also be
            // textureable. If the characterized surface wasn't textureable we allow the replay
            // dest to be textureable.
            return false;
        }

        if (characterization.isMipMapped() &&
            skgpu::Mipmapped::kNo == targetView.asTextureProxy()->mipmapped()) {
            // Fail if the DDL's surface was mipmapped but the replay surface is not.
            // Allow drawing to proceed if the DDL was not mipmapped but the replay surface is.
            return false;
        }
    }

    if (characterization.usesGLFBO0() != targetView.asRenderTargetProxy()->glRTFBOIDIs0()) {
        // FBO0-ness effects how MSAA and window rectangles work. If the characterization was
        // tagged as FBO0 it would never have been allowed to use window rectangles. If MSAA
        // was also never used then a DDL recorded with this characterization should be replayable
        // on a non-FBO0 surface.
        if (!characterization.usesGLFBO0() || characterization.sampleCount() > 1) {
            return false;
        }
    }

    GrBackendFormat format = targetView.asRenderTargetProxy()->backendFormat();
    int numSamples = targetView.asRenderTargetProxy()->numSamples();
    GrProtected isProtected = targetView.proxy()->isProtected();

    return characterization.contextInfo() &&
           characterization.contextInfo()->priv().matches(direct) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == targetView.origin() &&
           characterization.backendFormat() == format && characterization.width() == ii.width() &&
           characterization.height() == ii.height() &&
           characterization.colorType() == ii.colorType() &&
           characterization.sampleCount() == numSamples &&
           SkColorSpace::Equals(characterization.colorSpace(), ii.colorInfo().colorSpace()) &&
           characterization.isProtected() == isProtected &&
           characterization.surfaceProps() == fDevice->surfaceProps();
}

bool SkSurface_Ganesh::draw(sk_sp<const GrDeferredDisplayList> ddl) {
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct || direct->abandoned()) {
        return false;
    }

    GrSurfaceProxyView view = fDevice->readSurfaceView();

    direct->priv().createDDLTask(std::move(ddl), view.asRenderTargetProxyRef());
    return true;
}

sk_sp<const SkCapabilities> SkSurface_Ganesh::onCapabilities() {
    return fDevice->recordingContext()->skCapabilities();
}

///////////////////////////////////////////////////////////////////////////////

static bool validate_backend_texture(const GrCaps* caps,
                                     const GrBackendTexture& tex,
                                     int sampleCnt,
                                     GrColorType grCT,
                                     bool texturable) {
    if (!tex.isValid()) {
        RENDERENGINE_ABORTF("%s failed due to input texture being invalid", __func__);
        return false;
    }

    GrBackendFormat backendFormat = tex.getBackendFormat();
    if (!backendFormat.isValid()) {
        RENDERENGINE_ABORTF("%s failed due to an invalid format", __func__);
        return false;
    }

    if (!caps->areColorTypeAndFormatCompatible(grCT, backendFormat)) {
        RENDERENGINE_ABORTF("%s failed due to an invalid format and colorType combination",
                            __func__);
        return false;
    }

    if (!caps->isFormatAsColorTypeRenderable(grCT, backendFormat, sampleCnt)) {
        RENDERENGINE_ABORTF(
                "%s failed due to no supported rendering path for the selected "
                "format and colorType",
                __func__);
        return false;
    }

    if (texturable && !caps->isFormatTexturable(backendFormat, tex.textureType())) {
        RENDERENGINE_ABORTF(
                "%s failed due to no texturing support for the selected format and "
                "colorType",
                __func__);
        return false;
    }

    return true;
}

bool SkSurface_Ganesh::replaceBackendTexture(const GrBackendTexture& backendTexture,
                                             GrSurfaceOrigin origin,
                                             ContentChangeMode mode,
                                             TextureReleaseProc releaseProc,
                                             ReleaseContext releaseContext) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(releaseProc, releaseContext);

    auto rContext = fDevice->recordingContext();
    if (rContext->abandoned()) {
        return false;
    }
    if (!backendTexture.isValid()) {
        return false;
    }
    if (backendTexture.width() != this->width() || backendTexture.height() != this->height()) {
        return false;
    }
    auto* oldRTP = fDevice->targetProxy();
    auto oldProxy = sk_ref_sp(oldRTP->asTextureProxy());
    if (!oldProxy) {
        return false;
    }
    auto* oldTexture = oldProxy->peekTexture();
    if (!oldTexture) {
        return false;
    }
    if (!oldTexture->resourcePriv().refsWrappedObjects()) {
        return false;
    }
    if (oldTexture->backendFormat() != backendTexture.getBackendFormat()) {
        return false;
    }
    if (oldTexture->getBackendTexture().isSameTexture(backendTexture)) {
        return false;
    }
    SkASSERT(oldTexture->asRenderTarget());
    int sampleCnt = oldTexture->asRenderTarget()->numSamples();
    GrColorType grColorType = SkColorTypeToGrColorType(this->getCanvas()->imageInfo().colorType());
    if (!validate_backend_texture(
                rContext->priv().caps(), backendTexture, sampleCnt, grColorType,
                /* texturable= */ true)) {
        return false;
    }

    sk_sp<SkColorSpace> colorSpace = fDevice->imageInfo().refColorSpace();

    SkASSERT(sampleCnt > 0);
    sk_sp<GrTextureProxy> proxy(rContext->priv().proxyProvider()->wrapRenderableBackendTexture(
            backendTexture,
            sampleCnt,
            kBorrow_GrWrapOwnership,
            GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return false;
    }

    return fDevice->replaceBackingProxy(mode,
                                        sk_ref_sp(proxy->asRenderTargetProxy()),
                                        grColorType,
                                        std::move(colorSpace),
                                        origin,
                                        this->props());
}

bool validate_backend_render_target(const GrCaps* caps,
                                    const GrBackendRenderTarget& rt,
                                    GrColorType grCT) {
    if (!caps->areColorTypeAndFormatCompatible(grCT, rt.getBackendFormat())) {
        return false;
    }

    if (!caps->isFormatAsColorTypeRenderable(grCT, rt.getBackendFormat(), rt.sampleCnt())) {
        return false;
    }

    // We require the stencil bits to be either 0, 8, or 16.
    int stencilBits = rt.stencilBits();
    if (stencilBits != 0 && stencilBits != 8 && stencilBits != 16) {
        return false;
    }

    return true;
}

namespace SkSurfaces {
sk_sp<SkSurface> RenderTarget(GrRecordingContext* rContext,
                              const GrSurfaceCharacterization& c,
                              skgpu::Budgeted budgeted) {
    if (!rContext || !c.isValid()) {
        return nullptr;
    }

    if (c.usesGLFBO0()) {
        // If we are making the surface we will never use FBO0.
        return nullptr;
    }

    if (c.vulkanSecondaryCBCompatible()) {
        return nullptr;
    }

    auto device = rContext->priv().createDevice(budgeted,
                                                c.imageInfo(),
                                                SkBackingFit::kExact,
                                                c.sampleCount(),
                                                skgpu::Mipmapped(c.isMipMapped()),
                                                c.isProtected(),
                                                c.origin(),
                                                c.surfaceProps(),
                                                skgpu::ganesh::Device::InitContents::kClear);
    if (!device) {
        return nullptr;
    }

    sk_sp<SkSurface> result = sk_make_sp<SkSurface_Ganesh>(std::move(device));
#ifdef SK_DEBUG
    if (result) {
        SkASSERT(result->isCompatible(c));
    }
#endif

    return result;
}

sk_sp<SkSurface> RenderTarget(GrRecordingContext* rContext,
                              skgpu::Budgeted budgeted,
                              const SkImageInfo& info,
                              int sampleCount,
                              GrSurfaceOrigin origin,
                              const SkSurfaceProps* props,
                              bool shouldCreateWithMips,
                              bool isProtected) {
    if (!rContext) {
        return nullptr;
    }
    sampleCount = std::max(1, sampleCount);
    skgpu::Mipmapped mipmapped =
            shouldCreateWithMips ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    if (!rContext->priv().caps()->mipmapSupport()) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    auto device = rContext->priv().createDevice(budgeted,
                                                info,
                                                SkBackingFit::kExact,
                                                sampleCount,
                                                mipmapped,
                                                GrProtected(isProtected),
                                                origin,
                                                SkSurfacePropsCopyOrDefault(props),
                                                skgpu::ganesh::Device::InitContents::kClear);
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Ganesh>(std::move(device));
}

sk_sp<SkSurface> WrapBackendTexture(GrRecordingContext* rContext,
                                    const GrBackendTexture& tex,
                                    GrSurfaceOrigin origin,
                                    int sampleCnt,
                                    SkColorType colorType,
                                    sk_sp<SkColorSpace> colorSpace,
                                    const SkSurfaceProps* props,
                                    TextureReleaseProc textureReleaseProc,
                                    ReleaseContext releaseContext) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(textureReleaseProc, releaseContext);

    if (!rContext) {
        RENDERENGINE_ABORTF("%s failed due to a null context ", __func__);
        return nullptr;
    }
    sampleCnt = std::max(1, sampleCnt);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);
    if (grColorType == GrColorType::kUnknown) {
        RENDERENGINE_ABORTF("%s failed due to an unsupported colorType %d", __func__, colorType);
        return nullptr;
    }

    if (!validate_backend_texture(rContext->priv().caps(), tex, sampleCnt, grColorType, true)) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy(rContext->priv().proxyProvider()->wrapRenderableBackendTexture(
            tex,
            sampleCnt,
            kBorrow_GrWrapOwnership,
            GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        RENDERENGINE_ABORTF("%s failed to wrap the texture into a renderable target", __func__);
        return nullptr;
    }

    auto device = rContext->priv().createDevice(grColorType,
                                                std::move(proxy),
                                                std::move(colorSpace),
                                                origin,
                                                SkSurfacePropsCopyOrDefault(props),
                                                skgpu::ganesh::Device::InitContents::kUninit);
    if (!device) {
        RENDERENGINE_ABORTF("%s failed to wrap the renderTarget into a surface", __func__);
        return nullptr;
    }

    return sk_make_sp<SkSurface_Ganesh>(std::move(device));
}

sk_sp<SkSurface> WrapBackendRenderTarget(GrRecordingContext* rContext,
                                         const GrBackendRenderTarget& rt,
                                         GrSurfaceOrigin origin,
                                         SkColorType colorType,
                                         sk_sp<SkColorSpace> colorSpace,
                                         const SkSurfaceProps* props,
                                         RenderTargetReleaseProc relProc,
                                         ReleaseContext releaseContext) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(relProc, releaseContext);

    if (!rContext || !rt.isValid()) {
        return nullptr;
    }

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);
    if (grColorType == GrColorType::kUnknown) {
        return nullptr;
    }

    if (!validate_backend_render_target(rContext->priv().caps(), rt, grColorType)) {
        return nullptr;
    }

    auto proxyProvider = rContext->priv().proxyProvider();
    auto proxy = proxyProvider->wrapBackendRenderTarget(rt, std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    auto device = rContext->priv().createDevice(grColorType,
                                                std::move(proxy),
                                                std::move(colorSpace),
                                                origin,
                                                SkSurfacePropsCopyOrDefault(props),
                                                skgpu::ganesh::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Ganesh>(std::move(device));
}

GrBackendTexture GetBackendTexture(SkSurface* surface, BackendHandleAccess access) {
    if (surface == nullptr) {
        return GrBackendTexture();
    }
    auto sb = asSB(surface);
    if (!sb->isGaneshBacked()) {
        return GrBackendTexture();
    }
    return static_cast<SkSurface_Ganesh*>(surface)->getBackendTexture(access);
}

GrBackendRenderTarget GetBackendRenderTarget(SkSurface* surface, BackendHandleAccess access) {
    if (surface == nullptr) {
        return GrBackendRenderTarget();
    }
    auto sb = asSB(surface);
    if (!sb->isGaneshBacked()) {
        return GrBackendRenderTarget();
    }
    return static_cast<SkSurface_Ganesh*>(surface)->getBackendRenderTarget(access);
}

void ResolveMSAA(SkSurface* surface) {
    if (!surface) {
        return;
    }
    auto sb = asSB(surface);
    if (!sb->isGaneshBacked()) {
        return;
    }
    auto gs = static_cast<SkSurface_Ganesh*>(surface);
    gs->resolveMSAA();
}

}  // namespace SkSurfaces

namespace skgpu::ganesh {
GrSemaphoresSubmitted Flush(SkSurface* surface) {
    if (!surface) {
        return GrSemaphoresSubmitted::kNo;
    }
    if (auto rContext = surface->recordingContext(); rContext != nullptr) {
        return rContext->asDirectContext()->flush(surface, {});
    }
    return GrSemaphoresSubmitted::kNo;
}

GrSemaphoresSubmitted Flush(sk_sp<SkSurface> surface) {
    if (!surface) {
        return GrSemaphoresSubmitted::kNo;
    }
    if (auto rContext = surface->recordingContext(); rContext != nullptr) {
        return rContext->asDirectContext()->flush(surface.get(), {});
    }
    return GrSemaphoresSubmitted::kNo;
}

void FlushAndSubmit(SkSurface* surface) {
    if (!surface) {
        return;
    }
    if (auto rContext = surface->recordingContext(); rContext != nullptr) {
        rContext->asDirectContext()->flushAndSubmit(surface, GrSyncCpu::kNo);
    }
}

void FlushAndSubmit(sk_sp<SkSurface> surface) {
    if (!surface) {
        return;
    }
    if (auto rContext = surface->recordingContext(); rContext != nullptr) {
        rContext->asDirectContext()->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
    }
}

}  // namespace skgpu::ganesh
