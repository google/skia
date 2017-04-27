/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurface_DEFINED
#define GrBackendSurface_DEFINED

#include "GrTypes.h"

struct GrVkImageInfo;
struct GrGLTextureInfo;

class GrBackendTexture {
public:
    // The passed in GrVkImageInfo must live until the GrBackendTexture is no longer used in
    // creation of SkImages or SkSurfaces.
    GrBackendTexture(int width,
                     int height,
                     const GrVkImageInfo* vkInfo);

    // The passed in GrGLTextureInfo must live until the GrBackendTexture is no longer used in
    // creation of SkImages or SkSurfaces.
    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     const GrGLTextureInfo* glInfo);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrPixelConfig config() const { return fConfig; }
    GrBackend backend() const {return fBackend; }

    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr.
    const GrVkImageInfo* getVkImageInfo() const;

    // If the backend API is GL, this returns a pointer to the GrGLTextureInfo struct. Otherwise
    // it returns nullptr.
    const GrGLTextureInfo* getGLTextureInfo() const;

private:
    // Temporary constructor which can be used to convert from a GrBackendTextureDesc.
    GrBackendTexture(const GrBackendTextureDesc& desc, GrBackend backend);

    // Friending for access to above constructor taking a GrBackendTextureDesc
    friend class SkImage;
    friend class SkSurface;

    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrPixelConfig fConfig;
    GrBackend fBackend;

    union {
        const GrVkImageInfo*   fVkInfo;
        const GrGLTextureInfo* fGLInfo;
        GrBackendObject  fHandle;
    };
};

class GrBackendRenderTarget {
public:
    // The passed in GrVkImageInfo must live until the GrBackendTexture is no longer used in
    // creation of SkImages or SkSurfaces.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrVkImageInfo* vkInfo);

    // The passed in GrGLTextureInfo must live until the GrBackendTexture is no longer used in
    // creation of SkImages or SkSurfaces.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrPixelConfig config,
                          const GrGLTextureInfo* glInfo);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int sampleCnt() const { return fSampleCnt; }
    int stencilBits() const { return fStencilBits; }
    GrPixelConfig config() const { return fConfig; }
    GrBackend backend() const {return fBackend; }

    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr.
    const GrVkImageInfo* getVkImageInfo() const;

    // If the backend API is GL, this returns a pointer to the GrGLTextureInfo struct. Otherwise
    // it returns nullptr.
    const GrGLTextureInfo* getGLTextureInfo() const;

private:
    // Temporary constructor which can be used to convert from a GrBackendRenderTargetDesc.
    GrBackendRenderTarget(const GrBackendRenderTargetDesc& desc, GrBackend backend);

    // Friending for access to above constructor taking a GrBackendTextureDesc
    friend class SkSurface;

    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels

    int fSampleCnt;
    int fStencilBits;
    GrPixelConfig fConfig;

    GrBackend fBackend;

    union {
        const GrVkImageInfo*   fVkInfo;
        const GrGLTextureInfo* fGLInfo;
        GrBackendObject  fHandle;
    };
};

#endif

