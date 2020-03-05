/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkImagePriv.h"
#include "src/gpu/GrAHardwareBufferUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrRenderTargetProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/SkGpuDevice.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkSurface_Base.h"
#include "src/image/SkSurface_Gpu.h"

#if SK_SUPPORT_GPU

SkSurface_Gpu::SkSurface_Gpu(sk_sp<SkGpuDevice> device)
    : INHERITED(device->width(), device->height(), &device->surfaceProps())
    , fDevice(std::move(device)) {
    SkASSERT(fDevice->accessRenderTargetContext()->asSurfaceProxy()->priv().isExact());
}

SkSurface_Gpu::~SkSurface_Gpu() {
}

static GrRenderTarget* prepare_rt_for_external_access(SkSurface_Gpu* surface,
                                                      SkSurface::BackendHandleAccess access) {
    switch (access) {
        case SkSurface::kFlushRead_BackendHandleAccess:
            break;
        case SkSurface::kFlushWrite_BackendHandleAccess:
        case SkSurface::kDiscardWrite_BackendHandleAccess:
            // for now we don't special-case on Discard, but we may in the future.
            surface->notifyContentWillChange(SkSurface::kRetain_ContentChangeMode);
            break;
    }

    // Grab the render target *after* firing notifications, as it may get switched if CoW kicks in.
    surface->getDevice()->flush(SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo());
    GrRenderTargetContext* rtc = surface->getDevice()->accessRenderTargetContext();
    return rtc->accessRenderTarget();
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
    int sampleCount = fDevice->accessRenderTargetContext()->numSamples();
    GrSurfaceOrigin origin = fDevice->accessRenderTargetContext()->origin();
    // TODO: Make caller specify this (change virtual signature of onNewSurface).
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fDevice->context(), kBudgeted, info, sampleCount,
                                       origin, &this->props());
}

sk_sp<SkImage> SkSurface_Gpu::onNewImageSnapshot(const SkIRect* subset) {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    if (!rtc) {
        return nullptr;
    }

    GrContext* ctx = fDevice->context();

    if (!rtc->asSurfaceProxy()) {
        return nullptr;
    }

    SkBudgeted budgeted = rtc->asSurfaceProxy()->isBudgeted();

    GrSurfaceProxyView srcView;
    if (subset) {
        srcView = GrSurfaceProxy::Copy(ctx, rtc->asSurfaceProxy(), rtc->origin(),
                                       rtc->colorInfo().colorType(), rtc->mipMapped(), *subset,
                                       SkBackingFit::kExact, budgeted);
    } else if (!rtc->asTextureProxy() || rtc->priv().refsWrappedObjects()) {
        // If the original render target is a buffer originally created by the client, then we don't
        // want to ever retarget the SkSurface at another buffer we create. Force a copy now to
        // avoid copy-on-write.
        srcView = GrSurfaceProxy::Copy(ctx, rtc->asSurfaceProxy(), rtc->origin(),
                                       rtc->colorInfo().colorType(), rtc->mipMapped(),
                                       SkBackingFit::kExact, budgeted);
    } else {
        srcView = rtc->readSurfaceView();
    }

    const SkImageInfo info = fDevice->imageInfo();
    sk_sp<SkImage> image;
    if (srcView.asTextureProxy()) {
        // The renderTargetContext coming out of SkGpuDevice should always be exact and the
        // above copy creates a kExact surfaceContext.
        SkASSERT(srcView.proxy()->priv().isExact());
        image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID, std::move(srcView),
                                        info.colorType(), info.alphaType(), info.refColorSpace());
    }
    return image;
}

void SkSurface_Gpu::onWritePixels(const SkPixmap& src, int x, int y) {
    fDevice->writePixels(src, x, y);
}

void SkSurface_Gpu::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                                const SkIRect& srcRect,
                                                RescaleGamma rescaleGamma,
                                                SkFilterQuality rescaleQuality,
                                                ReadPixelsCallback callback,
                                                ReadPixelsContext context) {
    auto* rtc = this->fDevice->accessRenderTargetContext();
    rtc->asyncRescaleAndReadPixels(info, srcRect, rescaleGamma, rescaleQuality, callback, context);
}

void SkSurface_Gpu::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                      sk_sp<SkColorSpace> dstColorSpace,
                                                      const SkIRect& srcRect,
                                                      const SkISize& dstSize,
                                                      RescaleGamma rescaleGamma,
                                                      SkFilterQuality rescaleQuality,
                                                      ReadPixelsCallback callback,
                                                      ReadPixelsContext context) {
    auto* rtc = this->fDevice->accessRenderTargetContext();
    rtc->asyncRescaleAndReadPixelsYUV420(yuvColorSpace,
                                         std::move(dstColorSpace),
                                         srcRect,
                                         dstSize,
                                         rescaleGamma,
                                         rescaleQuality,
                                         callback,
                                         context);
}

