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
#include "src/gpu/GrAHardwareBufferUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/SkGpuDevice.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkSurface_Base.h"

#if SK_SUPPORT_GPU

SkSurface_Gpu::SkSurface_Gpu(sk_sp<SkGpuDevice> device)
    : INHERITED(device->width(), device->height(), &device->surfaceProps())
    , fDevice(std::move(device)) {
    SkASSERT(fDevice->surfaceDrawContext()->asSurfaceProxy()->priv().isExact());
}

SkSurface_Gpu::~SkSurface_Gpu() {
}

GrRecordingContext* SkSurface_Gpu::onGetRecordingContext() {
    return fDevice->recordingContext();
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

    GrSurfaceDrawContext* sdc = surface->getDevice()->surfaceDrawContext();
    dContext->priv().flushSurface(sdc->asSurfaceProxy());

    // Grab the render target *after* firing notifications, as it may get switched if CoW kicks in.
    sdc = surface->getDevice()->surfaceDrawContext();
    return sdc->accessRenderTarget();
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
    int sampleCount = fDevice->surfaceDrawContext()->numSamples();
    GrSurfaceOrigin origin = fDevice->surfaceDrawContext()->origin();
    // TODO: Make caller specify this (change virtual signature of onNewSurface).
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fDevice->recordingContext(), kBudgeted, info, sampleCount,
                                       origin, &this->props());
}

sk_sp<SkImage> SkSurface_Gpu::onNewImageSnapshot(const SkIRect* subset) {
    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();
    if (!sdc) {
        return nullptr;
    }

    auto rContext = fDevice->recordingContext();

    if (!sdc->asSurfaceProxy()) {
        return nullptr;
    }

    GrSurfaceProxyView srcView = sdc->readSurfaceView();

    SkBudgeted budgeted = sdc->asSurfaceProxy()->isBudgeted();

    if (subset || !srcView.asTextureProxy() || sdc->refsWrappedObjects()) {
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
        auto rect = subset ? *subset : SkIRect::MakeSize(sdc->dimensions());
        srcView = GrSurfaceProxyView::Copy(rContext, std::move(srcView), sdc->mipmapped(), rect,
                                           SkBackingFit::kExact, budgeted);
    }

    const SkImageInfo info = fDevice->imageInfo();
    sk_sp<SkImage> image;
    if (srcView.asTextureProxy()) {
        // The surfaceDrawContext coming out of SkGpuDevice should always be exact and the
        // above copy creates a kExact surfaceContext.
        SkASSERT(srcView.proxy()->priv().isExact());
        image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(rContext), kNeedNewImageUniqueID,
                                        std::move(srcView), info.colorType(), info.alphaType(),
                                        info.refColorSpace());
    }
    return image;
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
    auto* sdc = this->fDevice->surfaceDrawContext();
    // Context TODO: Elevate direct context requirement to public API.
    auto dContext = sdc->recordingContext()->asDirectContext();
    if (!dContext) {
        return;
    }
    sdc->asyncRescaleAndReadPixels(dContext, info, srcRect, rescaleGamma, rescaleMode, callback,
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
    auto* sdc = this->fDevice->surfaceDrawContext();
    // Context TODO: Elevate direct context requirement to public API.
    auto dContext = sdc->recordingContext()->asDirectContext();
    if (!dContext) {
        return;
    }
    sdc->asyncRescaleAndReadPixelsYUV420(dContext,
                                         yuvColorSpace,
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
    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

    // are we sharing our backing proxy with the image? Note this call should never create a new
    // image because onCopyOnWrite is only called when there is a cached image.
    sk_sp<SkImage> image = this->refCachedImage();
    SkASSERT(image);

    if (static_cast<SkImage_Gpu*>(image.get())->surfaceMustCopyOnWrite(sdc->asSurfaceProxy())) {
        fDevice->replaceSurfaceDrawContext(mode);
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Gpu::onDiscard();
    }
}

void SkSurface_Gpu::onDiscard() { fDevice->surfaceDrawContext()->discard(); }

GrSemaphoresSubmitted SkSurface_Gpu::onFlush(BackendSurfaceAccess access, const GrFlushInfo& info,
                                             const GrBackendSurfaceMutableState* newState) {

    auto dContext = fDevice->recordingContext()->asDirectContext();
    if (!dContext) {
        return GrSemaphoresSubmitted::kNo;
    }

    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

    return dContext->priv().flushSurface(sdc->asSurfaceProxy(), access, info, newState);
}

bool SkSurface_Gpu::onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
                           bool deleteSemaphoresAfterWait) {
    return fDevice->wait(numSemaphores, waitSemaphores, deleteSemaphoresAfterWait);
}

