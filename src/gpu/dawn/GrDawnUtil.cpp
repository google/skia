/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnUtil.h"

size_t GrDawnBytesPerPixel(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::BGRA8Unorm:
            return 4;
        case wgpu::TextureFormat::R8Unorm:
            return 1;
        case wgpu::TextureFormat::Depth24PlusStencil8:
            return 4;
        default:
            SkASSERT(false);
            return 4;
    }
}

bool GrDawnFormatIsRenderable(wgpu::TextureFormat format) {
    // For now, all the formats above are renderable. If a non-renderable format is added
    // (see dawn/src/dawn_native/Format.cpp), an exception should be added here.
    return true;
}

bool GrPixelConfigToDawnFormat(GrPixelConfig config, wgpu::TextureFormat* format) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kGray_8_GrPixelConfig:
            *format = wgpu::TextureFormat::RGBA8Unorm;
            return true;
        case kBGRA_8888_GrPixelConfig:
            *format = wgpu::TextureFormat::BGRA8Unorm;
            return true;
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
            *format = wgpu::TextureFormat::R8Unorm;
            return true;
        default:
            return false;
    }
}

size_t GrDawnRoundRowBytes(size_t rowBytes) {
    // Dawn requires that rowBytes be a multiple of 256. (This is actually imposed by D3D12.)
    return (rowBytes + 0xFF) & ~0xFF;
}

#if GR_TEST_UTILS
const char* GrDawnFormatToStr(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
            return "RGBA8Unorm";
        case wgpu::TextureFormat::BGRA8Unorm:
            return "BGRA8Unorm";
        case wgpu::TextureFormat::R8Unorm:
            return "R8Unorm";
        case wgpu::TextureFormat::Depth24PlusStencil8:
            return "Depth24PlusStencil8";
        default:
            SkASSERT(false);
            return "Unknown";
    }
}
#endif
