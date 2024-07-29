/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "include/gpu/MutableTextureState.h"
#include "src/gpu/graphite/BackendTexturePriv.h"

#ifdef SK_DAWN
#include "include/private/gpu/graphite/DawnTypesPriv.h"
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
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            fTextureData.reset();
            that.fTextureData->copyTo(fTextureData);
            break;
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawnTexture = that.fDawnTexture;
            fDawnTextureView = that.fDawnTextureView;
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
        case BackendApi::kMetal:
        case BackendApi::kVulkan:
            return fTextureData->equal(that.fTextureData.get());
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
        default:
            SK_ABORT("Unsupported Backend");
    }
    return true;
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

BackendTextureData::~BackendTextureData(){};

} // namespace skgpu::graphite

