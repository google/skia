/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkSurface_Gpu.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/BaseDevice.h"
#include "src/gpu/GrAHardwareBufferUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkSurface_Base.h"

#if SK_SUPPORT_GPU

SkSurface_Gpu::SkSurface_Gpu(sk_sp<skgpu::BaseDevice> device)
    : INHERITED(device->width(), device->height(), &device->surfaceProps())
    , fDevice(std::move(device)) {
    SkASSERT(fDevice->targetProxy()->priv().isExact());
}

SkSurface_Gpu::~SkSurface_Gpu() {
}

GrRecordingContext* SkSurface_Gpu::onGetRecordingContext() {
    return fDevice->recordingContext();
}

skgpu::BaseDevice* SkSurface_Gpu::getDevice() {
    return fDevice.get();
}

static GrRenderTarget* prepare_rt_for_external_access(SkSurface_Gpu* surface,
                                                      SkSurface::BackendHandleAccess access) {
    auto dContext = surface->recordingContext()->asDirectContext();
    if (!dContext) {
        return nullptr;
    }
    if (dContext->abandoned()) {
        return nullptr;
    }

    switch (access) {
        case SkSurface::kFlushRead_BackendHandleAccess:
            break;
        case SkSurface::kFlushWrite_BackendHandleAccess:
        case SkSurface::kDiscardWrite_BackendHandleAccess:
            // for now we don't special-case on Discard, but we may in the future.
            surface->notifyContentWillChange(SkSurface::kRetain_ContentChangeMode);
            break;
    }

    dContext->priv().flushSurface(surface->getDevice()->targetProxy());

    // Grab the render target *after* firing notifications, as it may get switched if CoW kicks in.
    return surface->getDevice()->targetProxy()->peekRenderTarget();
}

GrBackendTexture SkSurface_Gpu::onGetBackendTexture(BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    if (!rt) {
        return GrBackendTexture(); // invalid
    }
    GrTexture* texture = rt->asTexture();
    if (texture) {
        return texture->getBackendTexture();
    }
    return GrBackendTexture(); // invalid
}

GrBackendRenderTarget SkSurface_Gpu::onGetBackendRenderTarget(BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    if (!rt) {
        return GrBackendRenderTarget(); // invalid
    }

    return rt->getBackendRenderTarget();
}

SkCanvas* SkSurface_Gpu::onNewCanvas() { return new SkCanvas(fDevice); }

sk_sp<SkSurface> SkSurface_Gpu::onNewSurface(const SkImageInfo& info) {
    GrSurfaceProxyView targetView = fDevice->readSurfaceView();
    int sampleCount = targetView.asRenderTargetProxy()->numSamples();
    GrSurfaceOrigin origin = targetView.origin();
    // TODO: Make caller specify this (change virtual signature of onNewSurface).
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fDevice->recordingContext(), kBudgeted, info, sampleCount,
                                       origin, &this->props());
}

sk_sp<SkImage> SkSurface_Gpu::onNewImageSnapshot(const SkIRect* subset) {
    GrRenderTargetProxy* rtp = fDevice->targetProxy();
    if (!rtp) {
        return nullptr;
    }

    auto rContext = fDevice->recordingContext();

    GrSurfaceProxyView srcView = fDevice->readSurfaceView();

    SkBudgeted budgeted = rtp->isBudgeted();

    if (subset || !srcView.asTextureProxy() || rtp->refsWrappedObjects()) {
        // If the original render target is a buffer originally created by the client, then we don't
        // want to ever retarget the SkSurface at another buffer we create. If the source is a
        // texture (and the image is not subsetted) we make a dual-proxied SkImage that will
        // attempt to share the backing store until the surface writes to the shared backing store
        // at which point it uses a copy.
        if (!subset && srcView.asTextureProxy()) {
            return SkImage_Gpu::MakeWithVolatileSrc(sk_ref_sp(rContext),
                                                    srcView,
                                                    fDevice->imageInfo().colorInfo());
        }
        auto rect = subset ? *subset : SkIRect::MakeSize(srcView.dimensions());
        GrMipmapped mipmapped = srcView.mipmapped();
        srcView = GrSurfaceProxyView::Copy(rContext, std::move(srcView), mipmapped, rect,
                                           SkBackingFit::kExact, budgeted);
    }

    const SkImageInfo info = fDevice->imageInfo();
    if (!srcView.asTextureProxy()) {
        return nullptr;
    }
    // The surfaceDrawContext coming out of SkGpuDevice should always be exact and the
    // above copy creates a kExact surfaceContext.
    SkASSERT(srcView.proxy()->priv().isExact());
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(rContext),
                                   kNeedNewImageUniqueID,
                                   std::move(srcView),
                                   info.colorInfo());
}

