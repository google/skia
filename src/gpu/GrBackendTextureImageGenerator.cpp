/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendTextureImageGenerator.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrTexture.h"

#include "SkGr.h"
#include "SkMessageBus.h"

GrBackendTextureImageGenerator::RefHelper::~RefHelper() {
    SkASSERT(nullptr == fBorrowedTexture);
    SkASSERT(SK_InvalidGenID == fBorrowingContextID);

    // Generator has been freed, and no one is borrowing the texture. Notify the original cache
    // that it can free the last ref, so it happens on the correct thread.
    GrGpuResourceFreedMessage msg { fOriginalTexture, fOwningContextID };
    SkMessageBus<GrGpuResourceFreedMessage>::Post(msg);
}

// TODO: I copied this from SkImage_Gpu, perhaps we put a version of this somewhere else?
static GrBackendTexture make_backend_texture_from_handle(GrBackend backend,
                                                         int width, int height,
                                                         GrPixelConfig config,
                                                         GrBackendObject handle) {
    switch (backend) {
        case kOpenGL_GrBackend: {
            const GrGLTextureInfo* glInfo = (const GrGLTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *glInfo);
        }
#ifdef SK_VULKAN
        case kVulkan_GrBackend: {
            const GrVkImageInfo* vkInfo = (const GrVkImageInfo*)(handle);
            return GrBackendTexture(width, height, *vkInfo);
        }
#endif
        case kMock_GrBackend: {
            const GrMockTextureInfo* mockInfo = (const GrMockTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *mockInfo);
        }
        default:
            return GrBackendTexture();
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
    // in the correct thread/context. This adds the only ref to the texture that will persist from
    // this point. That ref will be released when the generator's RefHelper is freed.
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
            info, texture.get(), context->uniqueID(), std::move(semaphore), backendTexture));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(const SkImageInfo& info,
                                                               GrTexture* texture,
                                                               uint32_t owningContextID,
                                                               sk_sp<GrSemaphore> semaphore,
                                                               const GrBackendTexture& backendTex)
    : INHERITED(info)
    , fRefHelper(new RefHelper(texture, owningContextID))
    , fSemaphore(std::move(semaphore))
    , fLastBorrowingContextID(SK_InvalidGenID)
    , fBackendTexture(backendTex)
    , fSurfaceOrigin(texture->origin()) { }

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    fRefHelper->unref();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
void GrBackendTextureImageGenerator::ReleaseRefHelper_TextureReleaseProc(void* ctx) {
    RefHelper* refHelper = static_cast<RefHelper*>(ctx);
    SkASSERT(refHelper);

    // Release texture so another context can use it
    refHelper->fBorrowedTexture = nullptr;
    refHelper->fBorrowingContextID = SK_InvalidGenID;
    refHelper->unref();
}

sk_sp<GrTextureProxy> GrBackendTextureImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin,
        SkTransferFunctionBehavior) {
    SkASSERT(context);

    if (context->contextPriv().getBackend() != fBackendTexture.backend()) {
        return nullptr;
    }

    sk_sp<GrTexture> tex;

    if (fRefHelper->fBorrowingContextID == context->uniqueID()) {
        // If a client re-draws the same image multiple times, the texture we return will be cached
        // and re-used. If they draw a subset, though, we may be re-called. In that case, we want
        // to re-use the borrowed texture we've previously created.
        tex = sk_ref_sp(fRefHelper->fBorrowedTexture);
        SkASSERT(tex);
    } else {
        // The texture is available or borrwed by another context. Try for exclusive access.
        uint32_t expectedID = SK_InvalidGenID;
        if (!fRefHelper->fBorrowingContextID.compare_exchange(&expectedID, context->uniqueID())) {
            // Some other context is currently borrowing the texture. We aren't allowed to use it.
            return nullptr;
        } else {
            // Wait on a semaphore when a new context has just started borrowing the texture. This
            // is conservative, but shouldn't be too expensive.
            if (fSemaphore && fLastBorrowingContextID != context->uniqueID()) {
                context->getGpu()->waitSemaphore(fSemaphore);
                fLastBorrowingContextID = context->uniqueID();
            }
        }

        // We just gained access to the texture. If we're on the original context, we could use the
        // original texture, but we'd have no way of detecting that it's no longer in-use. So we
        // always make a wrapped copy, where the release proc informs us that the context is done
        // with it. This is unfortunate - we'll have two texture objects referencing the same GPU
        // object. However, no client can ever see the original texture, so this should be safe.
        tex = context->resourceProvider()->wrapBackendTexture(fBackendTexture, fSurfaceOrigin,
                                                              kBorrow_GrWrapOwnership);
        if (!tex) {
            fRefHelper->fBorrowingContextID = SK_InvalidGenID;
            return nullptr;
        }
        fRefHelper->fBorrowedTexture = tex.get();

        tex->setRelease(ReleaseRefHelper_TextureReleaseProc, fRefHelper);
        fRefHelper->ref();
    }

    SkASSERT(fRefHelper->fBorrowingContextID == context->uniqueID());

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(tex));

    if (0 == origin.fX && 0 == origin.fY &&
        info.width() == fBackendTexture.width() && info.height() == fBackendTexture.height()) {
        // If the caller wants the entire texture, we're done
        return proxy;
    } else {
        // Otherwise, make a copy of the requested subset. Make sure our temporary is renderable,
        // because Vulkan will want to do the copy as a draw.
        GrSurfaceDesc desc;
        desc.fConfig = proxy->config();
        desc.fWidth = info.width();
        desc.fHeight = info.height();
        desc.fOrigin = proxy->origin();
        desc.fIsMipMapped = proxy->isMipMapped();
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