// Create a new render target and, if necessary, copy the contents of the old
// render target into it. Note that this flushes the SkGpuDevice but
// doesn't force an OpenGL flush.
void SkSurface_Gpu::onCopyOnWrite(ContentChangeMode mode) {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();

    // are we sharing our backing proxy with the image? Note this call should never create a new
    // image because onCopyOnWrite is only called when there is a cached image.
    sk_sp<SkImage> image(this->refCachedImage());
    SkASSERT(image);

    GrSurfaceProxy* imageProxy = ((SkImage_Base*) image.get())->peekProxy();
    SkASSERT(imageProxy);

    if (rtc->asSurfaceProxy()->underlyingUniqueID() == imageProxy->underlyingUniqueID()) {
        fDevice->replaceRenderTargetContext(mode);
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Gpu::onDiscard();
    }
}

void SkSurface_Gpu::onDiscard() {
    fDevice->accessRenderTargetContext()->discard();
}

GrSemaphoresSubmitted SkSurface_Gpu::onFlush(BackendSurfaceAccess access,
                                             const GrFlushInfo& info) {
    return fDevice->flush(access, info);
}

bool SkSurface_Gpu::onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores) {
    return fDevice->wait(numSemaphores, waitSemaphores);
}

bool SkSurface_Gpu::onCharacterize(SkSurfaceCharacterization* characterization) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    size_t maxResourceBytes = ctx->getResourceCacheLimit();

    bool mipmapped = rtc->asTextureProxy() ? GrMipMapped::kYes == rtc->asTextureProxy()->mipMapped()
                                           : false;

    SkColorType ct = GrColorTypeToSkColorType(rtc->colorInfo().colorType());
    if (ct == kUnknown_SkColorType) {
        return false;
    }

    bool usesGLFBO0 = rtc->asRenderTargetProxy()->rtPriv().glRTFBOIDIs0();
    // We should never get in the situation where we have a texture render target that is also
    // backend by FBO 0.
    SkASSERT(!usesGLFBO0 || !SkToBool(rtc->asTextureProxy()));

    SkImageInfo ii = SkImageInfo::Make(rtc->width(), rtc->height(), ct, kPremul_SkAlphaType,
                                       rtc->colorInfo().refColorSpace());

    GrBackendFormat format = rtc->asSurfaceProxy()->backendFormat();

    characterization->set(ctx->threadSafeProxy(), maxResourceBytes, ii, format,
                          rtc->origin(), rtc->numSamples(),
                          SkSurfaceCharacterization::Textureable(SkToBool(rtc->asTextureProxy())),
                          SkSurfaceCharacterization::MipMapped(mipmapped),
                          SkSurfaceCharacterization::UsesGLFBO0(usesGLFBO0),
                          SkSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
                          rtc->asRenderTargetProxy()->isProtected(),
                          this->props());
    return true;
}

void SkSurface_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) {
    // If the dst is also GPU we try to not force a new image snapshot (by calling the base class
    // onDraw) since that may not always perform the copy-on-write optimization.
    auto tryDraw = [&] {
        SkASSERT(fDevice->context()->priv().asDirectContext());
        GrContext* context = fDevice->context();
        GrContext* canvasContext = canvas->getGrContext();
        if (!canvasContext) {
            return false;
        }
        if (!canvasContext->priv().asDirectContext() ||
            canvasContext->priv().contextID() != context->priv().contextID()) {
            return false;
        }
        GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
        if (!rtc) {
            return false;
        }
        sk_sp<GrTextureProxy> srcProxy = rtc->asTextureProxyRef();
        if (!srcProxy) {
            return false;
        }
        // Possibly we could skip making an image here if SkGpuDevice exposed a lower level way
        // of drawing a texture proxy.
        const SkImageInfo info = fDevice->imageInfo();
        GrSurfaceProxyView view(std::move(srcProxy), rtc->origin(), rtc->readSwizzle());
        sk_sp<SkImage> image;
        image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, std::move(view),
                                        info.colorType(), info.alphaType(), info.refColorSpace());
        canvas->drawImage(image, x, y, paint);
        return true;
    };
    if (!tryDraw()) {
        INHERITED::onDraw(canvas, x, y, paint);
    }
}

