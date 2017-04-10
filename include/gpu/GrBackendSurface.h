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

class GrBackendTexture {
public:
    GrBackendTexture(int width,
                     int height,
                     GrVkImageInfo* vkInfo);

    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     GrGLTextureInfo* glInfo);

    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr.
    GrVkImageInfo* getVkImageInfo();

    // If the backend API is GL, this returns a pointer to the GrGLTextureInfo struct. Otherwise
    // it returns nullptr.
    GrGLTextureInfo* getGLTextureInfo();

    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrPixelConfig fConfig;

private:
    // Temporary constructor which can be used to convert from a GrBackendTextureDesc.
    GrBackendTexture(const GrBackendTextureDesc& desc, GrBackend backend);

    // Friending for access to above constructor taking a GrBackendTextureDesc
    friend class SkImage;
    friend class SkSurface;

    GrBackend fBackend;

    union {
        GrVkImageInfo*   fVkInfo;
        GrGLTextureInfo* fGLInfo;
        GrBackendObject  fHandle;
    };
};

class GrBackendRenderTarget {
public:
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrVkImageInfo* vkInfo);

    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrPixelConfig config,
                          GrGLTextureInfo* glInfo);

    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr.
    GrVkImageInfo* getVkImageInfo();

    // If the backend API is GL, this returns a pointer to the GrGLTextureInfo struct. Otherwise
    // it returns nullptr.
    GrGLTextureInfo* getGLTextureInfo();

    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels

    int fSampleCnt;
    int fStencilBits;
    GrPixelConfig fConfig;

private:
    // Temporary constructor which can be used to convert from a GrBackendRenderTargetDesc.
    GrBackendRenderTarget(const GrBackendRenderTargetDesc& desc, GrBackend backend);

    // Friending for access to above constructor taking a GrBackendTextureDesc
    friend class SkSurface;

    GrBackend fBackend;

    union {
        GrVkImageInfo*   fVkInfo;
        GrGLTextureInfo* fGLInfo;
        GrBackendObject  fHandle;
    };
};

#endif

