/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurface_DEFINED
#define GrBackendSurface_DEFINED

#include "include/gpu/GrBackendSurfaceMutableState.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/GrGLTypesPriv.h"
#include "include/private/GrVkTypesPriv.h"

#ifdef SK_DAWN
#include "include/gpu/dawn/GrDawnTypes.h"
#endif

class GrBackendSurfaceMutableStateImpl;
class GrVkImageLayout;
class GrGLTextureParameters;
class GrColorFormatDesc;

#ifdef SK_DAWN
#include "dawn/webgpu_cpp.h"
#endif

#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#endif

#ifdef SK_DIRECT3D
#include "include/gpu/d3d/GrD3DTypesMinimal.h"
#include "include/private/GrD3DTypesPriv.h"
class GrD3DResourceState;
#endif

#if defined(SK_DEBUG) || GR_TEST_UTILS
class SkString;
#endif

#if !SK_SUPPORT_GPU

// SkSurfaceCharacterization always needs a minimal version of this
class SK_API GrBackendFormat {
public:
    bool isValid() const { return false; }
};

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
    bool isFramebufferOnly() const { return false; }
};
#else

enum class GrGLFormat;

class SK_API GrBackendFormat {
public:
    // Creates an invalid backend format.
    GrBackendFormat() {}
    GrBackendFormat(const GrBackendFormat&);
    GrBackendFormat& operator=(const GrBackendFormat&);

    static GrBackendFormat MakeGL(GrGLenum format, GrGLenum target) {
        return GrBackendFormat(format, target);
    }

    static GrBackendFormat MakeVk(VkFormat format) {
        return GrBackendFormat(format, GrVkYcbcrConversionInfo());
    }

    static GrBackendFormat MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo);

#ifdef SK_DAWN
    static GrBackendFormat MakeDawn(wgpu::TextureFormat format) {
        return GrBackendFormat(format);
    }
#endif

#ifdef SK_METAL
    static GrBackendFormat MakeMtl(GrMTLPixelFormat format) {
        return GrBackendFormat(format);
    }
#endif

#ifdef SK_DIRECT3D
    static GrBackendFormat MakeDxgi(DXGI_FORMAT format) {
        return GrBackendFormat(format);
    }
#endif

    static GrBackendFormat MakeMock(GrColorType colorType, SkImage::CompressionType compression,
                                    bool isStencilFormat = false);

    bool operator==(const GrBackendFormat& that) const;
    bool operator!=(const GrBackendFormat& that) const { return !(*this == that); }

    GrBackendApi backend() const { return fBackend; }
    GrTextureType textureType() const { return fTextureType; }

    /**
     * Gets the channels present in the format as a bitfield of SkColorChannelFlag values.
     * Luminance channels are reported as kGray_SkColorChannelFlag.
     */
    uint32_t channelMask() const;

    GrColorFormatDesc desc() const;

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
     * If the backend API is Dawn this gets the format as a wgpu::TextureFormat and returns true.
     * Otherwise, returns false.
     */
    bool asDawnFormat(wgpu::TextureFormat*) const;
#endif

#ifdef SK_METAL
    /**
     * If the backend API is Metal this gets the format as a GrMtlPixelFormat. Otherwise,
     * Otherwise, returns MTLPixelFormatInvalid.
     */
    GrMTLPixelFormat asMtlFormat() const;
#endif

#ifdef SK_DIRECT3D
    /**
     * If the backend API is Direct3D this gets the format as a DXGI_FORMAT and returns true.
     * Otherwise, returns false.
     */
    bool asDxgiFormat(DXGI_FORMAT*) const;
#endif

    /**
     * If the backend API is not Mock these three calls will return kUnknown, kNone or false,
     * respectively. Otherwise, only one of the following can be true. The GrColorType is not
     * kUnknown, the compression type is not kNone, or this is a mock stencil format.
     */
    GrColorType asMockColorType() const;
    SkImage::CompressionType asMockCompressionType() const;
    bool isMockStencilFormat() const;

    // If possible, copies the GrBackendFormat and forces the texture type to be Texture2D. If the
    // GrBackendFormat was for Vulkan and it originally had a GrVkYcbcrConversionInfo, we will
    // remove the conversion and set the format to be VK_FORMAT_R8G8B8A8_UNORM.
    GrBackendFormat makeTexture2D() const;

    // Returns true if the backend format has been initialized.
    bool isValid() const { return fValid; }