bool SkSurface_Gpu::onIsCompatible(const SkSurfaceCharacterization& characterization) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    if (!characterization.isValid()) {
        return false;
    }

    if (characterization.vulkanSecondaryCBCompatible()) {
        return false;
    }

    // As long as the current state if the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    // DDL TODO: should we just remove the resource check and ignore the cache limits on playback?
    size_t maxResourceBytes = ctx->getResourceCacheLimit();

    if (characterization.isTextureable()) {
        if (!rtc->asTextureProxy()) {
            // If the characterization was textureable we require the replay dest to also be
            // textureable. If the characterized surface wasn't textureable we allow the replay
            // dest to be textureable.
            return false;
        }

        if (characterization.isMipMapped() &&
            GrMipMapped::kNo == rtc->asTextureProxy()->mipMapped()) {
            // Fail if the DDL's surface was mipmapped but the replay surface is not.
            // Allow drawing to proceed if the DDL was not mipmapped but the replay surface is.
            return false;
        }
    }

    if (characterization.usesGLFBO0() != rtc->asRenderTargetProxy()->rtPriv().glRTFBOIDIs0()) {
        // FBO0-ness effects how MSAA and window rectangles work. If the characterization was
        // tagged as FBO0 it would never have been allowed to use window rectangles. If MSAA
        // was also never used then a DDL recorded with this characterization should be replayable
        // on a non-FBO0 surface.
        if (!characterization.usesGLFBO0() || characterization.sampleCount() > 1) {
            return false;
        }
    }

    SkColorType rtcColorType = GrColorTypeToSkColorType(rtc->colorInfo().colorType());
    if (rtcColorType == kUnknown_SkColorType) {
        return false;
    }

    GrProtected isProtected = rtc->asSurfaceProxy()->isProtected();

    return characterization.contextInfo() && characterization.contextInfo()->priv().matches(ctx) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == rtc->origin() &&
           characterization.backendFormat() == rtc->asSurfaceProxy()->backendFormat() &&
           characterization.width() == rtc->width() && characterization.height() == rtc->height() &&
           characterization.colorType() == rtcColorType &&
           characterization.sampleCount() == rtc->numSamples() &&
           SkColorSpace::Equals(characterization.colorSpace(), rtc->colorInfo().colorSpace()) &&
           characterization.isProtected() == isProtected &&
           characterization.surfaceProps() == rtc->surfaceProps();
}

bool SkSurface_Gpu::onDraw(const SkDeferredDisplayList* ddl) {
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    ctx->priv().copyRenderTasksFromDDL(ddl, rtc->asRenderTargetProxy());
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

    auto rtc = GrRenderTargetContext::Make(
            context, grColorType, c.refColorSpace(), SkBackingFit::kExact,
            {c.width(), c.height()}, c.sampleCount(), GrMipMapped(c.isMipMapped()), c.isProtected(),
            c.origin(), budgeted, &c.surfaceProps());
    if (!rtc) {
        return nullptr;
    }

    // CONTEXT TODO: remove this use of 'backdoor' to create an SkGpuDevice
    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context->priv().backdoor(), std::move(rtc),
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

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrContext* context,
                                                   const SkSurfaceCharacterization& c,
                                                   const GrBackendTexture& backendTexture,
                                                   TextureReleaseProc textureReleaseProc,
                                                   ReleaseContext releaseContext) {
    if (!context || !c.isValid()) {
        return nullptr;
    }

    if (c.usesGLFBO0()) {
        // If we are making the surface we will never use FBO0.
        return nullptr;
    }

    if (!c.isCompatible(backendTexture)) {
        return nullptr;
    }

    GrColorType grCT = SkColorTypeAndFormatToGrColorType(context->priv().caps(), c.colorType(),
                                                         backendTexture.getBackendFormat());
    if (grCT == GrColorType::kUnknown) {
        return nullptr;
    }

    if (!validate_backend_texture(context->priv().caps(), backendTexture,
                                  c.sampleCount(), grCT, true)) {
        return nullptr;
    }

    auto rtc = GrRenderTargetContext::MakeFromBackendTexture(
            context, grCT, c.refColorSpace(), backendTexture, c.sampleCount(), c.origin(),
            &c.surfaceProps(), textureReleaseProc, releaseContext);
    if (!rtc) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(rtc), SkGpuDevice::kUninit_InitContents);
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

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrContext* ctx, SkBudgeted budgeted,
                                             const SkImageInfo& info, int sampleCount,
                                             GrSurfaceOrigin origin, const SkSurfaceProps* props,
                                             bool shouldCreateWithMips) {
    if (!ctx) {
        return nullptr;
    }
    sampleCount = std::max(1, sampleCount);
    GrMipMapped mipMapped = shouldCreateWithMips ? GrMipMapped::kYes : GrMipMapped::kNo;

    if (!ctx->priv().caps()->mipMapSupport()) {
        mipMapped = GrMipMapped::kNo;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(
            ctx, budgeted, info, sampleCount, origin, props, mipMapped,
            SkGpuDevice::kClear_InitContents));
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface_Gpu::MakeWrappedRenderTarget(
        GrContext* context, std::unique_ptr<GrRenderTargetContext> rtc) {
    if (!context) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(rtc), SkGpuDevice::kUninit_InitContents);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrContext* context, const GrBackendTexture& tex,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   SkColorType colorType,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* props,
                                                   SkSurface::TextureReleaseProc textureReleaseProc,
                                                   SkSurface::ReleaseContext releaseContext) {
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

    auto rtc = GrRenderTargetContext::MakeFromBackendTexture(
            context, grColorType, std::move(colorSpace), tex, sampleCnt, origin, props,
            textureReleaseProc, releaseContext);
    if (!rtc) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(rtc), SkGpuDevice::kUninit_InitContents);
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

