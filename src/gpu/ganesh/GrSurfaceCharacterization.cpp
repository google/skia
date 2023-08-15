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

#ifdef SK_VULKAN
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#endif

#ifdef SK_DEBUG
void GrSurfaceCharacterization::validate() const {
    const GrCaps* caps = fContextInfo->priv().caps();

    GrColorType grCT = SkColorTypeToGrColorType(this->colorType());
    SkASSERT(fSampleCnt && caps->isFormatAsColorTypeRenderable(grCT, fBackendFormat, fSampleCnt));

    SkASSERT(caps->areColorTypeAndFormatCompatible(grCT, fBackendFormat));

    SkASSERT(MipMapped::kNo == fIsMipMapped || Textureable::kYes == fIsTextureable);
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
           fIsMipMapped == other.fIsMipMapped &&
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

    return GrSurfaceCharacterization(fContextInfo, fCacheMaxResourceBytes,
                                     fImageInfo.makeWH(width, height), fBackendFormat, fOrigin,
                                     fSampleCnt, fIsTextureable, fIsMipMapped, fUsesGLFBO0,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible,
                                     fIsProtected, fSurfaceProps);
}

GrSurfaceCharacterization GrSurfaceCharacterization::createColorSpace(
                                                                     sk_sp<SkColorSpace> cs) const {
    if (!this->isValid()) {
        return GrSurfaceCharacterization();
    }

    return GrSurfaceCharacterization(fContextInfo, fCacheMaxResourceBytes,
                                     fImageInfo.makeColorSpace(std::move(cs)), fBackendFormat,
                                     fOrigin, fSampleCnt, fIsTextureable, fIsMipMapped, fUsesGLFBO0,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible, fIsProtected, fSurfaceProps);
}

GrSurfaceCharacterization GrSurfaceCharacterization::createBackendFormat(
                                                    SkColorType colorType,
                                                    const GrBackendFormat& backendFormat) const {
    if (!this->isValid()) {
        return GrSurfaceCharacterization();
    }

    SkImageInfo newII = fImageInfo.makeColorType(colorType);

    return GrSurfaceCharacterization(fContextInfo, fCacheMaxResourceBytes, newII, backendFormat,
                                     fOrigin, fSampleCnt, fIsTextureable, fIsMipMapped, fUsesGLFBO0,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible, fIsProtected, fSurfaceProps);
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

    return GrSurfaceCharacterization(fContextInfo, fCacheMaxResourceBytes,
                                     fImageInfo, fBackendFormat,
                                     fOrigin, fSampleCnt, fIsTextureable, fIsMipMapped,
                                     usesGLFBO0 ? UsesGLFBO0::kYes : UsesGLFBO0::kNo,
                                     fVkRTSupportsInputAttachment,
                                     fVulkanSecondaryCBCompatible, fIsProtected, fSurfaceProps);
}

bool GrSurfaceCharacterization::isCompatible(const GrBackendTexture& backendTex) const {
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

    if (this->vkRTSupportsInputAttachment()) {
        if (backendTex.backend() != GrBackendApi::kVulkan) {
            return false;
        }
#ifdef SK_VULKAN
        GrVkImageInfo vkInfo;
        if (!GrBackendTextures::GetVkImageInfo(backendTex, &vkInfo)) {
            return false;
        }
        if (!SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
            return false;
        }
#endif  // SK_VULKAN
    }

    if (this->isMipMapped() && !backendTex.hasMipmaps()) {
        // backend texture is allowed to have mipmaps even if the characterization doesn't require
        // them.
        return false;
    }

    if (this->width() != backendTex.width() || this->height() != backendTex.height()) {
        return false;
    }

    if (this->isProtected() != GrProtected(backendTex.isProtected())) {
        return false;
    }

    return true;
}
