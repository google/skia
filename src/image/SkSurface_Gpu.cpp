/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Gpu.h"
#include "GrBackendSurface.h"
#include "GrCaps.h"
#include "GrContextPriv.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetContextPriv.h"
#include "GrRenderTargetProxyPriv.h"
#include "GrTexture.h"
#include "SkCanvas.h"
#include "SkDeferredDisplayList.h"
#include "SkGpuDevice.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkImage_Gpu.h"
#include "SkSurfaceCharacterization.h"
#include "SkSurface_Base.h"

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

SkCanvas* SkSurface_Gpu::onNewCanvas() {
    SkCanvas::InitFlags flags = SkCanvas::kDefault_InitFlags;
    flags = static_cast<SkCanvas::InitFlags>(flags | SkCanvas::kConservativeRasterClip_InitFlag);

    return new SkCanvas(fDevice, flags);
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
        image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(ctx), kNeedNewImageUniqueID, info.alphaType(),
                                        std::move(srcProxy), info.refColorSpace(), budgeted);
    }
    return image;
}

void SkSurface_Gpu::onWritePixels(const SkPixmap& src, int x, int y) {
    fDevice->writePixels(src, x, y);
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

bool SkSurface_Gpu::onCharacterize(SkSurfaceCharacterization* characterization) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    int maxResourceCount;
    size_t maxResourceBytes;
    ctx->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

    bool mipmapped = rtc->asTextureProxy() ? GrMipMapped::kYes == rtc->asTextureProxy()->mipMapped()
                                           : false;

    // TODO: the addition of colorType to the surfaceContext should remove this calculation
    SkColorType ct;
    if (!GrPixelConfigToColorType(rtc->colorSpaceInfo().config(), &ct)) {
        return false;
    }

    bool usesGLFBO0 = rtc->asRenderTargetProxy()->rtPriv().glRTFBOIDIs0();
    // We should never get in the situation where we have a texture render target that is also
    // backend by FBO 0.
    SkASSERT(!usesGLFBO0 || !SkToBool(rtc->asTextureProxy()));

    SkImageInfo ii = SkImageInfo::Make(rtc->width(), rtc->height(), ct, kPremul_SkAlphaType,
                                       rtc->colorSpaceInfo().refColorSpace());

    characterization->set(ctx->threadSafeProxy(), maxResourceBytes, ii, rtc->origin(),
                          rtc->colorSpaceInfo().config(), rtc->fsaaType(), rtc->numStencilSamples(),
                          SkSurfaceCharacterization::Textureable(SkToBool(rtc->asTextureProxy())),
                          SkSurfaceCharacterization::MipMapped(mipmapped),
                          SkSurfaceCharacterization::UsesGLFBO0(usesGLFBO0), this->props());

    return true;
}

bool SkSurface_Gpu::isCompatible(const SkSurfaceCharacterization& characterization) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    if (!characterization.isValid()) {
        return false;
    }

    // As long as the current state if the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    // DDL TODO: should we just remove the resource check and ignore the cache limits on playback?
    int maxResourceCount;
    size_t maxResourceBytes;
    ctx->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

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
        return false;
    }

    // TODO: the addition of colorType to the surfaceContext should remove this calculation
    SkColorType rtcColorType;
    if (!GrPixelConfigToColorType(rtc->colorSpaceInfo().config(), &rtcColorType)) {
        return false;
    }

    return characterization.contextInfo() && characterization.contextInfo()->matches(ctx) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == rtc->origin() &&
           characterization.config() == rtc->colorSpaceInfo().config() &&
           characterization.width() == rtc->width() &&
           characterization.height() == rtc->height() &&
           characterization.colorType() == rtcColorType &&
           characterization.fsaaType() == rtc->fsaaType() &&
           characterization.stencilCount() == rtc->numStencilSamples() &&
           SkColorSpace::Equals(characterization.colorSpace(),
                                rtc->colorSpaceInfo().colorSpace()) &&
           characterization.surfaceProps() == rtc->surfaceProps();
}

bool SkSurface_Gpu::onDraw(const SkDeferredDisplayList* ddl) {
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    ctx->contextPriv().copyOpListsFromDDL(ddl, rtc->asRenderTargetProxy());
    return true;
}


///////////////////////////////////////////////////////////////////////////////

bool SkSurface_Gpu::Valid(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kRGBA_F16_SkColorType:
        case kRGBA_F32_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        default:
            return !info.colorSpace();
    }
}

