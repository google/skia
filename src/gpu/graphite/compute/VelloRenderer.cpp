/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/VelloRenderer.h"

#include "include/core/SkPath.h"
#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"

#include <algorithm>

namespace skgpu::graphite {
namespace {

BufferView new_scratch_slice(ScratchBuffer& scratch) {
    size_t size = scratch.size();  // Use the whole buffer.
    BindBufferInfo info = scratch.suballocate(size);
    return {info, info ? size : 0};
}

BufferView new_indirect_slice(DrawBufferManager* mgr, size_t size) {
    BindBufferInfo info = mgr->getIndirectStorage(size, ClearBuffer::kYes);
    return {info, info ? size : 0};
}

::rust::Slice<uint8_t> to_slice(void* ptr, size_t size) {
    return {static_cast<uint8_t*>(ptr), size};
}

vello_cpp::Affine to_vello_affine(const SkMatrix& m) {
    // Vello currently doesn't support perspective scaling and the encoding only accepts a 2x3
    // affine transform matrix.
    return {{m.get(0), m.get(3), m.get(1), m.get(4), m.get(2), m.get(5)}};
}

vello_cpp::Point to_vello_point(const SkPoint& p) { return {p.x(), p.y()}; }

vello_cpp::Color to_vello_color(const SkColor4f& color) {
    SkColor c = color.toSkColor();
    return {
            static_cast<uint8_t>(SkColorGetR(c)),
            static_cast<uint8_t>(SkColorGetG(c)),
            static_cast<uint8_t>(SkColorGetB(c)),
            static_cast<uint8_t>(SkColorGetA(c)),
    };
}

WorkgroupSize to_wg_size(const vello_cpp::WorkgroupSize& src) {
    return WorkgroupSize(src.x, src.y, src.z);
}

vello_cpp::Fill to_fill_type(SkPathFillType fillType) {
    // Vello does not provide an encoding for inverse fill types. When Skia uses vello to render
    // a coverage mask for an inverse fill, it encodes a regular fill and inverts the coverage value
    // after sampling the mask.
    switch (fillType) {
        case SkPathFillType::kWinding:
        case SkPathFillType::kInverseWinding:
            return vello_cpp::Fill::NonZero;
        case SkPathFillType::kEvenOdd:
        case SkPathFillType::kInverseEvenOdd:
            return vello_cpp::Fill::EvenOdd;
    }
    return vello_cpp::Fill::NonZero;
}

vello_cpp::CapStyle to_cap_style(SkPaint::Cap cap) {
    switch (cap) {
        case SkPaint::Cap::kButt_Cap:
            return vello_cpp::CapStyle::Butt;
        case SkPaint::Cap::kRound_Cap:
            return vello_cpp::CapStyle::Round;
        case SkPaint::Cap::kSquare_Cap:
            return vello_cpp::CapStyle::Square;
    }
    SkUNREACHABLE;
}

vello_cpp::JoinStyle to_join_style(SkPaint::Join join) {
    switch (join) {
        case SkPaint::Join::kMiter_Join:
            return vello_cpp::JoinStyle::Miter;
        case SkPaint::Join::kBevel_Join:
            return vello_cpp::JoinStyle::Bevel;
        case SkPaint::Join::kRound_Join:
            return vello_cpp::JoinStyle::Round;
    }
    SkUNREACHABLE;
}

vello_cpp::Stroke to_stroke(const SkStrokeRec& style) {
    return vello_cpp::Stroke{
            /*width=*/style.getWidth(),
            /*miter_limit=*/style.getMiter(),
            /*cap*/ to_cap_style(style.getCap()),
            /*join*/ to_join_style(style.getJoin()),
    };
}

class PathIter : public vello_cpp::PathIterator {
public:
    PathIter(const SkPath& path, const Transform& t)
            : fIterate(path), fIter(fIterate.begin()), fTransform(t) {}

