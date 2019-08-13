/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkCaps_DEFINED
#define GrVkCaps_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/vk/GrVkStencilAttachment.h"

class GrShaderCaps;
class GrVkExtensions;
struct GrVkInterface;

/**
 * Stores some capabilities of a Vk backend.
 */
class GrVkCaps : public GrCaps {
public:
    typedef GrVkStencilAttachment::Format StencilFormat;

    /**
     * Creates a GrVkCaps that is set such that nothing is supported. The init function should
     * be called to fill out the caps.
     */
    GrVkCaps(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
             VkPhysicalDevice device, const VkPhysicalDeviceFeatures2& features,
             uint32_t instanceVersion, uint32_t physicalDeviceVersion,
             const GrVkExtensions& extensions, GrProtected isProtected = GrProtected::kNo);

    bool isFormatSRGB(const GrBackendFormat&) const override;
    bool isFormatCompressed(const GrBackendFormat&) const override;


    bool isFormatTexturableAndUploadable(GrColorType, const GrBackendFormat&) const override;
    bool isFormatTexturable(const GrBackendFormat&) const override;
    bool isVkFormatTexturable(VkFormat) const;

    bool isFormatCopyable(const GrBackendFormat&) const override { return true; }

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override;
    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override;
    bool isFormatRenderable(VkFormat, int sampleCount) const;

    int getRenderTargetSampleCount(int requestedCount, const GrBackendFormat&) const override;
    int getRenderTargetSampleCount(int requestedCount, VkFormat) const;

    int maxRenderTargetSampleCount(const GrBackendFormat&) const override;
    int maxRenderTargetSampleCount(VkFormat format) const;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override;

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;

    bool isVkFormatTexturableLinearly(VkFormat format) const {
        return SkToBool(FormatInfo::kTexturable_Flag & this->getFormatInfo(format).fLinearFlags);
    }

    bool formatCanBeDstofBlit(VkFormat format, bool linearTiled) const {
        const FormatInfo& info = this->getFormatInfo(format);
        const uint16_t& flags = linearTiled ? info.fLinearFlags : info.fOptimalFlags;
        return SkToBool(FormatInfo::kBlitDst_Flag & flags);
    }

    bool formatCanBeSrcofBlit(VkFormat format, bool linearTiled) const {
        const FormatInfo& info = this->getFormatInfo(format);
        const uint16_t& flags = linearTiled ? info.fLinearFlags : info.fOptimalFlags;
        return SkToBool(FormatInfo::kBlitSrc_Flag & flags);
    }

    // On Adreno vulkan, they do not respect the imageOffset parameter at least in
    // copyImageToBuffer. This flag says that we must do the copy starting from the origin always.
    bool mustDoCopiesFromOrigin() const {
        return fMustDoCopiesFromOrigin;
    }

    // Sometimes calls to QueueWaitIdle return before actually signalling the fences
    // on the command buffers even though they have completed. This causes an assert to fire when
    // destroying the command buffers. Therefore we add a sleep to make sure the fence signals.
    bool mustSleepOnTearDown() const {
        return fMustSleepOnTearDown;
    }

    // Returns true if we should always make dedicated allocations for VkImages.
    bool shouldAlwaysUseDedicatedImageMemory() const {
        return fShouldAlwaysUseDedicatedImageMemory;
    }

    // Always use a transfer buffer instead of vkCmdUpdateBuffer to upload data to a VkBuffer.
    bool avoidUpdateBuffers() const {
        return fAvoidUpdateBuffers;
    }

    /**
     * Returns both a supported and most preferred stencil format to use in draws.
     */
    const StencilFormat& preferredStencilFormat() const {
        return fPreferredStencilFormat;
    }

    // Returns whether the device supports VK_KHR_Swapchain. Internally Skia never uses any of the
    // swapchain functions, but we may need to transition to and from the
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR image layout, so we must know whether that layout is
    // supported.
    bool supportsSwapchain() const { return fSupportsSwapchain; }

    // Returns whether the device supports the ability to extend VkPhysicalDeviceProperties struct.
    bool supportsPhysicalDeviceProperties2() const { return fSupportsPhysicalDeviceProperties2; }
    // Returns whether the device supports the ability to extend VkMemoryRequirements struct.
    bool supportsMemoryRequirements2() const { return fSupportsMemoryRequirements2; }

    // Returns whether the device supports the ability to extend the vkBindMemory call.
    bool supportsBindMemory2() const { return fSupportsBindMemory2; }

    // Returns whether or not the device suports the various API maintenance fixes to Vulkan 1.0. In
    // Vulkan 1.1 all these maintenance are part of the core spec.
    bool supportsMaintenance1() const { return fSupportsMaintenance1; }
    bool supportsMaintenance2() const { return fSupportsMaintenance2; }
    bool supportsMaintenance3() const { return fSupportsMaintenance3; }

    // Returns true if the device supports passing in a flag to say we are using dedicated GPU when
    // allocating memory. For some devices this allows them to return more optimized memory knowning
    // they will never need to suballocate amonst multiple objects.
    bool supportsDedicatedAllocation() const { return fSupportsDedicatedAllocation; }

    // Returns true if the device supports importing of external memory into Vulkan memory.
    bool supportsExternalMemory() const { return fSupportsExternalMemory; }
    // Returns true if the device supports importing Android hardware buffers into Vulkan memory.
    bool supportsAndroidHWBExternalMemory() const { return fSupportsAndroidHWBExternalMemory; }

    // Returns true if it supports ycbcr conversion for samplers
    bool supportsYcbcrConversion() const { return fSupportsYcbcrConversion; }

