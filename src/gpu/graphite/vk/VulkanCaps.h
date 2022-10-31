/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanCaps_DEFINED
#define skgpu_graphite_VulkanCaps_DEFINED

#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/VulkanUtils.h"

namespace skgpu::graphite {
struct ContextOptions;

class VulkanCaps final : public Caps {
public:
    VulkanCaps(const skgpu::VulkanInterface*,
               VkPhysicalDevice device,
               uint32_t physicalDeviceVersion,
               const skgpu::VulkanExtensions*);
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

    bool isRenderable(const TextureInfo&) const override { return false; }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            Shareable,
                            GraphiteResourceKey*) const override {}

private:
    enum VkVendor {
        kAMD_VkVendor             = 4098,
        kARM_VkVendor             = 5045,
        kImagination_VkVendor     = 4112,
        kIntel_VkVendor           = 32902,
        kNvidia_VkVendor          = 4318,
        kQualcomm_VkVendor        = 20803,
    };

    void initFormatTable(const skgpu::VulkanInterface*,
                         VkPhysicalDevice,
                         const VkPhysicalDeviceProperties&);

    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override {
        return nullptr;
    }

    bool onIsTexturable(const TextureInfo&) const override { return false; }

    size_t getTransferBufferAlignment(size_t bytesPerPixel) const override { return 0; }

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
        void initSampleCounts(const skgpu::VulkanInterface*,
                              VkPhysicalDevice,
                              const VkPhysicalDeviceProperties&,
                              VkFormat);

        SkTDArray<int> fColorSampleCounts;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;

        VkFormatProperties fFormatProperties;
        // Indicates that a format is only supported if we are wrapping a texture with it.
        SkDEBUGCODE(bool fIsWrappedOnly;)
    };

    FormatInfo& getFormatInfo(VkFormat);
    const FormatInfo& getFormatInfo(VkFormat) const;

    VkFormat fColorTypeToFormatTable[kSkColorTypeCnt];
    void setColorType(SkColorType, std::initializer_list<VkFormat> formats);

    static const size_t kNumVkFormats = 22;
    FormatInfo fFormatTable[kNumVkFormats];

    // TODO: Assign real value to fSupportsYcbcrConversion.
    bool fSupportsYcbcrConversion = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCaps_DEFINED

