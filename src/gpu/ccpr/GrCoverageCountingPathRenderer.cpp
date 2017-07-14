/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCoverageCountingPathRenderer.h"

#include "GrCaps.h"
#include "GrClip.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "SkMakeUnique.h"
#include "SkMatrix.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetOpList.h"
#include "GrStyle.h"
#include "ccpr/GrCCPRPathProcessor.h"

using DrawPathsOp = GrCoverageCountingPathRenderer::DrawPathsOp;
using ScissorMode = GrCCPRCoverageOpsBuilder::ScissorMode;

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    return shaderCaps.geometryShaderSupport() &&
           shaderCaps.texelBufferSupport() &&
           shaderCaps.integerSupport() &&
           shaderCaps.flatInterpolationSupport() &&
           shaderCaps.maxVertexSamplers() >= 1 &&
           caps.instanceAttribSupport() &&
           caps.isConfigTexturable(kAlpha_half_GrPixelConfig) &&
           caps.isConfigRenderable(kAlpha_half_GrPixelConfig, /*withMSAA=*/false);
}

sk_sp<GrCoverageCountingPathRenderer>
GrCoverageCountingPathRenderer::CreateIfSupported(const GrCaps& caps) {
    return sk_sp<GrCoverageCountingPathRenderer>(IsSupported(caps) ?
                                                 new GrCoverageCountingPathRenderer : nullptr);
}

bool GrCoverageCountingPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (!args.fShape->style().isSimpleFill() ||
        args.fShape->inverseFilled() ||
        args.fViewMatrix->hasPerspective() ||
        GrAAType::kCoverage != args.fAAType) {
        return false;
    }

    SkPath path;
    args.fShape->asPath(&path);
    return !SkPathPriv::ConicWeightCnt(path);
}

bool GrCoverageCountingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    SkASSERT(!fFlushing);
    SkASSERT(!args.fShape->isEmpty());

    auto op = skstd::make_unique<DrawPathsOp>(this, args, args.fPaint.getColor());
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));

    return true;
}

GrCoverageCountingPathRenderer::DrawPathsOp::DrawPathsOp(GrCoverageCountingPathRenderer* ccpr,
                                                         const DrawPathArgs& args, GrColor color)
        : INHERITED(ClassID())
        , fCCPR(ccpr)
        , fSRGBFlags(GrPipeline::SRGBFlagsFromPaint(args.fPaint))
        , fProcessors(std::move(args.fPaint))
        , fTailDraw(&fHeadDraw)
        , fOwningRTPendingOps(nullptr) {
    SkDEBUGCODE(fBaseInstance = -1);
    SkDEBUGCODE(fDebugInstanceCount = 1;)

    GrRenderTargetContext* const rtc = args.fRenderTargetContext;

    SkRect devBounds;
    args.fViewMatrix->mapRect(&devBounds, args.fShape->bounds());

    args.fClip->getConservativeBounds(rtc->width(), rtc->height(), &fHeadDraw.fClipBounds, nullptr);
    fHeadDraw.fScissorMode = fHeadDraw.fClipBounds.contains(devBounds) ?
                             ScissorMode::kNonScissored : ScissorMode::kScissored;
    fHeadDraw.fMatrix = *args.fViewMatrix;
    args.fShape->asPath(&fHeadDraw.fPath);
    fHeadDraw.fColor = color; // Can't call args.fPaint.getColor() because it has been std::move'd.

    // FIXME: intersect with clip bounds to (hopefully) improve batching.
    // (This is nontrivial due to assumptions in generating the octagon cover geometry.)
    this->setBounds(devBounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);
}