    bool next_element(vello_cpp::PathElement* outElem) override {
        if (fConicQuadIdx < fConicConverter.countQuads()) {
            SkASSERT(fConicQuads != nullptr);
            outElem->verb = vello_cpp::PathVerb::QuadTo;
            int pointIdx = fConicQuadIdx * 2;
            outElem->points[0] = to_vello_point(fConicQuads[pointIdx]);
            outElem->points[1] = to_vello_point(fConicQuads[pointIdx + 1]);
            outElem->points[2] = to_vello_point(fConicQuads[pointIdx + 2]);
            fConicQuadIdx++;
            return true;
        }

        if (fIter == fIterate.end()) {
            return false;
        }

        SkASSERT(outElem);
        auto [verb, points, weights] = *fIter;
        fIter++;

        switch (verb) {
            case SkPathVerb::kMove:
                outElem->verb = vello_cpp::PathVerb::MoveTo;
                outElem->points[0] = to_vello_point(points[0]);
                break;
            case SkPathVerb::kLine:
                outElem->verb = vello_cpp::PathVerb::LineTo;
                outElem->points[0] = to_vello_point(points[0]);
                outElem->points[1] = to_vello_point(points[1]);
                break;
            case SkPathVerb::kConic:
                // The vello encoding API doesn't handle conic sections. Approximate it with
                // quadratic Béziers.
                SkASSERT(fConicQuadIdx >= fConicConverter.countQuads());  // No other conic->quad
                                                                          // conversions should be
                                                                          // in progress
                fConicQuads = fConicConverter.computeQuads(
                        points, *weights, 0.25 / fTransform.maxScaleFactor());
                outElem->verb = vello_cpp::PathVerb::QuadTo;
                outElem->points[0] = to_vello_point(fConicQuads[0]);
                outElem->points[1] = to_vello_point(fConicQuads[1]);
                outElem->points[2] = to_vello_point(fConicQuads[2]);

                // The next call to `next_element` will yield the next quad in the list (at index 1)
                // if `fConicConverter` contains more than 1 quad.
                fConicQuadIdx = 1;
                break;
            case SkPathVerb::kQuad:
                outElem->verb = vello_cpp::PathVerb::QuadTo;
                outElem->points[0] = to_vello_point(points[0]);
                outElem->points[1] = to_vello_point(points[1]);
                outElem->points[2] = to_vello_point(points[2]);
                break;
            case SkPathVerb::kCubic:
                outElem->verb = vello_cpp::PathVerb::CurveTo;
                outElem->points[0] = to_vello_point(points[0]);
                outElem->points[1] = to_vello_point(points[1]);
                outElem->points[2] = to_vello_point(points[2]);
                outElem->points[3] = to_vello_point(points[3]);
                break;
            case SkPathVerb::kClose:
                outElem->verb = vello_cpp::PathVerb::Close;
                break;
        }

        return true;
    }

private:
    SkPathPriv::Iterate fIterate;
    SkPathPriv::RangeIter fIter;