bool SkSurface_Gpu::onCharacterize(SkSurfaceCharacterization* characterization) const {
    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    size_t maxResourceBytes = direct->getResourceCacheLimit();

    bool mipmapped = sdc->asTextureProxy() ? GrMipmapped::kYes == sdc->asTextureProxy()->mipmapped()
                                           : false;

    SkColorType ct = GrColorTypeToSkColorType(sdc->colorInfo().colorType());
    if (ct == kUnknown_SkColorType) {
        return false;
    }

    bool usesGLFBO0 = sdc->asRenderTargetProxy()->glRTFBOIDIs0();
    // We should never get in the situation where we have a texture render target that is also
    // backend by FBO 0.
    SkASSERT(!usesGLFBO0 || !SkToBool(sdc->asTextureProxy()));

    bool vkRTSupportsInputAttachment = sdc->asRenderTargetProxy()->supportsVkInputAttachment();

    SkImageInfo ii = SkImageInfo::Make(sdc->width(), sdc->height(), ct, kPremul_SkAlphaType,
                                       sdc->colorInfo().refColorSpace());

    GrBackendFormat format = sdc->asSurfaceProxy()->backendFormat();

    characterization->set(
            direct->threadSafeProxy(),
            maxResourceBytes,
            ii,
            format,
            sdc->origin(),
            sdc->numSamples(),
            SkSurfaceCharacterization::Textureable(SkToBool(sdc->asTextureProxy())),
            SkSurfaceCharacterization::MipMapped(mipmapped),
            SkSurfaceCharacterization::UsesGLFBO0(usesGLFBO0),
            SkSurfaceCharacterization::VkRTSupportsInputAttachment(vkRTSupportsInputAttachment),
            SkSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
            sdc->asRenderTargetProxy()->isProtected(),
            this->props());
    return true;
}

void SkSurface_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                           const SkSamplingOptions& sampling, const SkPaint* paint) {
    // If the dst is also GPU we try to not force a new image snapshot (by calling the base class
    // onDraw) since that may not always perform the copy-on-write optimization.
    auto tryDraw = [&] {
        auto surfaceContext = fDevice->recordingContext();
        auto canvasContext = canvas->recordingContext()->asDirectContext();
        if (!canvasContext) {
            return false;
        }
        if (canvasContext->priv().contextID() != surfaceContext->priv().contextID()) {
            return false;
        }
        GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();
        if (!sdc) {
            return false;
        }
        sk_sp<GrTextureProxy> srcProxy = sdc->asTextureProxyRef();
        if (!srcProxy) {
            return false;
        }
        // Possibly we could skip making an image here if SkGpuDevice exposed a lower level way
        // of drawing a texture proxy.
        const SkImageInfo info = fDevice->imageInfo();
        GrSurfaceProxyView view(std::move(srcProxy), sdc->origin(), sdc->readSwizzle());
        sk_sp<SkImage> image;
        image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(canvasContext), kNeedNewImageUniqueID,
                                        std::move(view), info.colorType(), info.alphaType(),
                                        info.refColorSpace());
        canvas->drawImage(image.get(), x, y, sampling, paint);
        return true;
    };
    if (!tryDraw()) {
        INHERITED::onDraw(canvas, x, y, sampling, paint);
    }
}

bool SkSurface_Gpu::onIsCompatible(const SkSurfaceCharacterization& characterization) const {
    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

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

    // As long as the current state if the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    // DDL TODO: should we just remove the resource check and ignore the cache limits on playback?
    size_t maxResourceBytes = direct->getResourceCacheLimit();

    if (characterization.isTextureable()) {
        if (!sdc->asTextureProxy()) {
            // If the characterization was textureable we require the replay dest to also be
            // textureable. If the characterized surface wasn't textureable we allow the replay
            // dest to be textureable.
            return false;
        }

        if (characterization.isMipMapped() &&
            GrMipmapped::kNo == sdc->asTextureProxy()->mipmapped()) {
            // Fail if the DDL's surface was mipmapped but the replay surface is not.
            // Allow drawing to proceed if the DDL was not mipmapped but the replay surface is.
            return false;
        }
    }

    if (characterization.usesGLFBO0() != sdc->asRenderTargetProxy()->glRTFBOIDIs0()) {
        // FBO0-ness effects how MSAA and window rectangles work. If the characterization was
        // tagged as FBO0 it would never have been allowed to use window rectangles. If MSAA
        // was also never used then a DDL recorded with this characterization should be replayable
        // on a non-FBO0 surface.
        if (!characterization.usesGLFBO0() || characterization.sampleCount() > 1) {
            return false;
        }
    }

    SkColorType rtcColorType = GrColorTypeToSkColorType(sdc->colorInfo().colorType());
    if (rtcColorType == kUnknown_SkColorType) {
        return false;
    }

    GrProtected isProtected = sdc->asSurfaceProxy()->isProtected();

    return characterization.contextInfo() &&
           characterization.contextInfo()->priv().matches(direct) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == sdc->origin() &&
           characterization.backendFormat() == sdc->asSurfaceProxy()->backendFormat() &&
           characterization.width() == sdc->width() && characterization.height() == sdc->height() &&
           characterization.colorType() == rtcColorType &&
           characterization.sampleCount() == sdc->numSamples() &&
           SkColorSpace::Equals(characterization.colorSpace(), sdc->colorInfo().colorSpace()) &&
           characterization.isProtected() == isProtected &&
           characterization.surfaceProps() == sdc->surfaceProps();
}

