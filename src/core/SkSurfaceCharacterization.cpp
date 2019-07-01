/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurfaceCharacterization.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"

bool SkSurfaceCharacterization::operator==(const SkSurfaceCharacterization& other) const {
    if (!this->isValid() || !other.isValid()) {
        return false;
    }

    if (fContextInfo != other.fContextInfo) {
        return false;
    }

    return fCacheMaxResourceBytes == other.fCacheMaxResourceBytes &&
           fOrigin == other.fOrigin &&
           fImageInfo == other.fImageInfo &&
           fConfig == other.fConfig &&
           fSampleCnt == other.fSampleCnt &&
           fIsTextureable == other.fIsTextureable &&
           fIsMipMapped == other.fIsMipMapped &&
           fUsesGLFBO0 == other.fUsesGLFBO0 &&
           fVulkanSecondaryCBCompatible == other.fVulkanSecondaryCBCompatible &&
           fSurfaceProps == other.fSurfaceProps;
}

SkSurfaceCharacterization SkSurfaceCharacterization::createResized(int width, int height) const {
    const GrCaps* caps = fContextInfo->priv().caps();
    if (!caps) {
        return SkSurfaceCharacterization();
    }

    if (width <= 0 || height <= 0 || width > caps->maxRenderTargetSize() ||
        height > caps->maxRenderTargetSize()) {
        return SkSurfaceCharacterization();
    }

    return SkSurfaceCharacterization(fContextInfo, fCacheMaxResourceBytes,
                                     fImageInfo.makeWH(width, height), fOrigin, fConfig,
                                     fSampleCnt, fIsTextureable, fIsMipMapped, fUsesGLFBO0,
                                     fVulkanSecondaryCBCompatible, fSurfaceProps);
}

bool SkSurfaceCharacterization::isCompatible(const GrBackendTexture& backendTex) const {
    if (!this->isValid() || !backendTex.isValid()) {
        return false;
    }

    const GrCaps* caps = fContextInfo->priv().caps();

    // TODO: remove this block involving the pixel config
    {
        GrPixelConfig config = caps->getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                                this->colorType());
        if (GrPixelConfig::kUnknown_GrPixelConfig == config) {
            return false;
        }

        if (this->config() != config) {
            return false;
        }
    }

    if (this->usesGLFBO0()) {
        // It is a backend texture so can't be wrapping FBO0
        return false;
    }

    if (this->vulkanSecondaryCBCompatible()) {
        return false;
    }

    int maxColorSamples = caps->maxRenderTargetSampleCount(this->colorType(),
                                                           backendTex.getBackendFormat());
    if (0 == maxColorSamples) {
        return false;  // backendTex isn't renderable
    }

    if (this->isMipMapped() && !backendTex.hasMipMaps()) {
        // backend texture is allowed to have mipmaps even if the characterization doesn't require
        // them.
        return false;
    }

    if (!caps->areColorTypeAndFormatCompatible(this->colorType(), backendTex.getBackendFormat())) {
        return false;
    }

    if (this->width() != backendTex.width() || this->height() != backendTex.height()) {
        return false;
    }

    // TODO: need to check protected status here
    return true;
}


#endif
