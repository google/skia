/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Gpu.h"

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetContextPriv.h"
#include "GrTexture.h"

#include "SkCanvas.h"
#include "SkColorSpace_Base.h"
#include "SkDeferredDisplayList.h"
#include "SkGpuDevice.h"
#include "SkImage_Base.h"
#include "SkImage_Gpu.h"
#include "SkImagePriv.h"
#include "SkSurface_Base.h"
#include "SkSurfaceCharacterization.h"

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
    surface->getDevice()->flush();
    GrRenderTargetContext* rtc = surface->getDevice()->accessRenderTargetContext();
    return rtc->accessRenderTarget();
}

GrBackendObject SkSurface_Gpu::onGetTextureHandle(BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    if (!rt) {
        return 0;
    }
    GrTexture* texture = rt->asTexture();
    if (texture) {
        return texture->getTextureHandle();
    }
    return 0;
}

bool SkSurface_Gpu::onGetRenderTargetHandle(GrBackendObject* obj, BackendHandleAccess access) {
    GrRenderTarget* rt = prepare_rt_for_external_access(this, access);
    if (!rt) {
        return false;
    }
    *obj = rt->getRenderTargetHandle();
    return true;
}

SkCanvas* SkSurface_Gpu::onNewCanvas() {
    SkCanvas::InitFlags flags = SkCanvas::kDefault_InitFlags;
    flags = static_cast<SkCanvas::InitFlags>(flags | SkCanvas::kConservativeRasterClip_InitFlag);

    return new SkCanvas(fDevice.get(), flags);
}

sk_sp<SkSurface> SkSurface_Gpu::onNewSurface(const SkImageInfo& info) {
    int sampleCount = fDevice->accessRenderTargetContext()->numColorSamples();
    GrSurfaceOrigin origin = fDevice->accessRenderTargetContext()->origin();
    // TODO: Make caller specify this (change virtual signature of onNewSurface).
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fDevice->context(), kBudgeted, info, sampleCount,
                                       origin, &this->props());
}

sk_sp<SkImage> SkSurface_Gpu::onNewImageSnapshot() {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    if (!rtc) {
        return nullptr;
    }

    GrContext* ctx = fDevice->context();

    if (!rtc->asSurfaceProxy()) {
        return nullptr;
    }

    SkBudgeted budgeted = rtc->asSurfaceProxy()->isBudgeted();

    sk_sp<GrTextureProxy> srcProxy = rtc->asTextureProxyRef();
    // If the original render target is a buffer originally created by the client, then we don't
    // want to ever retarget the SkSurface at another buffer we create. Force a copy now to avoid
    // copy-on-write.
    if (!srcProxy || rtc->priv().refsWrappedObjects()) {
        SkASSERT(rtc->origin() == rtc->asSurfaceProxy()->origin());

        srcProxy = GrSurfaceProxy::Copy(ctx, rtc->asSurfaceProxy(), rtc->mipMapped(), budgeted);
    }

    const SkImageInfo info = fDevice->imageInfo();
    sk_sp<SkImage> image;
    if (srcProxy) {
        // The renderTargetContext coming out of SkGpuDevice should always be exact and the
        // above copy creates a kExact surfaceContext.
        SkASSERT(srcProxy->priv().isExact());
        image = sk_make_sp<SkImage_Gpu>(ctx, kNeedNewImageUniqueID,
                                        info.alphaType(), std::move(srcProxy),
                                        info.refColorSpace(), budgeted);
    }
    return image;
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
        fDevice->replaceRenderTargetContext(SkSurface::kRetain_ContentChangeMode == mode);
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Gpu::onDiscard();
    }
}

void SkSurface_Gpu::onDiscard() {
    fDevice->accessRenderTargetContext()->discard();
}

GrSemaphoresSubmitted SkSurface_Gpu::onFlush(int numSemaphores,
                                             GrBackendSemaphore signalSemaphores[]) {
    return fDevice->flushAndSignalSemaphores(numSemaphores, signalSemaphores);
}

bool SkSurface_Gpu::onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores) {
    return fDevice->wait(numSemaphores, waitSemaphores);
}

bool SkSurface_Gpu::onCharacterize(SkSurfaceCharacterization* data) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    int maxResourceCount;
    size_t maxResourceBytes;
    ctx->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

    data->set(ctx->threadSafeProxy(), maxResourceCount, maxResourceBytes,
              rtc->origin(), rtc->width(), rtc->height(),
              rtc->colorSpaceInfo().config(), rtc->fsaaType(), rtc->numStencilSamples(),
              rtc->colorSpaceInfo().refColorSpace(), this->props());

    return true;
}

bool SkSurface_Gpu::isCompatible(const SkSurfaceCharacterization& data) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    // As long as the current state if the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    int maxResourceCount;
    size_t maxResourceBytes;
    ctx->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

    return data.contextInfo() && data.contextInfo()->matches(ctx) &&
           data.cacheMaxResourceCount() <= maxResourceCount &&
           data.cacheMaxResourceBytes() <= maxResourceBytes &&
           data.origin() == rtc->origin() && data.width() == rtc->width() &&
           data.height() == rtc->height() && data.config() == rtc->colorSpaceInfo().config() &&
           data.fsaaType() == rtc->fsaaType() && data.stencilCount() == rtc->numStencilSamples() &&
           SkColorSpace::Equals(data.colorSpace(), rtc->colorSpaceInfo().colorSpace()) &&
           data.surfaceProps() == rtc->surfaceProps();
}

bool SkSurface_Gpu::onDraw(SkDeferredDisplayList* dl) {
    if (!this->isCompatible(dl->characterization())) {
        return false;
    }

    // Ultimately need to pass opLists from the DeferredDisplayList on to the
    // SkGpuDevice's renderTargetContext.
    return dl->draw(this);
}


