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
        return GrBackendTexture(width, height, config, *glInfo);
    } else {
        SkASSERT(kVulkan_GrBackend == backend);
        GrVkImageInfo* vkInfo = (GrVkImageInfo*)(handle);
        return GrBackendTexture(width, height, *vkInfo);
    }
}

std::unique_ptr<SkImageGenerator>
GrBackendTextureImageGenerator::Make(sk_sp<GrTexture> texture, sk_sp<GrSemaphore> semaphore,
                                     SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    if (colorSpace && (!colorSpace->gammaCloseToSRGB() && !colorSpace->gammaIsLinear())) {
        return nullptr;
    }

    SkColorType colorType = kUnknown_SkColorType;
    if (!GrPixelConfigToColorType(texture->config(), &colorType)) {
        return nullptr;
    }

    GrContext* context = texture->getContext();

    // Attach our texture to this context's resource cache. This ensures that deletion will happen
    // in the correct thread/context.
    context->getResourceCache()->insertCrossContextGpuResource(texture.get());

    GrBackend backend = context->contextPriv().getBackend();
    GrBackendTexture backendTexture = make_backend_texture_from_handle(backend,
                                                                       texture->width(),
                                                                       texture->height(),
                                                                       texture->config(),
                                                                       texture->getTextureHandle());

    SkImageInfo info = SkImageInfo::Make(texture->width(), texture->height(), colorType, alphaType,
                                         std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(new GrBackendTextureImageGenerator(
            info, std::move(texture), context->uniqueID(), std::move(semaphore), backendTexture));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(const SkImageInfo& info,
                                                               sk_sp<GrTexture> texture,
                                                               uint32_t owningContextID,
                                                               sk_sp<GrSemaphore> semaphore,
                                                               const GrBackendTexture& backendTex)
    : INHERITED(info)
    , fTexture(std::move(texture))
    , fOwningContextID(owningContextID)
    , fSemaphore(std::move(semaphore))
    , fLastContextID(SK_InvalidGenID)
    , fBackendTexture(backendTex)
    , fSurfaceOrigin(fTexture->origin()) { }

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    // Post a message so that the texture will be freed on the original context
    GrGpuResourceFreedMessage msg { fTexture.get(), fOwningContextID };
    SkMessageBus<GrGpuResourceFreedMessage>::Post(msg);
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

    // If we're on the original context, we can use the original texture. Otherwise, make a wrapped
    // copy of the texture for this context.
    sk_sp<GrTexture> tex = (context->uniqueID() == fOwningContextID)
        ? fTexture
        : context->resourceProvider()->wrapBackendTexture(fBackendTexture, fSurfaceOrigin,
                                                          kNone_GrBackendTextureFlag, 0,
                                                          kBorrow_GrWrapOwnership);
    if (!tex) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(tex));

    if (0 == origin.fX && 0 == origin.fY &&
        info.width() == fBackendTexture.width() && info.height() == fBackendTexture.height()) {
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
