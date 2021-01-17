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
#include "src/gpu/vk/GrVkAttachment.h"

class GrShaderCaps;
class GrVkExtensions;
struct GrVkInterface;

/**
 * Stores some capabilities of a Vk backend.
 */
class GrVkCaps : public GrCaps {
public:
    /**
     * Creates a GrVkCaps that is set such that nothing is supported. The init function should
     * be called to fill out the caps.
     */
    GrVkCaps(const GrContextOptions& contextOptions,
             const GrVkInterface* vkInterface,
             VkPhysicalDevice device,
             const VkPhysicalDeviceFeatures2& features,
             uint32_t instanceVersion,
             uint32_t physicalDeviceVersion,
             const GrVkExtensions& extensions,
             GrProtected isProtected = GrProtected::kNo);

    bool isFormatSRGB(const GrBackendFormat&) const override;

    bool isFormatTexturable(const GrBackendFormat&) const override;
    bool isVkFormatTexturable(VkFormat) const;

    bool isFormatCopyable(const GrBackendFormat&) const override { return true; }

    bool isFormatAsColorTypeRenderable(GrColorType ct,
                                       const GrBackendFormat& format,
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

    // Sometimes calls to QueueWaitIdle return before actually signalling the fences
    // on the command buffers even though they have completed. This causes an assert to fire when
    // destroying the command buffers. Therefore we add a sleep to make sure the fence signals.
    bool mustSleepOnTearDown() const { return fMustSleepOnTearDown; }

    // Returns true if we should always make dedicated allocations for VkImages.
    bool shouldAlwaysUseDedicatedImageMemory() const {
        return fShouldAlwaysUseDedicatedImageMemory;
    }

    // Always use a transfer buffer instead of vkCmdUpdateBuffer to upload data to a VkBuffer.
    bool avoidUpdateBuffers() const { return fAvoidUpdateBuffers; }

    /**
     * Returns both a supported and most preferred stencil format to use in draws.
     */
    VkFormat preferredStencilFormat() const { return fPreferredStencilFormat; }

    // Returns total number of bits used by stencil + depth + padding
    static int GetStencilFormatTotalBitCount(VkFormat format) {
        switch (format) {
            case VK_FORMAT_S8_UINT:
                return 8;
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return 32;
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                // can optionally have 24 unused bits at the end so we assume the total bits is 64.
                return 64;
            default:
                SkASSERT(false);
                return 0;
        }
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

    // Returns whether we prefer to record draws directly into a primary command buffer.
    bool preferPrimaryOverSecondaryCommandBuffers() const {
        return fPreferPrimaryOverSecondaryCommandBuffers;
    }

    int maxPerPoolCachedSecondaryCommandBuffers() const {
        return fMaxPerPoolCachedSecondaryCommandBuffers;
    }

    uint32_t maxInputAttachmentDescriptors() const { return fMaxInputAttachmentDescriptors; }

    bool preferCachedCpuMemory() const { return fPreferCachedCpuMemory; }

    bool mustInvalidatePrimaryCmdBufferStateAfterClearAttachments() const {
        return fMustInvalidatePrimaryCmdBufferStateAfterClearAttachments;
    }

    // The max draw count that can be passed into indirect draw calls.
    uint32_t  maxDrawIndirectDrawCount() const { return fMaxDrawIndirectDrawCount; }

    /**
     * Helpers used by canCopySurface. In all cases if the SampleCnt parameter is zero that means
     * the surface is not a render target, otherwise it is the number of samples in the render
     * target.
     */
    bool canCopyImage(VkFormat dstFormat,
                      int dstSampleCnt,
                      bool dstHasYcbcr,
                      VkFormat srcFormat,
                      int srcSamplecnt,
                      bool srcHasYcbcr) const;

    bool canCopyAsBlit(VkFormat dstConfig,
                       int dstSampleCnt,
                       bool dstIsLinear,
                       bool dstHasYcbcr,
                       VkFormat srcConfig,
                       int srcSampleCnt,
                       bool srcIsLinear,
                       bool srcHasYcbcr) const;

    bool canCopyAsResolve(VkFormat dstConfig,
                          int dstSampleCnt,
                          bool dstHasYcbcr,
                          VkFormat srcConfig,
                          int srcSamplecnt,
                          bool srcHasYcbcr) const;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    VkFormat getFormatFromColorType(GrColorType colorType) const {
        int idx = static_cast<int>(colorType);
        return fColorTypeToFormatTable[idx];
    }

    GrSwizzle getWriteSwizzle(const GrBackendFormat&, GrColorType) const override;

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    int getFragmentUniformBinding() const;
    int getFragmentUniformSet() const;

    void addExtraSamplerKey(GrProcessorKeyBuilder*,
                            GrSamplerState,
                            const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(GrRenderTarget*,
                           const GrProgramInfo&,
                           ProgramDescOverrideFlags) const override;

    GrInternalSurfaceFlags getExtraSurfaceFlagsForDeferredRT() const override;

    // If true then when doing MSAA draws, we will prefer to discard the msaa attachment on load
    // and stores. The use of this feature for specific draws depends on the render target having a
    // resolve attachment, and if we need to load previous data the resolve attachment must be
    // usable as an input attachment. Otherwise we will just write out and store the msaa attachment
    // like normal.
    // This flag is similar to enabling gl render to texture for msaa rendering.
    bool preferDiscardableMSAAAttachment() const { return fPreferDiscardableMSAAAttachment; }

    bool mustLoadFullImageWithDiscardableMSAA() const {
        return fMustLoadFullImageWithDiscardableMSAA;
    }

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
    GrBackendFormat onGetDefaultBackendFormat(GrColorType) const override;

    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType, const GrBackendFormat&,
                                                 GrColorType) const override;

    GrSwizzle onGetReadSwizzle(const GrBackendFormat&, GrColorType) const override;

    GrDstSampleType onGetDstSampleTypeForProxy(const GrRenderTargetProxy*) const override;

    // ColorTypeInfo for a specific format
    struct ColorTypeInfo {
        GrColorType fColorType = GrColorType::kUnknown;
        enum {
            kUploadData_Flag = 0x1,
            // Does Ganesh itself support rendering to this colorType & format pair. Renderability
            // still additionally depends on if the format itself is renderable.
            kRenderable_Flag = 0x2,
            // Indicates that this colorType is supported only if we are wrapping a texture with
            // the given format and colorType. We do not allow creation with this pair.
            kWrappedOnly_Flag = 0x4,
        };
        uint32_t fFlags = 0;

        GrSwizzle fReadSwizzle;
        GrSwizzle fWriteSwizzle;
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
    static const size_t kNumVkFormats = 22;
    FormatInfo fFormatTable[kNumVkFormats];

    FormatInfo& getFormatInfo(VkFormat);
    const FormatInfo& getFormatInfo(VkFormat) const;

    VkFormat fColorTypeToFormatTable[kGrColorTypeCnt];
    void setColorType(GrColorType, std::initializer_list<VkFormat> formats);

    VkFormat fPreferredStencilFormat;

    SkSTArray<1, GrVkYcbcrConversionInfo> fYcbcrInfos;

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

    bool fPreferPrimaryOverSecondaryCommandBuffers = true;
    bool fMustInvalidatePrimaryCmdBufferStateAfterClearAttachments = false;

    // We default this to 100 since we already cap the max render tasks at 100 before doing a
    // submission in the GrDrawingManager, so we shouldn't be going over 100 secondary command
    // buffers per primary anyways.
    int fMaxPerPoolCachedSecondaryCommandBuffers = 100;

    uint32_t fMaxInputAttachmentDescriptors = 0;

    bool fPreferCachedCpuMemory = true;

    bool fPreferDiscardableMSAAAttachment = false;
    bool fMustLoadFullImageWithDiscardableMSAA = false;

    uint32_t fMaxDrawIndirectDrawCount = 0;

    using INHERITED = GrCaps;
};

#endif
