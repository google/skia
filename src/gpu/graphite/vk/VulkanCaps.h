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

class VulkanCaps final : public Caps {
public:
    VulkanCaps(const skgpu::VulkanInterface*,
               VkPhysicalDevice device,
               uint32_t physicalDeviceVersion,
               const skgpu::VulkanExtensions*,
               const ContextOptions&);
    ~VulkanCaps() override;

    TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                             Mipmapped mipmapped,
                                             Protected,
                                             Renderable) const override;

    TextureInfo getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                          Discardable discardable) const override;

    TextureInfo getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                  uint32_t sampleCount,
                                                  Protected) const override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override { return {}; }
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override { return {}; }

    uint32_t channelMask(const TextureInfo&) const override;

    bool isRenderable(const TextureInfo&) const override { return false; }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            Shareable,
                            GraphiteResourceKey*) const override {}

    size_t bytesPerPixel(const TextureInfo&) const override;

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

private:
    enum VkVendor {
        kAMD_VkVendor             = 4098,
        kARM_VkVendor             = 5045,
        kImagination_VkVendor     = 4112,
        kIntel_VkVendor           = 32902,
        kNvidia_VkVendor          = 4318,
        kQualcomm_VkVendor        = 20803,
    };

    void init(const skgpu::VulkanInterface*,
              VkPhysicalDevice,
              uint32_t physicalDeviceVersion,
              const skgpu::VulkanExtensions*,
              const ContextOptions&);

    void applyDriverCorrectnessWorkarounds(const VkPhysicalDeviceProperties&);

    void initFormatTable(const skgpu::VulkanInterface*,
                         VkPhysicalDevice,
                         const VkPhysicalDeviceProperties&);

    void initDepthStencilFormatTable(const skgpu::VulkanInterface*,
                                     VkPhysicalDevice,
                                     const VkPhysicalDeviceProperties&);

    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override {
        return nullptr;
    }

    bool onIsTexturable(const TextureInfo&) const override { return false; }

    bool supportsWritePixels(const TextureInfo&) const override { return false; }
    bool supportsReadPixels(const TextureInfo&) const override { return false; }

    SkColorType supportedWritePixelsColorType(SkColorType dstColorType,
                                              const TextureInfo& dstTextureInfo,
                                              SkColorType srcColorType) const override {
        return kUnknown_SkColorType;
    }
    SkColorType supportedReadPixelsColorType(SkColorType srcColorType,
                                             const TextureInfo& srcTextureInfo,
                                             SkColorType dstColorType) const override {
        return kUnknown_SkColorType;
    }
    // Struct that determines and stores which sample count quantities a VkFormat supports.
    struct SupportedSampleCounts {
        void initSampleCounts(const skgpu::VulkanInterface*,
                              VkPhysicalDevice,
                              const VkPhysicalDeviceProperties&,
                              VkFormat,
                              VkImageUsageFlags);

        bool isSampleCountSupported(int requestedCount) const;

        SkTDArray<int> fSampleCounts;
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

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;

        VkFormatProperties fFormatProperties;
        SupportedSampleCounts fSupportedSampleCounts;

        // Indicates that a format is only supported if we are wrapping a texture with it.
        SkDEBUGCODE(bool fIsWrappedOnly;)

    private:
        bool isTexturable(VkFormatFeatureFlags) const;
        bool isRenderable(VkFormatFeatureFlags) const;
    };

    // Map SkColorType to VkFormat.
    VkFormat fColorTypeToFormatTable[kSkColorTypeCnt];
    void setColorType(SkColorType, std::initializer_list<VkFormat> formats);
    VkFormat getFormatFromColorType(SkColorType) const;

    // Map VkFormat to FormatInfo.
    static const size_t kNumVkFormats = 22;
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
    VkFormat getFormatFromDepthStencilFlags(const SkEnumBitMask<DepthStencilFlags>& flags) const;

    // Map depth/stencil VkFormats to DepthStencilFormatInfo.
    static const size_t kNumDepthStencilVkFormats = 3;
    DepthStencilFormatInfo fDepthStencilFormatTable[kNumDepthStencilVkFormats];

    DepthStencilFormatInfo& getDepthStencilFormatInfo(VkFormat);
    const DepthStencilFormatInfo& getDepthStencilFormatInfo(VkFormat) const;

    // Various bools to define whether certain Vulkan features are supported.
    bool fSupportsMemorylessAttachments = false;
    bool fSupportsYcbcrConversion = false; // TODO: Determine & assign real value.
    bool fShouldAlwaysUseDedicatedImageMemory = false;
    bool fGpuOnlyBuffersMorePerformant = false;
    bool fShouldPersistentlyMapCpuToGpuBuffers = true;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCaps_DEFINED
