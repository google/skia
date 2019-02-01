/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContextThreadSafeProxy.h"
#include "GrContextThreadSafeProxyPriv.h"

#include "GrBaseContextPriv.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrSkSLFPFactoryCache.h"
#include "SkSurface_Gpu.h"
#include "SkSurfaceCharacterization.h"

GrContextThreadSafeProxy::GrContextThreadSafeProxy(sk_sp<const GrCaps> caps, uint32_t contextID,
                                                   GrBackendApi backend,
                                                   const GrContextOptions& options,
                                                   sk_sp<GrSkSLFPFactoryCache> cache)
        : fCaps(std::move(caps))
        , fContextID(contextID)
        , fBackend(backend)
        , fOptions(options)
        , fFPFactoryCache(std::move(cache)) {}

GrContextThreadSafeProxy::~GrContextThreadSafeProxy() = default;

bool GrContextThreadSafeProxy::matches(GrContext_Base* context) const {
    return context->priv().contextID() == fContextID;
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

    if (!fCaps->mipMapSupport()) {
        isMipMapped = false;
    }

    GrPixelConfig config = fCaps->getConfigFromBackendFormat(backendFormat, ii.colorType());
    if (config == kUnknown_GrPixelConfig) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!SkSurface_Gpu::Valid(fCaps.get(), config, ii.colorSpace())) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    sampleCnt = fCaps->getRenderTargetSampleCount(sampleCnt, config);
    if (!sampleCnt) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    GrFSAAType FSAAType = GrFSAAType::kNone;
    if (sampleCnt > 1) {
        FSAAType = fCaps->usesMixedSamples() ? GrFSAAType::kMixedSamples : GrFSAAType::kUnifiedMSAA;
    }

    if (willUseGLFBO0 && isTextureable) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (isTextureable && !fCaps->isConfigTexturable(config)) {
        // Skia doesn't agree that this is textureable.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    return SkSurfaceCharacterization(sk_ref_sp<GrContextThreadSafeProxy>(this),
                                     cacheMaxResourceBytes, ii,
                                     origin, config, FSAAType, sampleCnt,
                                     SkSurfaceCharacterization::Textureable(isTextureable),
                                     SkSurfaceCharacterization::MipMapped(isMipMapped),
                                     SkSurfaceCharacterization::UsesGLFBO0(willUseGLFBO0),
                                     SkSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
                                     surfaceProps);
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrSkSLFPFactoryCache> GrContextThreadSafeProxyPriv::fpFactoryCache() const {
    return fProxy->fFPFactoryCache;
}
