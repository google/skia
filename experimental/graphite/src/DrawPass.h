/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawPass_DEFINED
#define skgpu_DrawPass_DEFINED

#include "experimental/graphite/src/DrawTypes.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/ResourceTypes.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkTBlockList.h"

#include <memory>

namespace skgpu {

class BoundsManager;
class CommandBuffer;
class DrawList;
class Recorder;
struct RenderPassDesc;
class ResourceProvider;
class TextureProxy;
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

    // TODO: Replace SDC with the SDC's surface proxy view
    static std::unique_ptr<DrawPass> Make(Recorder*,
                                          std::unique_ptr<DrawList>,
                                          sk_sp<TextureProxy>,
                                          std::pair<LoadOp, StoreOp>,
                                          std::array<float, 4> clearColor,
                                          const BoundsManager* occlusionCuller);

    // Defined relative to the top-left corner of the surface the DrawPass renders to, and is
    // contained within its dimensions.
    const SkIRect&      bounds() const { return fBounds;       }
    TextureProxy* target() const { return fTarget.get(); }
    std::pair<LoadOp, StoreOp> ops() const { return fOps; }
    std::array<float, 4> clearColor() const { return fClearColor; }

    bool requiresDstTexture() const { return false;            }
    bool requiresMSAA()       const { return fRequiresMSAA;    }

    Mask<DepthStencilFlags> depthStencilFlags() const { return fDepthStencilFlags; }

    size_t vertexBufferSize()  const { return 0; }
    size_t uniformBufferSize() const { return 0; }

    // TODO: Real return types, but it seems useful for DrawPass to report these as sets so that
    // task execution can compile necessary programs and track resources within a render pass.
    // Maybe these won't need to be exposed and RenderPassTask can do it per command as needed when
    // it iterates over the DrawPass contents.
    void samplers() const {}
    void programs() const {}

    // Transform this DrawPass into commands issued to the CommandBuffer. Assumes that the buffer
    // has already begun a correctly configured render pass matching this pass's target.
    void addCommands(Context*, CommandBuffer*, const RenderPassDesc&) const;

private:
    class SortKey;
    class Drawer;

    struct BindGraphicsPipeline {
        // Points to a GraphicsPipelineDesc in DrawPass's fPipelineDescs array. It will also
        // index into a parallel array of full GraphicsPipelines when commands are added to the CB.
        uint32_t fPipelineIndex;
    };
    struct BindUniformBuffer {
        BindBufferInfo fInfo;
        UniformSlot fSlot;
    };
    struct BindDrawBuffers {
        BindBufferInfo fVertices;
        BindBufferInfo fInstances;
        BindBufferInfo fIndices;
    };
    struct Draw {
        PrimitiveType fType;
        uint32_t fBaseVertex;
        uint32_t fVertexCount;
    };
    struct DrawIndexed {
        PrimitiveType fType;
        uint32_t fBaseIndex;
        uint32_t fIndexCount;
        uint32_t fBaseVertex;
    };
    struct DrawInstanced {
        PrimitiveType fType;
        uint32_t fBaseVertex;
        uint32_t fVertexCount;
        uint32_t fBaseInstance;
        uint32_t fInstanceCount;
    };
    struct DrawIndexedInstanced {
        PrimitiveType fType;
        uint32_t fBaseIndex;
        uint32_t fIndexCount;
        uint32_t fBaseVertex;
        uint32_t fBaseInstance;
        uint32_t fInstanceCount;
    };
    struct SetScissor {
        SkIRect fScissor;
    };

    // TODO: BindSampler

    enum class CommandType {
        kBindGraphicsPipeline,
        kBindUniformBuffer,
        kBindDrawBuffers,
        kDraw,
        kDrawIndexed,
        kDrawInstanced,
        kDrawIndexedInstanced,
        kSetScissor,
        // kBindSampler
    };
    // TODO: The goal is keep all command data in line, vs. type + void* to another data array, but
    // the current union is memory inefficient. It would be better to have a byte buffer with per
    // type advances, but then we need to work out alignment etc. so that will be easier to add
    // once we have something up and running.
    struct Command {
        CommandType fType;
        union {
            BindGraphicsPipeline fBindGraphicsPipeline;
            BindUniformBuffer    fBindUniformBuffer;
            BindDrawBuffers      fBindDrawBuffers;
            Draw                 fDraw;
            DrawIndexed          fDrawIndexed;
            DrawInstanced        fDrawInstanced;
            DrawIndexedInstanced fDrawIndexedInstanced;
            SetScissor           fSetScissor;
        };

        explicit Command(BindGraphicsPipeline d)
                : fType(CommandType::kBindGraphicsPipeline), fBindGraphicsPipeline(d) {}
        explicit Command(BindUniformBuffer d)
                : fType(CommandType::kBindUniformBuffer), fBindUniformBuffer(d) {}
        explicit Command(BindDrawBuffers d)
                : fType(CommandType::kBindDrawBuffers), fBindDrawBuffers(d) {}
        explicit Command(Draw d)
                : fType(CommandType::kDraw), fDraw(d) {}
        explicit Command(DrawIndexed d)
                : fType(CommandType::kDrawIndexed), fDrawIndexed(d) {}
        explicit Command(DrawInstanced d)
                : fType(CommandType::kDrawInstanced), fDrawInstanced(d) {}
        explicit Command(DrawIndexedInstanced d)
                : fType(CommandType::kDrawIndexedInstanced), fDrawIndexedInstanced(d) {}
        explicit Command(SetScissor d)
                : fType(CommandType::kSetScissor), fSetScissor(d) {}
    };
    // Not strictly necessary, but keeping Command trivially destructible means the command list
    // can be cleaned up efficiently once it's converted to a command buffer.
    static_assert(std::is_trivially_destructible<Command>::value);

    DrawPass(sk_sp<TextureProxy> target,
             std::pair<LoadOp, StoreOp> ops,
             std::array<float, 4> clearColor,
             int renderStepCount);

    SkTBlockList<Command, 32> fCommands;
    // The pipelines are referenced by index in BindGraphicsPipeline, but that will index into a
    // an array of actual GraphicsPipelines. fPipelineDescs only needs to accumulate encountered
    // GraphicsPipelineDescs and provide stable pointers, hence SkTBlockList.
    SkTBlockList<GraphicsPipelineDesc, 32> fPipelineDescs;

    sk_sp<TextureProxy> fTarget;
    SkIRect             fBounds;

    std::pair<LoadOp, StoreOp> fOps;
    std::array<float, 4>       fClearColor;

    Mask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;
    bool                    fRequiresMSAA = false;
};

} // namespace skgpu

#endif // skgpu_DrawPass_DEFINED