bool SkSurface_Gpu::onDraw(sk_sp<const SkDeferredDisplayList> ddl, SkIPoint offset) {
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    direct->priv().createDDLTask(std::move(ddl), sdc->asRenderTargetProxyRef(), offset);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext* context,
                                             const SkSurfaceCharacterization& c,
                                             SkBudgeted budgeted) {
    if (!context || !c.isValid()) {
        return nullptr;
    }

    if (c.usesGLFBO0()) {
        // If we are making the surface we will never use FBO0.
        return nullptr;
    }

    if (c.vulkanSecondaryCBCompatible()) {
        return nullptr;
    }

    GrColorType grColorType = SkColorTypeToGrColorType(c.colorType());

    auto sdc = GrSurfaceDrawContext::Make(
            context, grColorType, c.refColorSpace(), SkBackingFit::kExact,
            c.dimensions(), c.sampleCount(), GrMipmapped(c.isMipMapped()), c.isProtected(),
            c.origin(), budgeted, &c.surfaceProps());
    if (!sdc) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context, std::move(sdc),
                                                SkGpuDevice::kClear_InitContents));
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

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext* ctx, SkBudgeted budgeted,
                                             const SkImageInfo& info, int sampleCount,
                                             GrSurfaceOrigin origin, const SkSurfaceProps* props,
                                             bool shouldCreateWithMips) {
    if (!ctx) {
        return nullptr;
    }
    sampleCount = std::max(1, sampleCount);
    GrMipmapped mipMapped = shouldCreateWithMips ? GrMipmapped::kYes : GrMipmapped::kNo;

    if (!ctx->priv().caps()->mipmapSupport()) {
        mipMapped = GrMipmapped::kNo;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(
            ctx, budgeted, info, sampleCount, origin, props, mipMapped,
            SkGpuDevice::kClear_InitContents));
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface_Gpu::MakeWrappedRenderTarget(GrRecordingContext* context,
                                                        std::unique_ptr<GrSurfaceDrawContext> sdc) {
    if (!context) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(sdc), SkGpuDevice::kUninit_InitContents);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrRecordingContext* context,
                                                   const GrBackendTexture& tex,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   SkColorType colorType,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* props,
                                                   SkSurface::TextureReleaseProc textureReleaseProc,
                                                   SkSurface::ReleaseContext releaseContext) {
    auto releaseHelper = GrRefCntedCallback::Make(textureReleaseProc, releaseContext);

    if (!context) {
        return nullptr;
    }
    sampleCnt = std::max(1, sampleCnt);

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(context->priv().caps(), colorType,
                                                                tex.getBackendFormat());
    if (grColorType == GrColorType::kUnknown) {
        return nullptr;
    }

    if (!validate_backend_texture(context->priv().caps(), tex, sampleCnt, grColorType, true)) {
        return nullptr;
    }

    auto sdc = GrSurfaceDrawContext::MakeFromBackendTexture(
            context, grColorType, std::move(colorSpace), tex, sampleCnt, origin, props,
            std::move(releaseHelper));
    if (!sdc) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(sdc), SkGpuDevice::kUninit_InitContents);
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

    auto context = this->fDevice->recordingContext();
    if (context->abandoned()) {
        return false;
    }
    if (!backendTexture.isValid()) {
        return false;
    }
    if (backendTexture.width() != this->width() || backendTexture.height() != this->height()) {
        return false;
    }
    auto* oldSDC = fDevice->surfaceDrawContext();
    auto oldProxy = sk_ref_sp(oldSDC->asTextureProxy());
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
    auto colorSpace = sk_ref_sp(oldSDC->colorInfo().colorSpace());
    if (!validate_backend_texture(context->priv().caps(), backendTexture,
                                  sampleCnt, grColorType, true)) {
        return false;
    }
    auto sdc = GrSurfaceDrawContext::MakeFromBackendTexture(
            context, oldSDC->colorInfo().colorType(), std::move(colorSpace), backendTexture,
            sampleCnt, origin, &this->props(), std::move(releaseHelper));
    if (!sdc) {
        return false;
    }
    fDevice->replaceSurfaceDrawContext(std::move(sdc), mode);
    return true;
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

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrRecordingContext* context,
                                                        const GrBackendRenderTarget& rt,
                                                        GrSurfaceOrigin origin,
                                                        SkColorType colorType,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props,
                                                        SkSurface::RenderTargetReleaseProc relProc,
                                                        SkSurface::ReleaseContext releaseContext) {
    auto releaseHelper = GrRefCntedCallback::Make(relProc, releaseContext);

    if (!context) {
        return nullptr;
    }

    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(context->priv().caps(), colorType,
                                                                rt.getBackendFormat());
    if (grColorType == GrColorType::kUnknown) {
        return nullptr;
    }

    if (!validate_backend_render_target(context->priv().caps(), rt, grColorType)) {
        return nullptr;
    }

    auto sdc = GrSurfaceDrawContext::MakeFromBackendRenderTarget(context,
                                                                 grColorType,
                                                                 std::move(colorSpace),
                                                                 rt,
                                                                 origin,
                                                                 props,
                                                                 std::move(releaseHelper));
    if (!sdc) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(sdc), SkGpuDevice::kUninit_InitContents);
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
