/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawListLayer.h"

#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

void DrawListLayer::reset(LoadOp loadOp, SkColor4f color) {
    DrawListBase::reset(loadOp, color);

    fStorage.reset();
    fLayers.reset();
    fDrawCount = 0;
    fOrderCounter = CompressedPaintersOrder::First();
}

// Draws affected by depth only draws we call "clipped draws."
//
// Clipped draws must come *after* all depth only draws that affect them, and they must come *after*
// any preceding draws from the same renderstep. To accomodate this:
//      1) When recording the depth only draws, a pointer marking the latest layer inserted into is
//         passed between each draw. If a later draws inserts after an earlier draw, the pointer is
//         overwritten. This ensures that the pointer is always the *latest* layer.
//      2) How the pointer is used depends on the property of clipped draw:
//         - Clipped draws which do not dependOnDst use this as the start of the traversal, then
//           proceed FORWARDS until finding a suitable layer.
//         - Clipped draws which do dependOnDst must stop when encountering any shading intersecting
//           draw. Thus, forwards traversal becomes impractical because the draw must exhaustively
//           search layers to the tail to ensure that there are no intersections. Instead,
//           these draws must take the normal BACKWARDS traversal.
//      3) Each clipped draw updates the starting layer to the layer that it inserted into. Because
//         the stopLayer is treated exclusively, a sucessor renderstep stops its traversal before
//         the stopLayer, thus preserving the relative ordering between the draws. (Note: will be
//         changed in future CL, so kind of stub comment)
void DrawListLayer::recordBackwards(int stepIndex,
                                    bool isStencil,
                                    bool isDepthOnly,
                                    bool dependsOnDst,
                                    bool requiresBarrier,
                                    const RenderStep* step,
                                    const UniformDataCache::Index& uniformIndex,
                                    const LayerKey& key,
                                    const DrawParams* drawParams,
                                    const Insertion& stop,
                                    Insertion* capture,
                                    bool canForwardMerge) {
    Layer* current = nullptr;
    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;
    BindingList* forwardMerge = nullptr;
    // If we're an easy draw (!kIsStencil and !dependsOnDst), try the head first.
    if (!isStencil && !dependsOnDst) {
        // A valid stopLayer will never be null, because the depth draw will always return the layer
        // it drew into.
        targetLayer = stop.fLayer ? stop.fLayer : fLayers.head();
        if (targetLayer) {
            targetMatch = targetLayer->searchBinding</*kForwards=*/false>(
                    key, stop.fList, !fStorageBufferSupport);
        }
    } else {
        current = fLayers.tail();
        int32_t limit = kMaxSearchLimit;
        auto processLayer = [&](BindingList* boundary) -> bool {
            auto [overlapType, match] =
                    isStencil
                            ? current->test</*kIsStencil=*/true, /*kForwards=*/false>(
                                      isDepthOnly,
                                      drawParams->drawBounds(),
                                      key,
                                      requiresBarrier,
                                      boundary,
                                      !fStorageBufferSupport)
                            : current->test</*kIsStencil=*/false, /*kForwards=*/false>(
                                      isDepthOnly,
                                      drawParams->drawBounds(),
                                      key,
                                      requiresBarrier,
                                      boundary,
                                      !fStorageBufferSupport);

            if (overlapType == BoundsTest::kIncompatibleOverlap) {
                // If we need to read the dst, we cannot go earlier than this layer.
                if (dependsOnDst) {
                    // Forward merging attempts to pull an earlier, compatible draw out of the
                    // current layer and push it into a newly created layer to improve
                    // pipeline/texture batching.
                    //
                    // 1. Draw Type Restrictions (Single Renderstep & No Depth-Only):
                    //    Forward merging is strictly limited to single-renderstep shading draws. We
                    //    explicitly forbid depth-only draws (which pass `false` for
                    //    `canForwardMerge`), and the single-step requirement inherently excludes
                    //    stencil draws. If we allowed multi-step renderers to forward merge, we
                    //    would risk pulling a parent renderstep forward and over its
                    //    already-inserted child.
                    //
                    // 2. Directional & Spatial Validity:
                    //    Because we evaluate bindings backwards (tail to head), any binding matches
                    //    prior to intersection are necessarily execute *after* that intersecting
                    //    draw. Furthermore, because standard shading draws within the same layer
                    //    are guaranteed by the `test()` logic to be mutually disjoint, the matched
                    //    draw does not overlap with any of the later bindings we evaluated and
                    //    skipped. Therefore, it is visually safe to extract this disjoint match and
                    //    defer its execution to a new, subsequent layer without violating the
                    //    Painter's Algorithm.
                    //
                    // 3. The Tail-Only Restriction: We strictly limit forward merging to the *tail*
                    //    of the layer list. If we allowed forward merging from a middle layer, we
                    //    would be forced to insert the newly generated target layer into the middle
                    //    of the list. This would break the structural invariant that
                    //    `Layer::fOrder` strictly increases with the physical list order.
                    //
                    // 4. The Clip State Complication (Drawn/Undrawn Mix):
                    //    While depth-only draws never forward merge themselves, allowing forward
                    //    merging to middle-insert layers risks clip stack ordering issues. The clip
                    //    stack relies on the `CompressedPaintersOrder` invariant when processing a
                    //    mix of drawn and undrawn elements. `updateClipStateForDraw` uses
                    //    `Insertion::operator>` (which compares `fOrder`) to find the latest
                    //    insertion across all depth-only clips affecting a draw. If a layer were
                    //    middle-inserted via `addAfter`, assigning it a valid `fOrder` is
                    //    intractable:
                    //      - Case A (New Highest Order): If we give the middle layer the next
                    //        highest integer (e.g., L1(1) -> L_mid(3) -> L2(2)), the `max()`
                    //        calculation incorrectly flags `L_mid` as the absolute latest boundary.
                    //        A draw depending on a clip in `L2` will incorrectly take `L_mid` as
                    //        its stop layer and bypass its actual stop layer `L2`.
                    //      - Case B (Duplicate Order): If we duplicate the order to avoid Case A
                    //        (e.g., L1(1) -> L2(2) -> L2b(2)), the tie-breaker math breaks. If Clip
                    //        A inserts into `L2` and Clip B inserts into `L2b`, `max(A, B)` cannot
                    //        distinguish them because `2 > 2` is false. Depending on iteration
                    //        order, it may incorrectly return `L2` as the boundary, causing the
                    //        clipped draw to execute before Clip B's mask is rendered. Restricting
                    //        forward merges to the tail guarantees our assigned ordering is always
                    //        valid.
                    if (match && current == fLayers.tail() && canForwardMerge) {
                        if (current->fBindings.head() != current->fBindings.tail() &&
                            (!requiresBarrier ||
                             !match->fBounds.intersects(drawParams->drawBounds()))) {
                            forwardMerge = match;
                            targetMatch = forwardMerge;
                        }
                    }
                    return true;
                }
                // If !dependsOnDst, simply keep searching backwards. Since we guarantee stencil
                // atomicity to a layer, a stencil intersection can still safely bypass this layer,
                // because an insertion downstream cannot disrupt this stencil region.
                return false;
            } else {
                // Found a valid layer (Compatible or Disjoint)
                targetLayer = current;
                targetMatch = match;

                // In stencil-heavy scenes, we want to search deeper into the list than the first
                // compatible overlap we encounter. An earlier match likely contains fewer draws and
                // less draw coverage, while a later match is likely denser. Stopping at the first
                // match minimizes search time but fragments batching. Inserting early carries a
                // dual penalty: it 1) blocks subsequent draws from reaching those denser, later
                // candidates, and it 2) consumes draw space in the early layer that a succeeding
                // draw could have utilized.
                //
                // To mitigate this, we allow stencils to continue searching even after finding a
                // CompatibleOverlap, but we penalize the remaining search limit by subtracting half
                // of kMaxSearchLimit. This heuristic ensures:
                //  1) The search typically isn't blocked by the first compatible overlap, unless
                //     the match was found deep (over half the limit) into the search.
                //  2) If two matches are found, the search halts.
                //
                // Ultimately, this is an imprecise heuristic. In an ideal world, we would maximize
                // batching by exhaustively searching to the end of the list, but that would degrade
                // insertion performance to O(n^2).
                if (overlapType == BoundsTest::kCompatibleOverlap) {
                    if (isStencil) {
                        limit -= kMaxSearchLimit >> 1;
                    } else {
                        return true;
                    }
                }
                return false;
            }
            SkUNREACHABLE;
        };

        for (; limit >= 0; --limit) {
            if (current == stop.fLayer || processLayer(nullptr)) {
                break;
            }
            current = current->fPrev;
        }
        if (!targetMatch && current == stop.fLayer && current) {
            processLayer(stop.fList);
        }
    }

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        if (forwardMerge) {
            SkASSERT(current);
            SkASSERT(current == fLayers.tail());
            current->fBindings.remove(forwardMerge);
            targetLayer->fBindings.addToHead(forwardMerge);
            forwardMerge->fOrder = CompressedPaintersOrder::First();
        }
        fLayers.addToTail(targetLayer);
    }

    SkASSERT(targetLayer);
    Draw* draw = fStorage.make<Draw>(drawParams, uniformIndex);
    bool notParentList = targetMatch != stop.fList;
    // We pass `isDepthOnly` so that depth lists are prepended and shading lists are appended,
    // guaranteeing that depth draws always come before shading draws within a layer.
    BindingList* insertedList = targetLayer->add(
            isDepthOnly, &fStorage, targetMatch, key, draw, step, !dependsOnDst && notParentList);

    if (capture) {
        SkASSERT(insertedList);
        Insertion inserted = {targetLayer, insertedList};
        if (stepIndex > 0) {
            if (inserted > *capture) {
                *capture = inserted;
            }
        } else {
            *capture = inserted;
        }
    }
}

