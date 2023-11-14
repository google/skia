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

struct DawnTextureSpec {
    DawnTextureSpec()
            : fFormat(wgpu::TextureFormat::Undefined)
            , fUsage(wgpu::TextureUsage::None)
            , fAspect(wgpu::TextureAspect::All) {}
    DawnTextureSpec(const DawnTextureInfo& info)
            : fFormat(info.fFormat), fUsage(info.fUsage), fAspect(info.fAspect) {}

    bool operator==(const DawnTextureSpec& that) const {
        return fUsage == that.fUsage && fFormat == that.fFormat && fAspect == that.fAspect;
    }

    bool isCompatible(const DawnTextureSpec& that) const {
        // The usages may match or the usage passed in may be a superset of the usage stored within.
        // The aspect should either match the plane aspect or should be All.
        return fFormat == that.fFormat && (fUsage & that.fUsage) == fUsage &&
               (fAspect == that.fAspect || fAspect == wgpu::TextureAspect::All);
    }

    SkString toString() const {
        return SkStringPrintf("format=0x%08X,usage=0x%08X,aspect=0x%08X",
                              static_cast<unsigned int>(fFormat),
                              static_cast<unsigned int>(fUsage),
                              static_cast<unsigned int>(fAspect));
    }

    // `fFormat` is always single plane format or plane view format for a multiplanar wgpu::Texture.
    wgpu::TextureFormat fFormat;
    wgpu::TextureUsage fUsage;
    wgpu::TextureAspect fAspect;
};

DawnTextureInfo DawnTextureSpecToTextureInfo(const DawnTextureSpec& dawnSpec,
                                             uint32_t sampleCount,
                                             Mipmapped mipmapped);

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTypesPriv_DEFINED
