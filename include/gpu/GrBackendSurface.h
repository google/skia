/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurface_DEFINED
#define GrBackendSurface_DEFINED

#include "GrTypes.h"

class GrVkImageInfo;
class GrGLTextureInfo;

class GrBackendSurface {
public:
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    /**
     * If the render target flag is set and sample count is greater than 0
     * then Gr will create an MSAA buffer that resolves to the texture.
     */
    GrPixelConfig fConfig;

    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr.
    GrVkImageInfo* getVkImageInfo();

    // If the backend API is GL, this returns a pointer to the GrGLTextureInfo struct. Otherwise
    // it returns nullptr.
    GrGLTextureInfo* getGLTextureInfo();

protected:
    GrBackendSurface(int width,
                     int height,
                     GrVkImageInfo* vkInfo);

    GrBackendSurface(int width,
                     int height,
                     GrPixelConfig config,
                     GrGLTextureInfo* glInfo);

    // Temporary constructor to support the importing of GrBacknedTextureDesc and
    // GrBackendRenderTargetDesc.
    GrBackendSurface(int width,
                     int height,
                     GrPixelConfig config,
                     GrBackend backend,
                     GrBackendObject handle);

private:
    GrBackend fBackend;

    union {
        GrVkImageInfo*   fVkInfo;
        GrGLTextureInfo* fGLInfo;
        GrBackendObject  fHandle;
    };
};

class GrBackendTexture : public GrBackendSurface {
public:
    GrBackendTexture(int width,
                     int height,
                     GrVkImageInfo* vkInfo) : INHERITED(width, height, vkInfo) {}

    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     GrGLTextureInfo* glInfo) : INHERITED(width, height, config, glInfo) {}

private:
    // Temporary constructor which can be used to convert from a GrBackendTextureDesc.
    GrBackendTexture(const GrBackendTextureDesc& desc, GrBackend backend)
        : INHERITED(desc.fWidth, desc.fHeight, desc.fConfig, backend, desc.fTextureHandle) {}

    // Friending for access to above constructor taking a GrBackendTextureDesc
    friend class SkSurface;

    typedef GrBackendSurface INHERITED;
};

class GrBackendRenderTarget : public GrBackendSurface {
public:
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrVkImageInfo* vkInfo) : INHERITED(width, height, vkInfo) {}

    GrBackendRenderTarget(int width,
                          int height,
                          GrPixelConfig config,
                          int sampleCnt,
                          int stencilBits,
                          GrGLTextureInfo* glInfo) : INHERITED(width, height, config, glInfo) {}

    int fSampleCnt;
    int fStencilBits;

private:
    // Temporary constructor which can be used to convert from a GrBackendRenderTargetDesc.
    GrBackendRenderTarget(const GrBackendRenderTargetDesc& desc, GrBackend backend)
        : INHERITED(desc.fWidth, desc.fHeight, desc.fConfig, backend, desc.fRenderTargetHandle)
        , fSampleCnt(desc.fSampleCnt)
        , fStencilBits(desc.fStencilBits) {}

    // Friending for access to above constructor taking a GrBackendTextureDesc
    friend class SkSurface;

    typedef GrBackendSurface INHERITED;
};

#endif

