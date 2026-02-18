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
// STENCIL stub comment
void DrawListLayer::recordBackwards(int stepIndex,
                                    bool isStencil,
                                    bool dependsOnDst,
                                    bool requiresBarrier,
                                    const RenderStep* step,
                                    const UniformDataCache::Index& uniformIndex,
                                    const LayerKey& key,
                                    const DrawParams* drawParams,
                                    const Layer* stopLayer) {
    // Child stencils get a fast path to their parent
    if (isStencil) {
        if (stepIndex > 0) {
            ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
            SkASSERT(fLastRecordedDraw && !fLastRecordedDraw->fChildDraw);
            fLastRecordedDraw->fChildDraw = draw;
            fLastRecordedDraw = draw;
            return;
        }
    }

    SkTInternalLList<Layer>::Iter iter;
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    // If we're an easy draw (!kIsStencil and !dependsOnDst), try the head first.
    Layer* current;
    if (!isStencil && !dependsOnDst) {
        targetLayer = stopLayer ? stopLayer->fNext : fLayers.head();
        if (targetLayer) {
            targetMatch = targetLayer->searchBinding(key);
        }
        current = const_cast<Layer*>(stopLayer);
    } else {
        current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    }

    int limit = kMaxSearchLimit;
    // When stopLayer == nullptr this is effectively while(current)
    while (current != stopLayer && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fPrev);
#endif
        auto result = current->test(drawParams->drawBounds(), key, isStencil, requiresBarrier);

        if (result.first == BoundsTest::kIncompatibleOverlap) {
            // If we need to read the dst, we cannot go earlier than this layer.
            if (dependsOnDst) {
                // TODO (thomsmit): Test performance of forward merging
                break;
            }
            // If !dependsOnDst, we just keep searching backwards.
        } else {
            // Found a valid layer (Compatible or Disjoint)
            targetLayer = current;
            targetMatch = result.second;

            // If it was compatible, we expect a match. If disjoint, match is nullptr.
            if (result.first == BoundsTest::kCompatibleOverlap) {
                break;
            }
            // If Disjoint, we can theoretically stay here, but we keep searching backwards
            // to see if there is a 'Compatible' layer further back to batch with.
        }

        current = iter.prev();
        limit--;
    }

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        fLayers.addToTail(targetLayer);
    }
    SkASSERT(targetLayer);

    if (isStencil) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
        targetLayer->add(&fStorage, targetMatch, key, draw, step, !dependsOnDst);
        fLastRecordedDraw = draw;
    } else {
        SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
        targetLayer->add(&fStorage, targetMatch, key, draw, step, !dependsOnDst);
    }
}

void DrawListLayer::recordDepthOnly(int stepIndex,
                                    bool isStencil,
                                    bool dependsOnDst,
                                    bool requiresBarrier,
                                    const RenderStep* step,
                                    const UniformDataCache::Index& uniformIndex,
                                    const LayerKey& key,
                                    const DrawParams* drawParams,
                                    Layer** captureLayer) {
    if (stepIndex > 0) {
        SkASSERT(fParentDepthLayer);
        DepthDraw* deferredDraw = fStorage.make<DepthDraw>(key, step, drawParams, uniformIndex);
        fParentDepthLayer->addDepthOnlyDraw(&fStorage, deferredDraw, isStencil);
        return;
    }
    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fPrev);
#endif
        BoundsTest result = current->depthOnlyTest(drawParams->drawBounds(), key, isStencil,
                                                   requiresBarrier);
        if (result == BoundsTest::kIncompatibleOverlap) {
            // TODO (thomsmit): Test performance of forward merging
            break;
        } else {
            targetLayer = current;
            if (result == BoundsTest::kCompatibleOverlap) {
                break;
            }
        }

        current = iter.prev();
        limit--;
    }

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        fLayers.addToTail(targetLayer);
    }
    SkASSERT(targetLayer);

    DepthDraw* deferredDraw = fStorage.make<DepthDraw>(key, step, drawParams, uniformIndex);
    targetLayer->addDepthOnlyDraw(&fStorage, deferredDraw, isStencil);
    fParentDepthLayer = targetLayer;
    if (!(*captureLayer) || targetLayer->fOrder > (*captureLayer)->fOrder) {
        *captureLayer = targetLayer;
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
                                   const Layer* startLayer) {
    // Child stencils get a fast path to their parent
    if (isStencil && stepIndex > 0) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
        SkASSERT(fLastRecordedDraw && !fLastRecordedDraw->fChildDraw);
        fLastRecordedDraw->fChildDraw = draw;
        fLastRecordedDraw = draw;
        return;
    }

    Layer* current = const_cast<Layer*>(startLayer);
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fNext);
#endif
        auto result = current->test(drawParams->drawBounds(), key, isStencil, requiresBarrier);
        if (result.first != BoundsTest::kIncompatibleOverlap) {
            targetLayer = current;
            targetMatch = result.second;
            break;
        }
        current = current->fNext;
        limit--;
    }

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        // Note: addToTail produces visually correct images, but addAfter does not. Given that we
        // explicitly do not allow dependsOnDst draws to take the forward walking path, it is not
        // clear why this is happening. This should be remedied when we switch to the "pilot draw"
        // style.
        fLayers.addToTail(targetLayer);
    }

    SkASSERT(targetLayer);
    if (isStencil) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
        targetLayer->add(&fStorage, targetMatch, key, draw, step, !dependsOnDst);
        fLastRecordedDraw = draw;
    } else {
        SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
        bool notStartLayer = targetLayer != startLayer;
        targetLayer->add(&fStorage, targetMatch, key, draw, step, !dependsOnDst && notStartLayer);
    }
}

