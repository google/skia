/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Gpu.h"

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"

#include "SkCanvas.h"
#include "SkColorSpace_Base.h"
#include "SkGpuDevice.h"
#include "SkImage_Base.h"
#include "SkImage_Gpu.h"
#include "SkImagePriv.h"
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
        // MDB TODO: replace this with GrSurfaceProxy::Copy?
        GrSurfaceDesc desc;
        desc.fConfig = rtc->config();
        desc.fWidth = rtc->width();
        desc.fHeight = rtc->height();
        desc.fOrigin = rtc->origin();

        sk_sp<GrSurfaceContext> copyCtx = ctx->contextPriv().makeDeferredSurfaceContext(
                                                                desc,
                                                                SkBackingFit::kExact,
                                                                budgeted);
        if (!copyCtx) {
            return nullptr;
        }

        if (!copyCtx->copy(rtc->asSurfaceProxy())) {
            return nullptr;
        }

        srcProxy = copyCtx->asTextureProxyRef();
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
    GrRenderTarget* rt = fDevice->accessRenderTargetContext()->accessRenderTarget();
    if (!rt) {
        return;
    }
    // are we sharing our render target with the image? Note this call should never create a new
    // image because onCopyOnWrite is only called when there is a cached image.
    sk_sp<SkImage> image(this->refCachedImage());
    SkASSERT(image);
    // MDB TODO: this is unfortunate. The snapping of an Image_Gpu from a surface currently
    // funnels down to a GrTexture. Once Image_Gpus are proxy-backed we should be able to
    // compare proxy uniqueIDs.
    if (rt->asTexture()->getTextureHandle() == image->getTextureHandle(false)) {
        fDevice->replaceRenderTargetContext(SkSurface::kRetain_ContentChangeMode == mode);
    } else if (kDiscard_ContentChangeMode == mode) {
        this->SkSurface_Gpu::onDiscard();
    }
}

void SkSurface_Gpu::onDiscard() {
    fDevice->accessRenderTargetContext()->discard();
}

void SkSurface_Gpu::onPrepareForExternalIO() {
    fDevice->flush();
}

///////////////////////////////////////////////////////////////////////////////

bool SkSurface_Gpu::Valid(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kRGBA_F16_SkColorType:
            return !info.colorSpace() || info.colorSpace()->gammaIsLinear();
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
            return !colorSpace || colorSpace->gammaIsLinear();
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
                                             GrSurfaceOrigin origin, const SkSurfaceProps* props) {
    if (!SkSurface_Gpu::Valid(info)) {
        return nullptr;
    }

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(
            ctx, budgeted, info, sampleCount, origin, props, SkGpuDevice::kClear_InitContents));
    if (!device) {
        return nullptr;
    }
    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrContext* context,
                                                   const GrBackendTextureDesc& desc,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    GrBackendTexture tex(desc, context->contextPriv().getBackend());
    return MakeFromBackendTexture(context, tex, desc.fOrigin, desc.fSampleCnt, colorSpace, props);
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

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrContext* context,
                                                        const GrBackendRenderTargetDesc& desc,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }

    GrBackendRenderTarget backendRT(desc, context->contextPriv().getBackend());
    return MakeFromBackendRenderTarget(context, backendRT, desc.fOrigin,
                                       std::move(colorSpace), props);

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

sk_sp<SkSurface> SkSurface::MakeFromBackendTextureAsRenderTarget(GrContext* context,
                                                                 const GrBackendTextureDesc& desc,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* props) {
    if (!context) {
        return nullptr;
    }
    GrBackendTexture tex(desc, context->contextPriv().getBackend());
    return MakeFromBackendTextureAsRenderTarget(context, tex, desc.fOrigin, desc.fSampleCnt,
                                                std::move(colorSpace), props);
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

#endif