GrDrawOp::RequiresDstTexture DrawPathsOp::finalize(const GrCaps& caps, const GrAppliedClip* clip) {
    SingleDraw& onlyDraw = this->getOnlyPathDraw();
    GrProcessorSet::Analysis analysis = fProcessors.finalize(onlyDraw.fColor,
                                                        GrProcessorAnalysisCoverage::kSingleChannel,
                                                        clip, false, caps, &onlyDraw.fColor);
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

bool DrawPathsOp::onCombineIfPossible(GrOp* op, const GrCaps& caps) {
    DrawPathsOp* that = op->cast<DrawPathsOp>();
    SkASSERT(fCCPR == that->fCCPR);
    SkASSERT(fOwningRTPendingOps);
    SkASSERT(fDebugInstanceCount);
    SkASSERT(that->fDebugInstanceCount);

    if (this->getFillType() != that->getFillType() ||
        fSRGBFlags != that->fSRGBFlags ||
        fProcessors != that->fProcessors) {
        return false;
    }

    if (RTPendingOps* owningRTPendingOps = that->fOwningRTPendingOps) {
        SkASSERT(owningRTPendingOps == fOwningRTPendingOps);
        owningRTPendingOps->fOpList.remove(that);
    } else {
        // wasRecorded is not called when the op gets combined first. Count path items here instead.
        SingleDraw& onlyDraw = that->getOnlyPathDraw();
        fOwningRTPendingOps->fMaxBufferItems.countPathItems(onlyDraw.fScissorMode, onlyDraw.fPath);
    }

    fTailDraw->fNext = &fOwningRTPendingOps->fDrawsAllocator.push_back(that->fHeadDraw);
    fTailDraw = that->fTailDraw == &that->fHeadDraw ? fTailDraw->fNext : that->fTailDraw;

    this->joinBounds(*that);

    SkDEBUGCODE(fDebugInstanceCount += that->fDebugInstanceCount;)
    SkDEBUGCODE(that->fDebugInstanceCount = 0);
    return true;
}

void DrawPathsOp::wasRecorded(GrRenderTargetOpList* opList) {
    SkASSERT(!fOwningRTPendingOps);
    SingleDraw& onlyDraw = this->getOnlyPathDraw();
    fOwningRTPendingOps = &fCCPR->fRTPendingOpsMap[opList->uniqueID()];
    fOwningRTPendingOps->fOpList.addToTail(this);
    fOwningRTPendingOps->fMaxBufferItems.countPathItems(onlyDraw.fScissorMode, onlyDraw.fPath);
}

void GrCoverageCountingPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                              const uint32_t* opListIDs, int numOpListIDs,
                                              SkTArray<sk_sp<GrRenderTargetContext>>* results) {
    using PathInstance = GrCCPRPathProcessor::Instance;

    SkASSERT(!fPerFlushIndexBuffer);
    SkASSERT(!fPerFlushVertexBuffer);
    SkASSERT(!fPerFlushInstanceBuffer);
    SkASSERT(fPerFlushAtlases.empty());
    SkASSERT(!fFlushing);
    SkDEBUGCODE(fFlushing = true;)

    if (fRTPendingOpsMap.empty()) {
        return; // Nothing to draw.
    }

    SkTInternalLList<DrawPathsOp> flushingOps;
    GrCCPRCoverageOpsBuilder::MaxBufferItems maxBufferItems;

    for (int i = 0; i < numOpListIDs; ++i) {
        auto it = fRTPendingOpsMap.find(opListIDs[i]);
        if (fRTPendingOpsMap.end() != it) {
            RTPendingOps& rtPendingOps = it->second;
            SkASSERT(!rtPendingOps.fOpList.isEmpty());
            flushingOps.concat(std::move(rtPendingOps.fOpList));
            maxBufferItems += rtPendingOps.fMaxBufferItems;
        }
    }

    SkASSERT(flushingOps.isEmpty() == !maxBufferItems.fMaxPaths);
    if (flushingOps.isEmpty()) {
        return; // Still nothing to draw.
    }

    fPerFlushIndexBuffer = GrCCPRPathProcessor::FindOrMakeIndexBuffer(onFlushRP);
    if (!fPerFlushIndexBuffer) {
        SkDebugf("WARNING: failed to allocate ccpr path index buffer.\n");
        return;
    }

    fPerFlushVertexBuffer = GrCCPRPathProcessor::FindOrMakeVertexBuffer(onFlushRP);
    if (!fPerFlushVertexBuffer) {
        SkDebugf("WARNING: failed to allocate ccpr path vertex buffer.\n");
        return;
    }

    GrCCPRCoverageOpsBuilder atlasOpsBuilder;
    if (!atlasOpsBuilder.init(onFlushRP, maxBufferItems)) {
        SkDebugf("WARNING: failed to allocate buffers for coverage ops. No paths will be drawn.\n");
        return;
    }

    fPerFlushInstanceBuffer = onFlushRP->makeBuffer(kVertex_GrBufferType,
                                                   maxBufferItems.fMaxPaths * sizeof(PathInstance));
    if (!fPerFlushInstanceBuffer) {
        SkDebugf("WARNING: failed to allocate path instance buffer. No paths will be drawn.\n");
        return;
    }

    PathInstance* pathInstanceData = static_cast<PathInstance*>(fPerFlushInstanceBuffer->map());
    SkASSERT(pathInstanceData);
    int pathInstanceIdx = 0;

    GrCCPRAtlas* atlas = nullptr;
    SkDEBUGCODE(int skippedPaths = 0;)

    SkTInternalLList<DrawPathsOp>::Iter iter;
    iter.init(flushingOps, SkTInternalLList<DrawPathsOp>::Iter::kHead_IterStart);
    while (DrawPathsOp* op = iter.get()) {
        SkASSERT(op->fDebugInstanceCount > 0);
        SkASSERT(-1 == op->fBaseInstance);
        op->fBaseInstance = pathInstanceIdx;

        for (const DrawPathsOp::SingleDraw* draw = &op->fHeadDraw; draw; draw = draw->fNext) {
            // parsePath gives us two tight bounding boxes: one in device space, as well as a second
            // one rotated an additional 45 degrees. The path vertex shader uses these two bounding
            // boxes to generate an octagon that circumscribes the path.
            SkRect devBounds, devBounds45;
            atlasOpsBuilder.parsePath(draw->fScissorMode, draw->fMatrix, draw->fPath, &devBounds,
                                      &devBounds45);

            SkRect clippedDevBounds = devBounds;
            if (ScissorMode::kScissored == draw->fScissorMode &&
                !clippedDevBounds.intersect(devBounds, SkRect::Make(draw->fClipBounds))) {
                SkDEBUGCODE(--op->fDebugInstanceCount);
                SkDEBUGCODE(++skippedPaths;)
                continue;
            }

            SkIRect clippedDevIBounds;
            clippedDevBounds.roundOut(&clippedDevIBounds);
            const int h = clippedDevIBounds.height(), w = clippedDevIBounds.width();

            SkIPoint16 atlasLocation;
            if (atlas && !atlas->addRect(w, h, &atlasLocation)) {
                // The atlas is out of room and can't grow any bigger.
                auto atlasOp = atlasOpsBuilder.createIntermediateOp(atlas->drawBounds());
                if (auto rtc = atlas->finalize(onFlushRP, std::move(atlasOp))) {
                    results->push_back(std::move(rtc));
                }
                if (pathInstanceIdx > op->fBaseInstance) {
                    op->addAtlasBatch(atlas, pathInstanceIdx);
                }
                atlas = nullptr;
            }

            if (!atlas) {
                atlas = &fPerFlushAtlases.emplace_back(*onFlushRP->caps(), w, h);
                SkAssertResult(atlas->addRect(w, h, &atlasLocation));
            }

            const SkMatrix& m = draw->fMatrix;
            const int16_t offsetX = atlasLocation.x() - static_cast<int16_t>(clippedDevIBounds.x()),
                          offsetY = atlasLocation.y() - static_cast<int16_t>(clippedDevIBounds.y());

            pathInstanceData[pathInstanceIdx++] = {
                devBounds,
                devBounds45,
                {{m.getScaleX(), m.getSkewY(), m.getSkewX(), m.getScaleY()}},
                {{m.getTranslateX(), m.getTranslateY()}},
                {{offsetX, offsetY}},
                draw->fColor
            };

            atlasOpsBuilder.saveParsedPath(clippedDevIBounds, offsetX, offsetY);
        }

        SkASSERT(pathInstanceIdx == op->fBaseInstance + op->fDebugInstanceCount);
        op->addAtlasBatch(atlas, pathInstanceIdx);

        iter.next();
    }

    SkASSERT(pathInstanceIdx == maxBufferItems.fMaxPaths - skippedPaths);
    fPerFlushInstanceBuffer->unmap();

    std::unique_ptr<GrDrawOp> atlasOp = atlasOpsBuilder.finalize(atlas->drawBounds());
    if (auto rtc = atlas->finalize(onFlushRP, std::move(atlasOp))) {
        results->push_back(std::move(rtc));
    }

    // Erase these last, once we are done accessing data from the SingleDraw allocators.
    for (int i = 0; i < numOpListIDs; ++i) {
        fRTPendingOpsMap.erase(opListIDs[i]);
    }
}

