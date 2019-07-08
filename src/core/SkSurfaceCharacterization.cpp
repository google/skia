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

#ifdef SK_DEBUG
void SkSurfaceCharacterization::validate() const {
    const GrCaps* caps = fContextInfo->priv().caps();

    GrColorType grCT = SkColorTypeToGrColorType(this->colorType());
    int maxColorSamples = caps->maxRenderTargetSampleCount(this->colorType(), fBackendFormat);
    SkASSERT(maxColorSamples && fSampleCnt && fSampleCnt <= maxColorSamples);

    SkASSERT(caps->areColorTypeAndFormatCompatible(grCT, fBackendFormat));
}
#endif


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
           fBackendFormat == other.fBackendFormat &&
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
                                     fImageInfo.makeWH(width, height), fBackendFormat, fOrigin,
                                     fSampleCnt, fIsTextureable, fIsMipMapped, fUsesGLFBO0,
                                     fVulkanSecondaryCBCompatible, fSurfaceProps);
}

bool SkSurfaceCharacterization::isCompatible(const GrBackendTexture& backendTex) const {
    if (!this->isValid() || !backendTex.isValid()) {
        return false;
    }

    if (fBackendFormat != backendTex.getBackendFormat()) {
        return false;
    }

    if (this->usesGLFBO0()) {
        // It is a backend texture so can't be wrapping FBO0
        return false;
    }

    if (this->vulkanSecondaryCBCompatible()) {
        return false;
    }

    if (this->isMipMapped() && !backendTex.hasMipMaps()) {
        // backend texture is allowed to have mipmaps even if the characterization doesn't require
        // them.
        return false;
    }

    if (this->width() != backendTex.width() || this->height() != backendTex.height()) {
        return false;
    }

    // TODO: need to check protected status here
    return true;
}


#endif
