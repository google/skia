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
std::pair<Layer*, BindingList*> DrawListLayer::searchBackwards(
        const RenderStep* step,
        const LayerKey& key,
        SkEnumBitMask<BoundsFlags> testMask,
        const DrawParams* drawParams,
        CompressedPaintersOrder stop) {
    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;
    BindingList* forwardMerge = nullptr;

    // Forward merging attempts to pull an earlier, compatible draw out of the current layer and
    // push it into a newly created layer to improve pipeline/texture batching.
    //
    // 1. Draw Type Restrictions (Single Renderstep & No Depth-Only):
    //    Forward merging is strictly limited to single-renderstep shading draws. We explicitly
    //    forbid depth-only draws (which pass `false` for `canForwardMerge`), and the single-step
    //    requirement inherently excludes stencil draws. If we allowed multi-step renderers to
    //    forward merge, we would risk pulling a parent renderstep forward and over its
    //    already-inserted child.
    //
    // 2. Directional & Spatial Validity:
    //    Because we evaluate bindings backwards (tail to head), any binding matches found prior to
    //    intersecting a draw are executed *after* that intersecting draw. Furthermore, because
    //    standard shading draws within the same layer are guaranteed by the `test()` logic to be
    //    mutually disjoint, the matched draw does not overlap with any of the later bindings we
    //    evaluated and skipped. Therefore, it is visually safe to extract this disjoint match and
    //    defer its execution to a new, subsequent layer without violating the Painter's Algorithm.
    //
    // 3. The Tail-Only Restriction:
    //    We strictly limit forward merging to the *tail* of the layer list. If we allowed forward
    //    merging from a middle layer, we would be forced to insert the newly generated target layer
    //    into the middle of the list. This would break the structural invariant that
    //    `Layer::fOrder` strictly increases with the physical list order; this invariant necessary
    //    to ensure that a draw is inserted after *ALL* depth-only clip draws that affect it.
    bool canForwardMerge = key.isSimpleShading();

    Layer* current = fLayers.tail();
    for (int limit = kMaxSearchLimit; limit > 0 && current; --limit) {
        auto [result, match] = current->test(drawParams->drawBounds(), key, testMask);

        if (result & BoundsTestResult::kAllowedInLayer) {
            // Allowed in the layer, so remember it. In complex scenes, we want to search deeper
            // in the layer list than just the first compatible overlap we encounter. Stopping early
            // reduces search time but fragments batching. Inserting early blocks subsequent draws
            // from reaching those denser, later candidates (particularly when this is a clip draw
            // as that propagates into the stop layer for subsequent draws).
            targetLayer = current;
            targetMatch = match;
        } else if (match && canForwardMerge) {
            SkASSERT(result == BoundsTestResult::kBlocked &&
                     !SkToBool(key.fFlags & BoundsFlags::kMustBeDisjoint) &&
                     current == fLayers.tail());
            forwardMerge = match;
        }

        if (!SkToBool(result & BoundsTestResult::kAllowedBeforeLayer) || current->fOrder == stop) {
            break;
        } else {
            current = current->fPrev;
            canForwardMerge = false;

            // To support deeper searches while mitigating search time, if we found a matching
            // BindingList then we penalize the remaining search limit by subtracting half of
            // kMaxSearchLimit. Ultimately this is an imprecise heuristic. In an ideal world, we
            // would maximize batching by exhaustively searching to the end of the list, but that
            // would degrade insertion performance to O(n^2).
            if (match) {
                limit -= kMaxSearchLimit >> 1;
            }
        }
    }

    SkASSERT(!targetLayer || targetLayer->fOrder >= stop);

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        if (forwardMerge) {
            SkASSERT(current);
            SkASSERT(current == fLayers.tail());
            current->fBindings.remove(forwardMerge);
            targetLayer->fBindings.addToHead(forwardMerge);
            targetMatch = forwardMerge;
        }
        fLayers.addToTail(targetLayer);
    }

    if (!targetMatch) {
        targetMatch = targetLayer->addNewBinding(&fStorage, nullptr, key, step);
    } else {
        SkASSERT(targetLayer->fBindings.isInList(targetMatch));
    }

    return {targetLayer, targetMatch};
}

