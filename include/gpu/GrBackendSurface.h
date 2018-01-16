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

    // GrGLTextureInfo::fFormat is ignored
    // Deprecated: Should use version that does not take a GrPixelConfig instead
    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     const GrGLTextureInfo& glInfo);

    // GrGLTextureInfo::fFormat is ignored
    // Deprecated: Should use version that does not take a GrPixelConfig instead
    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     GrMipMapped,
                     const GrGLTextureInfo& glInfo);

    // The GrGLTextureInfo must have a valid fFormat.
    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
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

    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     GrMipMapped,
                     const GrMockTextureInfo& mockInfo);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    bool hasMipMaps() const { return GrMipMapped::kYes == fMipMapped; }
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

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fConfig != kUnknown_GrPixelConfig; }

private:
    // Friending for access to the GrPixelConfig
    friend class SkImage;
    friend class SkSurface;
    friend class GrBackendTextureImageGenerator;
    friend class GrGpu;
    friend class GrGLGpu;
    friend class GrVkGpu;
    GrPixelConfig config() const { return fConfig; }

    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrPixelConfig fConfig;
    GrMipMapped fMipMapped;
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
    // Creates an invalid backend texture.
    GrBackendRenderTarget() : fConfig(kUnknown_GrPixelConfig) {}

    // GrGLTextureInfo::fFormat is ignored
    // Deprecated: Should use version that does not take a GrPixelConfig instead
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrPixelConfig config,
                          const GrGLFramebufferInfo& glInfo);

    // The GrGLTextureInfo must have a valid fFormat.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
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
    GrBackend backend() const {return fBackend; }

    // If the backend API is GL, this returns a pointer to the GrGLFramebufferInfo struct. Otherwise
    // it returns nullptr.
    const GrGLFramebufferInfo* getGLFramebufferInfo() const;

#ifdef SK_VULKAN
    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr
    const GrVkImageInfo* getVkImageInfo() const;
#endif

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fConfig != kUnknown_GrPixelConfig; }

private:
    // Friending for access to the GrPixelConfig
    friend class SkSurface;
    friend class SkSurface_Gpu;
    friend class SkImage_Gpu;
    friend class GrGpu;
    friend class GrGLGpu;
    friend class GrVkGpu;
    GrPixelConfig config() const { return fConfig; }

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

