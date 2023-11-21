/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "include/gpu/MutableTextureState.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"

namespace skgpu::graphite {

BackendTexture::BackendTexture() = default;

BackendTexture::~BackendTexture() = default;

BackendTexture::BackendTexture(const BackendTexture& that) {
    *this = that;
}

BackendTexture& BackendTexture::operator=(const BackendTexture& that) {
    if (!that.isValid()) {
        fInfo = {};
        return *this;
    }
    // We shouldn't be mixing backends.
    SkASSERT(!this->isValid() || this->backend() == that.backend());
    fDimensions = that.fDimensions;
    fInfo = that.fInfo;

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawnTexture = that.fDawnTexture;
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlTexture = that.fMtlTexture;
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            fVkImage = that.fVkImage;
            fMutableState = that.fMutableState;
            fMemoryAlloc = that.fMemoryAlloc;
            break;
#endif
        default:
            SK_ABORT("Unsupported Backend");
    }
    return *this;
}

bool BackendTexture::operator==(const BackendTexture& that) const {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fDimensions != that.fDimensions || fInfo != that.fInfo) {
        return false;
    }

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            if (fDawnTexture != that.fDawnTexture) {
                return false;
            }
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            if (fMtlTexture != that.fMtlTexture) {
                return false;
            }
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            if (fVkImage != that.fVkImage) {
                return false;
            }
            break;
#endif
        default:
            SK_ABORT("Unsupported Backend");
    }
    return true;
}

void BackendTexture::setMutableState(const skgpu::MutableTextureState& newState) {
    fMutableState->set(newState);
}

sk_sp<MutableTextureState> BackendTexture::getMutableState() const {
    return fMutableState;
}

#ifdef SK_DAWN
BackendTexture::BackendTexture(WGPUTexture texture)
        : fDimensions{static_cast<int32_t>(wgpuTextureGetWidth(texture)),
                      static_cast<int32_t>(wgpuTextureGetHeight(texture))}
        , fInfo(DawnTextureInfo(wgpu::Texture(texture)))
        , fDawnTexture(texture) {}

BackendTexture::BackendTexture(SkISize planeDimensions,
                               const DawnTextureInfo& info,
                               WGPUTexture texture)
        : fDimensions(planeDimensions), fInfo(info), fDawnTexture(texture) {
    SkASSERT(info.fAspect == wgpu::TextureAspect::All ||
             info.fAspect == wgpu::TextureAspect::Plane0Only ||
             info.fAspect == wgpu::TextureAspect::Plane1Only ||
             info.fAspect == wgpu::TextureAspect::Plane2Only);
}

WGPUTexture BackendTexture::getDawnTexturePtr() const {
    if (this->isValid() && this->backend() == BackendApi::kDawn) {
        return fDawnTexture;
    }
    return {};
}
#endif

#ifdef SK_METAL
BackendTexture::BackendTexture(SkISize dimensions, CFTypeRef mtlTexture)
        : fDimensions(dimensions)
        , fInfo(MtlTextureInfo(mtlTexture))
        , fMtlTexture(mtlTexture) {}

CFTypeRef BackendTexture::getMtlTexture() const {
    if (this->isValid() && this->backend() == BackendApi::kMetal) {
        return fMtlTexture;
    }
    return nullptr;
}
#endif // SK_METAL

#ifdef SK_VULKAN
BackendTexture::BackendTexture(SkISize dimensions,
                               const VulkanTextureInfo& info,
                               VkImageLayout layout,
                               uint32_t queueFamilyIndex,
                               VkImage image,
                               VulkanAlloc vulkanMemoryAllocation)
        : fDimensions(dimensions)
        , fInfo(info)
        , fMutableState(sk_make_sp<MutableTextureState>(layout, queueFamilyIndex))
        , fMemoryAlloc(vulkanMemoryAllocation)
        , fVkImage(image) {}

VkImage BackendTexture::getVkImage() const {
    if (this->isValid() && this->backend() == BackendApi::kVulkan) {
        return fVkImage;
    }
    return VK_NULL_HANDLE;
}

VkImageLayout BackendTexture::getVkImageLayout() const {
    if (this->isValid() && this->backend() == BackendApi::kVulkan) {
        SkASSERT(fMutableState);
        return skgpu::MutableTextureStates::GetVkImageLayout(fMutableState.get());
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

uint32_t BackendTexture::getVkQueueFamilyIndex() const {
    if (this->isValid() && this->backend() == BackendApi::kVulkan) {
        SkASSERT(fMutableState);
        return skgpu::MutableTextureStates::GetVkQueueFamilyIndex(fMutableState.get());
    }
    return 0;
}

const VulkanAlloc* BackendTexture::getMemoryAlloc() const {
    if (this->isValid() && this->backend() == BackendApi::kVulkan) {
        return &fMemoryAlloc;
    }
    return {};
}
#endif // SK_VULKAN

} // namespace skgpu::graphite

