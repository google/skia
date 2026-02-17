/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurface_DEFINED
#define GrBackendSurface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/mock/GrMockTypes.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAnySubclass.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

enum class SkTextureCompressionType;
class GrBackendFormatData;
class GrBackendTextureData;
class GrBackendRenderTargetData;

namespace skgpu {
class MutableTextureState;
}

#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
class SkString;
#endif

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

class SK_API GrBackendFormat {
public:
    // Creates an invalid backend format.
    GrBackendFormat();
    GrBackendFormat(const GrBackendFormat&);
    GrBackendFormat& operator=(const GrBackendFormat&);
    ~GrBackendFormat();

    static GrBackendFormat MakeMock(GrColorType colorType,
                                    SkTextureCompressionType compression,
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
     * If the backend API is not Mock these three calls will return kUnknown, kNone or false,
     * respectively. Otherwise, only one of the following can be true. The GrColorType is not
     * kUnknown, the compression type is not kNone, or this is a mock stencil format.
     */
    GrColorType asMockColorType() const;
    SkTextureCompressionType asMockCompressionType() const;
    bool isMockStencilFormat() const;

    // If possible, copies the GrBackendFormat and forces the texture type to be Texture2D. If the
    // GrBackendFormat was for Vulkan and it originally had a skgpu::VulkanYcbcrConversionInfo,
    // we will remove the conversion and set the format to be VK_FORMAT_R8G8B8A8_UNORM.
    GrBackendFormat makeTexture2D() const;

    // Returns true if the backend format has been initialized.
    bool isValid() const { return fValid; }

#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
    SkString toStr() const;
#endif

private:
    // Size determined by looking at the GrBackendFormatData subclasses, then guessing-and-checking.
    // Compiler will complain if this is too small - in that case, just increase the number.
    inline constexpr static size_t kMaxSubclassSize = 80;
    using AnyFormatData = SkAnySubclass<GrBackendFormatData, kMaxSubclassSize>;

    friend class GrBackendSurfacePriv;
    friend class GrBackendFormatData;

    // Used by internal factories. Should not be used externally. Use factories like
    // GrBackendFormats::MakeGL instead.
    template <typename FormatData>
    GrBackendFormat(GrTextureType textureType, GrBackendApi api, const FormatData& formatData)
            : fBackend(api), fValid(true), fTextureType(textureType) {
        fFormatData.emplace<FormatData>(formatData);
    }

    GrBackendFormat(GrColorType, SkTextureCompressionType, bool isStencilFormat);

#ifdef SK_DEBUG
    bool validateMock() const;
#endif

    GrBackendApi fBackend = GrBackendApi::kMock;
    bool fValid = false;
    AnyFormatData fFormatData;

    struct {
        GrColorType fColorType;
        SkTextureCompressionType fCompressionType;
        bool fIsStencilFormat;
    } fMock;
    GrTextureType fTextureType = GrTextureType::kNone;
};

class SK_API GrBackendTexture {
public:
    // Creates an invalid backend texture.
    GrBackendTexture();

    GrBackendTexture(int width,
                     int height,
                     skgpu::Mipmapped,
                     const GrMockTextureInfo& mockInfo,
                     std::string_view label = {});

    GrBackendTexture(const GrBackendTexture& that);

    ~GrBackendTexture();

    GrBackendTexture& operator=(const GrBackendTexture& that);

    SkISize dimensions() const { return {fWidth, fHeight}; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    std::string_view getLabel() const { return fLabel; }
    skgpu::Mipmapped mipmapped() const { return fMipmapped; }
    bool hasMipmaps() const { return fMipmapped == skgpu::Mipmapped::kYes; }
    GrBackendApi backend() const {return fBackend; }
    GrTextureType textureType() const { return fTextureType; }

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
    void setMutableState(const skgpu::MutableTextureState&);

    // Returns true if we are working with protected content.
    bool isProtected() const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }

    // Returns true if both textures are valid and refer to the same API texture.
    bool isSameTexture(const GrBackendTexture&);

#if defined(GPU_TEST_UTILS)
    static bool TestingOnly_Equals(const GrBackendTexture&, const GrBackendTexture&);
#endif

private:
    // Size determined by looking at the GrBackendTextureData subclasses, then guessing-and-checking.
    // Compiler will complain if this is too small - in that case, just increase the number.
    inline constexpr static size_t kMaxSubclassSize = 176;
    using AnyTextureData = SkAnySubclass<GrBackendTextureData, kMaxSubclassSize>;

    friend class GrBackendSurfacePriv;
    friend class GrBackendTextureData;

    // Used by internal factories. Should not be used externally. Use factories like
    // GrBackendTextures::MakeGL instead.
    template <typename TextureData>
    GrBackendTexture(int width,
                     int height,
                     std::string_view label,
                     skgpu::Mipmapped mipped,
                     GrBackendApi backend,
                     GrTextureType texture,
                     const TextureData& textureData)
            : fIsValid(true)
            , fWidth(width)
            , fHeight(height)
            , fLabel(label)
            , fMipmapped(mipped)
            , fBackend(backend)
            , fTextureType(texture) {
        fTextureData.emplace<TextureData>(textureData);
    }

    friend class GrVkGpu;  // for getMutableState
    sk_sp<skgpu::MutableTextureState> getMutableState() const;

    bool fIsValid;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    const std::string fLabel;
    skgpu::Mipmapped fMipmapped;
    GrBackendApi fBackend;
    GrTextureType fTextureType;
    AnyTextureData fTextureData;

    GrMockTextureInfo fMockInfo;
};

class SK_API GrBackendRenderTarget {
public:
    // Creates an invalid backend texture.
    GrBackendRenderTarget();

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
    void setMutableState(const skgpu::MutableTextureState&);

    // Returns true if we are working with protected content.
    bool isProtected() const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }

#if defined(GPU_TEST_UTILS)
    static bool TestingOnly_Equals(const GrBackendRenderTarget&, const GrBackendRenderTarget&);
#endif

private:
    // Size determined by looking at the GrBackendRenderTargetData subclasses, then
    // guessing-and-checking. Compiler will complain if this is too small - in that case, just
    // increase the number.
    inline constexpr static size_t kMaxSubclassSize = 176;
    using AnyRenderTargetData = SkAnySubclass<GrBackendRenderTargetData, kMaxSubclassSize>;

    friend class GrBackendSurfacePriv;
    friend class GrBackendRenderTargetData;

    // Used by internal factories. Should not be used externally. Use factories like
    // GrBackendRenderTargets::MakeGL instead.
    template <typename RenderTargetData>
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrBackendApi backend,
                          bool framebufferOnly,
                          const RenderTargetData& rtData)
            : fIsValid(true)
            , fFramebufferOnly(framebufferOnly)
            , fWidth(width)
            , fHeight(height)
            , fSampleCnt(sampleCnt)
            , fStencilBits(stencilBits)
            , fBackend(backend) {
        fRTData.emplace<RenderTargetData>(rtData);
    }

    friend class GrVkGpu; // for getMutableState
    sk_sp<skgpu::MutableTextureState> getMutableState() const;

    bool fIsValid;
    bool fFramebufferOnly = false;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels

    int fSampleCnt;
    int fStencilBits;

    GrBackendApi fBackend;
    AnyRenderTargetData fRTData;

    GrMockRenderTargetInfo fMockInfo;
};

#endif
