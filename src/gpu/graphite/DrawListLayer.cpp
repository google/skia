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

std::pair<Layer*, BindingList*> DrawListLayer::searchBackwards(
        const RenderStep* step,
        const LayerKey& key,
        SkEnumBitMask<BoundsFlags> testMask,
        const DrawParams* drawParams,
        CompressedPaintersOrder stop) {
    // CPU performance is sensitive to increasing this value. Searching for longer *can* reduce the
    // draw count and pipeline change count
    static constexpr int kMaxSearchLimit = 8;

    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;
    BindingList* forwardMerge = nullptr;

    Rect::ComplementRect drawBounds{drawParams->drawBounds()};

    Layer* current = fLayers.tail();
    for (int limit = kMaxSearchLimit; limit > 0 && current; --limit) {
        auto [result, match] = current->test(drawBounds, key, testMask);

        if (result & BoundsTestResult::kAllowedInLayer) {
            // Allowed in the layer, so remember it. In complex scenes, we want to search deeper
            // in the layer list than just the first compatible overlap we encounter. Stopping early
            // reduces search time but fragments batching. Inserting early blocks subsequent draws
            // from reaching those denser, later candidates (particularly when this is a clip draw
            // as that propagates into the stop layer for subsequent draws).
            targetLayer = current;
            targetMatch = match;
        } else if (match) {
            // Save this for after we create a new targetLayer, at which point this will move from
            // current to the new layer and become targetMatch. Given the asserted conditions
            // below, this case will always exit the loop.
            SkASSERT(result == BoundsTestResult::kBlocked &&
                     !SkToBool(key.fFlags & BoundsFlags::kMustBeDisjoint) &&
                     current == fLayers.tail() &&
                     !targetLayer);
            forwardMerge = match;
        }

        if (!SkToBool(result & BoundsTestResult::kAllowedBeforeLayer) || current->fOrder == stop) {
            break;
        } else {
            current = current->fPrev;

            // To support deeper searches while mitigating search time, if we found a matching
            // BindingList then we penalize the remaining search limit halving it. Ultimately this
            // is an imprecise heuristic. In an ideal world, we would maximize batching by
            // exhaustively searching to the end of the list, but that would degrade insertion
            // performance to O(n^2).
            if (match) {
                limit /= 2;
            }
        }
    }

    SkASSERT(!targetLayer || targetLayer->fOrder >= stop);

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        if (forwardMerge) {
            SkASSERT(fLayers.tail()->fBindings.isInList(forwardMerge));
            SkASSERT(key.fFlags & BoundsFlags::kColor); // Moving depth draws would break clipping
            fLayers.tail()->fBindings.remove(forwardMerge);
            targetLayer->fBindings.addToHead(forwardMerge);
            targetMatch = forwardMerge;
        }
        fLayers.addToTail(targetLayer);
    }

    if (!targetMatch || !targetMatch->fKey.isEqual(key)) {
        // If targetMatch is just a pipeline match, we can insert right before it because such a
        // match is only returned when the new draw can be ordered in front of it.
        targetMatch = targetLayer->addNewBinding(&fStorage, targetMatch, key, step);
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
    // This is handled automatically by searchBinding's `parent` handling; when there are no
    // preceding bindings (e.g. parent && !parent->fPrev), `match` will just be null.
    BindingList* match = layer->searchBinding(key, parent);
    if (match) {
        if (match->fKey.isEqual(key)) {
            targetMatch = match;
        } else {
            // NOTE: Treat any pipeline match as the new parent that a new binding list will be
            // inserted before. Since the search started from the original parent (exclusive),
            // any found pipeline match will still be before that parent.
            parent = match;
        }
    }

    if (!targetMatch) {
        targetMatch = layer->addNewBinding(&fStorage, parent, key, step);
    }
    return targetMatch;
}

// Layer has dual purpose here:
//  1) (Producer) If recording a depth-only draw, the returned Layer* pointer is remembered as
//     the earliest possible layer that a later clipped draw can be added to. This is stored on the
//     ClipStack::Element that produced the depth-only draw.
//  2) (Consumer) If recording a clipped draw, the pointer is the latest layer inserted into across
//     *all depth only draws* which affect this draw. If the draw has no other bounds dependencies,
//     this represents the Layer that it can be directly added to.
std::pair<DrawParams*, Layer*> DrawListLayer::recordDraw(const Renderer* renderer,
                                                         const Transform& localToDevice,
                                                         const Geometry& geometry,
                                                         const Clip& clip,
                                                         DrawOrder ordering,
                                                         UniquePaintParamsID paintID,
                                                         SkEnumBitMask<DstUsage> dstUsage,
                                                         BarrierType barrierBeforeDraws,
                                                         PipelineDataGatherer* gatherer,
                                                         const StrokeStyle* stroke,
                                                         Layer* lastInsertion) {
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
        insertionLayer = lastInsertion ? lastInsertion : fLayers.head();
    }

    fRenderStepCount += renderer->numRenderSteps();
    for (int stepIndex = renderer->numRenderSteps() - 1; stepIndex >= 0; --stepIndex) {
        const RenderStep* const step = renderer->steps()[stepIndex];
        const bool performsShading = step->performsShading() && paintID.isValid();

        gatherer->markOffsetAndAlign(performsShading, step->uniformAlignment());

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                {step->renderStepID(),
                 performsShading ? paintID : UniquePaintParamsID::Invalid()});

        step->writeUniformsAndTextures(*drawParams, gatherer);

        auto [combinedUniforms, combinedTextures] =
                gatherer->endCombinedData(performsShading);

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
            CompressedPaintersOrder stop = lastInsertion ? lastInsertion->fOrder
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

    return {drawParams, insertionLayer};
}