///////////////////////////////////////////////////////////////////////////////

bool SkSurface_Gpu::Valid(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kRGBA_F16_SkColorType:
            return (!info.colorSpace()) || info.colorSpace()->gammaIsLinear();
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return !info.colorSpace() || info.colorSpace()->gammaCloseToSRGB();
        default:
            return !info.colorSpace();
    }
}

bool SkSurface_Gpu::Valid(GrContext* context, GrPixelConfig config, SkColorSpace* colorSpace) {
    switch (config) {
        case kRGBA_half_GrPixelConfig:
            return (!colorSpace) || colorSpace->gammaIsLinear();
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
            return context->caps()->srgbSupport() && colorSpace && colorSpace->gammaCloseToSRGB();
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
            // If we don't have sRGB support, we may get here with a color space. It still needs
            // to be sRGB-like (so that the application will work correctly on sRGB devices.)
            return !colorSpace ||
                (colorSpace->gammaCloseToSRGB() && !context->caps()->srgbSupport());
        default:
            return !colorSpace;
    }
}

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrContext* ctx, SkBudgeted budgeted,
                                             const SkImageInfo& info, int sampleCount,
                                             GrSurfaceOrigin origin, const SkSurfaceProps* props,
                                             bool shouldCreateWithMips) {
    if (!ctx) {
        return nullptr;
    }
    if (!SkSurface_Gpu::Valid(info)) {
        return nullptr;
    }

    GrMipMapped mipMapped = shouldCreateWithMips ? GrMipMapped::kYes : GrMipMapped::kNo;

    if (!ctx->caps()->mipMapSupport()) {
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

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrContext* context, const GrBackendTexture& tex,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    if (!SkSurface_Gpu::Valid(context, tex.config(), colorSpace.get())) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> rtc(context->contextPriv().makeBackendTextureRenderTargetContext(
                                                                    tex,
                                                                    origin,
                                                                    sampleCnt,
                                                                    std::move(colorSpace),
                                                                    props));
    if (!rtc) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context, std::move(rtc), tex.width(), tex.height(),
                                                SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

bool validate_backend_texture(GrContext* ctx, const GrBackendTexture& tex, GrPixelConfig* config,
                              int sampleCnt, SkColorType ct, sk_sp<SkColorSpace> cs,
                              bool texturable) {
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, kPremul_SkAlphaType, cs);

    if (!SkSurface_Gpu::Valid(info)) {
        return false;
    }

    if (!ctx->caps()->validateBackendTexture(tex, ct, config)) {
        return false;
    }

    if (!ctx->caps()->isConfigRenderable(*config, sampleCnt > 0)) {
        return false;
    }

    if (ctx->caps()->getSampleCount(sampleCnt, *config) != sampleCnt) {
        return false;
    }

    if (texturable && !ctx->caps()->isConfigTexturable(*config)) {
        return false;
    }
    return true;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrContext* context, const GrBackendTexture& tex,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   SkColorType colorType,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(context, texCopy, &texCopy.fConfig,
                                  sampleCnt, colorType, colorSpace, true)) {
        return nullptr;
    }

    return MakeFromBackendTexture(context, texCopy, origin, sampleCnt, colorSpace, props);
}

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrContext* context,
                                                        const GrBackendRenderTarget& backendRT,
                                                        GrSurfaceOrigin origin,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    if (!SkSurface_Gpu::Valid(context, backendRT.config(), colorSpace.get())) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> rtc(
        context->contextPriv().makeBackendRenderTargetRenderTargetContext(backendRT,
                                                                          origin,
                                                                          std::move(colorSpace),
                                                                          props));
    if (!rtc) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context, std::move(rtc),
                                                backendRT.width(), backendRT.height(),
                                                SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

bool validate_backend_render_target(GrContext* ctx, const GrBackendRenderTarget& rt,
                                    GrPixelConfig* config, SkColorType ct, sk_sp<SkColorSpace> cs) {
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, kPremul_SkAlphaType, cs);

    if (!SkSurface_Gpu::Valid(info)) {
        return false;
    }

    if (!ctx->caps()->validateBackendRenderTarget(rt, ct, config)) {
        return false;
    }

    if (!ctx->caps()->isConfigRenderable(*config, false)) {
        return false;
    }

    return true;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrContext* context,
                                                        const GrBackendRenderTarget& rt,
                                                        GrSurfaceOrigin origin,
                                                        SkColorType colorType,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    GrBackendRenderTarget rtCopy = rt;
    if (!validate_backend_render_target(context, rtCopy, &rtCopy.fConfig, colorType, colorSpace)) {
        return nullptr;
    }

    return MakeFromBackendRenderTarget(context, rtCopy, origin, colorSpace, props);
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTextureAsRenderTarget(GrContext* context,
                                                                 const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    if (!SkSurface_Gpu::Valid(context, tex.config(), colorSpace.get())) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> rtc(
        context->contextPriv().makeBackendTextureAsRenderTargetRenderTargetContext(
                                                                              tex,
                                                                              origin,
                                                                              sampleCnt,
                                                                              std::move(colorSpace),
                                                                              props));
    if (!rtc) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context, std::move(rtc), tex.width(), tex.height(),
                                                SkGpuDevice::kUninit_InitContents));
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
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(context, texCopy, &texCopy.fConfig,
                                  sampleCnt, colorType, colorSpace, false)) {
        return nullptr;
    }

    return MakeFromBackendTextureAsRenderTarget(context, texCopy, origin, sampleCnt, colorSpace,
                                                props);
}

#endif
