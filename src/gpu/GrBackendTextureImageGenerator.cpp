/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendTextureImageGenerator.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"

#include "SkGr.h"
#include "SkMessageBus.h"

// TODO: I copied this from SkImage_Gpu, perhaps we put a version of this somewhere else?
static GrBackendTexture make_backend_texture_from_handle(GrBackend backend,
                                                         int width, int height,
                                                         GrPixelConfig config,
                                                         GrBackendObject handle) {

    if (kOpenGL_GrBackend == backend) {
        GrGLTextureInfo* glInfo = (GrGLTextureInfo*)(handle);
        return GrBackendTexture(width, height, config, glInfo);
    } else {
        SkASSERT(kVulkan_GrBackend == backend);
        GrVkImageInfo* vkInfo = (GrVkImageInfo*)(handle);
        return GrBackendTexture(width, height, vkInfo);
    }
}

std::unique_ptr<SkImageGenerator>
GrBackendTextureImageGenerator::Make(GrContext* context, sk_sp<GrTextureProxy> proxy,
                                     sk_sp<GrSemaphore> semaphore, SkAlphaType alphaType,
                                     sk_sp<SkColorSpace> colorSpace) {
    if (colorSpace && (!colorSpace->gammaCloseToSRGB() && !colorSpace->gammaIsLinear())) {
        return nullptr;
    }

    SkColorType colorType = kUnknown_SkColorType;
    if (!GrPixelConfigToColorType(proxy->config(), &colorType)) {
        return nullptr;
    }

    // Attach our proxy to this context's resource cache. This ensures that deletion will happen
    // in the correct thread/context.
    context->getResourceCache()->insertCrossContextSurfaceProxy(proxy.get());

    GrTexture* texture = proxy->instantiate(context->resourceProvider());

    GrBackend backend = context->contextPriv().getBackend();
    GrBackendTexture backendTexture = make_backend_texture_from_handle(backend,
                                                                       proxy->width(),
                                                                       proxy->height(),
                                                                       proxy->config(),
                                                                       texture->getTextureHandle());

    SkImageInfo info = SkImageInfo::Make(proxy->width(), proxy->height(), colorType, alphaType,
                                         std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(new GrBackendTextureImageGenerator(
            info, std::move(proxy), std::move(semaphore), backendTexture));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(const SkImageInfo& info,
                                                               sk_sp<GrTextureProxy> proxy,
                                                               sk_sp<GrSemaphore> semaphore,
                                                               const GrBackendTexture& backendTex)
    : INHERITED(info)
    , fProxy(std::move(proxy))
    , fSemaphore(std::move(semaphore))
    , fLastContextID(SK_InvalidGenID)
    , fBackendTexture(backendTex)
    , fSurfaceOrigin(fProxy->origin()) { }

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    // Post a message so that the proxy will be freed on the original context
    SkMessageBus<GrSurfaceProxyFreedMessage>::Post(GrSurfaceProxyFreedMessage({ fProxy.get() }));
}

bool GrBackendTextureImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                 size_t rowBytes, SkPMColor ctable[],
                                                 int* ctableCount) {
    // TODO: Is there any way to implement this? I don't think so.
    return false;
}

bool GrBackendTextureImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels,
                                                 size_t rowBytes, const Options& opts) {

    // TODO: Is there any way to implement this? I don't think so.
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> GrBackendTextureImageGenerator::onGenerateTexture(GrContext* context,
                                                                        const SkImageInfo& info,
                                                                        const SkIPoint& origin) {
    SkASSERT(context);

    if (context->contextPriv().getBackend() != fBackendTexture.backend()) {
        return nullptr;
    }

    // Wait on our semaphore whenever a new context requests access to the texture. This is
    // conservative, but shouldn't be too expensive.
    if (fSemaphore && fLastContextID != context->uniqueID()) {
        context->getGpu()->waitSemaphore(fSemaphore);
        fLastContextID = context->uniqueID();
    }

    // Make a wrapped copy of the texture (and a proxy) for this context
    sk_sp<GrTexture> tex = context->resourceProvider()->wrapBackendTexture(
        fBackendTexture, fSurfaceOrigin, kNone_GrBackendTextureFlag, 0, kBorrow_GrWrapOwnership);
    if (!tex) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(tex));

    if (0 == origin.fX && 0 == origin.fY &&
        info.width() == fProxy->width() && info.height() == fProxy->height()) {
        // If the caller wants the entire texture, we're done
        return proxy;
    } else {
        // Otherwise, make a copy of the requested subset. Make sure our temporary is renderable,
        // because Vulkan will want to do the copy as a draw.
        GrSurfaceDesc desc = proxy->desc();
        desc.fWidth = info.width();
        desc.fHeight = info.height();
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeDeferredSurfaceContext(
            desc, SkBackingFit::kExact, SkBudgeted::kYes));
        if (!sContext) {
            return nullptr;
        }

        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());
        if (!sContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
            return nullptr;
        }

        return sContext->asTextureProxyRef();
    }
}
#endif
