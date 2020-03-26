/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnUtil_DEFINED
#define GrDawnUtil_DEFINED

#include "include/private/GrTypesPriv.h"
#include "dawn/webgpu_cpp.h"

size_t GrDawnBytesPerPixel(wgpu::TextureFormat format);
bool GrDawnFormatIsRenderable(wgpu::TextureFormat format);
bool GrColorTypeToDawnFormat(GrColorType colorType, wgpu::TextureFormat* format);
size_t GrDawnRoundRowBytes(size_t rowBytes);
#if GR_TEST_UTILS
const char* GrDawnFormatToStr(wgpu::TextureFormat format);
#endif

static constexpr uint32_t GrDawnFormatChannels(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:   return kRGBA_SkColorChannelFlags;
        case wgpu::TextureFormat::BGRA8Unorm:   return kRGBA_SkColorChannelFlags;
        case wgpu::TextureFormat::R8Unorm:      return kRed_SkColorChannelFlag;

        default:                                return 0;
    }
}

#endif // GrDawnUtil_DEFINED
