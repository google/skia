/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "include/gpu/MutableTextureState.h"

#ifdef SK_DAWN
#include "include/private/gpu/graphite/DawnTypesPriv.h"
#endif

#ifdef SK_VULKAN
#include "include/gpu/vk/VulkanMutableTextureState.h"
#endif

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
            fDawnTextureView = that.fDawnTextureView;
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
            if (fDawnTextureView != that.fDawnTextureView) {
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
        , fInfo(DawnTextureInfoFromWGPUTexture(texture))
        , fDawnTexture(texture)
        , fDawnTextureView(nullptr) {}

BackendTexture::BackendTexture(SkISize planeDimensions,
                               const DawnTextureInfo& info,
                               WGPUTexture texture)
        : fDimensions(planeDimensions)
        , fInfo(info)
        , fDawnTexture(texture)
        , fDawnTextureView(nullptr) {

#if defined(__EMSCRIPTEN__)
    SkASSERT(info.fAspect == wgpu::TextureAspect::All);
#else
    SkASSERT(info.fAspect == wgpu::TextureAspect::All ||
             info.fAspect == wgpu::TextureAspect::Plane0Only ||
             info.fAspect == wgpu::TextureAspect::Plane1Only ||
             info.fAspect == wgpu::TextureAspect::Plane2Only);
#endif
}

// When we only have a WGPUTextureView we can't actually take advantage of these TextureUsage bits
// because they require having the WGPUTexture.
static DawnTextureInfo strip_copy_usage(const DawnTextureInfo& info) {
    DawnTextureInfo result = info;
    result.fUsage &= ~(wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);
    return result;
}

BackendTexture::BackendTexture(SkISize dimensions,
                               const DawnTextureInfo& info,
                               WGPUTextureView textureView)
        : fDimensions(dimensions)
        , fInfo(strip_copy_usage(info))
        , fDawnTexture(nullptr)
        , fDawnTextureView(textureView) {}

WGPUTexture BackendTexture::getDawnTexturePtr() const {
    if (this->isValid() && this->backend() == BackendApi::kDawn) {
        return fDawnTexture;
    }
    return {};
}

WGPUTextureView BackendTexture::getDawnTextureViewPtr() const {
    if (this->isValid() && this->backend() == BackendApi::kDawn) {
        return fDawnTextureView;
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
        , fMutableState(sk_make_sp<skgpu::MutableTextureState>(
                  skgpu::MutableTextureStates::MakeVulkan(layout, queueFamilyIndex)))
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

