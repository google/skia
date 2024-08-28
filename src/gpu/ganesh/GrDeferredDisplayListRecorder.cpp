/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrDeferredDisplayListRecorder.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"

#include <functional>
#include <utility>

class GrResourceProvider;

GrDeferredDisplayListRecorder::GrDeferredDisplayListRecorder(const GrSurfaceCharacterization& c)
        : fCharacterization(c) {
    if (fCharacterization.isValid()) {
        fContext = GrRecordingContextPriv::MakeDDL(fCharacterization.refContextInfo());
    }
}

GrDeferredDisplayListRecorder::~GrDeferredDisplayListRecorder() {
    if (fContext) {
        auto proxyProvider = fContext->priv().proxyProvider();

        // This allows the uniquely keyed proxies to keep their keys but removes their back
        // pointer to the about-to-be-deleted proxy provider. The proxies will use their
        // unique key to reattach to cached versions of themselves or to appropriately tag new
        // resources (if a cached version was not found). This system operates independent of
        // the replaying context's proxy provider (i.e., these uniquely keyed proxies will not
        // appear in the replaying proxy providers uniquely keyed proxy map). This should be fine
        // since no one else should be trying to reconnect to the orphaned proxies and orphaned
        // proxies from different DDLs that share the same key should simply reconnect to the
        // same cached resource.
        proxyProvider->orphanAllUniqueKeys();
    }
}

bool GrDeferredDisplayListRecorder::init() {
    SkASSERT(fContext);
    SkASSERT(!fTargetProxy);
    SkASSERT(!fLazyProxyData);
    SkASSERT(!fSurface);

    if (!fCharacterization.isValid()) {
        return false;
    }

    fLazyProxyData = sk_sp<GrDeferredDisplayList::LazyProxyData>(
                                                    new GrDeferredDisplayList::LazyProxyData);

    auto proxyProvider = fContext->priv().proxyProvider();
    const GrCaps* caps = fContext->priv().caps();

    bool usesGLFBO0 = fCharacterization.usesGLFBO0();
    if (usesGLFBO0) {
        if (GrBackendApi::kOpenGL != fContext->backend() ||
            fCharacterization.isTextureable()) {
            return false;
        }
    }

    bool vkRTSupportsInputAttachment = fCharacterization.vkRTSupportsInputAttachment();
    if (vkRTSupportsInputAttachment && GrBackendApi::kVulkan != fContext->backend()) {
        return false;
    }

    if (fCharacterization.vulkanSecondaryCBCompatible()) {
        // Because of the restrictive API allowed for a GrVkSecondaryCBDrawContext, we know ahead
        // of time that we don't be able to support certain parameter combinations. Specifically we
        // fail on usesGLFBO0 since we can't mix GL and Vulkan. We can't have a texturable object.
        // We can't use it as in input attachment since we don't control the render pass this will
        // be played into and thus can't force it to have an input attachment and the correct
        // dependencies. And finally the GrVkSecondaryCBDrawContext always assumes a top left
        // origin.
        if (usesGLFBO0 ||
            vkRTSupportsInputAttachment ||
            fCharacterization.isTextureable() ||
            fCharacterization.origin() == kBottomLeft_GrSurfaceOrigin) {
            return false;
        }
    }

    GrColorType grColorType = SkColorTypeToGrColorType(fCharacterization.colorType());

    // What we're doing here is we're creating a lazy proxy to back the SkSurface. The lazy
    // proxy, when instantiated, will use the GrRenderTarget that backs the SkSurface that the
    // DDL is being replayed into.

    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNone;
    if (usesGLFBO0) {
        surfaceFlags |= GrInternalSurfaceFlags::kGLRTFBOIDIs0;
    } else if (fCharacterization.sampleCount() > 1 && !caps->msaaResolvesAutomatically() &&
               fCharacterization.isTextureable()) {
        surfaceFlags |= GrInternalSurfaceFlags::kRequiresManualMSAAResolve;
    }

    if (vkRTSupportsInputAttachment) {
        surfaceFlags |= GrInternalSurfaceFlags::kVkRTSupportsInputAttachment;
    }

    // FIXME: Why do we use skgpu::Mipmapped::kNo instead of
    // GrSurfaceCharacterization::fIsMipMapped?
    static constexpr GrProxyProvider::TextureInfo kTextureInfo{skgpu::Mipmapped::kNo,
                                                               GrTextureType::k2D};
    const GrProxyProvider::TextureInfo* optionalTextureInfo = nullptr;
    if (fCharacterization.isTextureable()) {
        optionalTextureInfo = &kTextureInfo;
    }

    fTargetProxy = proxyProvider->createLazyRenderTargetProxy(
            [lazyProxyData = fLazyProxyData](GrResourceProvider* resourceProvider,
                                             const GrSurfaceProxy::LazySurfaceDesc&) {
                // The proxy backing the destination surface had better have been instantiated
                // prior to this one (i.e., the proxy backing the DDL's surface).
                // Fulfill this lazy proxy with the destination surface's GrRenderTarget.
                SkASSERT(lazyProxyData->fReplayDest->peekSurface());
                auto surface = sk_ref_sp<GrSurface>(lazyProxyData->fReplayDest->peekSurface());
                return GrSurfaceProxy::LazyCallbackResult(std::move(surface));
            },
            fCharacterization.backendFormat(),
            fCharacterization.dimensions(),
            fCharacterization.sampleCount(),
            surfaceFlags,
            optionalTextureInfo,
            GrMipmapStatus::kNotAllocated,
            SkBackingFit::kExact,
            skgpu::Budgeted::kYes,
            fCharacterization.isProtected(),
            fCharacterization.vulkanSecondaryCBCompatible(),
            GrSurfaceProxy::UseAllocator::kYes);

    if (!fTargetProxy) {
        return false;
    }
    fTargetProxy->priv().setIsDDLTarget();

    auto device = fContext->priv().createDevice(grColorType,
                                                fTargetProxy,
                                                fCharacterization.refColorSpace(),
                                                fCharacterization.origin(),
                                                fCharacterization.surfaceProps(),
                                                skgpu::ganesh::Device::InitContents::kUninit);
    if (!device) {
        return false;
    }

    fSurface = sk_make_sp<SkSurface_Ganesh>(std::move(device));
    return SkToBool(fSurface.get());
}

SkCanvas* GrDeferredDisplayListRecorder::getCanvas() {
    if (!fContext) {
        return nullptr;
    }

    if (!fSurface && !this->init()) {
        return nullptr;
    }

    return fSurface->getCanvas();
}

sk_sp<GrDeferredDisplayList> GrDeferredDisplayListRecorder::detach() {
    if (!fContext || !fTargetProxy) {
        return nullptr;
    }

    if (fSurface) {
        SkCanvas* canvas = fSurface->getCanvas();

        canvas->restoreToCount(0);
    }

    auto ddl = sk_sp<GrDeferredDisplayList>(new GrDeferredDisplayList(fCharacterization,
                                                                      std::move(fTargetProxy),
                                                                      std::move(fLazyProxyData)));

    fContext->priv().moveRenderTasksToDDL(ddl.get());

    // We want a new lazy proxy target for each recorded DDL so force the (lazy proxy-backed)
    // SkSurface to be regenerated for each DDL.
    fSurface = nullptr;
    return ddl;
}
