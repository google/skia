/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"

namespace skgpu::graphite {

bool DawnTextureSpec::operator==(const DawnTextureSpec& that) const {
    return fUsage == that.fUsage && fFormat == that.fFormat &&
           fViewFormat == that.fViewFormat && fAspect == that.fAspect &&
#if !defined(__EMSCRIPTEN__)
           DawnDescriptorsAreEquivalent(fYcbcrVkDescriptor, that.fYcbcrVkDescriptor) &&
#endif
           fSlice == that.fSlice;
}

bool DawnTextureSpec::isCompatible(const DawnTextureSpec& that) const {
    // The usages may match or the usage passed in may be a superset of the usage stored within.
    // The YCbCrInfo must be equal.
    // The aspect should either match the plane aspect or should be All.
    return this->getViewFormat() == that.getViewFormat() &&
            (fUsage & that.fUsage) == fUsage &&
#if !defined(__EMSCRIPTEN__)
            DawnDescriptorsAreEquivalent(fYcbcrVkDescriptor, that.fYcbcrVkDescriptor) &&
#endif
            (fAspect == that.fAspect || fAspect == wgpu::TextureAspect::All);
}

SkString DawnTextureSpec::toString() const {
    return SkStringPrintf("format=%u,viewFormat=%u,usage=0x%08X,aspect=0x%08X,slice=%u",
                          static_cast<unsigned int>(fFormat),
                          static_cast<unsigned int>(fViewFormat),
                          static_cast<unsigned int>(fUsage),
                          static_cast<unsigned int>(fAspect),
                          fSlice);
}

DawnTextureInfo DawnTextureInfoFromWGPUTexture(WGPUTexture texture) {
    SkASSERT(texture);
    return DawnTextureInfo(
            wgpuTextureGetSampleCount(texture),
            wgpuTextureGetMipLevelCount(texture) > 1 ? Mipmapped::kYes : Mipmapped::kNo,
            /*format=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
            /*viewFormat=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
            static_cast<wgpu::TextureUsage>(wgpuTextureGetUsage(texture)),
            wgpu::TextureAspect::All,
            /*slice=*/0);
}

DawnTextureInfo DawnTextureSpecToTextureInfo(const DawnTextureSpec& dawnSpec,
                                             uint32_t sampleCount,
                                             Mipmapped mipmapped) {
    DawnTextureInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;

    // Dawn info
    info.fFormat = dawnSpec.fFormat;
    info.fViewFormat = dawnSpec.fViewFormat;
    info.fUsage = dawnSpec.fUsage;
    info.fAspect = dawnSpec.fAspect;
#if !defined(__EMSCRIPTEN__)
    info.fYcbcrVkDescriptor = dawnSpec.fYcbcrVkDescriptor;
#endif

    return info;
}

}  // namespace skgpu::graphite
