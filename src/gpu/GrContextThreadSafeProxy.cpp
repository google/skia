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
                                     bool isMipMapped, bool willUseGLFBO0, bool isTextureable) {
    if (!backendFormat.isValid()) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

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

    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, ii.colorType(), backendFormat);
    if (!sampleCnt) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (willUseGLFBO0 && isTextureable) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (isTextureable && !this->caps()->isFormatTexturable(ii.colorType(), backendFormat)) {
        // Skia doesn't agree that this is textureable.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendFormat, ii.colorType());
    if (kUnknown_GrPixelConfig == config) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    return SkSurfaceCharacterization(sk_ref_sp<GrContextThreadSafeProxy>(this),
                                     cacheMaxResourceBytes, ii,
                                     origin, config, sampleCnt,
                                     SkSurfaceCharacterization::Textureable(isTextureable),
                                     SkSurfaceCharacterization::MipMapped(isMipMapped),
                                     SkSurfaceCharacterization::UsesGLFBO0(willUseGLFBO0),
                                     SkSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
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

