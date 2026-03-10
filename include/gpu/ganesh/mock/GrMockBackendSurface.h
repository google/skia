/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockBackendSurface_DEFINED
#define GrMockBackendSurface_DEFINED

#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/mock/GrMockTypes.h"
#include "include/private/base/SkAPI.h"

#include <string_view>

class GrBackendFormat;
class GrBackendTexture;
class GrBackendRenderTarget;
enum class SkTextureCompressionType;
enum class GrColorType;

namespace GrBackendFormats {

SK_API GrBackendFormat MakeMockColorType(GrColorType colorType);
SK_API GrBackendFormat MakeMockCompressionType(SkTextureCompressionType compression);
SK_API GrBackendFormat MakeMockStencilFormat();

// Only one of these getters will return a non-unknown, non-false value
SK_API GrColorType AsMockColorType(const GrBackendFormat&);
SK_API SkTextureCompressionType AsMockCompressionType(const GrBackendFormat&);
SK_API bool IsMockStencilFormat(const GrBackendFormat&);

}  // namespace GrBackendFormats

namespace GrBackendTextures {

SK_API GrBackendTexture MakeMock(int width,
                                 int height,
                                 skgpu::Mipmapped,
                                 const GrMockTextureInfo& mockInfo,
                                 std::string_view label = {});

SK_API GrMockTextureInfo GetMockTextureInfo(const GrBackendTexture&);

}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {

SK_API GrBackendRenderTarget MakeMock(int width,
                                      int height,
                                      int sampleCnt,
                                      int stencilBits,
                                      const GrMockRenderTargetInfo& mockInfo);

SK_API GrMockRenderTargetInfo GetMockRenderTargetInfo(const GrBackendRenderTarget&);

}  // namespace GrBackendRenderTargets

#endif
