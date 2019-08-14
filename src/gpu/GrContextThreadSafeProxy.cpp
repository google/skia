/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"

#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/image/SkSurface_Gpu.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkCaps.h"
#endif

GrContextThreadSafeProxy::GrContextThreadSafeProxy(GrBackendApi backend,
                                                   const GrContextOptions& options,
                                                   uint32_t contextID)
        : INHERITED(backend, options, contextID) {
}

GrContextThreadSafeProxy::~GrContextThreadSafeProxy() = default;

bool GrContextThreadSafeProxy::init(sk_sp<const GrCaps> caps,
                                    sk_sp<GrSkSLFPFactoryCache> FPFactoryCache) {
    return INHERITED::init(std::move(caps), std::move(FPFactoryCache));
}

SkSurfaceCharacterization GrContextThreadSafeProxy::createCharacterization(
                                     size_t cacheMaxResourceBytes,
                                     const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                     int sampleCnt, GrSurfaceOrigin origin,
                                     const SkSurfaceProps& surfaceProps,
                                     bool isMipMapped, bool willUseGLFBO0, bool isTextureable,
                                     GrProtected isProtected) {
    if (!backendFormat.isValid()) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    SkASSERT(isTextureable || !isMipMapped);

    if (GrBackendApi::kOpenGL != backendFormat.backend() && willUseGLFBO0) {
        // The willUseGLFBO0 flags can only be used for a GL backend.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!this->caps()->mipMapSupport()) {
        isMipMapped = false;
    }

    if (!SkSurface_Gpu::Valid(this->caps(), backendFormat)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    GrColorType grColorType = SkColorTypeToGrColorType(ii.colorType());

    if (!this->caps()->areColorTypeAndFormatCompatible(grColorType, backendFormat)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!this->caps()->isFormatAsColorTypeRenderable(grColorType, backendFormat, sampleCnt)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, backendFormat);
    SkASSERT(sampleCnt);

    if (willUseGLFBO0 && isTextureable) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (isTextureable && !this->caps()->isFormatTexturable(backendFormat)) {
        // Skia doesn't agree that this is textureable.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (GrBackendApi::kVulkan == backendFormat.backend()) {
        if (GrBackendApi::kVulkan != this->backend()) {
            return SkSurfaceCharacterization(); // return an invalid characterization
        }

#ifdef SK_VULKAN
        const GrVkCaps* vkCaps = (const GrVkCaps*) this->caps();

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

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrSkSLFPFactoryCache> GrContextThreadSafeProxyPriv::fpFactoryCache() {
    return fProxy->fpFactoryCache();
}

sk_sp<GrContextThreadSafeProxy> GrContextThreadSafeProxyPriv::Make(
                             GrBackendApi backend,
                             const GrContextOptions& options,
                             uint32_t contextID,
                             sk_sp<const GrCaps> caps,
                             sk_sp<GrSkSLFPFactoryCache> cache) {
    sk_sp<GrContextThreadSafeProxy> proxy(new GrContextThreadSafeProxy(backend, options,
                                                                       contextID));

    if (!proxy->init(std::move(caps), std::move(cache))) {
        return nullptr;
    }
    return proxy;
}

