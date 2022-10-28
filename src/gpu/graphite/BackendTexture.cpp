/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "src/gpu/MutableTextureStateRef.h"

namespace skgpu::graphite {

BackendTexture::BackendTexture() {}

BackendTexture::~BackendTexture() {
    if (!this->isValid()) {
        return;
    }
#ifdef SK_DAWN
    if (this->backend() == BackendApi::kDawn) {
        // Only one of fDawnTexture and fDawnTextureView can be non null.
        SkASSERT(!(fDawn.fTexture && fDawn.fTextureView));
        // Release reference.
        fDawn.fTexture = nullptr;
        fDawn.fTextureView = nullptr;
    }
#endif
}

BackendTexture::BackendTexture(const BackendTexture& that) {
    *this = that;
}

BackendTexture& BackendTexture::operator=(const BackendTexture& that) {
    bool valid = this->isValid();
    if (!that.isValid()) {
        fInfo = {};
        return *this;
    } else if (valid && this->backend() != that.backend()) {
        valid = false;
    }
    fDimensions = that.fDimensions;
    fInfo = that.fInfo;

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawn = that.fDawn;
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlTexture = that.fMtlTexture;
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            // TODO: Actually fill this out
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
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
            if (fDawn != that.fDawn) {
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
            // TODO: Actually fill this out
            return false;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }
    return true;
}

void BackendTexture::setMutableState(const skgpu::MutableTextureState& newState) {
    fMutableState->set(newState);
}

#ifdef SK_DAWN
BackendTexture::BackendTexture(wgpu::Texture texture)
    : fDimensions{static_cast<int32_t>(texture.GetWidth()),
                  static_cast<int32_t>(texture.GetHeight())}
    , fInfo(DawnTextureInfo(texture))
    , fDawn(std::move(texture)) {}

BackendTexture::BackendTexture(SkISize dimensions,
                               const DawnTextureInfo& info,
                               wgpu::TextureView textureView)
        : fDimensions(dimensions)
        , fInfo(info)
        , fDawn(std::move(textureView)) {}

wgpu::Texture BackendTexture::getDawnTexture() const {
    if (this->isValid() && this->backend() == BackendApi::kDawn) {
        return fDawn.fTexture;
    }
    return {};
}

wgpu::TextureView BackendTexture::getDawnTextureView() const {
    if (this->isValid() && this->backend() == BackendApi::kDawn) {
        return fDawn.fTextureView;
    }
    return {};
}
#endif

#ifdef SK_METAL
BackendTexture::BackendTexture(SkISize dimensions, MtlHandle mtlTexture)
        : fDimensions(dimensions)
        , fInfo(MtlTextureInfo(mtlTexture))
        , fMtlTexture(mtlTexture) {}

MtlHandle BackendTexture::getMtlTexture() const {
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
                               VkImage image)
        : fDimensions(dimensions)
        , fInfo(info)
        , fMutableState(new MutableTextureStateRef(layout, queueFamilyIndex))
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
        return fMutableState->getImageLayout();
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

uint32_t BackendTexture::getVkQueueFamilyIndex() const {
    if (this->isValid() && this->backend() == BackendApi::kVulkan) {
        SkASSERT(fMutableState);
        return fMutableState->getQueueFamilyIndex();
    }
    return 0;
}
#endif // SK_VULKAN

} // namespace skgpu::graphite

