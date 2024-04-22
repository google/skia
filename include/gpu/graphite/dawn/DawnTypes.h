/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTypes_DEFINED
#define skgpu_graphite_DawnTypes_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {

struct DawnTextureInfo {
    uint32_t fSampleCount = 1;
    Mipmapped fMipmapped = Mipmapped::kNo;

    // wgpu::TextureDescriptor properties
    wgpu::TextureFormat fFormat = wgpu::TextureFormat::Undefined;
    // `fViewFormat` for multiplanar formats corresponds to the plane TextureView's format.
    wgpu::TextureFormat fViewFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureUsage fUsage = wgpu::TextureUsage::None;
    // TODO(b/308944094): Migrate aspect information to BackendTextureViews.
    wgpu::TextureAspect fAspect = wgpu::TextureAspect::All;
    uint32_t fSlice = 0;

    wgpu::TextureFormat getViewFormat() const {
        return fViewFormat != wgpu::TextureFormat::Undefined ? fViewFormat : fFormat;
    }

    DawnTextureInfo() = default;
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
            : fSampleCount(sampleCount)
            , fMipmapped(mipmapped)
            , fFormat(format)
            , fViewFormat(viewFormat)
            , fUsage(usage)
            , fAspect(aspect)
            , fSlice(slice) {}
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTypes_DEFINED


