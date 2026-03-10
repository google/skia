/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockBackendSurfacePriv_DEFINED
#define GrMockBackendSurfacePriv_DEFINED

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"

class GrMockBackendSurfacePriv {
public:
    static GrBackendFormat MakeMockFormat(GrColorType colorType,
                                          SkTextureCompressionType compression,
                                          bool isStencilFormat) {
        return GrBackendFormat::MakeMock(colorType, compression, isStencilFormat);
    }

    static GrColorType AsMockColorType(const GrBackendFormat& format) {
        return format.asMockColorType();
    }

    static SkTextureCompressionType AsMockCompressionType(const GrBackendFormat& format) {
        return format.asMockCompressionType();
    }

    static bool IsMockStencilFormat(const GrBackendFormat& format) {
        return format.isMockStencilFormat();
    }

    static GrBackendTexture MakeMockTexture(int width,
                                            int height,
                                            skgpu::Mipmapped mipmapped,
                                            const GrMockTextureInfo& mockInfo,
                                            std::string_view label) {
        return GrBackendTexture(width, height, mipmapped, mockInfo, label);
    }

    static GrMockTextureInfo GetMockTextureInfo(const GrBackendTexture& tex) {
        GrMockTextureInfo info;
        if (tex.getMockTextureInfo(&info)) {
            return info;
        }

        return {};
    }

    static GrBackendRenderTarget MakeMockRT(int width,
                                            int height,
                                            int sampleCnt,
                                            int stencilBits,
                                            const GrMockRenderTargetInfo& mockInfo) {
        return GrBackendRenderTarget(width, height, sampleCnt, stencilBits, mockInfo);
    }

    static GrMockRenderTargetInfo GetMockRenderTargetInfo(const GrBackendRenderTarget& rt) {
        GrMockRenderTargetInfo info;
        if (rt.getMockRenderTargetInfo(&info)) {
            return info;
        }

        return {};
    }
};

#endif