bool SkSurface_Gpu::onReplaceBackendTexture(const GrBackendTexture& backendTexture,
                                            GrSurfaceOrigin origin, ContentChangeMode mode, TextureReleaseProc releaseProc,
                                            ReleaseContext releaseContext) {
    auto context = this->fDevice->context();
    if (context->abandoned()) {
        return false;
    }
    if (!backendTexture.isValid()) {
        return false;
    }
    if (backendTexture.width() != this->width() || backendTexture.height() != this->height()) {
        return false;
    }
    auto* oldRTC = fDevice->accessRenderTargetContext();
    auto oldProxy = sk_ref_sp(oldRTC->asTextureProxy());
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
    auto colorSpace = sk_ref_sp(oldRTC->colorInfo().colorSpace());
    if (!validate_backend_texture(context->priv().caps(), backendTexture,
                                  sampleCnt, grColorType, true)) {
        return false;
    }
    auto rtc = GrRenderTargetContext::MakeFromBackendTexture(
            context, oldRTC->colorInfo().colorType(), std::move(colorSpace), backendTexture,
            sampleCnt, origin, &this->props(), releaseProc, releaseContext);
    if (!rtc) {
        return false;
    }
    fDevice->replaceRenderTargetContext(std::move(rtc), mode);
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
    return true;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrContext* context,
                                                        const GrBackendRenderTarget& rt,
                                                        GrSurfaceOrigin origin,
                                                        SkColorType colorType,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props,
                                                        SkSurface::RenderTargetReleaseProc relProc,
                                                        SkSurface::ReleaseContext releaseContext) {
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

    auto rtc = GrRenderTargetContext::MakeFromBackendRenderTarget(
            context, grColorType, std::move(colorSpace), rt, origin, props, relProc,
            releaseContext);
    if (!rtc) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(rtc), SkGpuDevice::kUninit_InitContents);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTextureAsRenderTarget(GrContext* context,
                                                                 const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt,
                                                                 SkColorType colorType,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }

    sampleCnt = std::max(1, sampleCnt);
    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(context->priv().caps(), colorType,
                                                                tex.getBackendFormat());
    if (grColorType == GrColorType::kUnknown) {
        return nullptr;
    }
    if (!validate_backend_texture(context->priv().caps(), tex, sampleCnt, grColorType, false)) {
        return nullptr;
    }

    auto rtc = GrRenderTargetContext::MakeFromBackendTextureAsRenderTarget(
            context, grColorType, std::move(colorSpace), tex, sampleCnt, origin, props);
    if (!rtc) {
        return nullptr;
    }

    auto device = SkGpuDevice::Make(context, std::move(rtc), SkGpuDevice::kUninit_InitContents);
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
sk_sp<SkSurface> SkSurface::MakeFromAHardwareBuffer(GrContext* context,
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
    bool isProtectedContent = SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);

    // We currently don't support protected content
    if (isProtectedContent) {
        SkDebugf("We currently don't support protected content on android\n");
        return nullptr;
    }

    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(context,
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

        GrBackendTexture backendTexture =
                GrAHardwareBufferUtils::MakeBackendTexture(context, hardwareBuffer,
                                                           bufferDesc.width, bufferDesc.height,
                                                           &deleteImageProc, &updateImageProc,
                                                           &deleteImageCtx, isProtectedContent,
                                                           backendFormat, true);
        if (!backendTexture.isValid()) {
            return nullptr;
        }

        SkColorType colorType =
                GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

        sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(context, backendTexture,
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

#endif