    // Returns true if the device supports protected memory.
    bool supportsProtectedMemory() const { return fSupportsProtectedMemory; }

    /**
     * Helpers used by canCopySurface. In all cases if the SampleCnt parameter is zero that means
     * the surface is not a render target, otherwise it is the number of samples in the render
     * target.
     */
    bool canCopyImage(VkFormat dstFormat, int dstSampleCnt, bool dstHasYcbcr,
                      VkFormat srcFormat, int srcSamplecnt, bool srcHasYcbcr) const;

    bool canCopyAsBlit(VkFormat dstConfig, int dstSampleCnt, bool dstIsLinear, bool dstHasYcbcr,
                       VkFormat srcConfig, int srcSampleCnt, bool srcIsLinear,
                       bool srcHasYcbcr) const;

    bool canCopyAsResolve(VkFormat dstConfig, int dstSampleCnt, bool dstHasYcbcr,
                          VkFormat srcConfig, int srcSamplecnt, bool srcHasYcbcr) const;

    GrColorType getYUVAColorTypeFromBackendFormat(const GrBackendFormat&,
                                                  bool isAlphaChannel) const override;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    bool canClearTextureOnCreation() const override;

    GrSwizzle getTextureSwizzle(const GrBackendFormat&, GrColorType) const override;
    GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const override;

#if GR_TEST_UTILS
    std::vector<TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    enum VkVendor {
        kAMD_VkVendor = 4098,
        kARM_VkVendor = 5045,
        kImagination_VkVendor = 4112,
        kIntel_VkVendor = 32902,
        kNvidia_VkVendor = 4318,
        kQualcomm_VkVendor = 20803,
    };

    void init(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
              VkPhysicalDevice device, const VkPhysicalDeviceFeatures2&,
              uint32_t physicalDeviceVersion, const GrVkExtensions&, GrProtected isProtected);
    void initGrCaps(const GrVkInterface* vkInterface,
                    VkPhysicalDevice physDev,
                    const VkPhysicalDeviceProperties&,
                    const VkPhysicalDeviceMemoryProperties&,
                    const VkPhysicalDeviceFeatures2&,
                    const GrVkExtensions&);
    void initShaderCaps(const VkPhysicalDeviceProperties&, const VkPhysicalDeviceFeatures2&);

    void initFormatTable(const GrVkInterface*, VkPhysicalDevice, const VkPhysicalDeviceProperties&);
    void initStencilFormat(const GrVkInterface* iface, VkPhysicalDevice physDev);

    void applyDriverCorrectnessWorkarounds(const VkPhysicalDeviceProperties&);

    bool onSurfaceSupportsWritePixels(const GrSurface*) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override;
    GrBackendFormat onGetDefaultBackendFormat(GrColorType, GrRenderable) const override;

    GrPixelConfig onGetConfigFromBackendFormat(const GrBackendFormat&, GrColorType) const override;
    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType, const GrBackendFormat&,
                                                 GrColorType) const override;

    // ColorTypeInfo for a specific format
    struct ColorTypeInfo {
        GrColorType fColorType = GrColorType::kUnknown;
        enum {
            kUploadData_Flag = 0x1,
            // Does Ganesh itself support rendering to this colorType & format pair. Renderability
            // still additionally depends on if the format itself is renderable.
            kRenderable_Flag = 0x2,
        };
        uint32_t fFlags = 0;

        GrSwizzle fTextureSwizzle;
        GrSwizzle fOutputSwizzle;
    };

    struct FormatInfo {
        uint32_t colorTypeFlags(GrColorType colorType) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == colorType) {
                    return fColorTypeInfos[i].fFlags;
                }
            }
            return 0;
        }

        void init(const GrVkInterface*, VkPhysicalDevice, const VkPhysicalDeviceProperties&,
                  VkFormat);
        static void InitFormatFlags(VkFormatFeatureFlags, uint16_t* flags);
        void initSampleCounts(const GrVkInterface*, VkPhysicalDevice,
                              const VkPhysicalDeviceProperties&, VkFormat);

        enum {
            kTexturable_Flag = 0x1,
            kRenderable_Flag = 0x2,
            kBlitSrc_Flag    = 0x4,
            kBlitDst_Flag    = 0x8,
        };

        uint16_t fOptimalFlags = 0;
        uint16_t fLinearFlags = 0;

        SkTDArray<int> fColorSampleCounts;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };
    static const size_t kNumVkFormats = 20;
    FormatInfo fFormatTable[kNumVkFormats];

    FormatInfo& getFormatInfo(VkFormat);
    const FormatInfo& getFormatInfo(VkFormat) const;

    StencilFormat fPreferredStencilFormat;

    SkSTArray<1, GrVkYcbcrConversionInfo> fYcbcrInfos;

    bool fMustDoCopiesFromOrigin = false;
    bool fMustSleepOnTearDown = false;
    bool fShouldAlwaysUseDedicatedImageMemory = false;

    bool fAvoidUpdateBuffers = false;

    bool fSupportsSwapchain = false;

    bool fSupportsPhysicalDeviceProperties2 = false;
    bool fSupportsMemoryRequirements2 = false;
    bool fSupportsBindMemory2 = false;
    bool fSupportsMaintenance1 = false;
    bool fSupportsMaintenance2 = false;
    bool fSupportsMaintenance3 = false;

    bool fSupportsDedicatedAllocation = false;
    bool fSupportsExternalMemory = false;
    bool fSupportsAndroidHWBExternalMemory = false;

    bool fSupportsYcbcrConversion = false;

    bool fSupportsProtectedMemory = false;

    typedef GrCaps INHERITED;
};

#endif
