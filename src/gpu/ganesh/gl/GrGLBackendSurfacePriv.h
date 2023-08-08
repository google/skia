/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBackendSurfacePriv_DEFINED
#define GrGLBackendSurfacePriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/gpu/ganesh/GrGLTypesPriv.h"

#include <string_view>

namespace skgpu { enum class Mipmapped : bool; }
struct GrGLTextureInfo;

namespace GrBackendTextures {
// The GrGLTextureInfo must have a valid fFormat.
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped,
                        const GrGLTextureInfo& glInfo,
                        sk_sp<GrGLTextureParameters> params,
                        std::string_view label = {});
}  // namespace GrBackendTextures

class GrGLBackendSurfacePriv {
public:
    static GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        sk_sp<GrGLTextureParameters> params,
                        std::string_view label) {
        return GrBackendTexture(width, height, mipped, glInfo, params, label);
    }
};

#endif
