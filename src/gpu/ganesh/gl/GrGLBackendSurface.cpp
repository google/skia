/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/gpu/ganesh/GrGLTypesPriv.h"
#include "src/gpu/ganesh/gl/GrGLBackendSurfacePriv.h"

namespace GrBackendFormats {
GrBackendFormat MakeGL(GrGLenum format, GrGLenum target) {
    return GrBackendFormat::MakeGL(format, target);
}

GrGLFormat AsGLFormat(const GrBackendFormat& format) {
    return format.asGLFormat();
}

GrGLenum AsGLFormatEnum(const GrBackendFormat& format) {
    return format.asGLFormatEnum();
}
}  // namespace GrBackendFormats

namespace GrBackendTextures {
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        std::string_view label) {
    return GrBackendTexture(width, height, mipped, glInfo, label);
}

GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        sk_sp<GrGLTextureParameters> params,
                        std::string_view label) {
    return GrGLBackendSurfacePriv::MakeGL(width, height, mipped, glInfo, params, label);
}

bool GetGLTextureInfo(const GrBackendTexture& tex, GrGLTextureInfo* outInfo) {
    return tex.getGLTextureInfo(outInfo);
}

void GLTextureParametersModified(GrBackendTexture* tex) {
    if (tex) {
        tex->glTextureParametersModified();
    }
}
}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {
// The GrGLTextureInfo must have a valid fFormat. If wrapping in an SkSurface we require the
// stencil bits to be either 0, 8 or 16.
GrBackendRenderTarget
MakeGL(int width, int height, int sampleCnt, int stencilBits, const GrGLFramebufferInfo& glInfo) {
    return GrBackendRenderTarget(width, height, sampleCnt, stencilBits, glInfo);
}

bool GetGLFramebufferInfo(const GrBackendRenderTarget& rt, GrGLFramebufferInfo* outInfo) {
    return rt.getGLFramebufferInfo(outInfo);
}

}  // namespace GrBackendRenderTargets
