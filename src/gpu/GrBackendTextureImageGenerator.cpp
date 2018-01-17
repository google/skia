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
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrResourceProviderPriv.h"
#include "GrSemaphore.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"

#include "SkGr.h"
#include "SkMessageBus.h"

GrBackendTextureImageGenerator::RefHelper::~RefHelper() {
    SkASSERT(nullptr == fBorrowedTexture);

    // Generator has been freed, and no one is borrowing the texture. Notify the original cache
    // that it can free the last ref, so it happens on the correct thread.
    GrGpuResourceFreedMessage msg { fOriginalTexture, fOwningContextID };
    SkMessageBus<GrGpuResourceFreedMessage>::Post(msg);
}

std::unique_ptr<SkImageGenerator>
GrBackendTextureImageGenerator::Make(sk_sp<GrTexture> texture, GrSurfaceOrigin origin,
                                     sk_sp<GrSemaphore> semaphore,
                                     SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    SkColorType colorType = kUnknown_SkColorType;
    if (!GrPixelConfigToColorType(texture->config(), &colorType)) {
        return nullptr;
    }

    GrContext* context = texture->getContext();

    // Attach our texture to this context's resource cache. This ensures that deletion will happen
    // in the correct thread/context. This adds the only ref to the texture that will persist from
    // this point. That ref will be released when the generator's RefHelper is freed.
    context->contextPriv().getResourceCache()->insertCrossContextGpuResource(texture.get());

    GrBackendTexture backendTexture = texture->getBackendTexture();

    SkImageInfo info = SkImageInfo::Make(texture->width(), texture->height(), colorType, alphaType,
                                         std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(new GrBackendTextureImageGenerator(
          info, texture.get(), origin, context->uniqueID(), std::move(semaphore), backendTexture));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(const SkImageInfo& info,
                                                               GrTexture* texture,
                                                               GrSurfaceOrigin origin,
                                                               uint32_t owningContextID,
                                                               sk_sp<GrSemaphore> semaphore,
                                                               const GrBackendTexture& backendTex)
    : INHERITED(info)
    , fRefHelper(new RefHelper(texture, owningContextID))
    , fSemaphore(std::move(semaphore))
    , fBackendTexture(backendTex)
    , fConfig(backendTex.config())
    , fSurfaceOrigin(origin) { }

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    fRefHelper->unref();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
void GrBackendTextureImageGenerator::ReleaseRefHelper_TextureReleaseProc(void* ctx) {
    RefHelper* refHelper = static_cast<RefHelper*>(ctx);
    SkASSERT(refHelper);

    refHelper->fBorrowedTexture = nullptr;
    refHelper->unref();
}

sk_sp<GrTextureProxy> GrBackendTextureImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin,
        SkTransferFunctionBehavior, bool willNeedMipMaps) {
    SkASSERT(context);

    if (context->contextPriv().getBackend() != fBackendTexture.backend()) {
        return nullptr;
    }

    auto proxyProvider = context->contextPriv().proxyProvider();

    uint32_t expectedID = SK_InvalidGenID;
    if (!fRefHelper->fBorrowingContextID.compare_exchange(&expectedID, context->uniqueID())) {
        if (fRefHelper->fBorrowingContextID != context->uniqueID()) {
            // Some other context is currently borrowing the texture. We aren't allowed to use it.
            return nullptr;
        }
    }

    SkASSERT(fRefHelper->fBorrowingContextID == context->uniqueID());

    GrSurfaceDesc desc;
    desc.fOrigin = fSurfaceOrigin;
    desc.fWidth = fBackendTexture.width();
    desc.fHeight = fBackendTexture.height();
    desc.fConfig = fConfig;
    GrMipMapped mipMapped = fBackendTexture.hasMipMaps() ? GrMipMapped::kYes : GrMipMapped::kNo;

    // Must make copies of member variables to capture in the lambda since this image generator may
    // be deleted before we actuallly execute the lambda.
    GrSurfaceOrigin surfaceOrigin = fSurfaceOrigin;
    sk_sp<GrSemaphore> semaphore = fSemaphore;
    GrBackendTexture backendTexture = fBackendTexture;
    RefHelper* refHelper = fRefHelper;
    refHelper->ref();

    sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
            [refHelper, semaphore, backendTexture, surfaceOrigin]
            (GrResourceProvider* resourceProvider, GrSurfaceOrigin* outOrigin) {
                if (!resourceProvider) {
                    // If we get here then we never created a texture to pass the refHelper ref off
                    // to. Thus we must unref it ourselves.
                    refHelper->unref();
                    return sk_sp<GrTexture>();
                }

                if (semaphore && !semaphore->hasSubmittedWait()) {
                    resourceProvider->priv().gpu()->waitSemaphore(semaphore);
                }

                sk_sp<GrTexture> tex;
                if (refHelper->fBorrowedTexture) {
                    // If a client re-draws the same image multiple times, the texture we return
                    // will be cached and re-used. If they draw a subset, though, we may be
                    // re-called. In that case, we want to re-use the borrowed texture we've
                    // previously created.
                    tex = sk_ref_sp(refHelper->fBorrowedTexture);
                    SkASSERT(tex);
                    // The texture is holding onto a ref to the refHelper so since we have a ref to
                    // the texture we don't need to hold a ref to the refHelper. This unref's the
                    // ref we grabbed on refHelper in the lambda capture for this proxy.
                    refHelper->unref();
                } else {
                    // We just gained access to the texture. If we're on the original context, we
                    // could use the original texture, but we'd have no way of detecting that it's
                    // no longer in-use. So we always make a wrapped copy, where the release proc
                    // informs us that the context is done with it. This is unfortunate - we'll have
                    // two texture objects referencing the same GPU object. However, no client can
                    // ever see the original texture, so this should be safe.
                    tex = resourceProvider->wrapBackendTexture(backendTexture,
                                                               kBorrow_GrWrapOwnership);
                    if (!tex) {
                        refHelper->unref();
                        return sk_sp<GrTexture>();
                    }
                    refHelper->fBorrowedTexture = tex.get();

                    // By setting this release proc on the texture we are passing our ref on the
                    // refHelper to the texture.
                    tex->setRelease(ReleaseRefHelper_TextureReleaseProc, refHelper);
                }

                *outOrigin = surfaceOrigin;
                return tex;

            }, desc, mipMapped, SkBackingFit::kExact, SkBudgeted::kNo);

    if (0 == origin.fX && 0 == origin.fY &&
        info.width() == fBackendTexture.width() && info.height() == fBackendTexture.height() &&
        (!willNeedMipMaps || GrMipMapped::kYes == proxy->mipMapped())) {
        // If the caller wants the entire texture and we have the correct mip support, we're done
        return proxy;
    } else {
        // Otherwise, make a copy of the requested subset. Make sure our temporary is renderable,
        // because Vulkan will want to do the copy as a draw. All other copies would require a
        // layout change in Vulkan and we do not change the layout of borrowed images.
        GrMipMapped mipMapped = willNeedMipMaps ? GrMipMapped::kYes : GrMipMapped::kNo;

        sk_sp<GrRenderTargetContext> rtContext(context->makeDeferredRenderTargetContext(
                SkBackingFit::kExact, info.width(), info.height(), proxy->config(), nullptr,
                0, mipMapped, proxy->origin(), nullptr, SkBudgeted::kYes));

        if (!rtContext) {
            return nullptr;
        }

        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());
        if (!rtContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
            return nullptr;
        }

        return rtContext->asTextureProxyRef();
    }
}
#endif
