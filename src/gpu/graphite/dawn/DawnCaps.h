/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnCaps_DEFINED
#define skgpu_graphite_DawnCaps_DEFINED

#include "src/gpu/graphite/Caps.h"

#include <array>

#include "webgpu/webgpu_cpp.h"

namespace skgpu::graphite {
struct ContextOptions;

class DawnCaps final : public Caps {
public:
    DawnCaps(const wgpu::Device&, const ContextOptions&);
    ~DawnCaps() override;

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
                                      const RenderPassDesc&) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override;
    uint32_t channelMask(const TextureInfo&) const override;
    bool isRenderable(const TextureInfo&) const override;
    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            Shareable,
                            GraphiteResourceKey*) const override;
    size_t bytesPerPixel(const TextureInfo&) const override;
    uint64_t getRenderPassDescKey(const RenderPassDesc& renderPassDesc) const;

private:
    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override;
    bool onIsTexturable(const TextureInfo&) const override;
    bool supportsWritePixels(const TextureInfo& textureInfo) const override;
    bool supportsReadPixels(const TextureInfo& textureInfo) const override;
    SkColorType supportedWritePixelsColorType(SkColorType dstColorType,
                                              const TextureInfo& dstTextureInfo,
                                              SkColorType srcColorType) const override;
    SkColorType supportedReadPixelsColorType(SkColorType srcColorType,
                                             const TextureInfo& srcTextureInfo,
                                             SkColorType dstColorType) const override;

    void initCaps(const wgpu::Device& device);
    void initShaderCaps();
    void initFormatTable(const wgpu::Device& device);

    wgpu::TextureFormat getFormatFromColorType(SkColorType colorType) const {
        int idx = static_cast<int>(colorType);
        return fColorTypeToFormatTable[idx];
    }

    uint32_t maxRenderTargetSampleCount(wgpu::TextureFormat format) const;
    bool isTexturable(wgpu::TextureFormat format) const;
    bool isRenderable(wgpu::TextureFormat format, uint32_t numSamples) const;

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
            kTexturable_Flag  = 0x1,
            kRenderable_Flag  = 0x2, // Color attachment and blendable
            kMSAA_Flag        = 0x4,
            kResolve_Flag     = 0x8,
        };
        static const uint16_t kAllFlags = kTexturable_Flag | kRenderable_Flag |
                                          kMSAA_Flag | kResolve_Flag;

        uint16_t fFlags = 0;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };
    std::array<FormatInfo, 8> fFormatTable;

    static size_t GetFormatIndex(wgpu::TextureFormat format);
    const FormatInfo& getFormatInfo(wgpu::TextureFormat format) const {
        size_t index = GetFormatIndex(format);
        return fFormatTable[index];
    }

    wgpu::TextureFormat fColorTypeToFormatTable[kSkColorTypeCnt];
    void setColorType(SkColorType, std::initializer_list<wgpu::TextureFormat> formats);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnCaps_DEFINED
