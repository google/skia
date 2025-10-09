/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawList_DEFINED
#define skgpu_graphite_DrawList_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawCommands.h"
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
class DrawList {
public:
    // The maximum number of render steps that can be recorded into a DrawList before it must be
    // converted to a DrawPass. The true fundamental limit is imposed by the limits of the depth
    // attachment and precision of CompressedPaintersOrder and PaintDepth. These values can be
    // shared by multiple draw calls so it's more difficult to reason about how much room is left
    // in a DrawList. Limiting it to this keeps tracking simple and ensures that the sequences in
    // DrawOrder cannot overflow since they are always less than or equal to the number of draws.
    // TODO(b/322840221): The theoretic max for this value is 16-bit, but we see markedly better
    // performance with smaller values. This should be understood and fixed directly rather than as
    // a magic side-effect, but for now, let it go fast.
    static constexpr int kMaxRenderSteps = 4096;
    static_assert(kMaxRenderSteps <= std::numeric_limits<uint16_t>::max());

    // Add a construtor to prevent default zero initialization of SkTBlockList members' storage.
    DrawList() {}

    // DrawList requires that all Transforms be valid and asserts as much; invalid transforms should
    // be detected at the Device level or similar. The provided Renderer must be compatible with the
    // 'shape' and 'stroke' parameters. If the renderer uses coverage AA, 'ordering' must have a
    // compressed painters order that reflects that. If the renderer uses stencil, the 'ordering'
    // must have a valid stencil index as well.
    void recordDraw(const Renderer* renderer,
                    const Transform& localToDevice,
                    const Geometry& geometry,
                    const Clip& clip,
                    DrawOrder ordering,
                    UniquePaintParamsID paintID,
                    SkEnumBitMask<DstUsage> dstUsage,
                    BarrierType barrierBeforeDraws,
                    PipelineDataGatherer* gatherer,
                    const StrokeStyle* stroke);

    std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                           sk_sp<TextureProxy> target,
                                           const SkImageInfo& targetInfo,
                                           const DstReadStrategy dstReadStrategy);

    int renderStepCount() const { return fRenderStepCount; }

    bool modifiesTarget() const {
        return this->renderStepCount() > 0 || fLoadOp == LoadOp::kClear;
    }

    bool samplesTexture(const TextureProxy* texture) const {
        return fTextureDataCache.hasTexture(texture);
    }

    // Discard all previously recorded draws and set to the requested load op (with optional clear
    // color).
    void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f});

    // Bounds for a dst read required by this DrawList. These bounds are only valid if drawsReadDst
    // returns true.
    const Rect& dstReadBounds() const { return fDstReadBounds; }
    const Rect& passBounds() const { return fPassBounds; }
    bool drawsReadDst() const { return !fDstReadBounds.isEmptyNegativeOrNaN(); }
    bool drawsRequireMSAA() const { return fRequiresMSAA; }
    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const { return fDepthStencilFlags; }

    SkDEBUGCODE(bool hasCoverageMaskDraws() const { return fCoverageMaskShapeDrawCount > 0; })

