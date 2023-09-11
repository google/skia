/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/DawnUtilsPriv.h"

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"

namespace skgpu {

// TODO: A lot of these values are not correct
size_t DawnFormatBytesPerBlock(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:            return 4;
        case wgpu::TextureFormat::BGRA8Unorm:            return 4;
        case wgpu::TextureFormat::R8Unorm:               return 1;
        case wgpu::TextureFormat::R16Unorm:              return 2;
        case wgpu::TextureFormat::RGBA16Float:           return 8;
        case wgpu::TextureFormat::R16Float:              return 2;
        case wgpu::TextureFormat::RG8Unorm:              return 2;
        case wgpu::TextureFormat::RG16Unorm:             return 4;
        case wgpu::TextureFormat::RGB10A2Unorm:          return 4;
        case wgpu::TextureFormat::RG16Float:             return 4;
        // The depth stencil values are not neccessarily correct in Dawn since Dawn is allowed to
        // implement Stencil8 as a real stencil8 or depth24stencil8 format. Similarly the depth in
        // Depth24PlusStencil8 can either be a 24 bit value or Depth32Float value. There is also
        // currently no way to query this in WebGPU so we just use the highest values here.
        case wgpu::TextureFormat::Stencil8:              return 4; // could be backed by d24s8
        case wgpu::TextureFormat::Depth32Float:          return 4;
        case wgpu::TextureFormat::Depth32FloatStencil8:  return 5;
        case wgpu::TextureFormat::Depth24PlusStencil8:   return 5; // could be backed by d32s8
        default:
            SkUNREACHABLE;
    }
}

uint32_t DawnFormatChannels(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:   return kRGBA_SkColorChannelFlags;
        case wgpu::TextureFormat::BGRA8Unorm:   return kRGBA_SkColorChannelFlags;
        case wgpu::TextureFormat::R8Unorm:      return kRed_SkColorChannelFlag;
        case wgpu::TextureFormat::R16Unorm:     return kRed_SkColorChannelFlag;
        case wgpu::TextureFormat::RGBA16Float:  return kRGBA_SkColorChannelFlags;
        case wgpu::TextureFormat::R16Float:     return kRed_SkColorChannelFlag;
        case wgpu::TextureFormat::RG8Unorm:     return kRG_SkColorChannelFlags;
        case wgpu::TextureFormat::RG16Unorm:    return kRG_SkColorChannelFlags;
        case wgpu::TextureFormat::RGB10A2Unorm: return kRGBA_SkColorChannelFlags;
        case wgpu::TextureFormat::RG16Float:    return kRG_SkColorChannelFlags;

        default:                                return 0;
    }
    SkUNREACHABLE;
}

} // namespace skgpu