// Layer has dual purpose here:
//  1) (Producer) If recording a depth only draw, the pointer is set to the *latest* layer inserted.
//  2) (Consumer) If recording a clipped draw, the pointer is the latest layer inserted into across
//     *all depth only draws* which affect this draw. Thus, it is the earliest possible layer that
//     the clipped draw could be inserted into, so it is used as the starting point for a *forward*
//     search.
std::pair<DrawParams*, Layer*> DrawListLayer::recordDraw(
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
        const Layer* latestDepthLayer) {

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

    Layer* stepLayer = nullptr;
    fRenderStepCount += renderer->numRenderSteps();
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

        if (paintID == UniquePaintParamsID::Invalid()) { // Invalid ID implies depth only draw
            this->recordDepthOnly(stepIndex,
                                  isStencil,
                                  dependsOnDst,
                                  requiresBarrier,
                                  step,
                                  uniformIndex,
                                  LayerKey{pipelineIndex, textureBindingIndex},
                                  drawParams,
                                  &stepLayer);
        } else {
            if (latestDepthLayer && !dependsOnDst) {
                this->recordForwards(stepIndex,
                                     isStencil,
                                     false,
                                     requiresBarrier,
                                     step,
                                     uniformIndex,
                                     LayerKey{pipelineIndex, textureBindingIndex},
                                     drawParams,
                                     latestDepthLayer);
            } else {
                this->recordBackwards(stepIndex,
                                      isStencil,
                                      dependsOnDst,
                                      requiresBarrier,
                                      step,
                                      uniformIndex,
                                      LayerKey{pipelineIndex, textureBindingIndex},
                                      drawParams,
                                      latestDepthLayer);
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

    return {drawParams, stepLayer};
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
        if (layer->fDepthInfo) {
            const DepthDraw* current = layer->fDepthInfo->fDraws.head();
            while (current) {
                if (!recordDraw(current->fKey,
                                current->fUniformIndex,
                                current->fStep,
                                *current->fDrawParams,
                                false)) {
                    return nullptr;
                }
                current = current->fNext;
            }
        }

        for (const BindingWrapper* binding : layer->fBindings) {
            if (binding->fType == BindingListType::kSingle) {
                const auto* singleList = static_cast<const BindingList<SingleDraw>*>(binding);
                SkASSERT(!singleList->fDraws.isEmpty());
                const SingleDraw* current = singleList->fDraws.head();
                if (!recordDraw(binding->fKey,
                                current->fUniformIndex,
                                binding->fStep,
                                *current->fDrawParams,
                                false)) {
                    return nullptr;
                }

                current = current->fNext;
                while (current) {
                    if (!recordDraw(binding->fKey,
                                    current->fUniformIndex,
                                    binding->fStep,
                                    *current->fDrawParams,
                                    true)) {
                        return nullptr;
                    }
                    current = current->fNext;
                }
            } else {
                // TODO Test the performance of transposed traversal of stencil draws!
                const auto* chainedList = static_cast<const BindingList<ChainedDraw>*>(binding);
                for (const ChainedDraw* headDraw : chainedList->fDraws) {
                    const ChainedDraw* current = headDraw;
                    while (current) {
                        if (!recordDraw(current->fKey,
                                        current->fUniformIndex,
                                        current->fStep,
                                        *current->fDrawParams,
                                        false)) {
                            return nullptr;
                        }
                        current = current->fChildDraw;
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
