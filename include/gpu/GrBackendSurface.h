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
#include "include/private/GrGLTypesPriv.h"
#include "include/private/GrVkTypesPriv.h"

#ifdef SK_DAWN
#include "include/gpu/dawn/GrDawnTypes.h"
#endif

class GrVkImageLayout;
class GrGLTextureParameters;

#ifdef SK_DAWN
#include "dawn/dawncpp.h"
#endif

#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#endif

#if GR_TEST_UTILS
class SkString;
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

enum class GrGLFormat;

class SK_API GrBackendFormat {
public:
    // Creates an invalid backend format.
    GrBackendFormat() {}

    GrBackendFormat(const GrBackendFormat& src);

    static GrBackendFormat MakeGL(GrGLenum format, GrGLenum target) {
        return GrBackendFormat(format, target);
    }

    static GrBackendFormat MakeVk(VkFormat format) {
        return GrBackendFormat(format, GrVkYcbcrConversionInfo());
    }

    static GrBackendFormat MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo);

#ifdef SK_DAWN
    static GrBackendFormat MakeDawn(dawn::TextureFormat format) {
        return GrBackendFormat(format);
    }
#endif

#ifdef SK_METAL
    static GrBackendFormat MakeMtl(GrMTLPixelFormat format) {
        return GrBackendFormat(format);
    }
#endif

    static GrBackendFormat MakeMock(GrColorType colorType) {
        return GrBackendFormat(colorType);
    }

    bool operator==(const GrBackendFormat& that) const;
    bool operator!=(const GrBackendFormat& that) const { return !(*this == that); }

    GrBackendApi backend() const { return fBackend; }
    GrTextureType textureType() const { return fTextureType; }

    /**
     * If the backend API is GL this gets the format as a GrGLFormat. Otherwise, returns
     * GrGLFormat::kUnknown.
     */
    GrGLFormat asGLFormat() const;

    /**
     * If the backend API is Vulkan this gets the format as a VkFormat and returns true. Otherwise,
     * returns false.
     */
    bool asVkFormat(VkFormat*) const;

    const GrVkYcbcrConversionInfo* getVkYcbcrConversionInfo() const;

#ifdef SK_DAWN
    /**
     * If the backend API is Dawn this gets the format as a dawn::TextureFormat and returns true.
     * Otherwise, returns false.
     */
    bool asDawnFormat(dawn::TextureFormat*) const;
#endif

#ifdef SK_METAL
    /**
     * If the backend API is Metal this gets the format as a GrMtlPixelFormat. Otherwise,
     * Otherwise, returns MTLPixelFormatInvalid.
     */
    GrMTLPixelFormat asMtlFormat() const;
#endif

    /**
     * If the backend API is Mock this gets the format as a GrColorType. Otherwise, returns
     * GrColorType::kUnknown.
     */
    GrColorType asMockColorType() const;

    // If possible, copies the GrBackendFormat and forces the texture type to be Texture2D. If the
    // GrBackendFormat was for Vulkan and it originally had a GrVkYcbcrConversionInfo, we will
    // remove the conversion and set the format to be VK_FORMAT_R8G8B8A8_UNORM.
    GrBackendFormat makeTexture2D() const;

    // Returns true if the backend format has been initialized.
    bool isValid() const { return fValid; }

#if GR_TEST_UTILS
    SkString toStr() const;
#endif

private:
    GrBackendFormat(GrGLenum format, GrGLenum target);

    GrBackendFormat(const VkFormat vkFormat, const GrVkYcbcrConversionInfo&);

#ifdef SK_DAWN
    GrBackendFormat(dawn::TextureFormat format);
#endif

#ifdef SK_METAL
    GrBackendFormat(const GrMTLPixelFormat mtlFormat);
