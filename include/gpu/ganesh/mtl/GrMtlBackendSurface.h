/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMtlBackendSurface_DEFINED
#define GrMtlBackendSurface_DEFINED

#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "include/private/base/SkAPI.h"

#include <string_view>

class GrBackendFormat;
class GrBackendTexture;
class GrBackendRenderTarget;

namespace GrBackendFormats {
SK_API GrBackendFormat MakeMtl(GrMTLPixelFormat format);

SK_API GrMTLPixelFormat AsMtlFormat(const GrBackendFormat&);

}  // namespace GrBackendFormats

namespace GrBackendTextures {
SK_API GrBackendTexture MakeMtl(int width,
                                int height,
                                skgpu::Mipmapped,
                                const GrMtlTextureInfo& mtlInfo,
                                std::string_view label = {});

SK_API bool GetMtlTextureInfo(const GrBackendTexture&, GrMtlTextureInfo*);

}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {
SK_API GrBackendRenderTarget MakeMtl(int width, int height, const GrMtlTextureInfo& mtlInfo);

SK_API bool GetMtlTextureInfo(const GrBackendRenderTarget&, GrMtlTextureInfo*);

}  // namespace GrBackendRenderTargets

#endif