private:
    friend class DrawPass;

    struct Draw {
    public:
        Draw(const Renderer* renderer, const Transform& transform, const Geometry& geometry,
             const Clip& clip, DrawOrder order, BarrierType barrierBeforeDraws,
             const StrokeStyle* stroke)
                : fRenderer(renderer)
                , fDrawParams(transform, geometry, clip, order, stroke)
                , fBarrierBeforeDraws(barrierBeforeDraws) {}

        const Renderer* renderer()                             const { return fRenderer;           }
        const DrawParams& drawParams()                         const { return fDrawParams;         }
        const BarrierType& barrierBeforeDraws()                const { return fBarrierBeforeDraws; }

    private:
        const Renderer* fRenderer; // Owned by SharedContext of Recorder that recorded the draw
        DrawParams fDrawParams; // The DrawParam's transform is owned by fTransforms of the DrawList
        BarrierType fBarrierBeforeDraws;
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
        SortKey(const DrawList::Draw* draw,
                int renderStep,
                GraphicsPipelineCache::Index pipelineIndex,
                UniformDataCache::Index geomUniformIndex,
                UniformDataCache::Index shadingUniformIndex,
                TextureDataCache::Index textureBindingIndex)
                : fPipelineKey(
                          ColorDepthOrderField::set(draw->drawParams().order().paintOrder().bits())
                          | StencilIndexField::set(draw->drawParams().order().stencilIndex().bits())
                          | RenderStepField::set(static_cast<uint32_t>(renderStep))
                          | PipelineField::set(pipelineIndex))
                , fUniformKey(GeometryUniformField::set(geomUniformIndex)   |
                              ShadingUniformField::set(shadingUniformIndex) |
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

        const DrawList::Draw& draw() const { return *fDraw; }

        GraphicsPipelineCache::Index pipelineIndex() const {
            return PipelineField::get(fPipelineKey);
        }
        UniformDataCache::Index geometryUniformIndex() const {
            return GeometryUniformField::get(fUniformKey);
        }
        UniformDataCache::Index shadingUniformIndex() const {
            return ShadingUniformField::get(fUniformKey);
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
        using GeometryUniformField = Bitfield<17, 47>; // bits >= 1+log2(max total steps)
        using ShadingUniformField  = Bitfield<17, 30>; // bits >= 1+log2(max total steps)
        using TextureBindingsField = Bitfield<30, 0>;  // bits >= 1+log2(max total steps)
        uint64_t fUniformKey;

        // Backpointer to the draw that produced the sort key
        const DrawList::Draw* fDraw;

        static_assert(ColorDepthOrderField::kBits >= sizeof(CompressedPaintersOrder));
        static_assert(StencilIndexField::kBits    >= sizeof(DisjointStencilIndex));
        static_assert(RenderStepField::kBits      >= SkNextLog2_portable(Renderer::kMaxRenderSteps));
        static_assert(PipelineField::kBits        >= SkNextLog2_portable(DrawList::kMaxRenderSteps));
        static_assert(GeometryUniformField::kBits >= 1+SkNextLog2_portable(DrawList::kMaxRenderSteps));
        static_assert(ShadingUniformField::kBits  >= 1+SkNextLog2_portable(DrawList::kMaxRenderSteps));
        static_assert(TextureBindingsField::kBits >= 1+SkNextLog2_portable(DrawList::kMaxRenderSteps));
    };


    // The returned Transform reference remains valid for the lifetime of the DrawList.
    const Transform& deduplicateTransform(const Transform&);

    SkTBlockList<Transform, 16> fTransforms{SkBlockAllocator::GrowthPolicy::kFibonacci};
    SkTBlockList<Draw, 16>      fDraws{SkBlockAllocator::GrowthPolicy::kFibonacci};

    // Running total of RenderSteps for all draws, assuming nothing is culled
    int fRenderStepCount = 0;

#if defined(SK_DEBUG)
    // The number of CoverageMask draws that have been recorded. Used in debugging.
    int fCoverageMaskShapeDrawCount = 0;
#endif

    // Tracked for all paints that read from the dst. If it is later determined that the
    // DstReadStrategy is not kTextureCopy, this value can simply be ignored.
    Rect fDstReadBounds = Rect::InfiniteInverted();
    Rect fPassBounds = Rect::InfiniteInverted();
    // Other properties of draws contained within this DrawList
    bool fRequiresMSAA = false;
    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;

    std::vector<SortKey> fSortKeys;

    UniformDataCache fGeometryUniformDataCache;
    UniformDataCache fShadingUniformDataCache;
    TextureDataCache fTextureDataCache;
    GraphicsPipelineCache fPipelineCache;

    LoadOp fLoadOp = LoadOp::kLoad;
    std::array<float, 4> fClearColor = {0.f, 0.f, 0.f, 0.f};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawList_DEFINED
