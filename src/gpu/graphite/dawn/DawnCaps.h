/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnCaps_DEFINED
#define skgpu_graphite_DawnCaps_DEFINED

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureFormat.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include <array>

namespace skgpu::graphite {
struct ContextOptions;

struct DawnBackendContext;

class DawnCaps final : public Caps {
public:
    DawnCaps(const DawnBackendContext&, const ContextOptions&);
    ~DawnCaps() override;

    bool supportsHalfPrecision() const { return fSupportsHalfPrecision; }
    bool useAsyncPipelineCreation() const { return fUseAsyncPipelineCreation; }
    bool allowScopedErrorChecks() const { return fAllowScopedErrorChecks; }

    // If this has no value then loading the resolve texture via a LoadOp is not supported.
    std::optional<wgpu::LoadOp> resolveTextureLoadOp() const {
        return fSupportedResolveTextureLoadOp;
    }
    bool supportsPartialLoadResolve() const { return fSupportsPartialLoadResolve; }
    bool supportsRenderPassRenderArea() const { return fSupportsRenderPassRenderArea; }

    SkISize getDepthAttachmentDimensions(const TextureInfo&,
                                         const SkISize colorAttachmentDimensions) const override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;
    bool extractGraphicsDescs(const UniqueKey&,
                              GraphicsPipelineDesc*,
                              RenderPassDesc*,
                              const RendererProvider*) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override;
    ImmutableSamplerInfo getImmutableSamplerInfo(const TextureInfo&) const override;
    std::string toString(const ImmutableSamplerInfo&) const override;

    bool loadOpAffectsMSAAPipelines() const override {
        return fSupportedResolveTextureLoadOp.has_value();
    }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            GraphiteResourceKey*) const override;
    // Compute render pass desc's key as 32 bits key. The key has room for additional flag which can
    // optionally be provided.
    uint32_t getRenderPassDescKeyForPipeline(const RenderPassDesc&,
                                             bool additionalFlag = false) const;

    bool supportsCommandBufferTimestamps() const { return fSupportsCommandBufferTimestamps; }

    // Whether we should emulate load/resolve with separate render passes.
    // TODO(b/399640773): This is currently used until Dawn supports true partial resolve feature
    // that can resolve a MSAA texture to a resolve texture with different size.
    bool emulateLoadStoreResolve() const { return fEmulateLoadStoreResolve; }

private:
    TextureInfo onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                        TextureFormat,
                                        SampleCount,
                                        Mipmapped,
                                        Protected,
                                        Discardable) const override;
    std::pair<SkEnumBitMask<TextureUsage>, Tiling> getTextureUsage(
            const TextureInfo&) const override;

    void initCaps(const DawnBackendContext&, const ContextOptions&);
    void initShaderCaps(const wgpu::Device&);
    void initFormatTable(const wgpu::Device&);

    // When supported, this value will hold the TransientAttachment usage symbol that is only
    // defined in Dawn native builds and not EMSCRIPTEN but this avoids having to #define guard it.
    wgpu::TextureUsage fSupportedTransientAttachmentUsage = wgpu::TextureUsage::None;
    // When supported this holds the ExpandResolveTexture load op, otherwise holds no value.
    std::optional<wgpu::LoadOp> fSupportedResolveTextureLoadOp;
    // When 'fSupportedResolveTextureLoadOp' is supported, it by default performs full size expand
    // and resolve. With this feature, we can do that partially according to the actual damage
    // region.
    bool fSupportsPartialLoadResolve = false;
    bool fSupportsRenderPassRenderArea = false;

    bool fEmulateLoadStoreResolve = false;

    bool fUseAsyncPipelineCreation = true;
    bool fAllowScopedErrorChecks = true;

    bool fSupportsCommandBufferTimestamps = false;
    bool fSupportsHalfPrecision = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnCaps_DEFINED
