/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"

#include <cstdint>

namespace skgpu::graphite {

DawnTextureInfo::DawnTextureInfo(WGPUTexture texture)
        : DawnTextureInfo(
                wgpuTextureGetSampleCount(texture),
                wgpuTextureGetMipLevelCount(texture) > 1 ? Mipmapped::kYes : Mipmapped::kNo,
                /*format=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
                /*viewFormat=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
                static_cast<wgpu::TextureUsage>(wgpuTextureGetUsage(texture)),
                wgpu::TextureAspect::All,
                /*slice=*/0) {}

TextureFormat DawnTextureInfo::viewFormat() const {
#if !defined(__EMSCRIPTEN__)
    if (fYcbcrVkDescriptor.externalFormat != 0) {
        return TextureFormat::kExternal;
    }
#endif
    return DawnFormatToTextureFormat(this->getViewFormat());
}

SkString DawnTextureInfo::toBackendString() const {
    return SkStringPrintf("wgpuFormat=%u,usage=0x%08X,aspect=0x%08X,slice=%u",
                          static_cast<unsigned int>(fFormat),
                          static_cast<unsigned int>(fUsage),
                          static_cast<unsigned int>(fAspect),
                          fSlice);
}

bool DawnTextureInfo::isCompatible(const TextureInfo& that, bool requireExact) const {
    const auto& dt = TextureInfoPriv::Get<DawnTextureInfo>(that);

    // The usages may match or the usage passed in may be a superset of the usage stored within. The
    // YCbCrInfo must be equal. The aspect should either match the plane aspect or should be All.
    return this->getViewFormat() == dt.getViewFormat() &&
            (fUsage & dt.fUsage) == fUsage &&
#if !defined(__EMSCRIPTEN__)
            DawnDescriptorsAreEquivalent(fYcbcrVkDescriptor, dt.fYcbcrVkDescriptor) &&
#endif
            (fAspect == dt.fAspect || (!requireExact && fAspect == wgpu::TextureAspect::All));
}

namespace TextureInfos {

TextureInfo MakeDawn(const DawnTextureInfo& dawnInfo) {
    return TextureInfoPriv::Make(dawnInfo);
}

bool GetDawnTextureInfo(const TextureInfo& info, DawnTextureInfo* out) {
    return TextureInfoPriv::Copy(info, out);
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite
