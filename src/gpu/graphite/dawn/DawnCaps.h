/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnCaps_DEFINED
#define skgpu_graphite_DawnCaps_DEFINED

#include "src/gpu/graphite/Caps.h"

namespace skgpu::graphite {
struct ContextOptions;

class DawnCaps final : public Caps {
public:
    DawnCaps();
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
                                      const RenderPassDesc&) const override { return {}; }
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override { return {}; }

    bool isRenderable(const TextureInfo&) const override { return false; }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            Shareable,
                            GraphiteResourceKey*) const override {}

private:
    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override {
        return nullptr;
    }
    bool onIsTexturable(const TextureInfo&) const override { return false; }
    size_t getTransferBufferAlignment(size_t bytesPerPixel) const override { return 0; }
    bool supportsWritePixels(const TextureInfo& textureInfo) const override { return false; }
    bool supportsReadPixels(const TextureInfo& textureInfo) const override { return false; }
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
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnCaps_DEFINED