#if defined(SK_DEBUG) || GR_TEST_UTILS
    SkString toStr() const;
#endif

private:
    GrBackendFormat(GrGLenum format, GrGLenum target);

    GrBackendFormat(const VkFormat vkFormat, const GrVkYcbcrConversionInfo&);

#ifdef SK_DAWN
    GrBackendFormat(wgpu::TextureFormat format);
#endif

#ifdef SK_METAL
    GrBackendFormat(const GrMTLPixelFormat mtlFormat);
#endif

#ifdef SK_DIRECT3D
    GrBackendFormat(DXGI_FORMAT dxgiFormat);
#endif

    GrBackendFormat(GrColorType, SkImage::CompressionType, bool isStencilFormat);

#ifdef SK_DEBUG
    bool validateMock() const;
#endif

    GrBackendApi fBackend = GrBackendApi::kMock;
    bool         fValid = false;

    union {
        GrGLenum fGLFormat; // the sized, internal format of the GL resource
        struct {
            VkFormat                 fFormat;
            GrVkYcbcrConversionInfo  fYcbcrConversionInfo;
        } fVk;
#ifdef SK_DAWN
        wgpu::TextureFormat fDawnFormat;
#endif

#ifdef SK_METAL
        GrMTLPixelFormat fMtlFormat;
#endif

#ifdef SK_DIRECT3D
        DXGI_FORMAT fDxgiFormat;
#endif
        struct {
            GrColorType              fColorType;
            SkImage::CompressionType fCompressionType;
            bool                     fIsStencilFormat;
        } fMock;
    };
    GrTextureType fTextureType = GrTextureType::kNone;
};

class SK_API GrBackendTexture {
public:
    // Creates an invalid backend texture.
    GrBackendTexture();

    // The GrGLTextureInfo must have a valid fFormat.
    GrBackendTexture(int width,
                     int height,
                     GrMipmapped,
                     const GrGLTextureInfo& glInfo);

#ifdef SK_VULKAN
    GrBackendTexture(int width,
                     int height,
                     const GrVkImageInfo& vkInfo);
#endif

#ifdef SK_METAL
    GrBackendTexture(int width,
                     int height,
                     GrMipmapped,
                     const GrMtlTextureInfo& mtlInfo);
#endif

#ifdef SK_DIRECT3D
    GrBackendTexture(int width,
                     int height,
                     const GrD3DTextureResourceInfo& d3dInfo);
#endif

#ifdef SK_DAWN
    GrBackendTexture(int width,
                     int height,
                     const GrDawnTextureInfo& dawnInfo);
#endif

    GrBackendTexture(int width,
                     int height,
                     GrMipmapped,
                     const GrMockTextureInfo& mockInfo);

    GrBackendTexture(const GrBackendTexture& that);

    ~GrBackendTexture();

    GrBackendTexture& operator=(const GrBackendTexture& that);

