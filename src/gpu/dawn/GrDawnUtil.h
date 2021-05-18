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

size_t GrDawnBytesPerBlock(wgpu::TextureFormat format);
int GrDawnFormatStencilBits(wgpu::TextureFormat format);
bool GrDawnFormatIsRenderable(wgpu::TextureFormat format);
bool GrColorTypeToDawnFormat(GrColorType colorType, wgpu::TextureFormat* format);
bool GrDawnFormatToGrColorType(wgpu::TextureFormat format, GrColorType* colorType);
size_t GrDawnRoundRowBytes(size_t rowBytes);
#if defined(SK_DEBUG) || GR_TEST_UTILS
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

static constexpr GrColorFormatDesc GrDawnFormatDesc(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case wgpu::TextureFormat::BGRA8Unorm:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case wgpu::TextureFormat::R8Unorm:
            return GrColorFormatDesc::MakeR(8, GrColorTypeEncoding::kUnorm);

        default:
            return GrColorFormatDesc::MakeInvalid();
    }
}

#endif // GrDawnUtil_DEFINED