void SkSurface_Gpu::onWritePixels(const SkPixmap& src, int x, int y) {
    fDevice->writePixels(src, x, y);
}

void SkSurface_Gpu::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                                const SkIRect& srcRect,
                                                RescaleGamma rescaleGamma,
                                                RescaleMode rescaleMode,
                                                ReadPixelsCallback callback,
                                                ReadPixelsContext context) {
    fDevice->asyncRescaleAndReadPixels(info,
                                       srcRect,
                                       rescaleGamma,
                                       rescaleMode,
                                       callback,
                                       context);
}

void SkSurface_Gpu::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                      sk_sp<SkColorSpace> dstColorSpace,
                                                      const SkIRect& srcRect,
                                                      const SkISize& dstSize,
                                                      RescaleGamma rescaleGamma,
                                                      RescaleMode rescaleMode,
                                                      ReadPixelsCallback callback,
                                                      ReadPixelsContext context) {
    fDevice->asyncRescaleAndReadPixelsYUV420(yuvColorSpace,
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
void SkSurface_Gpu::onCopyOnWrite(ContentChangeMode mode) {
    GrSurfaceProxyView readSurfaceView = fDevice->readSurfaceView();

    // are we sharing our backing proxy with the image? Note this call should never create a new
    // image because onCopyOnWrite is only called when there is a cached image.
    sk_sp<SkImage> image = this->refCachedImage();
    SkASSERT(image);

    if (static_cast<SkImage_Gpu*>(image.get())->surfaceMustCopyOnWrite(readSurfaceView.proxy())) {
        fDevice->replaceBackingProxy(mode);
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Gpu::onDiscard();
    }
}

void SkSurface_Gpu::onDiscard() { fDevice->discard(); }

GrSemaphoresSubmitted SkSurface_Gpu::onFlush(BackendSurfaceAccess access, const GrFlushInfo& info,
                                             const GrBackendSurfaceMutableState* newState) {

    auto dContext = fDevice->recordingContext()->asDirectContext();
    if (!dContext) {
        return GrSemaphoresSubmitted::kNo;
    }

    GrRenderTargetProxy* rtp = fDevice->targetProxy();

    return dContext->priv().flushSurface(rtp, access, info, newState);
}

bool SkSurface_Gpu::onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
                           bool deleteSemaphoresAfterWait) {
    return fDevice->wait(numSemaphores, waitSemaphores, deleteSemaphoresAfterWait);
}

bool SkSurface_Gpu::onCharacterize(SkSurfaceCharacterization* characterization) const {
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

    bool mipmapped = readSurfaceView.asTextureProxy()
                            ? GrMipmapped::kYes == readSurfaceView.asTextureProxy()->mipmapped()
                            : false;

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
            SkSurfaceCharacterization::Textureable(SkToBool(readSurfaceView.asTextureProxy())),
            SkSurfaceCharacterization::MipMapped(mipmapped),
            SkSurfaceCharacterization::UsesGLFBO0(usesGLFBO0),
            SkSurfaceCharacterization::VkRTSupportsInputAttachment(vkRTSupportsInputAttachment),
            SkSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
            isProtected,
            this->props());
    return true;
}

