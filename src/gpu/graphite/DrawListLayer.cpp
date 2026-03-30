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
//
// STENCIL stub comment: Removed by pilot draws in the future, so this will not be filled out.
template <bool kIsDepthOnly>
void DrawListLayer::recordBackwards(int stepIndex,
                                    bool isStencil,
                                    bool dependsOnDst,
                                    bool requiresBarrier,
                                    const RenderStep* step,
                                    const UniformDataCache::Index& uniformIndex,
                                    const LayerKey& key,
                                    const DrawParams* drawParams,
                                    const Insertion& stop,
                                    Insertion* capture,
                                    bool canForwardMerge) {
    // Child stencils get a fast path to their parent
    if (isStencil) {
        if (stepIndex > 0) {
            SkASSERT(fStencilLayer);
            SkASSERT(fStencilList);
            SkASSERT(fStencilWrapper);
            SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
            fStencilLayer->addStencil<kIsDepthOnly>(
                    &fStorage, fStencilWrapper, key, draw, step, &fStencilList);
            return;
        } else {
            fStencilList = nullptr;
        }
    }

    Layer* current = nullptr;
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;
    BindingWrapper* forwardMerge = nullptr;
    // If we're an easy draw (!kIsStencil and !dependsOnDst), try the head first.
    if (!isStencil && !dependsOnDst) {
        // A valid stopLayer will never be null, because the depth draw will always return the layer
        // it drew into.
        targetLayer = stop.fLayer ? stop.fLayer : fLayers.head();
        if (targetLayer) {
            targetMatch = targetLayer->searchBinding(key, stop.fWrapper);
        }
    } else {
        current = fLayers.tail();
        auto processLayer = [&](BindingWrapper* boundary) -> bool {
            auto result =
                    isStencil
                            ? current->test</*kIsStencil=*/true, kIsDepthOnly, /*kForwards=*/false>(
                                      drawParams->drawBounds(), key, requiresBarrier, boundary)
                            : current->test</*kIsStencil=*/false,
                                            kIsDepthOnly,
                                            /*kForwards=*/false>(
                                      drawParams->drawBounds(), key, requiresBarrier, boundary);

            if (result.first == BoundsTest::kIncompatibleOverlap) {
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
                    if (canForwardMerge && current == fLayers.tail()) {
                        if (result.second && current->fBindings.head() != current->fBindings.tail()
                            && (!requiresBarrier ||
                                !result.second->fBounds.intersects(drawParams->drawBounds()))) {
                            forwardMerge = result.second;
                            targetMatch = forwardMerge;
                        }
                    }
                    return true;
                } else {
                    // If !dependsOnDst, we just keep searching backwards.
                    return false;
                }
            } else {
                // Found a valid layer (Compatible or Disjoint)
                targetLayer = current;
                targetMatch = result.second;

                // If it was compatible, we expect a match. If disjoint, match is nullptr.
                return result.first == BoundsTest::kCompatibleOverlap;
            }
            SkUNREACHABLE;
        };

        // Check current here for safety?
        for (uint32_t limit = 0; limit < kMaxSearchLimit && current != stop.fLayer; ++limit) {
#if defined(__GNUC__) || defined(__clang__)
            __builtin_prefetch(current->fPrev);
#endif
            if (processLayer(nullptr)) {
                break;
            }
            current = current->fPrev;
        }
        if (current && current == stop.fLayer) {
            processLayer(stop.fWrapper);
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
    BindingWrapper* insertedWrapper;
    SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
    if (isStencil) {
        insertedWrapper = targetLayer->addStencil<kIsDepthOnly>(
                &fStorage, targetMatch, key, draw, step, &fStencilList);
        fStencilLayer = targetLayer;
        fStencilWrapper = targetMatch;
    } else {
        bool notStopLayer = targetLayer != stop.fLayer;
        insertedWrapper = targetLayer->add<kIsDepthOnly>(
                &fStorage, targetMatch, key, draw, step, !dependsOnDst && notStopLayer);
    }

    if constexpr (kIsDepthOnly) {
        SkASSERT(insertedWrapper);
        Insertion inserted = {targetLayer, insertedWrapper};
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
                                   const Insertion& start) {
    // Child stencils get a fast path to their parent
    if (isStencil) {
        if (stepIndex > 0) {
            SkASSERT(fStencilLayer);
            SkASSERT(fStencilList);
            SkASSERT(fStencilWrapper);
            SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
            fStencilLayer->addStencil(&fStorage, fStencilWrapper, key, draw, step, &fStencilList);
            return;
        } else {
            fStencilList = nullptr;
        }
    }

    Layer* current = const_cast<Layer*>(start.fLayer);
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;
    auto processLayer = [&](BindingWrapper* boundary) -> bool {
        auto result = isStencil ? current->test</*kIsStencil=*/true,
                                                /*kIsDepthOnly*/ false,
                                                /*kForwards=*/true>(
                                          drawParams->drawBounds(), key, requiresBarrier, boundary)
                                : current->test</*kIsStencil=*/false,
                                                /*kIsDepthOnly*/ false,
                                                /*kForwards=*/true>(
                                          drawParams->drawBounds(), key, requiresBarrier, boundary);
        if (result.first != BoundsTest::kIncompatibleOverlap) {
            targetLayer = current;
            targetMatch = result.second;
            return true;
        }
        return false;
    };

    if (current) {
        if (!processLayer(start.fWrapper)) {
            current = current->fNext;
            for (uint32_t limit = 0; limit < kMaxSearchLimit && current; ++limit) {
#if defined(__GNUC__) || defined(__clang__)
                __builtin_prefetch(current->fNext);
#endif
                if (processLayer(nullptr)) {
                    break;
                }
                current = current->fNext;
            }
        }
    }

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        if (start.fLayer) {
            fLayers.addAfter(targetLayer, start.fLayer);
        } else {
            fLayers.addToTail(targetLayer);
        }
    }

    SkASSERT(targetLayer);
    SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
    if (isStencil) {
        targetLayer->addStencil(&fStorage, targetMatch, key, draw, step, &fStencilList);
        fStencilLayer = targetLayer;
        fStencilWrapper = targetMatch;
    } else {
        bool notStartLayer = targetLayer != start.fLayer;
        targetLayer->add(&fStorage, targetMatch, key, draw, step, !dependsOnDst && notStartLayer);
    }
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

    // RENDERER not STEP because ATOMIC
    bool isStencil = SkToBool(renderer->depthStencilFlags() & DepthStencilFlags::kStencil);
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
            this->recordBackwards</*kIsDepthOnly=*/true>(
                    stepIndex,
                    isStencil,
                    true,
                    requiresBarrier,
                    step,
                    uniformIndex,
                    LayerKey{pipelineIndex, textureBindingIndex},
                    drawParams,
                    /*stop=*/{},
                    &stepInsertion,
                    false);
        } else {
            if (latestInsertion.fLayer && !dependsOnDst) {
                this->recordForwards(stepIndex,
                                     isStencil,
                                     false,
                                     requiresBarrier,
                                     step,
                                     uniformIndex,
                                     LayerKey{pipelineIndex, textureBindingIndex},
                                     drawParams,
                                     latestInsertion);
            } else {
                this->recordBackwards</*kIsDepthOnly=*/false>(
                        stepIndex,
                        isStencil,
                        dependsOnDst,
                        requiresBarrier,
                        step,
                        uniformIndex,
                        LayerKey{pipelineIndex, textureBindingIndex},
                        drawParams,
                        latestInsertion,
                        nullptr,
                        canForwardMerge);
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

    const Caps* caps = recorder->priv().caps();
    const bool useStorageBuffers = caps->storageBufferSupport();
    UniformTracker uniformTracker(useStorageBuffers);

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

        uint32_t uniformSsboIndex = useStorageBuffers ? uniformTracker.ssboIndex() : 0;
        renderStep->writeVertices(&drawWriter, drawParams, uniformSsboIndex);

        if (bufferMgr->hasMappingFailed()) {
            SKGPU_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
            return false;
        }

        priorDrawPaintOrder = drawParams.order().paintOrder();
        return true;
    };

    for (Layer* layer : fLayers) {
        for (const BindingWrapper* binding : layer->fBindings) {
            if (binding->fType == BindingListType::kSingle) {
                const auto* singleList = static_cast<const SingleDrawList*>(binding);
                SkASSERT(!singleList->fDraws.isEmpty());

                const SingleDraw* current = singleList->fDraws.head();
                if (!recordDraw(singleList->fKey,
                                current->fUniformIndex,
                                singleList->fStep,
                                *current->fDrawParams,
                                false)) {
                    return nullptr;
                }

                current = current->fNext;
                while (current) {
                    if (!recordDraw(singleList->fKey,
                                    current->fUniformIndex,
                                    singleList->fStep,
                                    *current->fDrawParams,
                                    true)) {
                        return nullptr;
                    }
                    current = current->fNext;
                }
            } else {
                const auto* stencilList = static_cast<const StencilDrawList*>(binding);
                for (const StencilDraws* sd : stencilList->fStencilDraws) {
                    SkASSERT(sd && !sd->fDraws.isEmpty());

                    const SingleDraw* first = sd->fDraws.head();
                    if (!recordDraw(sd->fKey,
                                    first->fUniformIndex,
                                    sd->fStep,
                                    *first->fDrawParams,
                                    false)) {
                        return nullptr;
                    }

                    for (const SingleDraw* current = first->fNext; current;
                         current = current->fNext) {
                        if (!recordDraw(sd->fKey,
                                        current->fUniformIndex,
                                        sd->fStep,
                                        *current->fDrawParams,
                                        true)) {
                            return nullptr;
                        }
                    }
                }
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
