/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TextureInfo.h"

#ifdef SK_DAWN
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"
#endif

#ifdef SK_METAL
namespace skgpu::graphite {
    // Including Metal types/headers here is tricky. This is defined in MtlGraphiteUtils.mm
    size_t MtlFormatBytesPerBlock(MtlPixelFormat);
}
#endif

#ifdef SK_VULKAN
#include "src/gpu/vk/VulkanUtilsPriv.h"
#endif

namespace skgpu::graphite {

TextureInfo& TextureInfo::operator=(const TextureInfo& that) {
    if (!that.isValid()) {
        fValid = false;
        return *this;
    }
    fBackend = that.fBackend;
    fSampleCount = that.fSampleCount;
    fMipmapped = that.fMipmapped;
    fProtected = that.fProtected;

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawnSpec = that.fDawnSpec;
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlSpec = that.fMtlSpec;
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            fVkSpec = that.fVkSpec;
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }

    fValid = true;
    return *this;
}

bool TextureInfo::operator==(const TextureInfo& that) const {
    if (!this->isValid() && !that.isValid()) {
        return true;
    }
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    if (fSampleCount != that.fSampleCount ||
        fMipmapped != that.fMipmapped ||
        fProtected != that.fProtected) {
        return false;
    }

    switch (fBackend) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return fDawnSpec == that.fDawnSpec;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            return fMtlSpec == that.fMtlSpec;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            return fVkSpec == that.fVkSpec;
#endif
        default:
            return false;
    }
}

bool TextureInfo::isCompatible(const TextureInfo& that) const {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fSampleCount != that.fSampleCount ||
        fMipmapped != that.fMipmapped ||
        fProtected != that.fProtected) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    switch (fBackend) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return fDawnSpec.isCompatible(that.fDawnSpec);
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            return fMtlSpec.isCompatible(that.fMtlSpec);
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            return fVkSpec.isCompatible(that.fVkSpec);
#endif
        default:
            return false;
    }
}

#ifdef SK_DAWN
bool TextureInfo::getDawnTextureInfo(DawnTextureInfo* info) const {
    if (!this->isValid() || fBackend != BackendApi::kDawn) {
        return false;
    }
    *info = DawnTextureSpecToTextureInfo(fDawnSpec, fSampleCount, fMipmapped);
    return true;
}
#endif

SkString TextureInfo::toString() const {
    SkString ret;
    switch (fBackend) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            ret.appendf("Dawn(%s,", fDawnSpec.toString().c_str());
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            ret.appendf("Metal(%s,", fMtlSpec.toString().c_str());
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            ret.appendf("Vulkan(%s,", fVkSpec.toString().c_str());
            break;
#endif
        case BackendApi::kMock:
            ret += "Mock(";
            break;
        default:
            ret += "Invalid(";
            break;
    }
    ret.appendf("sampleCount=%u,mipmapped=%d,protected=%d)",
                fSampleCount,
                static_cast<int>(fMipmapped),
                static_cast<int>(fProtected));
    return ret;
}

size_t TextureInfo::bytesPerPixel() const {
    if (!this->isValid()) {
        return 0;
    }

    switch (fBackend) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return DawnFormatBytesPerBlock(this->dawnTextureSpec().fFormat);
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            return MtlFormatBytesPerBlock(this->mtlTextureSpec().fFormat);
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            return VkFormatBytesPerBlock(this->vulkanTextureSpec().fFormat);
#endif
        default:
            return 0;
    }
}

} // namespace skgpu::graphite
