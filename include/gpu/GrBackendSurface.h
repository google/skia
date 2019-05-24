/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurface_DEFINED
#define GrBackendSurface_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/GrVkTypesPriv.h"

class GrVkImageLayout;

#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#include "include/private/GrMtlTypesPriv.h"
#endif

#if !SK_SUPPORT_GPU

// SkSurface and SkImage rely on a minimal version of these always being available
class SK_API GrBackendTexture {
public:
    GrBackendTexture() {}

    bool isValid() const { return false; }
};

class SK_API GrBackendRenderTarget {
public:
    GrBackendRenderTarget() {}

    bool isValid() const { return false; }
};
#else

class SK_API GrBackendFormat {
public:
    // Creates an invalid backend format.
    GrBackendFormat() : fValid(false) {}

    static GrBackendFormat MakeGL(GrGLenum format, GrGLenum target) {
        return GrBackendFormat(format, target);
    }

    static GrBackendFormat MakeVk(VkFormat format) {
        return GrBackendFormat(format, GrVkYcbcrConversionInfo());
    }

    // This is used for external textures and the VkFormat is assumed to be VK_FORMAT_UNDEFINED.
    // This call is only supported on Android since the GrVkYcbcrConvesionInfo contains an android
    // external format.
    static GrBackendFormat MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo);

#ifdef SK_METAL
    static GrBackendFormat MakeMtl(GrMTLPixelFormat format) {
        return GrBackendFormat(format);
    }
#endif

    static GrBackendFormat MakeMock(GrPixelConfig config) {
        return GrBackendFormat(config);
    }

    bool operator==(const GrBackendFormat& that) const;
    bool operator!=(const GrBackendFormat& that) const { return !(*this == that); }

    GrBackendApi backend() const { return fBackend; }
    GrTextureType textureType() const { return fTextureType; }

    // If the backend API is GL, these return a pointer to the format and target. Otherwise
    // it returns nullptr.
    const GrGLenum* getGLFormat() const;
    const GrGLenum* getGLTarget() const;

    // If the backend API is Vulkan, this returns a pointer to a VkFormat. Otherwise
    // it returns nullptr
    const VkFormat* getVkFormat() const;

    const GrVkYcbcrConversionInfo* getVkYcbcrConversionInfo() const;

#ifdef SK_METAL
    // If the backend API is Metal, this returns a pointer to a GrMTLPixelFormat. Otherwise
    // it returns nullptr
    const GrMTLPixelFormat* getMtlFormat() const;
#endif

    // If the backend API is Mock, this returns a pointer to a GrPixelConfig. Otherwise
    // it returns nullptr.
    const GrPixelConfig* getMockFormat() const;

    // If possible, copies the GrBackendFormat and forces the texture type to be Texture2D. If the
    // GrBackendFormat was for Vulkan and it originally had a GrVkYcbcrConversionInfo, we will
    // remove the conversion and set the format to be VK_FORMAT_R8G8B8A8_UNORM.
    GrBackendFormat makeTexture2D() const;

    // Returns true if the backend format has been initialized.
    bool isValid() const { return fValid; }

private:
    GrBackendFormat(GrGLenum format, GrGLenum target);

    GrBackendFormat(const VkFormat vkFormat, const GrVkYcbcrConversionInfo&);

#ifdef SK_METAL
    GrBackendFormat(const GrMTLPixelFormat mtlFormat);
#endif

    GrBackendFormat(const GrPixelConfig config);

    GrBackendApi fBackend;
    bool         fValid;

    union {
        GrGLenum         fGLFormat; // the sized, internal format of the GL resource
        struct {
            VkFormat                 fFormat;
            GrVkYcbcrConversionInfo  fYcbcrConversionInfo;
        }                fVk;
#ifdef SK_METAL
        GrMTLPixelFormat fMtlFormat;
#endif
        GrPixelConfig    fMockFormat;
    };
    GrTextureType fTextureType;
};

class SK_API GrBackendTexture {
public:
    // Creates an invalid backend texture.
    GrBackendTexture() : fIsValid(false) {}

