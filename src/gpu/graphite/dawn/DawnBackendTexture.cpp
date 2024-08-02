/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/BackendTexturePriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include <cstdint>

namespace skgpu::graphite {

class DawnBackendTextureData final : public BackendTextureData {
public:
    DawnBackendTextureData(WGPUTexture tex, WGPUTextureView tv) : fTexture(tex), fTextureView(tv) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kDawn; }
#endif

    WGPUTexture texture() const { return fTexture; }
    WGPUTextureView textureView() const { return fTextureView; }

private:
    WGPUTexture fTexture;
    WGPUTextureView fTextureView;

    void copyTo(AnyBackendTextureData& dstData) const override {
        // Don't assert that dstData has a Dawn type() because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<DawnBackendTextureData>(fTexture, fTextureView);
    }

    bool equal(const BackendTextureData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kDawn);
        if (auto otherDawn = static_cast<const DawnBackendTextureData*>(that)) {
            return fTexture == otherDawn->fTexture && fTextureView == otherDawn->fTextureView;
        }
        return false;
    }
};

static const DawnBackendTextureData* get_and_cast_data(const BackendTexture& tex) {
    auto data = BackendTexturePriv::GetData(tex);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kDawn);
    return static_cast<const DawnBackendTextureData*>(data);
}

// When we only have a WGPUTextureView we can't actually take advantage of these TextureUsage bits
// because they require having the WGPUTexture.
static DawnTextureInfo strip_copy_usage(const DawnTextureInfo& info) {
    DawnTextureInfo result = info;
    result.fUsage &= ~(wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);
    return result;
}

namespace BackendTextures {
BackendTexture MakeDawn(WGPUTexture texture) {
    return BackendTexturePriv::Make(
            {
                    static_cast<int32_t>(wgpuTextureGetWidth(texture)),
                    static_cast<int32_t>(wgpuTextureGetHeight(texture)),
            },
            TextureInfos::MakeDawn(DawnTextureInfoFromWGPUTexture(texture)),
            DawnBackendTextureData(texture, nullptr));
}

BackendTexture MakeDawn(SkISize planeDimensions, const DawnTextureInfo& info, WGPUTexture texture) {
#if defined(__EMSCRIPTEN__)
    SkASSERT(info.fAspect == wgpu::TextureAspect::All);
#else
    SkASSERT(info.fAspect == wgpu::TextureAspect::All ||
             info.fAspect == wgpu::TextureAspect::Plane0Only ||
             info.fAspect == wgpu::TextureAspect::Plane1Only ||
             info.fAspect == wgpu::TextureAspect::Plane2Only);
#endif
    return BackendTexturePriv::Make(planeDimensions,
                                    TextureInfos::MakeDawn(info),
                                    DawnBackendTextureData(texture, nullptr));
}

BackendTexture MakeDawn(SkISize dimensions,
                        const DawnTextureInfo& info,
                        WGPUTextureView textureView) {
    return BackendTexturePriv::Make(dimensions,
                                    TextureInfos::MakeDawn(strip_copy_usage(info)),
                                    DawnBackendTextureData(nullptr, textureView));
}

WGPUTexture GetDawnTexturePtr(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kDawn) {
        return nullptr;
    }
    const DawnBackendTextureData* dawnData = get_and_cast_data(tex);
    SkASSERT(dawnData);
    return dawnData->texture();
}

WGPUTextureView GetDawnTextureViewPtr(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kDawn) {
        return nullptr;
    }
    const DawnBackendTextureData* dawnData = get_and_cast_data(tex);
    SkASSERT(dawnData);
    return dawnData->textureView();
}

}  // namespace BackendTextures

}  // namespace skgpu::graphite
