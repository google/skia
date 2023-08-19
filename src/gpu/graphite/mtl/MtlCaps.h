/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlCaps_DEFINED
#define skgpu_graphite_MtlCaps_DEFINED

#include <vector>

#import <Metal/Metal.h>

#include "src/gpu/graphite/Caps.h"

namespace skgpu::graphite {
struct ContextOptions;

class MtlCaps final : public Caps {
public:
    MtlCaps(const id<MTLDevice>, const ContextOptions&);
    ~MtlCaps() override {}

    TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                             Mipmapped mipmapped,
                                             Protected,
                                             Renderable) const override;

    TextureInfo getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                             Mipmapped mipmapped) const override;

    TextureInfo getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                          Discardable discardable) const override;

    TextureInfo getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                  uint32_t sampleCount,
                                                  Protected) const override;

    TextureInfo getDefaultStorageTextureInfo(SkColorType) const override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override;

    // Get a sufficiently unique bit representation for the RenderPassDesc to be embedded in other
    // UniqueKeys (e.g. makeGraphicsPipelineKey).
    uint64_t getRenderPassDescKey(const RenderPassDesc&) const;

    bool isMac() const { return fGPUFamily == GPUFamily::kMac; }
    bool isApple()const  { return fGPUFamily == GPUFamily::kApple; }

    uint32_t channelMask(const TextureInfo&) const override;

    bool isRenderable(const TextureInfo&) const override;
    bool isStorage(const TextureInfo&) const override;

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            Shareable,
                            GraphiteResourceKey*) const override;

private:
    void initGPUFamily(const id<MTLDevice>);

    void initCaps(const id<MTLDevice>);
    void initShaderCaps();
    void initFormatTable();

    enum class GPUFamily {
        kMac,
        kApple,
    };
    static bool GetGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group);
    static bool GetGPUFamilyFromFeatureSet(id<MTLDevice> device, GPUFamily* gpuFamily,
                                           int* group);

    MTLPixelFormat getFormatFromColorType(SkColorType colorType) const {
        int idx = static_cast<int>(colorType);
        return fColorTypeToFormatTable[idx];
    }

    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override;

    bool onIsTexturable(const TextureInfo&) const override;
    bool isTexturable(MTLPixelFormat) const;
    bool isRenderable(MTLPixelFormat, uint32_t numSamples) const;
    uint32_t maxRenderTargetSampleCount(MTLPixelFormat) const;

    bool supportsWritePixels(const TextureInfo&) const override;
    bool supportsReadPixels(const TextureInfo&) const override;

    SkColorType supportedWritePixelsColorType(SkColorType dstColorType,
                                              const TextureInfo& dstTextureInfo,
                                              SkColorType srcColorType) const override;
    SkColorType supportedReadPixelsColorType(SkColorType srcColorType,
                                             const TextureInfo& srcTextureInfo,
                                             SkColorType dstColorType) const override;

    MTLStorageMode getDefaultMSAAStorageMode(Discardable discardable) const;

    struct FormatInfo {
        uint32_t colorTypeFlags(SkColorType colorType) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == colorType) {
                    return fColorTypeInfos[i].fFlags;
                }
            }
            return 0;
        }

        enum {
            kTexturable_Flag  = 0x01,
            kRenderable_Flag  = 0x02, // Color attachment and blendable
            kMSAA_Flag        = 0x04,
            kResolve_Flag     = 0x08,
            kStorage_Flag     = 0x10,
        };
        static const uint16_t kAllFlags =
                kTexturable_Flag | kRenderable_Flag | kMSAA_Flag | kResolve_Flag | kStorage_Flag;

        uint16_t fFlags = 0;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };
    inline static constexpr size_t kNumMtlFormats = 19;

    static size_t GetFormatIndex(MTLPixelFormat);
    FormatInfo fFormatTable[kNumMtlFormats];

    const FormatInfo& getFormatInfo(const MTLPixelFormat pixelFormat) const {
        size_t index = GetFormatIndex(pixelFormat);
        return fFormatTable[index];
    }

    MTLPixelFormat fColorTypeToFormatTable[kSkColorTypeCnt];
    void setColorType(SkColorType, std::initializer_list<MTLPixelFormat> formats);

    // A vector of the viable sample counts (e.g., { 1, 2, 4, 8 }).
    std::vector<uint32_t> fColorSampleCounts;

    GPUFamily fGPUFamily;
    int fFamilyGroup;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlCaps_DEFINED
