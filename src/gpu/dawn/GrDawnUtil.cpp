/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnUtil.h"

size_t GrDawnBytesPerBlock(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::BGRA8Unorm:
            return 4;
        case wgpu::TextureFormat::R8Unorm:
            return 1;
        case wgpu::TextureFormat::Depth24PlusStencil8:
            return 4;
        default:
            SkUNREACHABLE;
    }
}

int GrDawnFormatStencilBits(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::R8Unorm:
            return 0;
        case wgpu::TextureFormat::Depth24PlusStencil8:
            return 8;
        default:
            SkUNREACHABLE;
    }
}

bool GrDawnFormatIsRenderable(wgpu::TextureFormat format) {
    // For now, all the formats above are renderable. If a non-renderable format is added
    // (see dawn/src/dawn_native/Format.cpp), an exception should be added here.
    return true;
}

bool GrColorTypeToDawnFormat(GrColorType ct, wgpu::TextureFormat* format) {
    switch (ct) {
        case GrColorType::kRGBA_8888:
            *format = wgpu::TextureFormat::RGBA8Unorm;
            return true;
        case GrColorType::kBGRA_8888:
            *format = wgpu::TextureFormat::BGRA8Unorm;
            return true;
        case GrColorType::kAlpha_8:
        case GrColorType::kGray_8:
            *format = wgpu::TextureFormat::R8Unorm;
            return true;
        default:
            return false;
    }
}

bool GrDawnFormatToGrColorType(wgpu::TextureFormat format, GrColorType* colorType) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
            *colorType = GrColorType::kRGBA_8888;
            return true;
        case wgpu::TextureFormat::BGRA8Unorm:
            *colorType = GrColorType::kBGRA_8888;
            return true;
        case wgpu::TextureFormat::R8Unorm:
            *colorType = GrColorType::kR_8;
            return true;
        default:
            return false;
    }
}

size_t GrDawnRoundRowBytes(size_t rowBytes) {
    // Dawn requires that rowBytes be a multiple of 256. (This is actually imposed by D3D12.)
    return (rowBytes + 0xFF) & ~0xFF;
}

#if defined(SK_DEBUG) || GR_TEST_UTILS
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
            SkUNREACHABLE;
    }
}
#endif