    // Variables used to track conic to quadratic spline conversion. `fTransform` is used to
    // determine the subpixel error tolerance in device coordinate space.
    const Transform& fTransform;
    SkAutoConicToQuads fConicConverter;
    const SkPoint* fConicQuads = nullptr;
    int fConicQuadIdx = 0;
};

}  // namespace

VelloScene::VelloScene() : fEncoding(vello_cpp::new_encoding()) {}

void VelloScene::reset() {
    fEncoding->reset();
}

void VelloScene::solidFill(const SkPath& shape,
                           const SkColor4f& fillColor,
                           const SkPathFillType fillType,
                           const Transform& t) {
    PathIter iter(shape, t);
    fEncoding->fill(to_fill_type(fillType),
                    to_vello_affine(t),
                    {vello_cpp::BrushKind::Solid, {to_vello_color(fillColor)}},
                    iter);
}

void VelloScene::solidStroke(const SkPath& shape,
                             const SkColor4f& fillColor,
                             const SkStrokeRec& style,
                             const Transform& t) {
    // TODO: Obtain dashing pattern here and let Vello handle dashing on the CPU while
    // encoding the path?
    PathIter iter(shape, t);
    vello_cpp::Brush brush{vello_cpp::BrushKind::Solid, {to_vello_color(fillColor)}};
    fEncoding->stroke(to_stroke(style), to_vello_affine(t), brush, iter);
}

void VelloScene::pushClipLayer(const SkPath& shape, const Transform& t) {
    PathIter iter(shape, t);
    fEncoding->begin_clip(to_vello_affine(t), iter);
    SkDEBUGCODE(fLayers++;)
}

void VelloScene::popClipLayer() {
    SkASSERT(fLayers > 0);
    fEncoding->end_clip();
    SkDEBUGCODE(fLayers--;)
}

void VelloScene::append(const VelloScene& other) {
    fEncoding->append(*other.fEncoding);
}

VelloRenderer::VelloRenderer(const Caps* caps) {
    if (ComputeShaderCoverageMaskTargetFormat(caps) == kAlpha_8_SkColorType) {
        fFineArea = std::make_unique<VelloFineAreaAlpha8Step>();
        fFineMsaa16 = std::make_unique<VelloFineMsaa16Alpha8Step>();
        fFineMsaa8 = std::make_unique<VelloFineMsaa8Alpha8Step>();
    } else {
        fFineArea = std::make_unique<VelloFineAreaStep>();
        fFineMsaa16 = std::make_unique<VelloFineMsaa16Step>();
        fFineMsaa8 = std::make_unique<VelloFineMsaa8Step>();
    }
}

VelloRenderer::~VelloRenderer() = default;

std::unique_ptr<DispatchGroup> VelloRenderer::renderScene(const RenderParams& params,
                                                          const VelloScene& scene,
                                                          sk_sp<TextureProxy> target,
                                                          Recorder* recorder) const {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(target);

    if (scene.fEncoding->is_empty()) {
        return nullptr;
    }

    if (params.fWidth == 0 || params.fHeight == 0) {
        return nullptr;
    }

    // TODO: validate that the pixel format matches the pipeline layout.
    // Clamp the draw region to the target texture dimensions.
    const SkISize dims = target->dimensions();
    if (dims.isEmpty() || dims.fWidth < 0 || dims.fHeight < 0) {
        SKGPU_LOG_W("VelloRenderer: cannot render to an empty target");
        return nullptr;
    }

    SkASSERT(scene.fLayers == 0);  // Begin/end clips must be matched.
    auto config = scene.fEncoding->prepare_render(
            std::min(params.fWidth, static_cast<uint32_t>(dims.fWidth)),
            std::min(params.fHeight, static_cast<uint32_t>(dims.fHeight)),
            to_vello_color(params.fBaseColor));
    auto dispatchInfo = config->workgroup_counts();
    auto bufferSizes = config->buffer_sizes();

    DispatchGroup::Builder builder(recorder);

    // In total there are 25 resources that are used across the full pipeline stages. The sizes of
    // these resources depend on the encoded scene. We allocate all of them and assign them
    // directly to the builder here instead of delegating the logic to the ComputeSteps.
    DrawBufferManager* bufMgr = recorder->priv().drawBufferManager();

    size_t uboSize = config->config_uniform_buffer_size();
    auto [uboPtr, configBuf] = bufMgr->getUniformPointer(uboSize);
    if (!uboPtr || !config->write_config_uniform_buffer(to_slice(uboPtr, uboSize))) {
        return nullptr;
    }

    size_t sceneSize = config->scene_buffer_size();
    auto [scenePtr, sceneBuf] = bufMgr->getStoragePointer(sceneSize);
    if (!scenePtr || !config->write_scene_buffer(to_slice(scenePtr, sceneSize))) {
        return nullptr;
    }

    // TODO(b/285189802): The default sizes for the bump buffers (~97MB) exceed Graphite's resource
    // budget if multiple passes are necessary per frame (250MB, see ResouceCache.h). We apply a
    // crude size reduction here which seems to be enough for a 4k x 4k atlas render for the GMs
    // that we have tested. The numbers below are able to render GM_longpathdash with CPU-side
    // stroke expansion.
    //
    // We need to come up with a better approach to accurately predict the sizes for these buffers
    // based on the scene encoding and our resource budget. It should be possible to build a
    // conservative estimate using the total number of path verbs, some heuristic based on the verb
    // and the path's transform, and the total number of tiles.
    //
    // The following numbers amount to ~48MB
    const size_t lines_size = bufferSizes.lines;
    const size_t bin_data_size = bufferSizes.bin_data;
    const size_t tiles_size = bufferSizes.tiles;
    const size_t segments_size = bufferSizes.segments;
    const size_t seg_counts_size = bufferSizes.seg_counts;
    const size_t ptcl_size = bufferSizes.ptcl;

    // See the comments in VelloComputeSteps.h for an explanation of the logic here.

    builder.assignSharedBuffer({configBuf, uboSize}, kVelloSlot_ConfigUniform);
    builder.assignSharedBuffer({sceneBuf, sceneSize}, kVelloSlot_Scene);

    // Buffers get cleared ahead of the entire DispatchGroup. Allocate the bump buffer early to
    // avoid a potentially recycled (and prematurely cleared) scratch buffer.
    ScratchBuffer bump = bufMgr->getScratchStorage(bufferSizes.bump_alloc);
    builder.assignSharedBuffer(new_scratch_slice(bump), kVelloSlot_BumpAlloc, ClearBuffer::kYes);

    // path_reduce
    ScratchBuffer tagmonoids = bufMgr->getScratchStorage(bufferSizes.path_monoids);
    {
        // This can be immediately returned after input processing.
        ScratchBuffer pathtagReduceOutput = bufMgr->getScratchStorage(bufferSizes.path_reduced);
        builder.assignSharedBuffer(new_scratch_slice(pathtagReduceOutput),
                                   kVelloSlot_PathtagReduceOutput);
        builder.assignSharedBuffer(new_scratch_slice(tagmonoids), kVelloSlot_TagMonoid);
        builder.appendStep(&fPathtagReduce, to_wg_size(dispatchInfo.path_reduce));

        // If the input is too large to be fully processed by a single workgroup then a second
        // reduce step and two scan steps are necessary. Otherwise one reduce+scan pair is
        // sufficient.
        //
        // In either case, the result is `tagmonoids`.
        if (dispatchInfo.use_large_path_scan) {
            ScratchBuffer reduced2 = bufMgr->getScratchStorage(bufferSizes.path_reduced2);
            ScratchBuffer reducedScan = bufMgr->getScratchStorage(bufferSizes.path_reduced_scan);

            builder.assignSharedBuffer(new_scratch_slice(reduced2),
                                       kVelloSlot_LargePathtagReduceSecondPassOutput);
            builder.assignSharedBuffer(new_scratch_slice(reducedScan),
                                       kVelloSlot_LargePathtagScanFirstPassOutput);

            builder.appendStep(&fPathtagReduce2, to_wg_size(dispatchInfo.path_reduce2));
            builder.appendStep(&fPathtagScan1, to_wg_size(dispatchInfo.path_scan1));
            builder.appendStep(&fPathtagScanLarge, to_wg_size(dispatchInfo.path_scan));
        } else {
            builder.appendStep(&fPathtagScanSmall, to_wg_size(dispatchInfo.path_scan));
        }
    }

    // bbox_clear
    ScratchBuffer pathBboxes = bufMgr->getScratchStorage(bufferSizes.path_bboxes);
    builder.assignSharedBuffer(new_scratch_slice(pathBboxes), kVelloSlot_PathBBoxes);
    builder.appendStep(&fBboxClear, to_wg_size(dispatchInfo.bbox_clear));

    // flatten
    ScratchBuffer lines = bufMgr->getScratchStorage(lines_size);
    builder.assignSharedBuffer(new_scratch_slice(lines), kVelloSlot_Lines);
    builder.appendStep(&fFlatten, to_wg_size(dispatchInfo.flatten));

    tagmonoids.returnToPool();

    // draw_reduce
    ScratchBuffer drawReduced = bufMgr->getScratchStorage(bufferSizes.draw_reduced);
    builder.assignSharedBuffer(new_scratch_slice(drawReduced), kVelloSlot_DrawReduceOutput);
    builder.appendStep(&fDrawReduce, to_wg_size(dispatchInfo.draw_reduce));

    // draw_leaf
    ScratchBuffer drawMonoids = bufMgr->getScratchStorage(bufferSizes.draw_monoids);
    ScratchBuffer binData = bufMgr->getScratchStorage(bin_data_size);
    // A clip input buffer must still get bound even if the encoding doesn't contain any clips
    ScratchBuffer clipInput = bufMgr->getScratchStorage(bufferSizes.clip_inps);
    builder.assignSharedBuffer(new_scratch_slice(drawMonoids), kVelloSlot_DrawMonoid);
    builder.assignSharedBuffer(new_scratch_slice(binData), kVelloSlot_InfoBinData);
    builder.assignSharedBuffer(new_scratch_slice(clipInput), kVelloSlot_ClipInput);
    builder.appendStep(&fDrawLeaf, to_wg_size(dispatchInfo.draw_leaf));

    drawReduced.returnToPool();

    // clip_reduce, clip_leaf
    // The clip bbox buffer is always an input to the binning stage, even when the encoding doesn't
    // contain any clips
    ScratchBuffer clipBboxes = bufMgr->getScratchStorage(bufferSizes.clip_bboxes);
    builder.assignSharedBuffer(new_scratch_slice(clipBboxes), kVelloSlot_ClipBBoxes);
    WorkgroupSize clipReduceWgCount = to_wg_size(dispatchInfo.clip_reduce);
    WorkgroupSize clipLeafWgCount = to_wg_size(dispatchInfo.clip_leaf);
    bool doClipReduce = clipReduceWgCount.scalarSize() > 0u;
    bool doClipLeaf = clipLeafWgCount.scalarSize() > 0u;
    if (doClipReduce || doClipLeaf) {
        ScratchBuffer clipBic = bufMgr->getScratchStorage(bufferSizes.clip_bics);
        ScratchBuffer clipEls = bufMgr->getScratchStorage(bufferSizes.clip_els);
        builder.assignSharedBuffer(new_scratch_slice(clipBic), kVelloSlot_ClipBicyclic);
        builder.assignSharedBuffer(new_scratch_slice(clipEls), kVelloSlot_ClipElement);
        if (doClipReduce) {
            builder.appendStep(&fClipReduce, clipReduceWgCount);
        }
        if (doClipLeaf) {
            builder.appendStep(&fClipLeaf, clipLeafWgCount);
        }
    }

    clipInput.returnToPool();

    // binning
    ScratchBuffer drawBboxes = bufMgr->getScratchStorage(bufferSizes.draw_bboxes);
    ScratchBuffer binHeaders = bufMgr->getScratchStorage(bufferSizes.bin_headers);
    builder.assignSharedBuffer(new_scratch_slice(drawBboxes), kVelloSlot_DrawBBoxes);
    builder.assignSharedBuffer(new_scratch_slice(binHeaders), kVelloSlot_BinHeader);
    builder.appendStep(&fBinning, to_wg_size(dispatchInfo.binning));

    pathBboxes.returnToPool();
    clipBboxes.returnToPool();

    // tile_alloc
    ScratchBuffer paths = bufMgr->getScratchStorage(bufferSizes.paths);
    ScratchBuffer tiles = bufMgr->getScratchStorage(tiles_size);
    builder.assignSharedBuffer(new_scratch_slice(paths), kVelloSlot_Path);
    builder.assignSharedBuffer(new_scratch_slice(tiles), kVelloSlot_Tile);
    builder.appendStep(&fTileAlloc, to_wg_size(dispatchInfo.tile_alloc));

    drawBboxes.returnToPool();

    // path_count_setup
    auto indirectCountBuffer = new_indirect_slice(bufMgr, bufferSizes.indirect_count);
    builder.assignSharedBuffer(indirectCountBuffer, kVelloSlot_IndirectCount);
    builder.appendStep(&fPathCountSetup, to_wg_size(dispatchInfo.path_count_setup));

    // Rasterization stage scratch buffers.
    ScratchBuffer seg_counts = bufMgr->getScratchStorage(seg_counts_size);
    ScratchBuffer segments = bufMgr->getScratchStorage(segments_size);
    ScratchBuffer ptcl = bufMgr->getScratchStorage(ptcl_size);

    // path_count
    builder.assignSharedBuffer(new_scratch_slice(seg_counts), kVelloSlot_SegmentCounts);
    builder.appendStepIndirect(&fPathCount, indirectCountBuffer);

    // backdrop
    builder.appendStep(&fBackdrop, to_wg_size(dispatchInfo.backdrop));

    // coarse
    builder.assignSharedBuffer(new_scratch_slice(ptcl), kVelloSlot_PTCL);
    builder.appendStep(&fCoarse, to_wg_size(dispatchInfo.coarse));

    // path_tiling_setup
    builder.appendStep(&fPathTilingSetup, to_wg_size(dispatchInfo.path_tiling_setup));

    // path_tiling
    builder.assignSharedBuffer(new_scratch_slice(segments), kVelloSlot_Segments);
    builder.appendStepIndirect(&fPathTiling, indirectCountBuffer);

    // fine
    builder.assignSharedTexture(std::move(target), kVelloSlot_OutputImage);
    const ComputeStep* fineVariant = nullptr;
    switch (params.fAaConfig) {
        case VelloAaConfig::kAnalyticArea:
            fineVariant = fFineArea.get();
            break;
        case VelloAaConfig::kMSAA16:
            fineVariant = fFineMsaa16.get();
            break;
        case VelloAaConfig::kMSAA8:
            fineVariant = fFineMsaa8.get();
            break;
    }
    SkASSERT(fineVariant != nullptr);
    builder.appendStep(fineVariant, to_wg_size(dispatchInfo.fine));

    return builder.finalize();
}

}  // namespace skgpu::graphite
