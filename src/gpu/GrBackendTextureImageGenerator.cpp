/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkMessageBus.h"
#include "src/gpu/GrBackendTextureImageGenerator.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/gl/GrGLTexture.h"

GrBackendTextureImageGenerator::RefHelper::RefHelper(
                    GrTexture* texture,
                    GrDirectContext::DirectContextID owningContextID,
                    std::unique_ptr<GrSemaphore> semaphore)
        : fOriginalTexture(texture)
        , fOwningContextID(owningContextID)
        , fBorrowingContextReleaseProc(nullptr)
        , fSemaphore(std::move(semaphore)) {}

GrBackendTextureImageGenerator::RefHelper::~RefHelper() {
    SkASSERT(!fBorrowingContextID.isValid());

    // Generator has been freed, and no one is borrowing the texture. Notify the original cache
    // that it can free the last ref, so it happens on the correct thread.
    GrTextureFreedMessage msg { fOriginalTexture, fOwningContextID };
    SkMessageBus<GrTextureFreedMessage, GrDirectContext::DirectContextID>::Post(msg);
}

std::unique_ptr<SkImageGenerator>
GrBackendTextureImageGenerator::Make(sk_sp<GrTexture> texture, GrSurfaceOrigin origin,
                                     std::unique_ptr<GrSemaphore> semaphore, SkColorType colorType,
                                     SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    GrDirectContext* dContext = texture->getContext();

    // Attach our texture to this context's resource cache. This ensures that deletion will happen
    // in the correct thread/context. This adds the only ref to the texture that will persist from
    // this point. That ref will be released when the generator's RefHelper is freed.
    dContext->priv().getResourceCache()->insertDelayedTextureUnref(texture.get());

    GrBackendTexture backendTexture = texture->getBackendTexture();

    if (!dContext->priv().caps()->areColorTypeAndFormatCompatible(
            SkColorTypeToGrColorType(colorType), backendTexture.getBackendFormat())) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(texture->width(), texture->height(), colorType, alphaType,
                                         std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(new GrBackendTextureImageGenerator(
          info, texture.get(), origin, dContext->directContextID(),
          std::move(semaphore), backendTexture));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(
                    const SkImageInfo& info,
                    GrTexture* texture,
                    GrSurfaceOrigin origin,
                    GrDirectContext::DirectContextID owningContextID,
                    std::unique_ptr<GrSemaphore> semaphore,
                    const GrBackendTexture& backendTex)
        : INHERITED(info)
        , fRefHelper(new RefHelper(texture, owningContextID, std::move(semaphore)))
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
    refHelper->fBorrowingContextID.makeInvalid();
    refHelper->unref();
}

GrSurfaceProxyView GrBackendTextureImageGenerator::onGenerateTexture(
        GrRecordingContext* rContext,
        const SkImageInfo& info,
        const SkIPoint& origin,
        GrMipmapped mipMapped,
        GrImageTexGenPolicy texGenPolicy) {
    SkASSERT(rContext);

    // We currently limit GrBackendTextureImageGenerators to direct contexts since
    // only Flutter uses them and doesn't use recording/DDL contexts. Ideally, the
    // cross context texture functionality can be subsumed by the thread-safe cache
    // working with utility contexts.
    auto dContext = rContext->asDirectContext();
    if (!dContext) {
        return {};
    }

    if (dContext->backend() != fBackendTexture.backend()) {
        return {};
    }
    if (info.colorType() != this->getInfo().colorType()) {
        return {};
    }

    auto proxyProvider = dContext->priv().proxyProvider();

    fBorrowingMutex.acquire();
    sk_sp<GrRefCntedCallback> releaseProcHelper;
    if (fRefHelper->fBorrowingContextID.isValid()) {
        if (fRefHelper->fBorrowingContextID != dContext->directContextID()) {
            fBorrowingMutex.release();
            rContext->priv().printWarningMessage(
                    "GrBackendTextureImageGenerator: Trying to use texture on two GrContexts!\n");
            return {};
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
        releaseProcHelper =
                GrRefCntedCallback::Make(ReleaseRefHelper_TextureReleaseProc, fRefHelper);
        fRefHelper->fBorrowingContextReleaseProc = releaseProcHelper.get();
    }
    fRefHelper->fBorrowingContextID = dContext->directContextID();
    if (!fRefHelper->fBorrowedTextureKey.isValid()) {
        static const auto kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&fRefHelper->fBorrowedTextureKey, kDomain, 1);
        builder[0] = this->uniqueID();
    }
    fBorrowingMutex.release();

    SkASSERT(fRefHelper->fBorrowingContextID == dContext->directContextID());

    GrBackendFormat backendFormat = fBackendTexture.getBackendFormat();
    SkASSERT(backendFormat.isValid());

    GrColorType grColorType = SkColorTypeToGrColorType(info.colorType());

    GrMipmapped textureIsMipMapped = fBackendTexture.hasMipmaps() ? GrMipmapped::kYes
                                                                  : GrMipmapped::kNo;

    // Ganesh assumes that, when wrapping a mipmapped backend texture from a client, that its
    // mipmaps are fully fleshed out.
    GrMipmapStatus mipmapStatus = fBackendTexture.hasMipmaps()
            ? GrMipmapStatus::kValid : GrMipmapStatus::kNotAllocated;

    GrSwizzle readSwizzle = dContext->priv().caps()->getReadSwizzle(backendFormat, grColorType);

    // Must make copies of member variables to capture in the lambda since this image generator may
    // be deleted before we actually execute the lambda.
    sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
            [refHelper = fRefHelper, releaseProcHelper, backendTexture = fBackendTexture](
                    GrResourceProvider* resourceProvider,
                    const GrSurfaceProxy::LazySurfaceDesc&) -> GrSurfaceProxy::LazyCallbackResult {
                if (refHelper->fSemaphore) {
                    resourceProvider->priv().gpu()->waitSemaphore(refHelper->fSemaphore.get());
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
                    // We just gained access to the texture. If we're on the original
                    // context, we could use the original texture, but we'd have no way of
                    // detecting that it's no longer in-use. So we always make a wrapped
                    // copy, where the release proc informs us that the context is done with
                    // it. This is unfortunate - we'll have two texture objects referencing
                    // the same GPU object. However, no client can ever see the original
                    // texture, so this should be safe. We make the texture uncacheable so
                    // that the release proc is called ASAP.
                    tex = resourceProvider->wrapBackendTexture(
                            backendTexture, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                            kRead_GrIOType);
                    if (!tex) {
                        return {};
                    }
                    tex->setRelease(releaseProcHelper);
                    tex->resourcePriv().setUniqueKey(refHelper->fBorrowedTextureKey);
                }
                // We use keys to avoid re-wrapping the GrBackendTexture in a GrTexture.
                // This is unrelated to the whatever SkImage key may be assigned to the
                // proxy.
                return {std::move(tex), true, GrSurfaceProxy::LazyInstantiationKeyMode::kUnsynced};
            },
            backendFormat, fBackendTexture.dimensions(), textureIsMipMapped, mipmapStatus,
            GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact, SkBudgeted::kNo,
            GrProtected::kNo, GrSurfaceProxy::UseAllocator::kYes);
    if (!proxy) {
        return {};
    }

    if (texGenPolicy == GrImageTexGenPolicy::kDraw && origin.isZero() &&
        info.dimensions() == fBackendTexture.dimensions() &&
        (mipMapped == GrMipmapped::kNo || proxy->mipmapped() == GrMipmapped::kYes)) {
        // If the caller wants the entire texture and we have the correct mip support, we're done
        return GrSurfaceProxyView(std::move(proxy), fSurfaceOrigin, readSwizzle);
    } else {
        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());

        SkBudgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                      ? SkBudgeted::kNo
                                      : SkBudgeted::kYes;

        auto copy = GrSurfaceProxy::Copy(dContext,
                                         std::move(proxy),
                                         fSurfaceOrigin,
                                         mipMapped,
                                         subset,
                                         SkBackingFit::kExact,
                                         budgeted);
        return {std::move(copy), fSurfaceOrigin, readSwizzle};
    }
}
