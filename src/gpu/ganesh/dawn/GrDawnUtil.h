/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnUtil_DEFINED
#define GrDawnUtil_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "webgpu/webgpu_cpp.h"

int GrDawnFormatStencilBits(wgpu::TextureFormat format);
bool GrDawnFormatIsRenderable(wgpu::TextureFormat format);
bool GrColorTypeToDawnFormat(GrColorType colorType, wgpu::TextureFormat* format);
bool GrDawnFormatToGrColorType(wgpu::TextureFormat format, GrColorType* colorType);
size_t GrDawnRoundRowBytes(size_t rowBytes);
#if defined(SK_DEBUG) || GR_TEST_UTILS
const char* GrDawnFormatToStr(wgpu::TextureFormat format);
#endif

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