void DrawListLayer::recordForwards(int stepIndex,
                                   bool isStencil,
                                   bool dependsOnDst,
                                   bool requiresBarrier,
                                   const RenderStep* step,
                                   const UniformDataCache::Index& uniformIndex,
                                   const LayerKey& key,
                                   const DrawParams* drawParams,
                                   Insertion& start) {
    // If we're recording forwards, there better have been a draw that recorded backwards first!
    SkASSERT(start.fLayer);
    SkASSERT(start.fList);
    BindingList* targetMatch = nullptr;
    if (start.fList->fNext) {
        targetMatch = start.fLayer->searchBinding</*kForwards=*/true>(
                key, start.fList->fNext, !fStorageBufferSupport);
    }
    Draw* draw = fStorage.make<Draw>(drawParams, uniformIndex);
    // Because depth-only draws exclusively `recordBackwards`, it is safe to pass false for
    // `isDepthOnly`. This guarantees that new BindingLists append to the end of the layer and
    // draws after their parent.
    BindingList* insertedList = start.fLayer->add(
            /*isDepthOnly=*/false, &fStorage, targetMatch, key, draw, step, true);
    start.fList = insertedList;
}

// Layer has dual purpose here:
//  1) (Producer) If recording a depth only draw, the pointer is set to the *latest* layer inserted.
//  2) (Consumer) If recording a clipped draw, the pointer is the latest layer inserted into across
//     *all depth only draws* which affect this draw. Thus, it is the earliest possible layer that
//     the clipped draw could be inserted into, so it is used as the starting point for a *forward*
//     search.
std::pair<DrawParams*, Insertion> DrawListLayer::recordDraw(const Renderer* renderer,
                                                            const Transform& localToDevice,
                                                            const Geometry& geometry,
                                                            const Clip& clip,
                                                            DrawOrder ordering,
                                                            UniquePaintParamsID paintID,
                                                            SkEnumBitMask<DstUsage> dstUsage,
                                                            BarrierType barrierBeforeDraws,
                                                            PipelineDataGatherer* gatherer,
                                                            const StrokeStyle* stroke,
                                                            const Insertion& latestInsertion) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());

    // Stencil-based renderers consist of a non-shading "producer" step, which writes into the
    // stencil buffer, and shading "consumer" render steps which test against the stencil mask and
    // clear the buffer afterwards. Because both types of step modify the buffer, we treat all steps
    // as stenciling operations.
    //
    // Previously, interleaving one stencil sequence into another could corrupt the stencil buffer
    // state, forcing stencil steps to immediately halt upon any overlap to protect "stencil
    // regions." However, the current system guarantees stencil atomicity within a single layer. The
    // first step of a stencil sequence finds a safe layer, and all subsequent steps are explicitly
    // recorded forwards into that exact same layer.
    //
    // Because an entire stencil sequence is fully self-contained within a single layer, incoming
    // stencil draws that encounter an incompatible overlap during their backwards traversal do not
    // need to halt. They can safely bypass the intersecting layer and continue searching backwards.
    bool rendererIsStencil = SkToBool(renderer->depthStencilFlags() & DepthStencilFlags::kStencil);
    bool dependsOnDst = SkToBool(dstUsage & DstUsage::kDependsOnDst);
    bool requiresBarrier = barrierBeforeDraws != BarrierType::kNone;

    // Currently, the draw params are created once per record draw call, and the pointer is passed
    // to each draw call. This is storage effecient but will still introduce some pointer chasing,
    // because the params will likely no longer be on the same cache line for successor render
    // steps. We should test whether it is faster for each step to hold a copy of the params except
    // in the case of clipped draws (which must share a copy because they are mutated later).
    DrawParams* drawParams = fStorage.make<DrawParams>(this->deduplicateTransform(localToDevice),
                                                       geometry,
                                                       clip,
                                                       ordering,
                                                       stroke,
                                                       barrierBeforeDraws);

    Insertion stepInsertion = {nullptr, nullptr};
    fRenderStepCount += renderer->numRenderSteps();
    bool canForwardMerge = renderer->numRenderSteps() == 1;
    for (int stepIndex = 0; stepIndex < renderer->numRenderSteps(); ++stepIndex) {
        const RenderStep* const step = renderer->steps()[stepIndex];

        gatherer->markOffsetAndAlign(step->performsShading(), step->uniformAlignment());

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                {step->renderStepID(),
                 step->performsShading() ? paintID : UniquePaintParamsID::Invalid()});

        step->writeUniformsAndTextures(*drawParams, gatherer);

        auto [combinedUniforms, combinedTextures] =
                gatherer->endCombinedData(step->performsShading());

        UniformDataCache::Index uniformIndex = combinedUniforms
                                                       ? fUniformDataCache.insert(combinedUniforms)
                                                       : UniformDataCache::kInvalidIndex;
        TextureDataCache::Index textureBindingIndex =
                combinedTextures ? fTextureDataCache.insert(combinedTextures)
                                 : TextureDataCache::kInvalidIndex;

        if (paintID == UniquePaintParamsID::Invalid()) {  // Invalid ID implies depth only draw
            this->recordBackwards(
                    stepIndex,
                    rendererIsStencil,
                    /*isDepthOnly=*/true,
                    true,
                    requiresBarrier,
                    step,
                    uniformIndex,
                    LayerKey{pipelineIndex, textureBindingIndex, uniformIndex},
                    drawParams,
                    /*stop=*/{},
                    &stepInsertion,
                    /*canForwardMerge=*/false);
        } else {
            if (stepIndex == 0) {
                this->recordBackwards(
                        stepIndex,
                        rendererIsStencil,
                        /*isDepthOnly=*/false,
                        dependsOnDst,
                        requiresBarrier,
                        step,
                        uniformIndex,
                        LayerKey{pipelineIndex, textureBindingIndex, uniformIndex},
                        drawParams,
                        latestInsertion,
                        &stepInsertion,
                        canForwardMerge);
            } else {
                this->recordForwards(stepIndex,
                                     rendererIsStencil,
                                     false,
                                     requiresBarrier,
                                     step,
                                     uniformIndex,
                                     LayerKey{pipelineIndex, textureBindingIndex, uniformIndex},
                                     drawParams,
                                     stepInsertion);
            }
        }
        gatherer->rewindForRenderStep();
    }

    fDrawCount++;
    fPassBounds.join(clip.drawBounds());
    fRequiresMSAA |= renderer->requiresMSAA();
    fDepthStencilFlags |= renderer->depthStencilFlags();
    if (dstUsage & DstUsage::kDstReadRequired) {
        // For paints that read from the dst, update the bounds. It may later be determined that the
        // DstReadStrategy does not require them, but they are inexpensive to track.
        fDstReadBounds.join(clip.drawBounds());
    }

