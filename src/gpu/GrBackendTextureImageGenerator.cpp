/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendTextureImageGenerator.h"

#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"

#include "SkGr.h"

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
GrBackendTextureImageGenerator::Make(sk_sp<GrTexture> texture, sk_sp<GrSemaphore> semaphore,
                                     SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    if (colorSpace && (!colorSpace->gammaCloseToSRGB() && !colorSpace->gammaIsLinear())) {
        return nullptr;
    }

    SkColorType colorType = kUnknown_SkColorType;
    if (!GrPixelConfigToColorType(texture->config(), &colorType)) {
        return nullptr;
    }

    GrBackend backend = texture->getContext()->contextPriv().getBackend();
    GrBackendTexture backendTexture = make_backend_texture_from_handle(backend,
                                                                       texture->width(),
                                                                       texture->height(),
                                                                       texture->config(),
                                                                       texture->getTextureHandle());

    SkImageInfo info = SkImageInfo::Make(texture->width(), texture->height(), colorType, alphaType,
                                         std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(new GrBackendTextureImageGenerator(
            info, std::move(texture), std::move(semaphore), backendTexture));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(const SkImageInfo& info,
                                                               sk_sp<GrTexture> texture,
                                                               sk_sp<GrSemaphore> semaphore,
                                                               const GrBackendTexture& backendTex)
        : INHERITED(info)
        , fTexture(std::move(texture))
        , fSemaphore(std::move(semaphore))
        , fLastContextID(SK_InvalidGenID)
        , fBackendTexture(backendTex) {
    fSurfaceOrigin = fTexture->origin();
}

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() { }

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
        info.width() == fTexture->width() && info.height() == fTexture->height()) {
        // If the caller wants the entire texture, we're done
        return proxy;
    } else {
        // Otherwise, make a copy of the requested subset (using a draw, so we don't change the
        // image layout in the Vulkan backend)
        sk_sp<GrRenderTargetContext> rtContext(context->makeDeferredRenderTargetContext(
            SkBackingFit::kExact, info.width(), info.height(), proxy->config(), nullptr));
        if (!rtContext) {
            return nullptr;
        }

        GrPaint paint;
        paint.setGammaCorrect(true);
        SkMatrix matrix = SkMatrix::MakeTrans(origin.fX, origin.fY);
        paint.addColorTextureProcessor(context->resourceProvider(), std::move(proxy), nullptr,
                                       matrix);
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        rtContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                            SkRect::MakeWH(info.width(), info.height()));

        return rtContext->asTextureProxyRef();
    }
}
#endif
