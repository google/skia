/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_VelloComputeSteps_DEFINED
#define skgpu_graphite_compute_VelloComputeSteps_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "third_party/vello/cpp/vello.h"

#include <string_view>

namespace skgpu::graphite {

// This file defines ComputeSteps for all Vello compute stages and their permutations. The
// declaration of each ComputeStep subclass mirrors the name of the pipeline stage as defined in the
// shader metadata.
//
// The compute stages all operate over a shared set of buffer and image resources. The
// `kVelloSlot_*` constant definitions below each uniquely identify a shared resource that must be
// instantiated when assembling the ComputeSteps into a DispatchGroup.
//
// === Monoids and Prefix Sums ===
//
// Vello's GPU algorithms make repeated use of parallel prefix sums techniques. These occur
// frequently in path rasterization (e.g. winding number accummulation across a scanline can be
// thought of as per-pixel prefix sums) but Vello also uses them to calculate buffer offsets for
// associated entries across its variable length encoding streams.
//
// For instance, given a scene that contains Bézier paths, each path gets encoded as a transform,
// a sequence of path tags (verbs), and zero or more 2-D points associated with each
// tag. N paths will often map to N transforms, N + M tags, and N + M + L points (where N > 0, M >
// 0, L >= 0). These entries are stored in separate parallel transform, path tag, and path data
// streams. The correspondence between entries of these independent streams is implicit. To keep
// CPU encoding of these streams fast, the offsets into each buffer for a given "path object" is
// computed dynamically and in parallel on the GPU. Since the offsets for each object build
// additively on offsets that appear before it in the stream, parallel computation of
// offsets can be treated as a dynamic programming problem that maps well to parallel prefix sums
// where each object is a "monoid" (https://en.wikipedia.org/wiki/Monoid) that supports algebraic
// addition/subtraction over data encoded in the path tags themselves.
//
// Once computed, a monoid contains the offsets into the input (and sometimes output) buffers for a
// given object. The parallel prefix sums operation is defined as a monoidal reduce + pre-scan pair.
// (Prefix Sums and Their Applications, Blelloch, G., https://www.cs.cmu.edu/~guyb/papers/Ble93.pdf)
//
// While these concepts are an implementation detail they are core to the Vello algorithm and are
// reflected in the pipeline names and data slot definitions.
//
// === Full Pipeline ===
//
// The full Vello pipeline stages are as follows and should be dispatched in the following order:
//
// I. Build the path monoid stream:
//   If the input fits within the workgroup size:
//     pathtag_reduce, pathtag_scan_small
//   else
//     pathtag_reduce, pathtag_reduce2, pathtag_scan1, pathtag_scan_large
//
// II. Compute path bounding boxes, convert path segments into cubics:
//   bbox_clear, pathseg
//
// III. Process the draw object stream to build the draw monoids and inputs to the clip stage:
//   draw_reduce, draw_leaf
//
// IV. Compute the bounding boxes for the clip stack from the input stream, if the scene contains
// clips:
//   clip_reduce, clip_leaf
//
// V. Allocate tile and segment buffers for the individual bins and prepare for coarse rasterization
//   binning, tile_alloc, path_coarse
//
// VI. Coarse rasterization
//   backdrop_dyn, coarse
//
// VII. Fine rasterization
//   fine
//
// TODO: Document the coverage mask pipeline once it has been re-implemented.
// TODO: Update this when the stroke rework is complete.

// ***
// Shared buffers that are accessed by various stages.
//
// The render configration uniform buffer.
constexpr int kVelloSlot_ConfigUniform = 0;

// The scene encoding buffer.
constexpr int kVelloSlot_Scene = 1;

// ***
// Buffers used during the element processing stage. This stage converts the stream of variable
// length path tags, transforms, brushes into a "path monoid" stream containing buffer offsets for
// the subsequent stages that associate the input streams with individual draw elements. This stage
// performs a parallel prefix sum (reduce + scan) which can be performed in two dispatches if the
// entire input can be processed by a single workgroup per dispatch. Otherwise, the algorithm
// requires two additional dispatches to continue the traversal (this is due to a lack of primitives
// to synchronize execution across workgroups in MSL and WGSL).
//
// Single pass variant pipelines: pathtag_reduce, pathtag_scan_small
// Multi-pass variant pipelines: pathtag_reduce, pathtag_reduce2, pathtag_scan1, pathtag_scan_large
constexpr int kVelloSlot_TagMonoid = 2;

// Single pass variant slots:
constexpr int kVelloSlot_PathtagReduceOutput = 3;

// Multi pass variant slots:
constexpr int kVelloSlot_LargePathtagReduceFirstPassOutput = kVelloSlot_PathtagReduceOutput;
constexpr int kVelloSlot_LargePathtagReduceSecondPassOutput = 4;
constexpr int kVelloSlot_LargePathtagScanFirstPassOutput = 5;

// ***
// The second part of element processing converts path tags (moveTo, lineTo, quadTo, etc) into a
// fixed-stride buffer of cubic beziers (using the tag monoids computed earlier) and computes their
// bounding boxes.
//
// Pipelines: bbox_clear, pathseg
constexpr int kVelloSlot_PathBBoxes = 6;
constexpr int kVelloSlot_Cubics = 7;

// ***
// The next part prepares the draw object stream (entries in the per-tile command list aka PTCL)
// and additional metadata for the subsequent clipping and binning stages.
//
// Pipelines: draw_reduce, draw_leaf
constexpr int kVelloSlot_DrawReduceOutput = 8;
constexpr int kVelloSlot_DrawMonoid = 9;
constexpr int kVelloSlot_InfoBinData = 10;
constexpr int kVelloSlot_ClipInput = 11;

// ***
// Clipping. The outputs of this stage are the finalized draw monoid and the clip bounding-boxes.
// Clipping involves evaluating the stack monoid: refer to the following paper for the meaning of
// these buffers: https://arxiv.org/pdf/2205.11659.pdf,
// https://en.wikipedia.org/wiki/Bicyclic_semigroup
//
// Pipelines: clip_reduce, clip_leaf
constexpr int kVelloSlot_ClipBicyclic = 12;
constexpr int kVelloSlot_ClipElement = 13;
constexpr int kVelloSlot_ClipBBoxes = 14;

// ***
// Buffers containing bump allocated data, including path segment, tile, bounding box, and binning
// metadata, and the per-tile command list assembled from the draw monoids.
//
// Pipelines: binning, tile_alloc, path_coarse_full, backdrop_dyn, coarse
constexpr int kVelloSlot_DrawBBoxes = 15;
constexpr int kVelloSlot_BumpAlloc = 16;
constexpr int kVelloSlot_BinHeader = 17;

constexpr int kVelloSlot_Path = 18;
constexpr int kVelloSlot_Tile = 19;
constexpr int kVelloSlot_Segments = 20;
constexpr int kVelloSlot_PTCL = 21;

// ***
// Texture resources used by the fine rasterization stage. The gradient image needs to get populated
// on the CPU with pre-computed gradient ramps. The image atlas is intended to hold pre-uploaded
// images that are composited into the scene.
//
// The output image contains the final render.
constexpr int kVelloSlot_OutputImage = 22;
constexpr int kVelloSlot_GradientImage = 23;
constexpr int kVelloSlot_ImageAtlas = 24;

std::string_view VelloStageName(vello_cpp::ShaderStage);
WorkgroupSize VelloStageLocalSize(vello_cpp::ShaderStage);
skia_private::TArray<ComputeStep::WorkgroupBufferDesc> VelloWorkgroupBuffers(
        vello_cpp::ShaderStage);
std::string_view VelloNativeShaderSource(vello_cpp::ShaderStage, ComputeStep::NativeShaderFormat);

template <vello_cpp::ShaderStage S>
class VelloStep : public ComputeStep {
public:
    ~VelloStep() override = default;

    NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
        return {VelloNativeShaderSource(S, format), "main_"};
    }

protected:
    explicit VelloStep(SkSpan<const ResourceDesc> resources)
            : ComputeStep(VelloStageName(S),
                          VelloStageLocalSize(S),
                          resources,
                          AsSpan<ComputeStep::WorkgroupBufferDesc>(VelloWorkgroupBuffers(S)),
                          Flags::kSupportsNativeShader) {}

private:
    // Helper that creates a SkSpan from a universal reference to a container. Generally, creating a
    // SkSpan from an rvalue reference is not safe since the pointer stored in the SkSpan will
    // dangle beyond the constructor expression. In our usage in the constructor above,
    // the lifetime of the temporary TArray should match that of the SkSpan, both of which should
    // live through the constructor call expression.
    //
    // From https://en.cppreference.com/w/cpp/language/reference_initialization#Lifetime_of_a_temporary:
    //
    //     a temporary bound to a reference parameter in a function call exists until the end of the
    //     full expression containing that function call
    //
    template <typename T, typename C>
    static SkSpan<const T> AsSpan(C&& container) {
        return SkSpan(std::data(container), std::size(container));
    }
};

#define VELLO_COMPUTE_STEP(stage)                                                      \
    class Vello##stage##Step final : public VelloStep<vello_cpp::ShaderStage::stage> { \
    public:                                                                            \
        Vello##stage##Step();                                                          \
    };

VELLO_COMPUTE_STEP(Backdrop);
VELLO_COMPUTE_STEP(BackdropDyn);
VELLO_COMPUTE_STEP(BboxClear);
VELLO_COMPUTE_STEP(Binning);
VELLO_COMPUTE_STEP(ClipLeaf);
VELLO_COMPUTE_STEP(ClipReduce);
VELLO_COMPUTE_STEP(Coarse);
VELLO_COMPUTE_STEP(DrawLeaf);
VELLO_COMPUTE_STEP(DrawReduce);
VELLO_COMPUTE_STEP(PathCoarse);
VELLO_COMPUTE_STEP(PathCoarseFull);
VELLO_COMPUTE_STEP(Pathseg);
VELLO_COMPUTE_STEP(PathtagReduce);
VELLO_COMPUTE_STEP(PathtagReduce2);
VELLO_COMPUTE_STEP(PathtagScan1);
VELLO_COMPUTE_STEP(PathtagScanLarge);
VELLO_COMPUTE_STEP(PathtagScanSmall);
VELLO_COMPUTE_STEP(TileAlloc);

#undef VELLO_COMPUTE_STEP

class VelloFineStep final : public VelloStep<vello_cpp::ShaderStage::Fine> {
public:
    VelloFineStep();

    // We need to return a texture format for the bound textures.
    std::tuple<SkISize, SkColorType> calculateTextureParameters(int, const ResourceDesc&) const override;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_VelloComputeSteps_DEFINED