    // The GrGLTextureInfo must have a valid fFormat.
    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
                     const GrGLTextureInfo& glInfo);

    GrBackendTexture(int width,
                     int height,
                     const GrVkImageInfo& vkInfo);

#ifdef SK_METAL
    // This will take a strong reference to the MTLTexture in mtlInfo.
    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
                     const GrMtlTextureInfo& mtlInfo);
#endif

    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
                     const GrMockTextureInfo& mockInfo);

    GrBackendTexture(const GrBackendTexture& that);

    ~GrBackendTexture();

    GrBackendTexture& operator=(const GrBackendTexture& that);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    bool hasMipMaps() const { return GrMipMapped::kYes == fMipMapped; }
    GrBackendApi backend() const {return fBackend; }

    // If the backend API is GL, copies a snapshot of the GrGLTextureInfo struct into the passed in
    // pointer and returns true. Otherwise returns false if the backend API is not GL.
    bool getGLTextureInfo(GrGLTextureInfo*) const;

    // If the backend API is Vulkan, copies a snapshot of the GrVkImageInfo struct into the passed
    // in pointer and returns true. This snapshot will set the fImageLayout to the current layout
    // state. Otherwise returns false if the backend API is not Vulkan.
    bool getVkImageInfo(GrVkImageInfo*) const;

    // Anytime the client changes the VkImageLayout of the VkImage captured by this
    // GrBackendTexture, they must call this function to notify Skia of the changed layout.
    void setVkImageLayout(VkImageLayout);

#ifdef SK_METAL
    // If the backend API is Metal, copies a snapshot of the GrMtlTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Metal.
    bool getMtlTextureInfo(GrMtlTextureInfo*) const;
#endif

    // Get the GrBackendFormat for this texture (or an invalid format if this is not valid).
    GrBackendFormat getBackendFormat() const;

    // If the backend API is Mock, copies a snapshot of the GrMockTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Mock.
    bool getMockTextureInfo(GrMockTextureInfo*) const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }

    // Returns true if both textures are valid and refer to the same API texture.
    bool isSameTexture(const GrBackendTexture&);

#if GR_TEST_UTILS
    // We can remove the pixelConfig getter and setter once we remove the GrPixelConfig from the
    // GrBackendTexture and plumb the GrPixelconfig manually throughout our code (or remove all use
    // of GrPixelConfig in general).
    GrPixelConfig pixelConfig() const { return fConfig; }
    void setPixelConfig(GrPixelConfig config) { fConfig = config; }

    static bool TestingOnly_Equals(const GrBackendTexture& , const GrBackendTexture&);
#endif

private:
    // Friending for access to the GrPixelConfig
    friend class SkImage;
    friend class SkImage_Gpu;
    friend class SkImage_GpuBase;
    friend class SkImage_GpuYUVA;
    friend class SkPromiseImageHelper;
    friend class SkSurface;
    friend class SkSurface_Gpu;
    friend class GrAHardwareBufferImageGenerator;
    friend class GrBackendTextureImageGenerator;
    friend class GrProxyProvider;
    friend class GrGpu;
    friend class GrGLGpu;
    friend class GrVkGpu;
    friend class GrMtlGpu;
    friend class PromiseImageHelper;

    GrPixelConfig config() const { return fConfig; }

   // Requires friending of GrVkGpu (done above already)
   sk_sp<GrVkImageLayout> getGrVkImageLayout() const;

   friend class GrVkTexture;
#ifdef SK_VULKAN
   GrBackendTexture(int width,
                    int height,
                    const GrVkImageInfo& vkInfo,
                    sk_sp<GrVkImageLayout> layout);
#endif

    // Free and release and resources being held by the GrBackendTexture.
    void cleanup();

    bool fIsValid;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrPixelConfig fConfig;
    GrMipMapped fMipMapped;
    GrBackendApi fBackend;

    union {
        GrGLTextureInfo fGLInfo;
        GrVkBackendSurfaceInfo fVkInfo;
        GrMockTextureInfo fMockInfo;
    };
