/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkMessageBus.h"
#include "src/gpu/GrBackendTextureImageGenerator.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/gl/GrGLTexture.h"

GrBackendTextureImageGenerator::RefHelper::RefHelper(GrTexture* texture, uint32_t owningContextID)
        : fOriginalTexture(texture)
        , fOwningContextID(owningContextID)
        , fBorrowingContextReleaseProc(nullptr)
        , fBorrowingContextID(SK_InvalidGenID) {}

GrBackendTextureImageGenerator::RefHelper::~RefHelper() {
    SkASSERT(fBorrowingContextID == SK_InvalidUniqueID);

    // Generator has been freed, and no one is borrowing the texture. Notify the original cache
    // that it can free the last ref, so it happens on the correct thread.
    GrGpuResourceFreedMessage msg { fOriginalTexture, fOwningContextID };
    SkMessageBus<GrGpuResourceFreedMessage>::Post(msg);
}

std::unique_ptr<SkImageGenerator>
GrBackendTextureImageGenerator::Make(sk_sp<GrTexture> texture, GrSurfaceOrigin origin,
                                     sk_sp<GrSemaphore> semaphore, SkColorType colorType,
                                     SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    GrContext* context = texture->getContext();

    // Attach our texture to this context's resource cache. This ensures that deletion will happen
    // in the correct thread/context. This adds the only ref to the texture that will persist from
    // this point. That ref will be released when the generator's RefHelper is freed.
    context->priv().getResourceCache()->insertDelayedResourceUnref(texture.get());

    GrBackendTexture backendTexture = texture->getBackendTexture();

    // TODO: delete this block
    {
        GrBackendFormat backendFormat = backendTexture.getBackendFormat();
        if (!backendFormat.isValid()) {
            return nullptr;
        }

        backendTexture.fConfig = context->priv().caps()->getConfigFromBackendFormat(
                                                            backendFormat,
                                                            SkColorTypeToGrColorType(colorType));
        if (backendTexture.fConfig == kUnknown_GrPixelConfig) {
            return nullptr;
        }
    }

    SkImageInfo info = SkImageInfo::Make(texture->width(), texture->height(), colorType, alphaType,
                                         std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(new GrBackendTextureImageGenerator(
          info, texture.get(), origin, context->priv().contextID(),
          std::move(semaphore), backendTexture));
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
        , fSurfaceOrigin(origin) {}

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    fRefHelper->unref();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void GrBackendTextureImageGenerator::ReleaseRefHelper_TextureReleaseProc(void* ctx) {
    RefHelper* refHelper = static_cast<RefHelper*>(ctx);
    SkASSERT(refHelper);

    refHelper->fBorrowingContextReleaseProc = nullptr;
    refHelper->fBorrowingContextID = SK_InvalidGenID;
    refHelper->unref();
}

sk_sp<GrTextureProxy> GrBackendTextureImageGenerator::onGenerateTexture(
        GrRecordingContext* context, const SkImageInfo& info,
        const SkIPoint& origin, bool willNeedMipMaps) {
    SkASSERT(context);

    if (context->backend() != fBackendTexture.backend()) {
        return nullptr;
    }
    if (info.colorType() != this->getInfo().colorType()) {
        return nullptr;
    }

    auto proxyProvider = context->priv().proxyProvider();
    const GrCaps* caps = context->priv().caps();

    fBorrowingMutex.acquire();
    sk_sp<GrRefCntedCallback> releaseProcHelper;
    if (SK_InvalidGenID != fRefHelper->fBorrowingContextID) {
        if (fRefHelper->fBorrowingContextID != context->priv().contextID()) {
            fBorrowingMutex.release();
            return nullptr;
        } else {
            SkASSERT(fRefHelper->fBorrowingContextReleaseProc);
            // Ref the release proc to be held by the proxy we make below
            releaseProcHelper = sk_ref_sp(fRefHelper->fBorrowingContextReleaseProc);
        }
    } else {
        SkASSERT(!fRefHelper->fBorrowingContextReleaseProc);
        // The ref we add to fRefHelper here will be passed into and owned by the
        // GrRefCntedCallback.
        fRefHelper->ref();
        releaseProcHelper.reset(
                new GrRefCntedCallback(ReleaseRefHelper_TextureReleaseProc, fRefHelper));
        fRefHelper->fBorrowingContextReleaseProc = releaseProcHelper.get();
    }
    fRefHelper->fBorrowingContextID = context->priv().contextID();
    if (!fRefHelper->fBorrowedTextureKey.isValid()) {
        static const auto kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&fRefHelper->fBorrowedTextureKey, kDomain, 1);
        builder[0] = this->uniqueID();
    }
    fBorrowingMutex.release();

    SkASSERT(fRefHelper->fBorrowingContextID == context->priv().contextID());

    GrBackendFormat backendFormat = fBackendTexture.getBackendFormat();
    SkASSERT(backendFormat.isValid());

    GrColorType grColorType = SkColorTypeToGrColorType(info.colorType());

    GrPixelConfig config = caps->getConfigFromBackendFormat(backendFormat, grColorType);
    if (kUnknown_GrPixelConfig == config) {
        return nullptr;
    }

    SkASSERT(GrCaps::AreConfigsCompatible(fBackendTexture.config(), config));

    GrSurfaceDesc desc;
    desc.fWidth = fBackendTexture.width();
    desc.fHeight = fBackendTexture.height();
    desc.fConfig = config;
    GrMipMapped mipMapped = fBackendTexture.hasMipMaps() ? GrMipMapped::kYes : GrMipMapped::kNo;

    // Must make copies of member variables to capture in the lambda since this image generator may
    // be deleted before we actually execute the lambda.
    sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
            [refHelper = fRefHelper, releaseProcHelper, semaphore = fSemaphore,
             backendTexture = fBackendTexture, grColorType](GrResourceProvider* resourceProvider)
                    -> GrSurfaceProxy::LazyInstantiationResult {
                if (semaphore) {
                    resourceProvider->priv().gpu()->waitSemaphore(semaphore);
                }

                // If a client re-draws the same image multiple times, the texture we return
                // will be cached and re-used. If they draw a subset, though, we may be
                // re-called. In that case, we want to re-use the borrowed texture we've
                // previously created.
                sk_sp<GrTexture> tex;
                SkASSERT(refHelper->fBorrowedTextureKey.isValid());
                auto surf = resourceProvider->findByUniqueKey<GrSurface>(
                        refHelper->fBorrowedTextureKey);
                if (surf) {
                    SkASSERT(surf->asTexture());
                    tex = sk_ref_sp(surf->asTexture());
                } else {
                    // We just gained access to the texture. If we're on the original context, we
                    // could use the original texture, but we'd have no way of detecting that it's
                    // no longer in-use. So we always make a wrapped copy, where the release proc
                    // informs us that the context is done with it. This is unfortunate - we'll have
                    // two texture objects referencing the same GPU object. However, no client can
                    // ever see the original texture, so this should be safe.
                    // We make the texture uncacheable so that the release proc is called ASAP.
                    tex = resourceProvider->wrapBackendTexture(
                            backendTexture, grColorType, kBorrow_GrWrapOwnership,
                            GrWrapCacheable::kNo, kRead_GrIOType);
                    if (!tex) {
                        return {};
                    }
                    tex->setRelease(releaseProcHelper);
                    tex->resourcePriv().setUniqueKey(refHelper->fBorrowedTextureKey);
                }
                // We use keys to avoid re-wrapping the GrBackendTexture in a GrTexture. This is
                // unrelated to the whatever SkImage key may be assigned to the proxy.
                return {std::move(tex), GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced};
            },
            backendFormat, desc, GrRenderable::kNo, 1, fSurfaceOrigin, mipMapped,
            GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact, SkBudgeted::kNo,
            GrProtected::kNo);
    if (!proxy) {
        return nullptr;
    }

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
        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());

        return GrSurfaceProxy::Copy(context, proxy.get(), mipMapped, subset, SkBackingFit::kExact,
                                    SkBudgeted::kYes);
    }
}
