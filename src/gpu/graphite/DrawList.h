/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawList_DEFINED
#define skgpu_graphite_DrawList_DEFINED

#include "src/gpu/graphite/DrawListBase.h"

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/DrawListTypes.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>
#include <limits>
#include <optional>

namespace skgpu::graphite {

class DrawPass;
class Geometry;
class Renderer;

/**
 * A DrawList represents a collection of drawing commands (and related clip/shading state) in
 * a form that closely mirrors what can be rendered efficiently and directly by the GPU backend
 * (while balancing how much pre-processing to do for draws that might get eliminated later due to
 * occlusion culling).
 *
 * A draw command combines:
 *   - a shape
 *   - a transform
 *   - a primitive clip (not affected by the transform)
 *   - optional shading description (shader, color filter, blend mode, etc)
 *   - a draw ordering (compressed painters index, stencil set, and write/test depth)
 *
 * Commands are accumulated in an arbitrary order and then sorted by increasing sort z when the list
 * is prepared into an actual command buffer. The result of a draw command is the rasterization of
 * the transformed shape, restricted by its primitive clip (e.g. a scissor rect) and a depth test
 * of "GREATER" vs. its write/test z. (A test of GREATER, as opposed to GEQUAL, avoids double hits
 * for draws that may have overlapping geometry, e.g. stroking.) If the command has a shading
 * description, the color buffer will be modified; if not, it will be a depth-only draw.
 *
 * In addition to sorting the collected commands, the command list can be optimized during
 * preparation. Commands that are fully occluded by later operations can be skipped entirely without
 * affecting the final results. Adjacent commands (post sort) that would use equivalent GPU
 * pipelines are merged to produce fewer (but larger) operations on the GPU.
 *
 * Other than flush-time optimizations (sort, cull, and merge), the command list does what you tell
 * it to. Draw-specific simplification, style application, and advanced clipping should be handled
 * at a higher layer.
 */
class DrawList : public DrawListBase {
public:
    // Add a constructor to prevent default zero initialization of SkTBlockList members' storage.
    DrawList() {}

    // DrawList requires that all Transforms be valid and asserts as much; invalid transforms should
    // be detected at the Device level or similar. The provided Renderer must be compatible with the
    // 'shape' and 'stroke' parameters. If the renderer uses coverage AA, 'ordering' must have a
    // compressed painters order that reflects that. If the renderer uses stencil, the 'ordering'
    // must have a valid stencil index as well.
    std::pair<DrawParams*, Layer*> recordDraw(
            const Renderer* renderer,
            const Transform& localToDevice,
            const Geometry& geometry,
            const Clip& clip,
            DrawOrder ordering,
            UniquePaintParamsID paintID,
            SkEnumBitMask<DstUsage> dstUsage,
            BarrierType barrierBeforeDraws,
            PipelineDataGatherer* gatherer,
            const StrokeStyle* stroke,
            const Layer* latestDepthLayer) override;

    std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                           sk_sp<TextureProxy> target,
                                           const SkImageInfo& targetInfo,
                                           DstReadStrategy dstReadStrategy) override;

    // Discard all previously recorded draws and set to the requested load op (with optional clear
    // color).
    void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f}) override;