void DrawPathsOp::onExecute(GrOpFlushState* flushState) {
    SkASSERT(fCCPR->fFlushing);

    if (!fCCPR->fPerFlushInstanceBuffer) {
        return; // Setup failed.
    }

    GrPipeline pipeline;
    GrPipeline::InitArgs args;
    args.fAppliedClip = flushState->drawOpArgs().fAppliedClip;
    args.fCaps = &flushState->caps();
    args.fProcessors = &fProcessors;
    args.fFlags = fSRGBFlags;
    args.fRenderTarget = flushState->drawOpArgs().fRenderTarget;
    args.fDstProxy = flushState->drawOpArgs().fDstProxy;
    pipeline.init(args);

    int baseInstance = fBaseInstance;

    for (int i = 0; i < fAtlasBatches.count(); baseInstance = fAtlasBatches[i++].fEndInstanceIdx) {
        const AtlasBatch& batch = fAtlasBatches[i];
        SkASSERT(batch.fEndInstanceIdx > baseInstance);

        if (!batch.fAtlas->textureProxy()) {
            continue; // Atlas failed to allocate.
        }

        GrCCPRPathProcessor coverProc(flushState->resourceProvider(), batch.fAtlas->textureProxy(),
                                     this->getFillType(), *flushState->gpu()->caps()->shaderCaps());

        GrMesh mesh(GrPrimitiveType::kTriangles);
        mesh.setIndexedInstanced(fCCPR->fPerFlushIndexBuffer.get(),
                                 GrCCPRPathProcessor::kPerInstanceIndexCount,
                                 fCCPR->fPerFlushInstanceBuffer.get(),
                                 batch.fEndInstanceIdx - baseInstance, baseInstance);
        mesh.setVertexData(fCCPR->fPerFlushVertexBuffer.get());

        flushState->commandBuffer()->draw(pipeline, coverProc, &mesh, nullptr, 1, this->bounds());
    }

    SkASSERT(baseInstance == fBaseInstance + fDebugInstanceCount);
}

void GrCoverageCountingPathRenderer::postFlush() {
    SkASSERT(fFlushing);
    fPerFlushAtlases.reset();
    fPerFlushInstanceBuffer.reset();
    fPerFlushVertexBuffer.reset();
    fPerFlushIndexBuffer.reset();
    SkDEBUGCODE(fFlushing = false;)
}
