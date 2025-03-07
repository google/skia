/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTypes_DEFINED
#define skgpu_graphite_DawnTypes_DEFINED

#include "include/core/SkSize.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAPI.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

class SkStream;
class SkWStream;

namespace skgpu::graphite {
class BackendTexture;

class SK_API DawnTextureInfo final : public TextureInfo::Data {
public:
    // wgpu::TextureDescriptor properties
    wgpu::TextureFormat fFormat = wgpu::TextureFormat::Undefined;
    // `fViewFormat` for multiplanar formats corresponds to the plane TextureView's format.
    wgpu::TextureFormat fViewFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureUsage fUsage = wgpu::TextureUsage::None;
    // TODO(b/308944094): Migrate aspect information to BackendTextureViews.
    wgpu::TextureAspect fAspect = wgpu::TextureAspect::All;
    uint32_t fSlice = 0;

#if !defined(__EMSCRIPTEN__)
    // The descriptor of the YCbCr info (if any) for this texture. Dawn's YCbCr
    // sampling will be used for this texture if this info is set. Setting the
    // info is supported only on Android and only if using Vulkan as the
    // underlying GPU driver.
    wgpu::YCbCrVkDescriptor fYcbcrVkDescriptor = {};
#endif

    wgpu::TextureFormat getViewFormat() const {
        return fViewFormat != wgpu::TextureFormat::Undefined ? fViewFormat : fFormat;
    }

    DawnTextureInfo() = default;

    DawnTextureInfo(WGPUTexture texture);

    DawnTextureInfo(uint32_t sampleCount,
                    Mipmapped mipmapped,
                    wgpu::TextureFormat format,
                    wgpu::TextureUsage usage,
                    wgpu::TextureAspect aspect)
            : DawnTextureInfo(sampleCount,
                              mipmapped,
                              /*format=*/format,
                              /*viewFormat=*/format,
                              usage,
                              aspect,
                              /*slice=*/0) {}

    DawnTextureInfo(uint32_t sampleCount,
                    Mipmapped mipmapped,
                    wgpu::TextureFormat format,
                    wgpu::TextureFormat viewFormat,
                    wgpu::TextureUsage usage,
                    wgpu::TextureAspect aspect,
                    uint32_t slice)
            : Data(sampleCount, mipmapped)
            , fFormat(format)
            , fViewFormat(viewFormat)
            , fUsage(usage)
            , fAspect(aspect)
            , fSlice(slice) {}

#if !defined(__EMSCRIPTEN__)
    DawnTextureInfo(uint32_t sampleCount,
                    Mipmapped mipmapped,
                    wgpu::TextureFormat format,
                    wgpu::TextureFormat viewFormat,
                    wgpu::TextureUsage usage,
                    wgpu::TextureAspect aspect,
                    uint32_t slice,
                    wgpu::YCbCrVkDescriptor ycbcrVkDescriptor)
            : Data(sampleCount, mipmapped)
            , fFormat(format)
            , fViewFormat(viewFormat)
            , fUsage(usage)
            , fAspect(aspect)
            , fSlice(slice)
            , fYcbcrVkDescriptor(ycbcrVkDescriptor) {}
#endif

private:
    friend class TextureInfo;
    friend class TextureInfoPriv;

    // Non-virtual template API for TextureInfo::Data accessed directly when backend type is known.
    static constexpr skgpu::BackendApi kBackend = skgpu::BackendApi::kDawn;

    Protected isProtected() const { return Protected::kNo; }
    TextureFormat viewFormat() const;

    bool serialize(SkWStream*) const;
    bool deserialize(SkStream*);

    // Virtual API when the specific backend type is not available.
    SkString toBackendString() const override;

    void copyTo(TextureInfo::AnyTextureInfoData& dstData) const override {
        dstData.emplace<DawnTextureInfo>(*this);
    }
    bool isCompatible(const TextureInfo& that, bool requireExact) const override;
};

namespace TextureInfos {
SK_API TextureInfo MakeDawn(const DawnTextureInfo& dawnInfo);

SK_API bool GetDawnTextureInfo(const TextureInfo&, DawnTextureInfo*);
}  // namespace TextureInfos

namespace BackendTextures {
// Create a BackendTexture from a WGPUTexture. Texture info will be queried from the texture.
//
// This is the recommended way of specifying a BackendTexture for Dawn. See the note below on
// the constructor that takes a WGPUTextureView for a fuller explanation.
//
// The BackendTexture will not call retain or release on the passed in WGPUTexture. Thus, the
// client must keep the WGPUTexture valid until they are no longer using the BackendTexture.
// However, any SkImage or SkSurface that wraps the BackendTexture *will* retain and release
// the WGPUTexture.
SK_API BackendTexture MakeDawn(WGPUTexture);

// Create a BackendTexture from a WGPUTexture. Texture planeDimensions, plane aspect and
// info have to be provided. This is intended to be used only when accessing a plane
// of a WGPUTexture.
//
// The BackendTexture will not call retain or release on the passed in WGPUTexture. Thus, the
// client must keep the WGPUTexture valid until they are no longer using the BackendTexture.
// However, any SkImage or SkSurface that wraps the BackendTexture *will* retain and release
// the WGPUTexture.
SK_API BackendTexture MakeDawn(SkISize planeDimensions, const DawnTextureInfo&, WGPUTexture);

// Create a BackendTexture from a WGPUTextureView. Texture dimensions and
// info have to be provided.
//
// Using a WGPUTextureView rather than a WGPUTexture is less effecient for operations that
// require buffer transfers to or from the texture (e.g. methods on graphite::Context that read
// pixels or SkSurface::writePixels). In such cases an intermediate copy to or from a
// WGPUTexture is required. Thus, it is recommended to use this functionality only for cases
// where a WGPUTexture is unavailable, in particular when using wgpu::SwapChain.
//
// The BackendTexture will not call retain or release on the passed in WGPUTextureView. Thus,
// the client must keep the WGPUTextureView valid until they are no longer using the
// BackendTexture. However, any SkImage or SkSurface that wraps the BackendTexture *will* retain
// and release the WGPUTextureView.
SK_API BackendTexture MakeDawn(SkISize dimensions,
                               const DawnTextureInfo& info,
                               WGPUTextureView textureView);
}  // namespace BackendTextures

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTypes_DEFINED