private:
    struct Draw {
    public:
        Draw(const Renderer* renderer, const Transform& transform, const Geometry& geometry,
             const Clip& clip, DrawOrder order, BarrierType barrierBeforeDraws,
             const StrokeStyle* stroke)
                : fRenderer(renderer)
                , fDrawParams(transform, geometry, clip, order, stroke, barrierBeforeDraws) {}

        const Renderer* renderer()              const { return fRenderer;   }
        const DrawParams& drawParams()          const { return fDrawParams; }

    private:
        const Renderer* fRenderer;
        DrawParams fDrawParams;
    };

    template <uint64_t Bits, uint64_t Offset>
    struct Bitfield {
        static constexpr uint64_t kMask = ((uint64_t) 1 << Bits) - 1;
        static constexpr uint64_t kOffset = Offset;
        static constexpr uint64_t kBits = Bits;

        static uint32_t get(uint64_t v) { return static_cast<uint32_t>((v >> kOffset) & kMask); }
        static uint64_t set(uint32_t v) { return (v & kMask) << kOffset; }
    };

    /**
     * Each Draw in a DrawList might be processed by multiple RenderSteps (determined by the Draw's
     * Renderer), which can be sorted independently. Each (step, draw) pair produces its own
     * SortKey.
     *
     * The goal of sorting draws for the DrawPass is to minimize pipeline transitions and dynamic
     * binds within a pipeline, while still respecting the overall painter's order. This decreases
     * the number of low-level draw commands in a command buffer and increases the size of those,
     * allowing the GPU to operate more efficiently and have fewer bubbles within its own
     * instruction stream.
     *
     * The Draw's CompresssedPaintersOrder and DisjointStencilIndex represent the most significant
     * bits of the key, and are shared by all SortKeys produced by the same draw. Next, the pipeline
     * description is encoded in two steps:
     *  1. The index of the RenderStep packed in the high bits to ensure each step for a draw is
     *     ordered correctly.
     *  2. An index into a cache of pipeline descriptions is used to encode the identity of the
     *     pipeline (SortKeys that differ in the bits from #1 necessarily would have different
     *     descriptions, but then the specific ordering of the RenderSteps isn't enforced). Last,
     *     the SortKey encodes an index into the set of uniform bindings accumulated for a DrawPass.
     *     This allows the SortKey to cluster draw steps that have both a compatible pipeline and do
     *     not require rebinding uniform data or other state (e.g. scissor). Since the uniform data
     *     index and the pipeline description index are packed into indices and not actual pointers,
     *     a given SortKey is only valid for the a specific DrawList->DrawPass conversion.
     */
    class SortKey {
    public:
        SortKey(const Draw* draw, int renderStep,
                GraphicsPipelineCache::Index pipelineIndex,
                UniformDataCache::Index uniformIndex,
                TextureDataCache::Index textureBindingIndex)
                : fPipelineKey(
                          ColorDepthOrderField::set(draw->drawParams().order().paintOrder().bits())
                          | StencilIndexField::set(draw->drawParams().order().stencilIndex().bits())
                          | RenderStepField::set(static_cast<uint32_t>(renderStep))
                          | PipelineField::set(pipelineIndex))
                , fUniformKey(UniformField::set(uniformIndex) |
                              TextureBindingsField::set(textureBindingIndex))
                , fDraw(draw) {
            SkASSERT(pipelineIndex < GraphicsPipelineCache::kInvalidIndex);
            SkASSERT(renderStep <= draw->renderer()->numRenderSteps());
        }

        bool operator<(const SortKey& k) const {
            return fPipelineKey < k.fPipelineKey ||
                (fPipelineKey == k.fPipelineKey && fUniformKey < k.fUniformKey);
        }

        const RenderStep& renderStep() const {
            return fDraw->renderer()->step(RenderStepField::get(fPipelineKey));
        }

        const Draw& draw() const { return *fDraw; }

        GraphicsPipelineCache::Index pipelineIndex() const {
            return PipelineField::get(fPipelineKey);
        }
        UniformDataCache::Index uniformIndex() const {
            return UniformField::get(fUniformKey);
        }
        TextureDataCache::Index textureBindingIndex() const {
            return TextureBindingsField::get(fUniformKey);
        }

    private:
        // Fields are ordered from most-significant to least when sorting by 128-bit value.
        // NOTE: We don't use C++ bit fields because field ordering is implementation defined and we
        // need to sort consistently.
        using ColorDepthOrderField = Bitfield<16, 48>; // sizeof(CompressedPaintersOrder)
        using StencilIndexField    = Bitfield<16, 32>; // sizeof(DisjointStencilIndex)
        using RenderStepField      = Bitfield<2,  30>; // bits >= log2(Renderer::kMaxRenderSteps)
        using PipelineField        = Bitfield<30, 0>;  // bits >= log2(max total steps in draw list)
        uint64_t fPipelineKey;

        // The uniform/texture index fields need 1 extra bit to encode "no-data". Values that are
        // greater than or equal to 2^(bits-1) represent "no-data", while values between
        // [0, 2^(bits-1)-1] can access data arrays without extra logic.
        using UniformField         = Bitfield<34, 30>; // bits >= 1+log2(max total steps)
        using TextureBindingsField = Bitfield<30, 0>;  // bits >= 1+log2(max total steps)
        uint64_t fUniformKey;

        // Backpointer to the draw that produced the sort key
        const Draw* fDraw;

// https://issues.skia.org/issues/485303056
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-value-range-compare"
#endif
        static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
        static_assert(StencilIndexField::kBits    >= sizeof(DisjointStencilIndex));
        static_assert(RenderStepField::kBits      >= SkNextLog2(Renderer::kMaxRenderSteps));
        static_assert(PipelineField::kBits        >= SkNextLog2(DrawListBase::kMaxRenderSteps));
        static_assert(UniformField::kBits         >= 1+SkNextLog2(DrawListBase::kMaxRenderSteps));
        static_assert(TextureBindingsField::kBits >= 1+SkNextLog2(DrawListBase::kMaxRenderSteps));
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    };

    SkTBlockList<Draw, 4> fDraws{SkBlockAllocator::GrowthPolicy::kFibonacci};

    std::vector<SortKey> fSortKeys;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawList_DEFINED
