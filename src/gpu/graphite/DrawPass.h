/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawPass_DEFINED
#define skgpu_graphite_DrawPass_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineHandle.h"

struct SkImageInfo;

namespace skgpu::graphite {

class CommandBuffer;
class DrawList;
class FloatStorageManager;
class GraphicsPipeline;
struct RenderPassDesc;
class ResourceProvider;
class RuntimeEffectDictionary;
class TextureProxy;

enum class DstReadStrategy : uint8_t;
enum class LoadOp : uint8_t;
enum class StoreOp : uint8_t;

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

    // Defined relative to the top-left corner of the surface the DrawPass renders to, and is
    // contained within its dimensions.
    const SkIRect&      bounds() const { return fBounds;       }
    TextureProxy* target() const { return fTarget.get(); }
    FloatStorageManager* floatStorageManager() const { return fFloatStorageManager.get(); }
    std::pair<LoadOp, StoreOp> ops() const { return fOps; }
    std::array<float, 4> clearColor() const { return fClearColor; }

    size_t vertexBufferSize()  const { return 0; }
    size_t uniformBufferSize() const { return 0; }

    // Instantiate and prepare any resources used by the DrawPass that require the Recorder's
    // ResourceProvider. This includes things likes GraphicsPipelines, sampled Textures, Samplers,
    // etc.
    // Note that, due to possible threaded compilation, the Pipelines are not guaranteed to be
    // complete until Context::insertRecording time.
    bool prepareResources(ResourceProvider*,
                          sk_sp<const RuntimeEffectDictionary>,
                          const RenderPassDesc&);

    DrawPassCommands::List::Iter commands() const {
        return fCommandList.commands();
    }

    const GraphicsPipeline* getPipeline(size_t index) const {
        return fFullPipelines[index].get();
    }

    // Proxies are always valid but may not be instantiated until after prepareResources() is called
    SkSpan<const sk_sp<TextureProxy>> sampledTextures() const { return fSampledTextures; }
    // Not valid until after prepareResources() is called
    SkSpan<const sk_sp<GraphicsPipeline>> pipelines() const { return fFullPipelines; }

    [[nodiscard]] bool addResourceRefs(ResourceProvider*, CommandBuffer*);

private:
    friend class DrawList; // For the constructor

    DrawPass(sk_sp<TextureProxy> target,
             std::pair<LoadOp, StoreOp> ops,
             std::array<float, 4> clearColor,
             sk_sp<FloatStorageManager> floatStorageManager);

    DrawPassCommands::List fCommandList;

    sk_sp<TextureProxy> fTarget;
    SkIRect fBounds;

    std::pair<LoadOp, StoreOp> fOps;
    std::array<float, 4> fClearColor;

    // The pipelines are referenced by index in BindGraphicsPipeline, but that will index into
    // an array of actual GraphicsPipelines (i.e., fFullPipelines).
    skia_private::TArray<GraphicsPipelineDesc> fPipelineDescs;
    skia_private::TArray<float> fPipelineDrawAreas;

    // These resources all get instantiated during prepareResources.
    skia_private::TArray<GraphicsPipelineHandle> fPipelineHandles;
    skia_private::TArray<sk_sp<TextureProxy>> fSampledTextures;

    // These get resolved (from the GraphicsPipelineHandles) in prepareResources
    skia_private::TArray<sk_sp<GraphicsPipeline>> fFullPipelines;

    sk_sp<FloatStorageManager> fFloatStorageManager;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawPass_DEFINED
