/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanCaps_DEFINED
#define skgpu_graphite_VulkanCaps_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/Caps.h"
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

    bool isSampleCountSupported(TextureFormat, uint8_t requestedSampleCount) const override;
    TextureFormat getDepthStencilFormat(SkEnumBitMask<DepthStencilFlags>) const override;

    TextureInfo getDefaultAttachmentTextureInfo(AttachmentDesc,
                                                Protected,
                                                Discardable) const override;

    TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                             Mipmapped mipmapped,
                                             Protected,
                                             Renderable) const override;

    TextureInfo getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                             Mipmapped mipmapped) const override;

    TextureInfo getDefaultCompressedTextureInfo(SkTextureCompressionType,
                                                Mipmapped mipmapped,
                                                Protected) const override;

    TextureInfo getDefaultStorageTextureInfo(SkColorType) const override;

    // Override Caps's implementation in order to consult Vulkan-specific texture properties.
    DstReadStrategy getDstReadStrategy() const override;

    ImmutableSamplerInfo getImmutableSamplerInfo(const TextureInfo&) const override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;
    bool extractGraphicsDescs(const UniqueKey&,
                              GraphicsPipelineDesc*,
                              RenderPassDesc*,
                              const RendererProvider*) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override { return {}; }

    bool isTexturable(const VulkanTextureInfo&) const;
    bool isRenderable(const VulkanTextureInfo&) const;

    bool isRenderable(const TextureInfo&) const override;
    bool isStorage(const TextureInfo&) const override;

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

    bool isTransferSrc(const VulkanTextureInfo&) const;
    bool isTransferDst(const VulkanTextureInfo&) const;

    bool mustLoadFullImageForMSAA() const { return fMustLoadFullImageForMSAA; }

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
    };
    EnabledFeatures getEnabledFeatures(const VkPhysicalDeviceFeatures2* features,
                                       uint32_t physicalDeviceVersion);

    struct PhysicalDeviceProperties {
        VkPhysicalDeviceProperties2 fBase;
        VkPhysicalDeviceDriverProperties fDriver;
        VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT fGpl;
    };
    void getProperties(const skgpu::VulkanInterface* vkInterface,
                       VkPhysicalDevice physDev,
                       uint32_t physicalDeviceVersion,
                       const skgpu::VulkanExtensions* extensions,
                       const EnabledFeatures& features,
                       PhysicalDeviceProperties* props);

    void applyDriverCorrectnessWorkarounds(const PhysicalDeviceProperties&);

    void initFormatTable(const skgpu::VulkanInterface*,
                         VkPhysicalDevice,
                         const VkPhysicalDeviceProperties&);

    void initDepthStencilFormatTable(const skgpu::VulkanInterface*,
                                     VkPhysicalDevice,
                                     const VkPhysicalDeviceProperties&);

    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override;

    bool onIsTexturable(const TextureInfo&) const override;

    bool supportsWritePixels(const TextureInfo&) const override;
    bool supportsReadPixels(const TextureInfo&) const override;

    std::pair<SkColorType, bool /*isRGBFormat*/> supportedWritePixelsColorType(
            SkColorType dstColorType,
            const TextureInfo& dstTextureInfo,
            SkColorType srcColorType) const override;
    std::pair<SkColorType, bool /*isRGBFormat*/> supportedReadPixelsColorType(
            SkColorType srcColorType,
            const TextureInfo& srcTextureInfo,
            SkColorType dstColorType) const override;

    // Encode the parts of the render pass desc that is relevant to pipelines; this excludes all the
    // things that make the render pass compatible.
    uint32_t getRenderPassDescKeyForPipeline(const RenderPassDesc&) const;

    // Struct that determines and stores which sample count quantities a VkFormat supports.
    struct SupportedSampleCounts {
        void initSampleCounts(const skgpu::VulkanInterface*,
                              VkPhysicalDevice,
                              const VkPhysicalDeviceProperties&,
                              VkFormat,
                              VkImageUsageFlags);

        bool isSampleCountSupported(int requestedCount) const;

        VkSampleCountFlags fSampleCounts;
    };

    // Struct that determines and stores useful information about VkFormats.
    struct FormatInfo {
        uint32_t colorTypeFlags(SkColorType colorType) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == colorType) {
                    return fColorTypeInfos[i].fFlags;
                }
            }
            return 0;
        }

        void init(const skgpu::VulkanInterface*,
                  VkPhysicalDevice,
                  const VkPhysicalDeviceProperties&,
                  VkFormat);

        bool isTexturable(VkImageTiling) const;
        bool isRenderable(VkImageTiling, uint32_t sampleCount) const;
        bool isStorage(VkImageTiling) const;
        bool isTransferSrc(VkImageTiling) const;
        bool isTransferDst(VkImageTiling) const;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;

        VkFormatProperties fFormatProperties;
        SupportedSampleCounts fSupportedSampleCounts;

        // Indicates that a format is only supported if we are wrapping a texture with it.
        SkDEBUGCODE(bool fIsWrappedOnly = false;)
    };

    // Map SkColorType to VkFormat.
    VkFormat fColorTypeToFormatTable[kSkColorTypeCnt];
    void setColorType(SkColorType, std::initializer_list<VkFormat> formats);
    VkFormat getFormatFromColorType(SkColorType) const;

    // Map VkFormat to FormatInfo.
    static const size_t kNumVkFormats = 23;
    FormatInfo fFormatTable[kNumVkFormats];

    FormatInfo& getFormatInfo(VkFormat);
    const FormatInfo& getFormatInfo(VkFormat) const;

    // A more lightweight equivalent to FormatInfo for depth/stencil VkFormats.
    struct DepthStencilFormatInfo {
        void init(const skgpu::VulkanInterface*,
                  VkPhysicalDevice,
                  const VkPhysicalDeviceProperties&,
                  VkFormat);
        bool isDepthStencilSupported(VkFormatFeatureFlags) const;

        VkFormatProperties fFormatProperties;
        SupportedSampleCounts fSupportedSampleCounts;
    };

    // Map DepthStencilFlags to VkFormat.
    static const size_t kNumDepthStencilFlags = 4;
    VkFormat fDepthStencilFlagsToFormatTable[kNumDepthStencilFlags];

    // Map depth/stencil VkFormats to DepthStencilFormatInfo.
    static const size_t kNumDepthStencilVkFormats = 5;
    DepthStencilFormatInfo fDepthStencilFormatTable[kNumDepthStencilVkFormats];

    DepthStencilFormatInfo& getDepthStencilFormatInfo(VkFormat);
    const DepthStencilFormatInfo& getDepthStencilFormatInfo(VkFormat) const;

    uint32_t fMaxVertexAttributes;
    uint64_t fMaxUniformBufferRange;
    uint64_t fMaxStorageBufferRange;
    VkPhysicalDeviceMemoryProperties2 fPhysicalDeviceMemoryProperties2;

    // ColorTypeInfo struct for use w/ external formats.
    const ColorTypeInfo fExternalFormatColorTypeInfo = {SkColorType::kRGBA_8888_SkColorType,
                                                        SkColorType::kRGBA_8888_SkColorType,
                                                        /*flags=*/0,
                                                        skgpu::Swizzle::RGBA(),
                                                        skgpu::Swizzle::RGBA()};

    // Various bools to define whether certain Vulkan features are supported.
    bool fSupportsMemorylessAttachments = false;
    bool fSupportsYcbcrConversion = false;
    bool fShouldAlwaysUseDedicatedImageMemory = false;
    bool fGpuOnlyBuffersMorePerformant = false;
    bool fShouldPersistentlyMapCpuToGpuBuffers = true;
    bool fSupportsDeviceFaultInfo = false;
    bool fSupportsRasterizationOrderColorAttachmentAccess = false;
    bool fIsInputAttachmentReadCoherent = false;

    // Flags to enable workarounds for driver bugs
    bool fMustLoadFullImageForMSAA = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCaps_DEFINED