void SkSurface_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                           const SkSamplingOptions& sampling, const SkPaint* paint) {
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
        sk_sp<SkImage> image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(canvasContext),
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

bool SkSurface_Gpu::onIsCompatible(const SkSurfaceCharacterization& characterization) const {
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
            GrMipmapped::kNo == targetView.asTextureProxy()->mipmapped()) {
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
           characterization.backendFormat() == format &&
           characterization.width() == ii.width() &&
           characterization.height() == ii.height() &&
           characterization.colorType() == ii.colorType() &&
           characterization.sampleCount() == numSamples &&
           SkColorSpace::Equals(characterization.colorSpace(), ii.colorInfo().colorSpace()) &&
           characterization.isProtected() == isProtected &&
           characterization.surfaceProps() == fDevice->surfaceProps();
}

bool SkSurface_Gpu::onDraw(sk_sp<const SkDeferredDisplayList> ddl, SkIPoint offset) {
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    GrSurfaceProxyView view = fDevice->readSurfaceView();

    direct->priv().createDDLTask(std::move(ddl), view.asRenderTargetProxyRef(), offset);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext* rContext,
                                             const SkSurfaceCharacterization& c,
                                             SkBudgeted budgeted) {
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

    auto device = rContext->priv().createDevice(budgeted, c.imageInfo(), SkBackingFit::kExact,
                                                c.sampleCount(), GrMipmapped(c.isMipMapped()),
                                                c.isProtected(), c.origin(), c.surfaceProps(),
                                                skgpu::BaseDevice::kClear_InitContents);
    if (!device) {
        return nullptr;
    }

    sk_sp<SkSurface> result = sk_make_sp<SkSurface_Gpu>(std::move(device));
#ifdef SK_DEBUG
    if (result) {
        SkASSERT(result->isCompatible(c));
    }
#endif

    return result;
}

static bool validate_backend_texture(const GrCaps* caps, const GrBackendTexture& tex,
                                     int sampleCnt, GrColorType grCT,
                                     bool texturable) {
    if (!tex.isValid()) {
        return false;
    }

    GrBackendFormat backendFormat = tex.getBackendFormat();
    if (!backendFormat.isValid()) {
        return false;
    }

    if (!caps->areColorTypeAndFormatCompatible(grCT, backendFormat)) {
        return false;
    }

    if (!caps->isFormatAsColorTypeRenderable(grCT, backendFormat, sampleCnt)) {
        return false;
    }

    if (texturable && !caps->isFormatTexturable(backendFormat)) {
        return false;
    }

    return true;
}

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext* rContext, SkBudgeted budgeted,
                                             const SkImageInfo& info, int sampleCount,
                                             GrSurfaceOrigin origin, const SkSurfaceProps* props,
                                             bool shouldCreateWithMips) {
    if (!rContext) {
        return nullptr;
    }
    sampleCount = std::max(1, sampleCount);
    GrMipmapped mipMapped = shouldCreateWithMips ? GrMipmapped::kYes : GrMipmapped::kNo;

    if (!rContext->priv().caps()->mipmapSupport()) {
        mipMapped = GrMipmapped::kNo;
    }

    auto device = rContext->priv().createDevice(budgeted, info, SkBackingFit::kExact,
                                                sampleCount, mipMapped, GrProtected::kNo, origin,
                                                SkSurfacePropsCopyOrDefault(props),
                                                skgpu::BaseDevice::kClear_InitContents);
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrRecordingContext* rContext,
                                                   const GrBackendTexture& tex,
                                                   GrSurfaceOrigin origin,
                                                   int sampleCnt,
                                                   SkColorType colorType,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* props,
                                                   SkSurface::TextureReleaseProc textureReleaseProc,
                                                   SkSurface::ReleaseContext releaseContext) {
    auto releaseHelper = GrRefCntedCallback::Make(textureReleaseProc, releaseContext);

    if (!rContext) {
        return nullptr;
    }
    sampleCnt = std::max(1, sampleCnt);

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(rContext->priv().caps(), colorType,
                                                                tex.getBackendFormat());
    if (grColorType == GrColorType::kUnknown) {
        return nullptr;
    }

    if (!validate_backend_texture(rContext->priv().caps(), tex, sampleCnt, grColorType, true)) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy(rContext->priv().proxyProvider()->wrapRenderableBackendTexture(
            tex, sampleCnt, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return nullptr;
    }

    auto device = rContext->priv().createDevice(grColorType, std::move(proxy),
                                                std::move(colorSpace), origin,
                                                SkSurfacePropsCopyOrDefault(props),
                                                skgpu::BaseDevice::kUninit_InitContents);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

bool SkSurface_Gpu::onReplaceBackendTexture(const GrBackendTexture& backendTexture,
                                            GrSurfaceOrigin origin,
                                            ContentChangeMode mode,
                                            TextureReleaseProc releaseProc,
                                            ReleaseContext releaseContext) {
    auto releaseHelper = GrRefCntedCallback::Make(releaseProc, releaseContext);

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
    if (!validate_backend_texture(rContext->priv().caps(), backendTexture,
                                  sampleCnt, grColorType, true)) {
        return false;
    }

    sk_sp<SkColorSpace> colorSpace = fDevice->imageInfo().refColorSpace();

    SkASSERT(sampleCnt > 0);
    sk_sp<GrTextureProxy> proxy(rContext->priv().proxyProvider()->wrapRenderableBackendTexture(
            backendTexture, sampleCnt, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return false;
    }

    return fDevice->replaceBackingProxy(mode, sk_ref_sp(proxy->asRenderTargetProxy()), grColorType,
                                        std::move(colorSpace), origin, this->props());
}

bool validate_backend_render_target(const GrCaps* caps, const GrBackendRenderTarget& rt,
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

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrRecordingContext* rContext,
                                                        const GrBackendRenderTarget& rt,
                                                        GrSurfaceOrigin origin,
                                                        SkColorType colorType,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props,
                                                        SkSurface::RenderTargetReleaseProc relProc,
                                                        SkSurface::ReleaseContext releaseContext) {
    auto releaseHelper = GrRefCntedCallback::Make(relProc, releaseContext);

    if (!rContext) {
        return nullptr;
    }

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(rContext->priv().caps(), colorType,
                                                                rt.getBackendFormat());
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

    auto device = rContext->priv().createDevice(grColorType, std::move(proxy),
                                                std::move(colorSpace), origin,
                                                SkSurfacePropsCopyOrDefault(props),
                                                skgpu::BaseDevice::kUninit_InitContents);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
sk_sp<SkSurface> SkSurface::MakeFromAHardwareBuffer(GrDirectContext* dContext,
                                                    AHardwareBuffer* hardwareBuffer,
                                                    GrSurfaceOrigin origin,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkSurfaceProps* surfaceProps) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT)) {
        return nullptr;
    }

    bool isTextureable = SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE);

    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(dContext,
                                                                             hardwareBuffer,
                                                                             bufferDesc.format,
                                                                             true);
    if (!backendFormat.isValid()) {
        return nullptr;
    }

    if (isTextureable) {
        GrAHardwareBufferUtils::DeleteImageProc deleteImageProc = nullptr;
        GrAHardwareBufferUtils::UpdateImageProc updateImageProc = nullptr;
        GrAHardwareBufferUtils::TexImageCtx deleteImageCtx = nullptr;

        bool isProtectedContent =
                SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);

        GrBackendTexture backendTexture =
                GrAHardwareBufferUtils::MakeBackendTexture(dContext, hardwareBuffer,
                                                           bufferDesc.width, bufferDesc.height,
                                                           &deleteImageProc, &updateImageProc,
                                                           &deleteImageCtx, isProtectedContent,
                                                           backendFormat, true);
        if (!backendTexture.isValid()) {
            return nullptr;
        }

        SkColorType colorType =
                GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

        sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(dContext, backendTexture,
                origin, 0, colorType, std::move(colorSpace), surfaceProps, deleteImageProc,
                deleteImageCtx);

        if (!surface) {
            SkASSERT(deleteImageProc);
            deleteImageProc(deleteImageCtx);
        }

        return surface;
    } else {
        return nullptr;
    }
}
#endif

void SkSurface::flushAndSubmit(bool syncCpu) {
    this->flush(BackendSurfaceAccess::kNoAccess, GrFlushInfo());

    auto direct = GrAsDirectContext(this->recordingContext());
    if (direct) {
        direct->submit(syncCpu);
    }
}

#endif
