/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanCaps_DEFINED
#define skgpu_graphite_VulkanCaps_DEFINED

#include "include/private/SkTArray.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

namespace skgpu::graphite {
struct ContextOptions;
class VulkanTextureInfo;

class VulkanCaps final : public Caps {
public:
    VulkanCaps(const ContextOptions&,
               const skgpu::VulkanInterface*,
               VkPhysicalDevice,
               uint32_t physicalDeviceVersion,
               const VkPhysicalDeviceFeatures2*,
               const skgpu::VulkanExtensions*,
               Protected);
    ~VulkanCaps() override;

    // Override Caps's implementation in order to consult Vulkan-specific texture properties.
    DstReadStrategy getDstReadStrategy() const override;

    ImmutableSamplerInfo getImmutableSamplerInfo(const TextureInfo&) const override;
    std::string toString(const ImmutableSamplerInfo&) const override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;
    bool extractGraphicsDescs(const UniqueKey&,
                              GraphicsPipelineDesc*,
                              RenderPassDesc*,
                              const RendererProvider*) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override { return {}; }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            GraphiteResourceKey*) const override;

    bool shouldAlwaysUseDedicatedImageMemory() const {
        return fShouldAlwaysUseDedicatedImageMemory;
    }

    // Returns whether a pure GPU accessible buffer is more performant to read than a buffer that is
    // also host visible. If so then in some cases we may prefer the cost of doing a copy to the
    // buffer. This typically would only be the case for buffers that are written once and read
    // many times on the gpu.
    bool gpuOnlyBuffersMorePerformant() const { return fGpuOnlyBuffersMorePerformant; }

    // For our CPU write and GPU read buffers (vertex, uniform, etc.), we should keep these buffers
    // persistently mapped. In general, the answer will be yes. The main case where we don't do this
    // is when using special memory that is DEVICE_LOCAL and HOST_VISIBLE on discrete GPUs.
    bool shouldPersistentlyMapCpuToGpuBuffers() const {
        return fShouldPersistentlyMapCpuToGpuBuffers;
    }

    bool supportsYcbcrConversion() const { return fSupportsYcbcrConversion; }
    bool supportsDeviceFaultInfo() const { return fSupportsDeviceFaultInfo; }

    // Whether a barrier is required before reading from input attachments (barrier is needed if
    // !coherent).
    bool isInputAttachmentReadCoherent() const { return fIsInputAttachmentReadCoherent; }
    // isInputAttachmentReadCoherent() is based on whether
    // VK_EXT_rasterization_order_attachment_access is supported, but is also enabled on a few
    // architectures where it's known a priori that input attachment reads are coherent. The
    // following determines whether that extension is enabled (in which case a pipeline creation
    // flag is necessary) or not. When disabled, a subpass self-dependency is needed instead.
    bool supportsRasterizationOrderColorAttachmentAccess() const {
        return fSupportsRasterizationOrderColorAttachmentAccess;
    }

    uint32_t maxVertexAttributes()   const { return fMaxVertexAttributes;   }
    uint64_t maxUniformBufferRange() const { return fMaxUniformBufferRange; }
    uint64_t maxStorageBufferRange() const { return fMaxStorageBufferRange; }

    const VkPhysicalDeviceMemoryProperties2& physicalDeviceMemoryProperties2() const {
        return fPhysicalDeviceMemoryProperties2;
    }

    bool mustLoadFullImageForMSAA() const { return fMustLoadFullImageForMSAA; }

    bool supportsFrameBoundary() const { return fSupportsFrameBoundary; }

    bool supportsPipelineCreationCacheControl() const {
        return fSupportsPipelineCreationCacheControl;
    }

    bool supportsOcclusionQueryPrecise() const { return fOcclusionQueryPrecise; }

    uint32_t timestampValidBits(uint32_t queueIndex) const {
        return fQueueFamilyTimestampValidBits[queueIndex];
    }

    float timestampPeriod() const { return fTimestampPeriod; }

private:
    void init(const ContextOptions&,
              const skgpu::VulkanInterface*,
              VkPhysicalDevice,
              uint32_t physicalDeviceVersion,
              const VkPhysicalDeviceFeatures2*,
              const skgpu::VulkanExtensions*,
              Protected);

