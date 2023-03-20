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

// TODO: Alot of these values are not cor
size_t DawnFormatBytesPerBlock(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:            return 4;
        case wgpu::TextureFormat::BGRA8Unorm:            return 4;
        case wgpu::TextureFormat::R8Unorm:               return 1;
        case wgpu::TextureFormat::RGBA16Float:           return 8;
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
        case wgpu::TextureFormat::RGBA16Float:  return kRGBA_SkColorChannelFlags;

        default:                                return 0;
    }
    SkUNREACHABLE;
}

} // namespace skgpu