bool SkSurface_Gpu::Valid(const GrCaps* caps, GrPixelConfig config, SkColorSpace* colorSpace) {
    switch (config) {
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
            return caps->srgbSupport();
        case kRGBA_half_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
            return true;
        default:
            return !colorSpace;
    }
}

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrContext* context,
                                             const SkSurfaceCharacterization& c,
                                             SkBudgeted budgeted) {
    if (!context || !c.isValid()) {
        return nullptr;
    }

    if (c.usesGLFBO0()) {
        // If we are making the surface we will never use FBO0.
        return nullptr;
    }

    if (!SkSurface_Gpu::Valid(context->contextPriv().caps(), c.config(), c.colorSpace())) {
        return nullptr;
    }

    // In order to ensure compatibility we have to match the backend format (i.e. the GrPixelConfig
    // of the characterization)
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = c.width();
    desc.fHeight = c.height();
    desc.fConfig = c.config();
    desc.fSampleCnt = c.stencilCount();

    sk_sp<GrSurfaceContext> sc(
            context->contextPriv().makeDeferredSurfaceContext(desc, c.origin(),
                                                              GrMipMapped(c.isMipMapped()),
                                                              SkBackingFit::kExact, budgeted,
                                                              c.refColorSpace(),
                                                              &c.surfaceProps()));
    if (!sc || !sc->asRenderTargetContext()) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context, sk_ref_sp(sc->asRenderTargetContext()),
                                                c.width(), c.height(),
                                                SkGpuDevice::kClear_InitContents));
    if (!device) {
        return nullptr;
    }

    sk_sp<SkSurface> s = sk_make_sp<SkSurface_Gpu>(std::move(device));
#ifdef SK_DEBUG
    if (s) {
        SkSurface_Gpu* gpuSurface = static_cast<SkSurface_Gpu*>(s.get());
        SkASSERT(gpuSurface->isCompatible(c));
    }
#endif

    return s;
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
    sampleCount = SkTMax(1, sampleCount);
    GrMipMapped mipMapped = shouldCreateWithMips ? GrMipMapped::kYes : GrMipMapped::kNo;

    if (!ctx->contextPriv().caps()->mipMapSupport()) {
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

sk_sp<SkSurface> SkSurface_Gpu::MakeWrappedRenderTarget(GrContext* context,
                                                        sk_sp<GrRenderTargetContext> rtc) {
    if (!context) {
        return nullptr;
    }

    int w = rtc->width();
    int h = rtc->height();
    sk_sp<SkGpuDevice> device(
            SkGpuDevice::Make(context, std::move(rtc), w, h, SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

bool validate_backend_texture(GrContext* ctx, const GrBackendTexture& tex, GrPixelConfig* config,
                              int sampleCnt, SkColorType ct, sk_sp<SkColorSpace> cs,
                              bool texturable) {
    if (!tex.isValid()) {
        return false;
    }
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, kPremul_SkAlphaType, cs);

    if (!SkSurface_Gpu::Valid(info)) {
        return false;
    }

    if (!ctx->contextPriv().caps()->validateBackendTexture(tex, ct, config)) {
        return false;
    }

    // We don't require that the client gave us an exact valid sample cnt. However, it must be
    // less than the max supported sample count and 1 if MSAA is unsupported for the color type.
    if (!ctx->contextPriv().caps()->getRenderTargetSampleCount(sampleCnt, *config)) {
        return false;
    }

    if (texturable && !ctx->contextPriv().caps()->isConfigTexturable(*config)) {
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
    sampleCnt = SkTMax(1, sampleCnt);
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(context, texCopy, &texCopy.fConfig,
                                  sampleCnt, colorType, colorSpace, true)) {
        return nullptr;
    }

    if (!context) {
        return nullptr;
    }
    if (!SkSurface_Gpu::Valid(context->contextPriv().caps(), texCopy.config(), colorSpace.get())) {
        return nullptr;
    }
    sampleCnt = SkTMax(1, sampleCnt);

    sk_sp<GrRenderTargetContext> rtc(context->contextPriv().makeBackendTextureRenderTargetContext(
        texCopy,
        origin,
        sampleCnt,
        std::move(colorSpace),
        props));
    if (!rtc) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context, std::move(rtc), texCopy.width(),
                                                texCopy.height(),
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

    if (!ctx->contextPriv().caps()->validateBackendRenderTarget(rt, ct, config)) {
        return false;
    }

    if (rt.sampleCnt() > 1) {
        if (ctx->contextPriv().caps()->maxRenderTargetSampleCount(*config) <= 1) {
            return false;
        }
    } else if (!ctx->contextPriv().caps()->isConfigRenderable(*config)) {
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
    if (!SkSurface_Gpu::Valid(context->contextPriv().caps(), rtCopy.config(), colorSpace.get())) {
        return nullptr;
    }

    if (!context) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> rtc(
            context->contextPriv().makeBackendRenderTargetRenderTargetContext(
                    rtCopy, origin, std::move(colorSpace), props));
    if (!rtc) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(context, std::move(rtc), rtCopy.width(),
                                                rtCopy.height(),
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

    sampleCnt = SkTMax(1, sampleCnt);
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(context, texCopy, &texCopy.fConfig,
                                  sampleCnt, colorType, colorSpace, false)) {
        return nullptr;
    }

    if (!SkSurface_Gpu::Valid(context->contextPriv().caps(), texCopy.config(), colorSpace.get())) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> rtc(
        context->contextPriv().makeBackendTextureAsRenderTargetRenderTargetContext(
            texCopy,
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

#endif