    SkISize dimensions() const { return {fWidth, fHeight}; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrMipmapped mipmapped() const { return fMipmapped; }
    bool hasMipmaps() const { return fMipmapped == GrMipmapped::kYes; }
    /** deprecated alias of hasMipmaps(). */
    bool hasMipMaps() const { return this->hasMipmaps(); }
    GrBackendApi backend() const {return fBackend; }

    // If the backend API is GL, copies a snapshot of the GrGLTextureInfo struct into the passed in
    // pointer and returns true. Otherwise returns false if the backend API is not GL.
    bool getGLTextureInfo(GrGLTextureInfo*) const;

    // Call this to indicate that the texture parameters have been modified in the GL context
    // externally to GrContext.
    void glTextureParametersModified();

#ifdef SK_DAWN
    // If the backend API is Dawn, copies a snapshot of the GrDawnTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Dawn.
    bool getDawnTextureInfo(GrDawnTextureInfo*) const;
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

#ifdef SK_DIRECT3D
    // If the backend API is Direct3D, copies a snapshot of the GrD3DTextureResourceInfo struct into
    // the passed in pointer and returns true. This snapshot will set the fResourceState to the
    // current resource state. Otherwise returns false if the backend API is not D3D.
    bool getD3DTextureResourceInfo(GrD3DTextureResourceInfo*) const;

    // Anytime the client changes the D3D12_RESOURCE_STATES of the D3D12_RESOURCE captured by this
    // GrBackendTexture, they must call this function to notify Skia of the changed layout.
    void setD3DResourceState(GrD3DResourceStateEnum);
#endif

    // Get the GrBackendFormat for this texture (or an invalid format if this is not valid).
    GrBackendFormat getBackendFormat() const;

    // If the backend API is Mock, copies a snapshot of the GrMockTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Mock.
    bool getMockTextureInfo(GrMockTextureInfo*) const;

    // If the client changes any of the mutable backend of the GrBackendTexture they should call
    // this function to inform Skia that those values have changed. The backend API specific state
    // that can be set from this function are:
    //
    // Vulkan: VkImageLayout and QueueFamilyIndex
    void setMutableState(const GrBackendSurfaceMutableState&);

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
    friend class GrVkGpu;  // for getMutableState
    sk_sp<GrBackendSurfaceMutableStateImpl> getMutableState() const;

#ifdef SK_GL
    friend class GrGLTexture;
    friend class GrGLGpu;    // for getGLTextureParams
    GrBackendTexture(int width,
                     int height,
                     GrMipmapped,
                     const GrGLTextureInfo,
                     sk_sp<GrGLTextureParameters>);
    sk_sp<GrGLTextureParameters> getGLTextureParams() const;
#endif

#ifdef SK_VULKAN
    friend class GrVkTexture;
    GrBackendTexture(int width,
                     int height,
                     const GrVkImageInfo& vkInfo,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState);
#endif

#ifdef SK_DIRECT3D
    friend class GrD3DTexture;
    friend class GrD3DGpu;     // for getGrD3DResourceState
    GrBackendTexture(int width,
                     int height,
                     const GrD3DTextureResourceInfo& vkInfo,
                     sk_sp<GrD3DResourceState> state);
    sk_sp<GrD3DResourceState> getGrD3DResourceState() const;
#endif

    // Free and release and resources being held by the GrBackendTexture.
    void cleanup();

    bool fIsValid;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrMipmapped fMipmapped;
    GrBackendApi fBackend;

    union {
#ifdef SK_GL
        GrGLBackendTextureInfo fGLInfo;
#endif
        GrVkBackendSurfaceInfo fVkInfo;
        GrMockTextureInfo fMockInfo;
#ifdef SK_DIRECT3D
        GrD3DBackendSurfaceInfo fD3DInfo;
#endif
    };
#ifdef SK_METAL
    GrMtlTextureInfo fMtlInfo;
#endif
#ifdef SK_DAWN
    GrDawnTextureInfo fDawnInfo;
#endif

    sk_sp<GrBackendSurfaceMutableStateImpl> fMutableState;
};

class SK_API GrBackendRenderTarget {
public:
    // Creates an invalid backend texture.
    GrBackendRenderTarget();

    // The GrGLTextureInfo must have a valid fFormat. If wrapping in an SkSurface we require the
    // stencil bits to be either 0, 8 or 16.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrGLFramebufferInfo& glInfo);

#ifdef SK_DAWN
    // If wrapping in an SkSurface we require the stencil bits to be either 0, 8 or 16.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrDawnRenderTargetInfo& dawnInfo);
#endif

#ifdef SK_VULKAN
    /** Deprecated. Sample count is now part of GrVkImageInfo. */
    GrBackendRenderTarget(int width, int height, int sampleCnt, const GrVkImageInfo& vkInfo);

