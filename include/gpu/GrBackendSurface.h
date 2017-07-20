/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurface_DEFINED
#define GrBackendSurface_DEFINED

#include "GrTypes.h"
#include "gl/GrGLTypes.h"
#include "mock/GrMockTypes.h"

#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#endif

class SK_API GrBackendTexture {
public:
    // Creates an invalid backend texture.
    GrBackendTexture() : fConfig(kUnknown_GrPixelConfig) {}

    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     const GrGLTextureInfo& glInfo);

#ifdef SK_VULKAN
    GrBackendTexture(int width,
                     int height,
                     const GrVkImageInfo& vkInfo);
#endif

    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     const GrMockTextureInfo& mockInfo);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrPixelConfig config() const { return fConfig; }
    GrBackend backend() const {return fBackend; }

    // If the backend API is GL, this returns a pointer to the GrGLTextureInfo struct. Otherwise
    // it returns nullptr.
    const GrGLTextureInfo* getGLTextureInfo() const;

#ifdef SK_VULKAN
    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr.
    const GrVkImageInfo* getVkImageInfo() const;
#endif

    // If the backend API is Mock, this returns a pointer to the GrMockTextureInfo struct. Otherwise
    // it returns nullptr.
    const GrMockTextureInfo* getMockTextureInfo() const;

private:
    bool isValid() const { return fConfig != kUnknown_GrPixelConfig; }

    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrPixelConfig fConfig;
    GrBackend fBackend;

    union {
        GrGLTextureInfo fGLInfo;
#ifdef SK_VULKAN
        GrVkImageInfo   fVkInfo;
#endif
        GrMockTextureInfo fMockInfo;
    };
};

class SK_API GrBackendRenderTarget {
public:
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrPixelConfig config,
                          const GrGLFramebufferInfo& glInfo);

#ifdef SK_VULKAN
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrVkImageInfo& vkInfo);
#endif

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int sampleCnt() const { return fSampleCnt; }
    int stencilBits() const { return fStencilBits; }
    GrPixelConfig config() const { return fConfig; }
    GrBackend backend() const {return fBackend; }

    // If the backend API is GL, this returns a pointer to the GrGLFramebufferInfo struct. Otherwise
    // it returns nullptr.
    const GrGLFramebufferInfo* getGLFramebufferInfo() const;

#ifdef SK_VULKAN
    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr
    const GrVkImageInfo* getVkImageInfo() const;
#endif

private:
    // Temporary constructor which can be used to convert from a GrBackendRenderTargetDesc.
    GrBackendRenderTarget(const GrBackendRenderTargetDesc& desc, GrBackend backend);

    // Friending for access to above constructor taking a GrBackendRenderTargetDesc
    friend class SkSurface;

    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels

    int fSampleCnt;
    int fStencilBits;
    GrPixelConfig fConfig;

    GrBackend fBackend;

    union {
        GrGLFramebufferInfo fGLInfo;
#ifdef SK_VULKAN
        GrVkImageInfo   fVkInfo;
#endif
    };
};

#endif

