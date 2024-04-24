/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawPass_DEFINED
#define skgpu_graphite_DrawPass_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkEnumBitMask.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureProxy.h"

#include <memory>

struct SkImageInfo;

namespace skgpu::graphite {

class BoundsManager;
class CommandBuffer;
class DrawList;
class GraphicsPipeline;
class Recorder;
struct RenderPassDesc;
class ResourceProvider;
class RuntimeEffectDictionary;
class Sampler;
class TextureDataBlock;
class Texture;
enum class UniformSlot;

/**
 * DrawPass is analogous to a subpass, storing the drawing operations in the order they are stored
 * in the eventual command buffer, as well as the surface proxy the operations are intended for.
 * DrawPasses are grouped into a RenderPassTask for execution within a single render pass if the
 * subpasses are compatible with each other.
 *
 * Unlike DrawList, DrawPasses are immutable and represent as closely as possible what will be
 * stored in the command buffer while being flexible as to how the pass is incorporated. Depending
 * on the backend, it may even be able to write accumulated vertex and uniform data directly to
 * mapped GPU memory, although that is the extent of the CPU->GPU work they perform before they are
 * executed by a RenderPassTask.
 */
class DrawPass {
public:
    ~DrawPass();

    // Create a DrawPass that renders the DrawList into `target` with the given load/store ops and
    // clear color. If the DrawList has draws that required a dst readback texture copy to sample
    // from in the shader, it must be provided in `dstCopy` and a copy task must be executed before
    // the DrawPass is executed.
    static std::unique_ptr<DrawPass> Make(Recorder*,
                                          std::unique_ptr<DrawList>,
                                          sk_sp<TextureProxy> target,
                                          const SkImageInfo& targetInfo,
                                          std::pair<LoadOp, StoreOp>,
                                          std::array<float, 4> clearColor,
                                          sk_sp<TextureProxy> dstCopy,
                                          SkIPoint dstCopyOffset);

    // Defined relative to the top-left corner of the surface the DrawPass renders to, and is
    // contained within its dimensions.
    const SkIRect&      bounds() const { return fBounds;       }
    TextureProxy* target() const { return fTarget.get(); }
    std::pair<LoadOp, StoreOp> ops() const { return fOps; }
    std::array<float, 4> clearColor() const { return fClearColor; }

    bool requiresDstTexture() const { return false;            }
    bool requiresMSAA()       const { return fRequiresMSAA;    }

    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const { return fDepthStencilFlags; }

    size_t vertexBufferSize()  const { return 0; }
    size_t uniformBufferSize() const { return 0; }

    // Instantiate and prepare any resources used by the DrawPass that require the Recorder's
    // ResourceProvider. This includes things likes GraphicsPipelines, sampled Textures, Samplers,
    // etc.
    bool prepareResources(ResourceProvider*,
                          const RuntimeEffectDictionary*,
                          const RenderPassDesc&);

    DrawPassCommands::List::Iter commands() const {
        return fCommandList.commands();
    }

    const GraphicsPipeline* getPipeline(size_t index) const {
        return fFullPipelines[index].get();
    }
    const Texture* getTexture(size_t index) const;
    const Sampler* getSampler(size_t index) const;

    skia_private::TArray<sk_sp<TextureProxy>> sampledTextures() const { return fSampledTextures; }

    void addResourceRefs(CommandBuffer*) const;

private:
    class SortKey;

    DrawPass(sk_sp<TextureProxy> target,
             std::pair<LoadOp, StoreOp> ops,
             std::array<float, 4> clearColor);

    DrawPassCommands::List fCommandList;

    sk_sp<TextureProxy> fTarget;
    SkIRect fBounds;

    std::pair<LoadOp, StoreOp> fOps;
    std::array<float, 4> fClearColor;

    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;
    bool fRequiresMSAA = false;

    // The pipelines are referenced by index in BindGraphicsPipeline, but that will index into a
    // an array of actual GraphicsPipelines.
    skia_private::TArray<GraphicsPipelineDesc> fPipelineDescs;
    skia_private::TArray<SamplerDesc> fSamplerDescs;

    // These resources all get instantiated during prepareResources.
    skia_private::TArray<sk_sp<GraphicsPipeline>> fFullPipelines;
    skia_private::TArray<sk_sp<TextureProxy>> fSampledTextures;
    skia_private::TArray<sk_sp<Sampler>> fSamplers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawPass_DEFINED