    GrBackendRenderTarget(int width, int height, const GrVkImageInfo& vkInfo);
#endif

#ifdef SK_METAL
    GrBackendRenderTarget(int width,
                          int height,
                          const GrMtlTextureInfo& mtlInfo);
    /** Deprecated. Sample count is ignored and is instead retrieved from the MtlTexture. */
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          const GrMtlTextureInfo& mtlInfo);
#endif

#ifdef SK_DIRECT3D
    GrBackendRenderTarget(int width,
                          int height,
                          const GrD3DTextureResourceInfo& d3dInfo);
#endif

    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrMockRenderTargetInfo& mockInfo);

    ~GrBackendRenderTarget();

    GrBackendRenderTarget(const GrBackendRenderTarget& that);
    GrBackendRenderTarget& operator=(const GrBackendRenderTarget&);

    SkISize dimensions() const { return {fWidth, fHeight}; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int sampleCnt() const { return fSampleCnt; }
    int stencilBits() const { return fStencilBits; }
    GrBackendApi backend() const {return fBackend; }
    bool isFramebufferOnly() const { return fFramebufferOnly; }

    // If the backend API is GL, copies a snapshot of the GrGLFramebufferInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not GL.
    bool getGLFramebufferInfo(GrGLFramebufferInfo*) const;

#ifdef SK_DAWN
    // If the backend API is Dawn, copies a snapshot of the GrDawnRenderTargetInfo struct into the
    // passed-in pointer and returns true. Otherwise returns false if the backend API is not Dawn.
    bool getDawnRenderTargetInfo(GrDawnRenderTargetInfo*) const;
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

#ifdef SK_DIRECT3D
    // If the backend API is Direct3D, copies a snapshot of the GrMtlTextureInfo struct into the
    // passed in pointer and returns true. Otherwise returns false if the backend API is not D3D.
    bool getD3DTextureResourceInfo(GrD3DTextureResourceInfo*) const;

    // Anytime the client changes the D3D12_RESOURCE_STATES of the D3D12_RESOURCE captured by this
    // GrBackendTexture, they must call this function to notify Skia of the changed layout.
    void setD3DResourceState(GrD3DResourceStateEnum);
#endif

    // Get the GrBackendFormat for this render target (or an invalid format if this is not valid).
    GrBackendFormat getBackendFormat() const;

    // If the backend API is Mock, copies a snapshot of the GrMockTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Mock.
    bool getMockRenderTargetInfo(GrMockRenderTargetInfo*) const;

    // If the client changes any of the mutable backend of the GrBackendTexture they should call
    // this function to inform Skia that those values have changed. The backend API specific state
    // that can be set from this function are:
    //
    // Vulkan: VkImageLayout and QueueFamilyIndex
    void setMutableState(const GrBackendSurfaceMutableState&);

    // Returns true if we are working with protected content.
    bool isProtected() const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }


#if GR_TEST_UTILS
    static bool TestingOnly_Equals(const GrBackendRenderTarget&, const GrBackendRenderTarget&);
#endif

private:
    friend class GrVkGpu; // for getMutableState
    sk_sp<GrBackendSurfaceMutableStateImpl> getMutableState() const;

#ifdef SK_VULKAN
    friend class GrVkRenderTarget;
    GrBackendRenderTarget(int width,
                          int height,
                          const GrVkImageInfo& vkInfo,
                          sk_sp<GrBackendSurfaceMutableStateImpl> mutableState);
#endif

#ifdef SK_DIRECT3D
    friend class GrD3DGpu;
    friend class GrD3DRenderTarget;
    GrBackendRenderTarget(int width,
                          int height,
                          const GrD3DTextureResourceInfo& d3dInfo,
                          sk_sp<GrD3DResourceState> state);
    sk_sp<GrD3DResourceState> getGrD3DResourceState() const;
#endif

    // Free and release and resources being held by the GrBackendTexture.
    void cleanup();

    bool fIsValid;
    bool fFramebufferOnly = false;
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
#ifdef SK_DIRECT3D
        GrD3DBackendSurfaceInfo fD3DInfo;
#endif
    };
#ifdef SK_METAL
    GrMtlTextureInfo fMtlInfo;
#endif
#ifdef SK_DAWN
    GrDawnRenderTargetInfo  fDawnInfo;
#endif
    sk_sp<GrBackendSurfaceMutableStateImpl> fMutableState;
};

#endif

#endif

