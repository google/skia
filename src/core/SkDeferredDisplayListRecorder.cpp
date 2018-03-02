/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayListRecorder.h"

#if SK_SUPPORT_GPU
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrTexture.h"

#include "SkGpuDevice.h"
#include "SkGr.h"
#include "SkSurface_Gpu.h"
#endif

#include "SkCanvas.h" // TODO: remove
#include "SkDeferredDisplayList.h"
#include "SkSurface.h"
#include "SkSurfaceCharacterization.h"

SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(
                    const SkSurfaceCharacterization& characterization)
        : fCharacterization(characterization) {
}

SkDeferredDisplayListRecorder::~SkDeferredDisplayListRecorder() {
#if SK_SUPPORT_GPU && !defined(SK_RASTER_RECORDER_IMPLEMENTATION)
    auto proxyProvider = fContext->contextPriv().proxyProvider();

    // DDL TODO: Remove this. DDL contexts should allow for deletion while still having live
    // uniquely keyed proxies.
    proxyProvider->removeAllUniqueKeys();
#endif
}


bool SkDeferredDisplayListRecorder::init() {
    SkASSERT(!fSurface);

    if (!fCharacterization.isValid()) {
        return false;
    }

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    // Use raster right now to allow threading
    const SkImageInfo ii = SkImageInfo::Make(fCharacterization.width(), fCharacterization.height(),
                                             kN32_SkColorType, kOpaque_SkAlphaType,
                                             fCharacterization.refColorSpace());

    fSurface = SkSurface::MakeRaster(ii, &fCharacterization.surfaceProps());
    return SkToBool(fSurface.get());
#else
    SkASSERT(!fLazyProxyData);

#if SK_SUPPORT_GPU
    if (!fContext) {
        fContext = GrContextPriv::MakeDDL(fCharacterization.refContextInfo());
        if (!fContext) {
            return false;
        }
    }

    fLazyProxyData = sk_sp<SkDeferredDisplayList::LazyProxyData>(
                                                    new SkDeferredDisplayList::LazyProxyData);

    auto proxyProvider = fContext->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = fCharacterization.origin();
    desc.fWidth = fCharacterization.width();
    desc.fHeight = fCharacterization.height();
    desc.fConfig = fCharacterization.config();
    desc.fSampleCnt = fCharacterization.stencilCount();

    sk_sp<SkDeferredDisplayList::LazyProxyData> lazyProxyData = fLazyProxyData;

    // What we're doing here is we're creating a lazy proxy to back the SkSurface. The lazy
    // proxy, when instantiated, will use the GrRenderTarget that backs the SkSurface that the
    // DDL is being replayed into.

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [ lazyProxyData ] (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrSurface>();
                }

                // The proxy backing the destination surface had better have been instantiated
                // prior to the proxy backing the DLL's surface. Steal its GrRenderTarget.
                SkASSERT(lazyProxyData->fReplayDest->priv().peekSurface());
                return sk_ref_sp<GrSurface>(lazyProxyData->fReplayDest->priv().peekSurface());
            },
            desc,
            GrRenderTargetFlags::kNone,
            GrProxyProvider::Textureable(fCharacterization.isTextureable()),
            GrMipMapped::kNo,
            SkBackingFit::kExact,
            SkBudgeted::kYes);

    sk_sp<GrSurfaceContext> c = fContext->contextPriv().makeWrappedSurfaceContext(
                                                                 std::move(proxy),
                                                                 fCharacterization.refColorSpace(),
                                                                 &fCharacterization.surfaceProps());
    fSurface = SkSurface_Gpu::MakeWrappedRenderTarget(fContext.get(),
                                                      sk_ref_sp(c->asRenderTargetContext()));
    return SkToBool(fSurface.get());
#else
    return false;
#endif

#endif
}

SkCanvas* SkDeferredDisplayListRecorder::getCanvas() {
    if (!fSurface) {
        if (!this->init()) {
            return nullptr;
        }
    }

    return fSurface->getCanvas();
}

std::unique_ptr<SkDeferredDisplayList> SkDeferredDisplayListRecorder::detach() {
#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    sk_sp<SkImage> img = fSurface->makeImageSnapshot();
    fSurface.reset();

    // TODO: need to wrap the opLists associated with the deferred draws
    // in the SkDeferredDisplayList.
    return std::unique_ptr<SkDeferredDisplayList>(
                            new SkDeferredDisplayList(fCharacterization, std::move(img)));
#else

#if SK_SUPPORT_GPU
    auto ddl = std::unique_ptr<SkDeferredDisplayList>(
                           new SkDeferredDisplayList(fCharacterization, std::move(fLazyProxyData)));

    fContext->contextPriv().moveOpListsToDDL(ddl.get());
    return ddl;
#else
    return nullptr;
#endif

#endif // SK_RASTER_RECORDER_IMPLEMENTATION

}