#ifdef SK_METAL
    GrMtlBackendSurfaceInfo fMtlInfo;
#endif
};

class SK_API GrBackendRenderTarget {
public:
    // Creates an invalid backend texture.
    GrBackendRenderTarget() : fIsValid(false) {}

    // The GrGLTextureInfo must have a valid fFormat.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrGLFramebufferInfo& glInfo);

    /** Deprecated, use version that does not take stencil bits. */
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrVkImageInfo& vkInfo);
    GrBackendRenderTarget(int width, int height, int sampleCnt, const GrVkImageInfo& vkInfo);

#ifdef SK_METAL
    // This will take a strong reference to the MTLTexture in mtlInfo.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          const GrMtlTextureInfo& mtlInfo);
#endif

    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrMockRenderTargetInfo& mockInfo);

    ~GrBackendRenderTarget();

    GrBackendRenderTarget(const GrBackendRenderTarget& that);
    GrBackendRenderTarget& operator=(const GrBackendRenderTarget&);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int sampleCnt() const { return fSampleCnt; }
    int stencilBits() const { return fStencilBits; }
    GrBackendApi backend() const {return fBackend; }

    // If the backend API is GL, copies a snapshot of the GrGLFramebufferInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not GL.
    bool getGLFramebufferInfo(GrGLFramebufferInfo*) const;

    // If the backend API is Vulkan, copies a snapshot of the GrVkImageInfo struct into the passed
    // in pointer and returns true. This snapshot will set the fImageLayout to the current layout
    // state. Otherwise returns false if the backend API is not Vulkan.
    bool getVkImageInfo(GrVkImageInfo*) const;

    // Anytime the client changes the VkImageLayout of the VkImage captured by this
    // GrBackendRenderTarget, they must call this function to notify Skia of the changed layout.
    void setVkImageLayout(VkImageLayout);

#ifdef SK_METAL
    // If the backend API is Metal, copies a snapshot of the GrMtlTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Metal.
    bool getMtlTextureInfo(GrMtlTextureInfo*) const;
#endif

    // If the backend API is Mock, copies a snapshot of the GrMockTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Mock.
    bool getMockRenderTargetInfo(GrMockRenderTargetInfo*) const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }


#if GR_TEST_UTILS
    // We can remove the pixelConfig getter and setter once we remove the pixel config from the
    // GrBackendRenderTarget and plumb the pixel config manually throughout our code (or remove all
    // use of GrPixelConfig in general).
    GrPixelConfig pixelConfig() const { return fConfig; }
    void setPixelConfig(GrPixelConfig config) { fConfig = config; }

    static bool TestingOnly_Equals(const GrBackendRenderTarget&, const GrBackendRenderTarget&);
#endif

private:
    // Friending for access to the GrPixelConfig
    friend class SkSurface;
    friend class SkSurface_Gpu;
    friend class SkImage_Gpu;
    friend class GrGpu;
    friend class GrGLGpu;
    friend class GrProxyProvider;
    friend class GrVkGpu;
    friend class GrMtlGpu;
    GrPixelConfig config() const { return fConfig; }

   // Requires friending of GrVkGpu (done above already)
   sk_sp<GrVkImageLayout> getGrVkImageLayout() const;

   friend class GrVkRenderTarget;
   GrBackendRenderTarget(int width, int height, int sampleCnt, const GrVkImageInfo& vkInfo,
                         sk_sp<GrVkImageLayout> layout);

    // Free and release and resources being held by the GrBackendTexture.
    void cleanup();

    bool fIsValid;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels

    int fSampleCnt;
    int fStencilBits;
    GrPixelConfig fConfig;
    GrBackendApi fBackend;

    union {
        GrGLFramebufferInfo fGLInfo;
        GrVkBackendSurfaceInfo fVkInfo;
        GrMockRenderTargetInfo fMockInfo;
    };
#ifdef SK_METAL
    GrMtlBackendSurfaceInfo fMtlInfo;
#endif
};

#endif

#endif

