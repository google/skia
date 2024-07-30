/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTypesPriv_DEFINED
#define skgpu_graphite_DawnTypesPriv_DEFINED

#include "include/core/SkString.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"

namespace skgpu::graphite {
#if !defined(__EMSCRIPTEN__)
namespace ycbcrUtils {
bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor&, const wgpu::YCbCrVkDescriptor&);
}
#endif

struct DawnTextureSpec {
    DawnTextureSpec() = default;
    DawnTextureSpec(const DawnTextureInfo& info)
            : fFormat(info.fFormat)
            , fViewFormat(info.fViewFormat)
            , fUsage(info.fUsage)
            , fAspect(info.fAspect)
#if !defined(__EMSCRIPTEN__)
            , fYcbcrVkDescriptor(info.fYcbcrVkDescriptor)
#endif
            , fSlice(info.fSlice) {
    }

    bool operator==(const DawnTextureSpec& that) const {
        return fUsage == that.fUsage && fFormat == that.fFormat &&
               fViewFormat == that.fViewFormat && fAspect == that.fAspect &&
#if !defined(__EMSCRIPTEN__)
               ycbcrUtils::DawnDescriptorsAreEquivalent(fYcbcrVkDescriptor,
                                                        that.fYcbcrVkDescriptor) &&
#endif
               fSlice == that.fSlice;
    }

    bool isCompatible(const DawnTextureSpec& that) const {
        // The usages may match or the usage passed in may be a superset of the usage stored within.
        // The YCbCrInfo must be equal.
        // The aspect should either match the plane aspect or should be All.
        return getViewFormat() == that.getViewFormat() && (fUsage & that.fUsage) == fUsage &&
#if !defined(__EMSCRIPTEN__)
               ycbcrUtils::DawnDescriptorsAreEquivalent(fYcbcrVkDescriptor,
                                                        that.fYcbcrVkDescriptor) &&
#endif
               (fAspect == that.fAspect || fAspect == wgpu::TextureAspect::All);
    }

    wgpu::TextureFormat getViewFormat() const {
        return fViewFormat != wgpu::TextureFormat::Undefined ? fViewFormat : fFormat;
    }

    SkString toString() const;

    wgpu::TextureFormat fFormat = wgpu::TextureFormat::Undefined;
    // `fViewFormat` is always single plane format or plane view format for a multiplanar
    // wgpu::Texture.
    wgpu::TextureFormat fViewFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureUsage fUsage = wgpu::TextureUsage::None;
    wgpu::TextureAspect fAspect = wgpu::TextureAspect::All;
#if !defined(__EMSCRIPTEN__)
    wgpu::YCbCrVkDescriptor fYcbcrVkDescriptor = {};
#endif
    uint32_t fSlice = 0;
};

DawnTextureInfo DawnTextureSpecToTextureInfo(const DawnTextureSpec& dawnSpec,
                                             uint32_t sampleCount,
                                             Mipmapped mipmapped);

DawnTextureInfo DawnTextureInfoFromWGPUTexture(WGPUTexture texture);

namespace TextureInfos {
DawnTextureSpec GetDawnTextureSpec(const TextureInfo& dawnInfo);

wgpu::TextureFormat GetDawnViewFormat(const TextureInfo& dawnInfo);
wgpu::TextureAspect GetDawnAspect(const TextureInfo& dawnInfo);
}  // namespace TextureInfos

namespace BackendTextures {
WGPUTexture GetDawnTexturePtr(const BackendTexture&);
WGPUTextureView GetDawnTextureViewPtr(const BackendTexture&);
}  // namespace BackendTextures

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnTypesPriv_DEFINED
