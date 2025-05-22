/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrBackendTextureImageGenerator.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrResourceProviderPriv.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGaneshRecorder.h"
#include "src/gpu/ganesh/SkGr.h"

#include <functional>
#include <utility>

GrBackendTextureImageGenerator::RefHelper::RefHelper(
        sk_sp<GrTexture> texture,
        GrDirectContext::DirectContextID owningContextID,
        std::unique_ptr<GrSemaphore> semaphore)
        : fOriginalTexture(std::move(texture))
        , fOwningContextID(owningContextID)
        , fBorrowingContextReleaseProc(nullptr)
        , fSemaphore(std::move(semaphore)) {}

GrBackendTextureImageGenerator::RefHelper::~RefHelper() {
    SkASSERT(!fBorrowingContextID.isValid());
    // Generator has been freed, and no one is borrowing the texture. Notify the original cache
    // that it can free the last ref, so it happens on the correct thread.
    GrResourceCache::ReturnResourceFromThread(std::move(fOriginalTexture), fOwningContextID);
}

std::unique_ptr<GrTextureGenerator>
GrBackendTextureImageGenerator::Make(const sk_sp<GrTexture>& texture,
                                     GrSurfaceOrigin origin,
                                     std::unique_ptr<GrSemaphore> semaphore,
                                     SkColorType colorType,
                                     SkAlphaType alphaType,
                                     sk_sp<SkColorSpace> colorSpace) {
    GrDirectContext* dContext = texture->getContext();

    if (!dContext->priv().caps()->areColorTypeAndFormatCompatible(
                SkColorTypeToGrColorType(colorType), texture->backendFormat())) {
        return nullptr;
    }

    SkColorInfo info(colorType, alphaType, std::move(colorSpace));
    return std::unique_ptr<GrTextureGenerator>(new GrBackendTextureImageGenerator(
            info,
            texture,
            origin,
            dContext->directContextID(),
            std::move(semaphore)));
}

GrBackendTextureImageGenerator::GrBackendTextureImageGenerator(
        const SkColorInfo& info,
        const sk_sp<GrTexture>& texture,
        GrSurfaceOrigin origin,
        GrDirectContext::DirectContextID owningContextID,
        std::unique_ptr<GrSemaphore> semaphore)
        : INHERITED(SkImageInfo::Make(texture->dimensions(), info))
        , fRefHelper(new RefHelper(texture, owningContextID, std::move(semaphore)))
        , fBackendTexture(texture->getBackendTexture())
        , fSurfaceOrigin(origin) {}

GrBackendTextureImageGenerator::~GrBackendTextureImageGenerator() {
    fRefHelper->unref();
}

bool GrBackendTextureImageGenerator::onIsProtected() const {
    return fBackendTexture.isProtected();
}

bool GrBackendTextureImageGenerator::onIsValid(GrRecordingContext* context) const {
    if (!context) {
        return false;
    }
    return context->abandoned();
}

bool GrBackendTextureImageGenerator::onIsValid(SkRecorder* recorder) const {
    if (!recorder) {
        return false;
    }
    if (recorder->type() != SkRecorder::Type::kGanesh) {
        return false;
    }
    return this->onIsValid(static_cast<SkGaneshRecorder*>(recorder)->recordingContext());
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
        skgpu::Mipmapped mipmapped,
        GrImageTexGenPolicy texGenPolicy) {
    SkASSERT(rContext);
    SkASSERT_RELEASE(info.dimensions() == fBackendTexture.dimensions());

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
    sk_sp<skgpu::RefCntedCallback> releaseProcHelper;
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
        // skgpu::RefCntedCallback.
        fRefHelper->ref();
        releaseProcHelper =
                skgpu::RefCntedCallback::Make(ReleaseRefHelper_TextureReleaseProc, fRefHelper);
        fRefHelper->fBorrowingContextReleaseProc = releaseProcHelper.get();
    }
    fRefHelper->fBorrowingContextID = dContext->directContextID();
    if (!fRefHelper->fBorrowedTextureKey.isValid()) {
        static const auto kDomain = skgpu::UniqueKey::GenerateDomain();
        skgpu::UniqueKey::Builder builder(&fRefHelper->fBorrowedTextureKey, kDomain, 1);
        builder[0] = this->uniqueID();
    }
    fBorrowingMutex.release();

    SkASSERT(fRefHelper->fBorrowingContextID == dContext->directContextID());

    GrBackendFormat backendFormat = fBackendTexture.getBackendFormat();
    SkASSERT(backendFormat.isValid());

    GrColorType grColorType = SkColorTypeToGrColorType(info.colorType());

    skgpu::Mipmapped textureIsMipMapped =
            fBackendTexture.hasMipmaps() ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    // Ganesh assumes that, when wrapping a mipmapped backend texture from a client, that its
    // mipmaps are fully fleshed out.
    GrMipmapStatus mipmapStatus = fBackendTexture.hasMipmaps()
            ? GrMipmapStatus::kValid : GrMipmapStatus::kNotAllocated;

    skgpu::Swizzle readSwizzle = dContext->priv().caps()->getReadSwizzle(backendFormat,
                                                                         grColorType);

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
            backendFormat,
            fBackendTexture.dimensions(),
            textureIsMipMapped,
            mipmapStatus,
            GrInternalSurfaceFlags::kReadOnly,
            SkBackingFit::kExact,
            skgpu::Budgeted::kNo,
            GrProtected::kNo,
            GrSurfaceProxy::UseAllocator::kYes,
            "BackendTextureImageGenerator");
    if (!proxy) {
        return {};
    }

    if (texGenPolicy == GrImageTexGenPolicy::kDraw &&
        (mipmapped == skgpu::Mipmapped::kNo || proxy->mipmapped() == skgpu::Mipmapped::kYes)) {
        // If we have the correct mip support, we're done
        return GrSurfaceProxyView(std::move(proxy), fSurfaceOrigin, readSwizzle);
    } else {
        skgpu::Budgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                           ? skgpu::Budgeted::kNo
                                           : skgpu::Budgeted::kYes;

        auto copy = GrSurfaceProxy::Copy(dContext,
                                         std::move(proxy),
                                         fSurfaceOrigin,
                                         mipmapped,
                                         SkIRect::MakeWH(info.width(), info.height()),
                                         SkBackingFit::kExact,
                                         budgeted,
                                         /*label=*/"BackendTextureImageGenerator_GenerateTexture");
        return {std::move(copy), fSurfaceOrigin, readSwizzle};
    }
}
