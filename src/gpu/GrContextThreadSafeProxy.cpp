/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"

#include "include/core/SkSurfaceCharacterization.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/image/SkSurface_Gpu.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkCaps.h"
#endif

static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};
    int32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidGenID);
    return id;
}

GrContextThreadSafeProxy::GrContextThreadSafeProxy(GrBackendApi backend,
                                                   const GrContextOptions& options)
        : fBackend(backend), fOptions(options), fContextID(next_id()) {
}

GrContextThreadSafeProxy::~GrContextThreadSafeProxy() = default;

void GrContextThreadSafeProxy::init(sk_sp<const GrCaps> caps) {
    fCaps = std::move(caps);
    fTextBlobCache = std::make_unique<GrTextBlobCache>(fContextID);
}

SkSurfaceCharacterization GrContextThreadSafeProxy::createCharacterization(
                                     size_t cacheMaxResourceBytes,
                                     const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                     int sampleCnt, GrSurfaceOrigin origin,
                                     const SkSurfaceProps& surfaceProps,
                                     bool isMipMapped, bool willUseGLFBO0, bool isTextureable,
                                     GrProtected isProtected) {
    SkASSERT(fCaps);
    if (!backendFormat.isValid()) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    SkASSERT(isTextureable || !isMipMapped);

    if (GrBackendApi::kOpenGL != backendFormat.backend() && willUseGLFBO0) {
        // The willUseGLFBO0 flags can only be used for a GL backend.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!fCaps->mipmapSupport()) {
        isMipMapped = false;
    }

    GrColorType grColorType = SkColorTypeToGrColorType(ii.colorType());

    if (!fCaps->areColorTypeAndFormatCompatible(grColorType, backendFormat)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!fCaps->isFormatAsColorTypeRenderable(grColorType, backendFormat, sampleCnt)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    sampleCnt = fCaps->getRenderTargetSampleCount(sampleCnt, backendFormat);
    SkASSERT(sampleCnt);

    if (willUseGLFBO0 && isTextureable) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (isTextureable && !fCaps->isFormatTexturable(backendFormat)) {
        // Skia doesn't agree that this is textureable.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (GrBackendApi::kVulkan == backendFormat.backend()) {
        if (GrBackendApi::kVulkan != fBackend) {
            return SkSurfaceCharacterization(); // return an invalid characterization
        }

#ifdef SK_VULKAN
        const GrVkCaps* vkCaps = (const GrVkCaps*) fCaps.get();

        // The protection status of the characterization and the context need to match
        if (isProtected != GrProtected(vkCaps->supportsProtectedMemory())) {
            return SkSurfaceCharacterization(); // return an invalid characterization
        }
#endif
    }

    return SkSurfaceCharacterization(sk_ref_sp<GrContextThreadSafeProxy>(this),
                                     cacheMaxResourceBytes, ii, backendFormat,
                                     origin, sampleCnt,
                                     SkSurfaceCharacterization::Textureable(isTextureable),
                                     SkSurfaceCharacterization::MipMapped(isMipMapped),
                                     SkSurfaceCharacterization::UsesGLFBO0(willUseGLFBO0),
                                     SkSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
                                     isProtected,
                                     surfaceProps);
}

GrBackendFormat GrContextThreadSafeProxy::defaultBackendFormat(SkColorType skColorType,
                                                               GrRenderable renderable) const {
    SkASSERT(fCaps);
    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);

    GrBackendFormat format = fCaps->getDefaultBackendFormat(grColorType, renderable);
    if (!format.isValid()) {
        return GrBackendFormat();
    }

    SkASSERT(renderable == GrRenderable::kNo ||
             fCaps->isFormatAsColorTypeRenderable(grColorType, format));

    return format;
}

void GrContextThreadSafeProxy::abandonContext() {
    if (!fAbandoned.exchange(true)) {
        fTextBlobCache->freeAll();
    }
}

bool GrContextThreadSafeProxy::abandoned() const {
    return fAbandoned;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrContextThreadSafeProxy> GrContextThreadSafeProxyPriv::Make(
                             GrBackendApi backend,
                             const GrContextOptions& options) {
    return sk_sp<GrContextThreadSafeProxy>(new GrContextThreadSafeProxy(backend, options));
}