    struct EnabledFeatures {
        // VkPhysicalDeviceFeatures
        bool fDualSrcBlend = false;
        // Vulkan 1.0 core:
        bool fOcclusionQueryPrecise = false;
        // Vulkan 1.1 core:
        bool fProtectedMemory = false;
        // From VkPhysicalDeviceSamplerYcbcrConversionFeatures or VkPhysicalDeviceVulkan11Features:
        bool fSamplerYcbcrConversion = false;
        // From VkPhysicalDeviceFaultFeaturesEXT:
        bool fDeviceFault = false;
        // From VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT:
        bool fAdvancedBlendModes = false;
        bool fCoherentAdvancedBlendModes = false;
        // From VK_EXT_rasterization_order_attachment_access:
        bool fRasterizationOrderColorAttachmentAccess = false;
        // From VkPhysicalDeviceExtendedDynamicStateFeaturesEXT or Vulkan 1.3 (no features):
        bool fExtendedDynamicState = false;
        // From VkPhysicalDeviceExtendedDynamicState2FeaturesEXT or Vulkan 1.3 (no features):
        bool fExtendedDynamicState2 = false;
        // From VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT:
        bool fVertexInputDynamicState = false;
        // From VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT:
        bool fGraphicsPipelineLibrary = false;
        // From VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT:
        bool fMultisampledRenderToSingleSampled = false;
        // From VkPhysicalDeviceHostImageCopyFeatures:
        bool fHostImageCopy = false;
        // From VkPhysicalDeviceFrameBoundaryFeaturesEXT:
        bool fFrameBoundary = false;
        // From VkPhysicalDevicePipelineCreationCacheControlFeatures or
        // VkPhysicalDeviceVulkan13Features
        bool fPipelineCreationCacheControl = false;
        // From VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT
        bool fFormatRGBA10x6WithoutYCbCrSampler = false;
    };
    EnabledFeatures getEnabledFeatures(const VkPhysicalDeviceFeatures2*,
                                       uint32_t physicalDeviceVersion);

    struct PhysicalDeviceProperties {
        VkPhysicalDeviceProperties2 fBase;
        VkPhysicalDeviceDriverProperties fDriver;
        VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT fGpl;
        VkPhysicalDeviceHostImageCopyPropertiesEXT fHic;
        bool fHicHasShaderReadOnlyDstLayout = false;
    };
    void getProperties(const skgpu::VulkanInterface*,
                       VkPhysicalDevice,
                       uint32_t physicalDeviceVersion,
                       const skgpu::VulkanExtensions*,
                       const EnabledFeatures&,
                       PhysicalDeviceProperties*);

    void applyDriverCorrectnessWorkarounds(const PhysicalDeviceProperties&);

    void initShaderCaps(const EnabledFeatures, const uint32_t vendorID);

    void initFormatTable(const skgpu::VulkanInterface*,
                         VkPhysicalDevice,
                         const VkPhysicalDeviceProperties&,
                         const EnabledFeatures&);

    TextureInfo onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                        TextureFormat,
                                        SampleCount,
                                        Mipmapped,
                                        Protected,
                                        Discardable) const override;
    std::pair<SkEnumBitMask<TextureUsage>, Tiling> getTextureUsage(
            const TextureInfo&) const override;

    // Helper functions to compute supported texture usage and sample counts, only called during
    // initialization of VulkanCaps and then cached in `fFormatSupport`.
    SkEnumBitMask<SampleCount> getSupportedSampleCounts(const skgpu::VulkanInterface* interface,
                                                        VkPhysicalDevice physDev,
                                                        VkFormat format,
                                                        VkImageUsageFlags usage) const;
    bool isEfficientWithHostCopy(const skgpu::VulkanInterface* interface,
                                 VkPhysicalDevice physDev,
                                 VkFormat format) const;
    std::pair<SkEnumBitMask<TextureUsage>, SkEnumBitMask<SampleCount>> getTextureSupport(
            const skgpu::VulkanInterface* interface,
            VkPhysicalDevice physDev,
            TextureFormat format,
            Tiling tiling,
            const VkFormatProperties& props) const;

    uint32_t fMaxVertexAttributes;
    uint64_t fMaxUniformBufferRange;
    uint64_t fMaxStorageBufferRange;
    VkPhysicalDeviceMemoryProperties2 fPhysicalDeviceMemoryProperties2;

    // Various bools to define whether certain Vulkan features are supported.
    bool fSupportsMemorylessAttachments = false;
    bool fSupportsYcbcrConversion = false;
    bool fShouldAlwaysUseDedicatedImageMemory = false;
    bool fGpuOnlyBuffersMorePerformant = false;
    bool fShouldPersistentlyMapCpuToGpuBuffers = true;
    bool fSupportsDeviceFaultInfo = false;
    bool fSupportsRasterizationOrderColorAttachmentAccess = false;
    bool fIsInputAttachmentReadCoherent = false;
    bool fSupportsFrameBoundary = false;
    bool fSupportsPipelineCreationCacheControl = false;
    bool fOcclusionQueryPrecise = false;

    // Flags to enable workarounds for driver bugs
    bool fMustLoadFullImageForMSAA = false;

    skia_private::TArray<uint32_t> fQueueFamilyTimestampValidBits;
    float fTimestampPeriod = 1.0f;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCaps_DEFINED
