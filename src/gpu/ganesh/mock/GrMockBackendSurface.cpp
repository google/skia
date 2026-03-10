/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/mock/GrMockBackendSurface.h"

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "src/gpu/ganesh/mock/GrMockBackendSurfacePriv.h"

namespace GrBackendFormats {

GrBackendFormat MakeMockColorType(GrColorType colorType) {
    return GrMockBackendSurfacePriv::MakeMockFormat(
            colorType, SkTextureCompressionType::kNone, false);
}

GrBackendFormat MakeMockCompressionType(SkTextureCompressionType compression) {
    return GrMockBackendSurfacePriv::MakeMockFormat(GrColorType::kUnknown, compression, false);
}

GrBackendFormat MakeMockStencilFormat() {
    return GrMockBackendSurfacePriv::MakeMockFormat(
            GrColorType::kUnknown, SkTextureCompressionType::kNone, true);
}

GrColorType AsMockColorType(const GrBackendFormat& format) {
    return GrMockBackendSurfacePriv::AsMockColorType(format);
}

SkTextureCompressionType AsMockCompressionType(const GrBackendFormat& format) {
    return GrMockBackendSurfacePriv::AsMockCompressionType(format);
}

bool IsMockStencilFormat(const GrBackendFormat& format) {
    return GrMockBackendSurfacePriv::IsMockStencilFormat(format);
}

}  // namespace GrBackendFormats

namespace GrBackendTextures {

GrBackendTexture MakeMock(int width,
                          int height,
                          skgpu::Mipmapped mipmapped,
                          const GrMockTextureInfo& mockInfo,
                          std::string_view label) {
    return GrMockBackendSurfacePriv::MakeMockTexture(width, height, mipmapped, mockInfo, label);
}

GrMockTextureInfo GetMockTextureInfo(const GrBackendTexture& tex) {
    return GrMockBackendSurfacePriv::GetMockTextureInfo(tex);
}

}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {

GrBackendRenderTarget MakeMock(int width,
                               int height,
                               int sampleCnt,
                               int stencilBits,
                               const GrMockRenderTargetInfo& mockInfo) {
    return GrMockBackendSurfacePriv::MakeMockRT(
            width, height, std::max(1, sampleCnt), stencilBits, mockInfo);
}

GrMockRenderTargetInfo GetMockRenderTargetInfo(const GrBackendRenderTarget& rt) {
    return GrMockBackendSurfacePriv::GetMockRenderTargetInfo(rt);
}

}  // namespace GrBackendRenderTargets
