/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrSurfaceCharacterization.h"

#include "include/core/SkColorSpace.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"

#ifdef SK_DEBUG
void GrSurfaceCharacterization::validate() const {
    const GrCaps* caps = fContextInfo->priv().caps();

    GrColorType grCT = SkColorTypeToGrColorType(this->colorType());
    SkASSERT(fSampleCnt && caps->isFormatAsColorTypeRenderable(grCT, fBackendFormat, fSampleCnt));

    SkASSERT(caps->areColorTypeAndFormatCompatible(grCT, fBackendFormat));

    SkASSERT(skgpu::Mipmapped::kNo == fIsMipmapped || Textureable::kYes == fIsTextureable);
    SkASSERT(Textureable::kNo == fIsTextureable || UsesGLFBO0::kNo == fUsesGLFBO0);
    auto backend = fBackendFormat.backend();
    SkASSERT(UsesGLFBO0::kNo == fUsesGLFBO0 || backend == GrBackendApi::kOpenGL);
    SkASSERT((VulkanSecondaryCBCompatible::kNo == fVulkanSecondaryCBCompatible &&
              VkRTSupportsInputAttachment::kNo == fVkRTSupportsInputAttachment) ||
             backend == GrBackendApi::kVulkan);
    SkASSERT(VulkanSecondaryCBCompatible::kNo == fVulkanSecondaryCBCompatible ||
             VkRTSupportsInputAttachment::kNo == fVkRTSupportsInputAttachment);
    SkASSERT(Textureable::kNo == fIsTextureable ||
             VulkanSecondaryCBCompatible::kNo == fVulkanSecondaryCBCompatible);
}
#endif


bool GrSurfaceCharacterization::operator==(const GrSurfaceCharacterization& other) const {
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
           fIsMipmapped == other.fIsMipmapped &&
           fUsesGLFBO0 == other.fUsesGLFBO0 &&
           fVulkanSecondaryCBCompatible == other.fVulkanSecondaryCBCompatible &&
           fIsProtected == other.fIsProtected &&
           fSurfaceProps == other.fSurfaceProps;
}

GrSurfaceCharacterization GrSurfaceCharacterization::createResized(int width, int height) const {
    const GrCaps* caps = fContextInfo->priv().caps();
    if (!caps) {
        return GrSurfaceCharacterization();
    }

    if (width <= 0 || height <= 0 || width > caps->maxRenderTargetSize() ||
        height > caps->maxRenderTargetSize()) {
        return GrSurfaceCharacterization();
    }

    return GrSurfaceCharacterization(fContextInfo,
                                     fCacheMaxResourceBytes,
                                     fImageInfo.makeWH(width, height),
                                     fBackendFormat,
                                     fOrigin,
                                     fSampleCnt,
                                     fIsTextureable,
                                     fIsMipmapped,
                                     fUsesGLFBO0,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible,
                                     fIsProtected,
                                     fSurfaceProps);
}

GrSurfaceCharacterization GrSurfaceCharacterization::createColorSpace(
                                                                     sk_sp<SkColorSpace> cs) const {
    if (!this->isValid()) {
        return GrSurfaceCharacterization();
    }

    return GrSurfaceCharacterization(fContextInfo,
                                     fCacheMaxResourceBytes,
                                     fImageInfo.makeColorSpace(std::move(cs)),
                                     fBackendFormat,
                                     fOrigin,
                                     fSampleCnt,
                                     fIsTextureable,
                                     fIsMipmapped,
                                     fUsesGLFBO0,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible,
                                     fIsProtected,
                                     fSurfaceProps);
}

GrSurfaceCharacterization GrSurfaceCharacterization::createBackendFormat(
                                                    SkColorType colorType,
                                                    const GrBackendFormat& backendFormat) const {
    if (!this->isValid()) {
        return GrSurfaceCharacterization();
    }

    SkImageInfo newII = fImageInfo.makeColorType(colorType);

    return GrSurfaceCharacterization(fContextInfo,
                                     fCacheMaxResourceBytes,
                                     newII,
                                     backendFormat,
                                     fOrigin,
                                     fSampleCnt,
                                     fIsTextureable,
                                     fIsMipmapped,
                                     fUsesGLFBO0,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible,
                                     fIsProtected,
                                     fSurfaceProps);
}

GrSurfaceCharacterization GrSurfaceCharacterization::createFBO0(bool usesGLFBO0) const {
    if (!this->isValid()) {
        return GrSurfaceCharacterization();
    }

    // We can't create an FBO0 characterization that is textureable or has any non-gl specific flags
    if (fIsTextureable == Textureable::kYes ||
        fVkRTSupportsInputAttachment == VkRTSupportsInputAttachment::kYes ||
        fVulkanSecondaryCBCompatible == VulkanSecondaryCBCompatible::kYes) {
        return GrSurfaceCharacterization();
    }

    return GrSurfaceCharacterization(fContextInfo,
                                     fCacheMaxResourceBytes,
                                     fImageInfo,
                                     fBackendFormat,
                                     fOrigin,
                                     fSampleCnt,
                                     fIsTextureable,
                                     fIsMipmapped,
                                     usesGLFBO0 ? UsesGLFBO0::kYes : UsesGLFBO0::kNo,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible,
                                     fIsProtected,
                                     fSurfaceProps);
}
