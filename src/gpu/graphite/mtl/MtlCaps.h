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
#include "src/gpu/graphite/TextureFormat.h"

namespace skgpu::graphite {
struct ContextOptions;

class MtlCaps final : public Caps {
public:
    MtlCaps(const id<MTLDevice>, const ContextOptions&);
    ~MtlCaps() override {}

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override;

    bool extractGraphicsDescs(const UniqueKey&,
                              GraphicsPipelineDesc*,
                              RenderPassDesc*,
                              const RendererProvider*) const override;

    // Get a sufficiently unique bit representation for the RenderPassDesc to be embedded in other
    // UniqueKeys (e.g. makeGraphicsPipelineKey).
    uint32_t getRenderPassDescKey(const RenderPassDesc&) const;

    bool isMac() const { return fGPUFamily == MTLGPUFamilyMac2; }
    bool isApple() const {
        // Apple silicon vs. legacy Mac hardware is mutually exclusive
        SkASSERT(this->isMac() || (fGPUFamily >= MTLGPUFamilyApple2 &&
                                   fGPUFamily <= MTLGPUFamilyApple9));
        return !this->isMac();
    }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            GraphiteResourceKey*) const override;

private:
    void initGPUFamily(const id<MTLDevice>);

    void initCaps(const id<MTLDevice>);
    void initFormatTable(const id<MTLDevice>);

    TextureInfo onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                        TextureFormat,
                                        SampleCount,
                                        Mipmapped,
                                        Protected,
                                        Discardable) const override;
    std::pair<SkEnumBitMask<TextureUsage>, Tiling> getTextureUsage(
            const TextureInfo&) const override;

    std::pair<SkEnumBitMask<TextureUsage>, SkEnumBitMask<SampleCount>> getTextureSupport(
            TextureFormat format, Tiling tiling) const override {
        if (tiling == Tiling::kLinear) {
            return {{}, {}}; // Linear tiling is not supported
        }
        return fFormatSupport[static_cast<int>(format)];
    }

    // Cache conversion from MTLPixelFormat support queries to TextureFormat and TextureUsage
    // support and per-format supported SampleCounts.
    std::array<std::pair<SkEnumBitMask<TextureUsage>, SkEnumBitMask<SampleCount>>,
               kTextureFormatCount> fFormatSupport;

    MTLGPUFamily fGPUFamily;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlCaps_DEFINED
