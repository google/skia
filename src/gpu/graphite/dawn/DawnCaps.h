/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnCaps_DEFINED
#define skgpu_graphite_DawnCaps_DEFINED

#include "src/gpu/graphite/Caps.h"

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
    bool isRenderable(const TextureInfo&) const override;
    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            Shareable,
                            GraphiteResourceKey*) const override;

    size_t getMinBufferAlignment() const { return 4; }

private:
    const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const override;
    bool onIsTexturable(const TextureInfo&) const override;
    size_t getTransferBufferAlignment(size_t bytesPerPixel) const override;
    bool supportsWritePixels(const TextureInfo& textureInfo) const override;
    bool supportsReadPixels(const TextureInfo& textureInfo) const override;
    SkColorType supportedWritePixelsColorType(SkColorType dstColorType,
                                              const TextureInfo& dstTextureInfo,
                                              SkColorType srcColorType) const override;
    SkColorType supportedReadPixelsColorType(SkColorType srcColorType,
                                             const TextureInfo& srcTextureInfo,
                                             SkColorType dstColorType) const override;

    void initCaps(const wgpu::Device& device);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnCaps_DEFINED