#endif

    GrBackendFormat(GrColorType colorType);

    GrBackendApi fBackend = GrBackendApi::kMock;
    bool         fValid = false;

    union {
        GrGLenum         fGLFormat; // the sized, internal format of the GL resource
        struct {
            VkFormat                 fFormat;
            GrVkYcbcrConversionInfo  fYcbcrConversionInfo;
        }                fVk;
#ifdef SK_DAWN
        dawn::TextureFormat fDawnFormat;
#endif

#ifdef SK_METAL
        GrMTLPixelFormat fMtlFormat;
#endif
        GrColorType      fMockColorType;
    };
    GrTextureType fTextureType = GrTextureType::kNone;
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
    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
                     const GrMtlTextureInfo& mtlInfo);
#endif

#ifdef SK_DAWN
    GrBackendTexture(int width,
                     int height,
                     const GrDawnImageInfo& dawnInfo);
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

    // Call this to indicate that the texture parameters have been modified in the GL context
    // externally to GrContext.
    void glTextureParametersModified();

#ifdef SK_DAWN
    // If the backend API is Dawn, copies a snapshot of the GrDawnImageInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Dawn.
    bool getDawnImageInfo(GrDawnImageInfo*) const;
#endif

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

    // Returns true if we are working with protected content.
    bool isProtected() const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }

    // Returns true if both textures are valid and refer to the same API texture.
    bool isSameTexture(const GrBackendTexture&);

#if GR_TEST_UTILS
    static bool TestingOnly_Equals(const GrBackendTexture& , const GrBackendTexture&);
#endif

private:

#ifdef SK_GL
    friend class GrGLTexture;
    friend class GrGLGpu;    // for getGLTextureParams
    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
                     const GrGLTextureInfo,
                     sk_sp<GrGLTextureParameters>);
    sk_sp<GrGLTextureParameters> getGLTextureParams() const;
#endif

#ifdef SK_VULKAN
    friend class GrVkTexture;
    friend class GrVkGpu;    // for getGrVkImageLayout
    GrBackendTexture(int width,
                     int height,
                     const GrVkImageInfo& vkInfo,
                     sk_sp<GrVkImageLayout> layout);
    sk_sp<GrVkImageLayout> getGrVkImageLayout() const;
#endif

    // Free and release and resources being held by the GrBackendTexture.
    void cleanup();

    bool fIsValid;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrMipMapped fMipMapped;
    GrBackendApi fBackend;

    union {
#ifdef SK_GL
        GrGLBackendTextureInfo fGLInfo;
#endif
        GrVkBackendSurfaceInfo fVkInfo;
        GrMockTextureInfo fMockInfo;
    };
#ifdef SK_METAL
    GrMtlTextureInfo fMtlInfo;
#endif
#ifdef SK_DAWN
    GrDawnImageInfo  fDawnInfo;
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

#ifdef SK_DAWN
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrDawnImageInfo& dawnInfo);
#endif

    /** Deprecated, use version that does not take stencil bits. */
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrVkImageInfo& vkInfo);
    GrBackendRenderTarget(int width, int height, int sampleCnt, const GrVkImageInfo& vkInfo);

#ifdef SK_METAL
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

#ifdef SK_DAWN
    // If the backend API is Dawn, copies a snapshot of the GrDawnImageInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Dawn.
    bool getDawnImageInfo(GrDawnImageInfo*) const;
#endif

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

    // Get the GrBackendFormat for this render target (or an invalid format if this is not valid).
    GrBackendFormat getBackendFormat() const;

    // If the backend API is Mock, copies a snapshot of the GrMockTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Mock.
    bool getMockRenderTargetInfo(GrMockRenderTargetInfo*) const;

    // Returns true if we are working with protected content.
    bool isProtected() const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }


#if GR_TEST_UTILS
    static bool TestingOnly_Equals(const GrBackendRenderTarget&, const GrBackendRenderTarget&);
#endif

private:
    friend class GrVkGpu; // for getGrVkImageLayout
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

    GrBackendApi fBackend;

    union {
#ifdef SK_GL
        GrGLFramebufferInfo fGLInfo;
#endif
        GrVkBackendSurfaceInfo fVkInfo;
        GrMockRenderTargetInfo fMockInfo;
    };
#ifdef SK_METAL
    GrMtlTextureInfo fMtlInfo;
#endif
#ifdef SK_DAWN
    GrDawnImageInfo  fDawnInfo;
#endif
};

#endif

#endif