std::unique_ptr<DrawPass> DrawListLayer::snapDrawPass(Recorder* recorder,
                                                      StorageContext* storageContext,
                                                      sk_sp<TextureProxy> target,
                                                      const SkImageInfo& targetInfo,
                                                      const DstReadStrategy dstReadStrategy) {
    TRACE_EVENT1_ALWAYS("skia.gpu", TRACE_FUNC, "draw count", fDrawCount);

    std::unique_ptr<DrawPass> drawPass(new DrawPass(target,
                                                    {fLoadOp, StoreOp::kStore},
                                                    fClearColor));
    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);

    UniformTracker uniformTracker(fStorageBufferSupport);
    TextureTracker textureBindingTracker(&fTextureDataCache);

    const bool rebindTexturesOnPipelineChange = dstReadStrategy == DstReadStrategy::kTextureCopy;

    if (fStorageBufferSupport) {
        SkASSERT(storageContext);
        storageContext->finalizePrecachedStorageData();
    }

    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    const SkIRect targetBounds = SkIRect::MakeSize(targetInfo.dimensions());
    SkIRect lastScissor = targetBounds;

    SkASSERT(drawPass->fTarget->isFullyLazy() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    // Accumulate rough pixel area touched by each pipeline
    drawPass->fPipelineDrawAreas.push_back_n(fPipelineCache.count(), 0.f);

    auto recordDraw = [&](const LayerKey& key,
                          const RenderStep* renderStep,
                          const Draw* draw,
                          bool bindingsAreInvariant,
                          bool startOfLayer) -> const Draw* {
        SkASSERT(renderStep && draw);
        const DrawParams& drawParams = *draw->fDrawParams;

        bool pipelineChange = false;
        bool textureBindingsChange = false;

        if (!bindingsAreInvariant) {
            pipelineChange = key.fPipelineIndex != lastPipeline;

            textureBindingsChange =
                    textureBindingTracker.setCurrentTextureBindings(key.fTextureIndex) ||
                    (rebindTexturesOnPipelineChange && pipelineChange &&
                     key.fTextureIndex != TextureDataCache::kInvalidIndex);
        }

        // Uniforms are binding invariant when SSBOs are disabled, but it's simpler to just let
        // `uniformBindingChange` eval to false more often. The uniform index must come from the
        // Draw to get the right value when SSBOs are enabled.
        bool uniformBindingChange =
                uniformTracker.writeUniforms(fUniformDataCache, bufferMgr, draw->fUniformIndex);

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
        } else if (drawParams.barrierBeforeDraws() != BarrierType::kNone && startOfLayer) {
            // Taking this branch means there were no state or pipeline changes between old layer
            // and this layer's first draw. This only happens if the draws overlap, so flush the
            // drawWriter since the draw requires a barrier.
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

        // Either stop early on failure, or advance to the next Draw
        return bufferMgr->hasMappingFailed() ? nullptr : draw->fNext;
    };

    for (Layer* layer : fLayers) {
        for (const BindingList* list : layer->fBindings) {
            SkASSERT(!list->fDraws.isEmpty());

            // The first draw of the BindingList will be changing bindings
            const Draw* current = recordDraw(list->fKey, list->fStep, list->fDraws.head(),
                                             /*bindingsAreInvariant=*/false,
                                             /*startOfLayer=*/!list->fPrev);
            while (current) {
                // Any remaining draws can skip checking for pipeline/texture binding changes.
                current = recordDraw(list->fKey, list->fStep, current,
                                     /*bindingsAreInvariant=*/true,
                                     /*startOfLayer=*/false);
            }
        }
    }

    drawWriter.flush();

    if (fStorageBufferSupport) {
        SkASSERT(storageContext);
        drawPass->fStorageBufferInfo = storageContext->finalize(bufferMgr);
        if (!storageContext->isEmpty() && !drawPass->fStorageBufferInfo) SK_UNLIKELY {
            SKIA_LOG_W("Failed to write Storage Data for Draw pass, dropping!");
            this->reset(LoadOp::kLoad);
            return nullptr;
        }
    }

    drawPass->fBounds = fPassBounds.roundOut().asSkIRect();
    drawPass->fPipelineDescs = fPipelineCache.detach();
    drawPass->fSampledTextures = fTextureDataCache.detachTextures();

    TRACE_EVENT_INSTANT2_ALWAYS("skia.gpu",
                                "DrawPass Stats",
                                TRACE_EVENT_SCOPE_THREAD,
                                "# commands", drawPass->fCommandList.count(),
                                "# textures", drawPass->fSampledTextures.size());

    this->reset(LoadOp::kLoad);

    if (bufferMgr->hasMappingFailed()) {
        SKIA_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
        return nullptr;
    } else {
        return drawPass;
    }
}

}  // namespace skgpu::graphite