BindingList* DrawListLayer::findOrCreateBindingInLayer(Layer* layer,
                                                       BindingList* parent,
                                                       const RenderStep* step,
                                                       const LayerKey& key) {
    // If we're recording a new step in the layer, there better have been a draw that searched
    // backwards for the layer first!
    SkASSERT(layer);
    // If we have a parent step's BindingList to insert before, it must be in `layer`.
    SkASSERT(!parent || layer->fBindings.isInList(parent));

    BindingList* targetMatch = nullptr;

    // If we don't have a parent, search through all bindings of the layer as this is the first time
    // through the layer. If we do have a parent, search through the preceding bindings (exclusive).
    if (!parent || parent->fPrev) {
        targetMatch = layer->searchBinding(key, parent ? parent->fPrev : nullptr);
    } // else there are no preceding bindings so we know we have to add a new one

    if (!targetMatch) {
        targetMatch = layer->addNewBinding(&fStorage, parent, key, step);
    }
    return targetMatch;
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

    // `testMask` limits what we test against when searching backwards, which is based on the
    // Renderer's aggregate requirements so that the layer we find will be valid for all steps. This
    // is particularly important for stencil-based renderers, which consist of a non-shading
    // "producer" step, which writes into the stencil buffer, and shading "consumer" render steps
    // which test against the stencil mask and clear the buffer afterwards. This guarantees
    // atomicity within a single layer, where the last step finds a safe layer and all earlier steps
    // are explicitly inserted before that. This minimizes pipeline switches as rendering can
    // proceed through the steps in bulk.
    SkEnumBitMask<BoundsFlags> testMask;
    if (SkToBool(renderer->depthStencilFlags() & DepthStencilFlags::kStencil)) {
        testMask |= BoundsFlags::kStencil;
    }
    // Draws that blend must respect painter's order, and clipping depth-only draws cannot be
    // ordered in front of shading draws.
    const bool isDepthOnly = !paintID.isValid();
    const bool dependsOnDst = SkToBool(dstUsage & DstUsage::kDependsOnDst);
    if (dependsOnDst || isDepthOnly) {
        testMask |= BoundsFlags::kColor;
    }

    // In simple situations, we can allow overlaps within a BindingList and let GPU rasterization
    // resolve the rendering order automatically. This does not apply if barriers are required,
    // and it does not apply when the Renderer has multiple steps (must keep the sets of draws in
    // each step disjoint so there isn't interference).
    SkEnumBitMask<BoundsFlags> baseLayerMask = BoundsFlags::kNone;
    if (barrierBeforeDraws != BarrierType::kNone || renderer->numRenderSteps() > 1) {
        baseLayerMask |= BoundsFlags::kMustBeDisjoint;
    }

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

    Layer* insertionLayer = nullptr;
    BindingList* lastStepBinding = nullptr;
    // If we're an easy draw, jump to the latestInsertion layer since we don't have to test
    if (testMask == BoundsFlags::kNone && baseLayerMask == BoundsFlags::kNone) {
        insertionLayer = latestInsertion ? latestInsertion.fLayer : fLayers.head();
    }

    fRenderStepCount += renderer->numRenderSteps();
    for (int stepIndex = renderer->numRenderSteps() - 1; stepIndex >= 0; --stepIndex) {
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


        // `layerMask` defines what this draw will block in new draws from going backwards. This is
        // per-step so that stencil-only draws can be grouped between shading and clip draws.
        SkEnumBitMask<BoundsFlags> layerMask = baseLayerMask;
        if (step->depthStencilFlags() & DepthStencilFlags::kStencil) {
            layerMask |= BoundsFlags::kStencil;
        }
        if (step->performsShading() && paintID.isValid()) {
            // NOTE: This is not dependsOnDst because it represents what is written by the draw,
            // not what might be read for blending the draw.
            layerMask |= BoundsFlags::kColor;
        }

        LayerKey key{pipelineIndex,
                     textureBindingIndex,
                     fStorageBufferSupport ? UniformDataCache::kInvalidIndex : uniformIndex,
                     layerMask};

        if (!insertionLayer) {
            // Since we don't have a layer yet, search from the most recent layer back.
            CompressedPaintersOrder stop = latestInsertion ? latestInsertion.fLayer->fOrder
                                                           : DrawOrder::kNoIntersection;
            std::tie(insertionLayer, lastStepBinding) = this->searchBackwards(step,
                                                                              key,
                                                                              testMask,
                                                                              drawParams,
                                                                              stop);
        } else {
            // Put the earlier steps in the same layer (valid because we used BoundsFlags for the
            // whole Renderer).
            lastStepBinding = this->findOrCreateBindingInLayer(insertionLayer,
                                                               lastStepBinding,
                                                               step,
                                                               key);
        }

        SkASSERT(lastStepBinding);
        lastStepBinding->addDraw(fStorage.make<Draw>(drawParams, uniformIndex),
                                     /*backToFront=*/dependsOnDst);

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

    return {drawParams, {insertionLayer, lastStepBinding}};
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
            SKIA_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
            this->reset(LoadOp::kLoad);
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
