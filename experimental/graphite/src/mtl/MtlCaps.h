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

#include "experimental/graphite/src/Caps.h"

namespace skgpu::graphite {

class MtlCaps final : public skgpu::Caps {
public:
    MtlCaps(const id<MTLDevice>);
    ~MtlCaps() override {}

    skgpu::TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                                    uint32_t levelCount,
                                                    Protected,
                                                    Renderable) const override;

    skgpu::TextureInfo getDefaultMSAATextureInfo(SkColorType,
                                                 uint32_t sampleCount,
                                                 Protected) const override;

    skgpu::TextureInfo getDefaultDepthStencilTextureInfo(Mask<DepthStencilFlags>,
                                                         uint32_t sampleCount,
                                                         Protected) const override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;

    bool isMac() const { return fGPUFamily == GPUFamily::kMac; }
    bool isApple()const  { return fGPUFamily == GPUFamily::kApple; }

    size_t getMinBufferAlignment() const { return this->isMac() ? 4 : 1; }

    bool isRenderable(const skgpu::TextureInfo&) const override;

    void buildKeyForTexture(SkISize dimensions,
                            const skgpu::TextureInfo&,
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

    const ColorTypeInfo* getColorTypeInfo(SkColorType, const skgpu::TextureInfo&) const override;

    bool onIsTexturable(const skgpu::TextureInfo&) const override;
    bool isTexturable(MTLPixelFormat) const;
    bool isRenderable(MTLPixelFormat, uint32_t numSamples) const;
    uint32_t maxRenderTargetSampleCount(MTLPixelFormat) const;

    size_t getTransferBufferAlignment(size_t bytesPerPixel) const override;

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
    inline static constexpr size_t kNumMtlFormats = 8;

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