#if defined(SK_DEBUG)
    if (geometry.isCoverageMaskShape()) {
        fCoverageMaskShapeDrawCount++;
    }
#endif

    return {drawParams, stepInsertion};
}

std::unique_ptr<DrawPass> DrawListLayer::snapDrawPass(Recorder* recorder,
                                                      sk_sp<TextureProxy> target,
                                                      const SkImageInfo& targetInfo,
                                                      const DstReadStrategy dstReadStrategy) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", fDrawCount);

    std::unique_ptr<DrawPass> drawPass(new DrawPass(target,
                                                    {fLoadOp, StoreOp::kStore},
                                                    fClearColor,
                                                    recorder->priv().refFloatStorageManager()));
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);

    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    const SkIRect targetBounds = SkIRect::MakeSize(targetInfo.dimensions());
    SkIRect lastScissor = targetBounds;

    SkASSERT(drawPass->fTarget->isFullyLazy() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    UniformTracker uniformTracker(fStorageBufferSupport);

    const bool rebindTexturesOnPipelineChange = dstReadStrategy == DstReadStrategy::kTextureCopy;
    CompressedPaintersOrder priorDrawPaintOrder{};

    // Accumulate rough pixel area touched by each pipeline
    drawPass->fPipelineDrawAreas.push_back_n(fPipelineCache.count(), 0.f);

    TextureTracker textureBindingTracker(&fTextureDataCache);

    auto recordDraw = [&](const LayerKey& key,
                          const UniformDataCache::Index uniformIndex,
                          const RenderStep* renderStep,
                          const DrawParams& drawParams,
                          bool bindingsAreInvariant) -> bool {
        SkASSERT(renderStep);

        bool pipelineChange = false;
        bool textureBindingsChange = false;

        if (!bindingsAreInvariant) {
            pipelineChange = key.fPipelineIndex != lastPipeline;

            textureBindingsChange =
                    textureBindingTracker.setCurrentTextureBindings(key.fTextureIndex) ||
                    (rebindTexturesOnPipelineChange && pipelineChange &&
                     key.fTextureIndex != TextureDataCache::kInvalidIndex);
        }

        bool uniformBindingChange =
                uniformTracker.writeUniforms(fUniformDataCache, bufferMgr, uniformIndex);

        drawPass->fPipelineDrawAreas[key.fPipelineIndex] += drawParams.drawBounds().area();

        std::optional<SkIRect> newScissor =
                renderStep->getScissor(drawParams, lastScissor, targetBounds);

        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep->primitiveType(),
                                        renderStep->staticDataStride(),
                                        renderStep->appendDataStride(),
                                        renderStep->getRenderStateFlags(),
                                        drawParams.barrierBeforeDraws());
        } else if (uniformBindingChange || textureBindingsChange || newScissor.has_value()) {
            drawWriter.newDynamicState();
        } else if (drawParams.barrierBeforeDraws() != BarrierType::kNone) {
            drawWriter.flush();
        }

        if (pipelineChange) {
            drawPass->fCommandList.bindGraphicsPipeline(key.fPipelineIndex);
            lastPipeline = key.fPipelineIndex;
        }
        if (uniformBindingChange) {
            uniformTracker.bindUniforms(UniformSlot::kCombinedUniforms, &drawPass->fCommandList);
        }
        if (textureBindingsChange) {
            textureBindingTracker.bindTextures(&drawPass->fCommandList);
        }
        if (newScissor.has_value()) {
            drawPass->fCommandList.setScissor(*newScissor);
            lastScissor = *newScissor;
        }

        uint32_t uniformSsboIndex = fStorageBufferSupport ? uniformTracker.ssboIndex() : 0;
        renderStep->writeVertices(&drawWriter, drawParams, uniformSsboIndex);

        if (bufferMgr->hasMappingFailed()) {
            SKGPU_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
            return false;
        }

        priorDrawPaintOrder = drawParams.order().paintOrder();
        return true;
    };

    for (Layer* layer : fLayers) {
        for (const BindingList* list : layer->fBindings) {
            SkASSERT(!list->fDraws.isEmpty());
            const Draw* current = list->fDraws.head();

            if (!recordDraw(list->fKey,
                            current->fUniformIndex,
                            list->fStep,
                            *current->fDrawParams,
                            false)) {
                return nullptr;
            }
            current = current->fNext;

            while (current) {
                if (!recordDraw(list->fKey,
                                current->fUniformIndex,
                                list->fStep,
                                *current->fDrawParams,
                                true)) {
                    return nullptr;
                }
                current = current->fNext;
            }
        }
    }

    drawWriter.flush();

    drawPass->fBounds = fPassBounds.roundOut().asSkIRect();
    drawPass->fPipelineDescs = fPipelineCache.detach();
    drawPass->fSampledTextures = fTextureDataCache.detachTextures();

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    this->reset(LoadOp::kLoad);

    return drawPass;
}

}  // namespace skgpu::graphite
