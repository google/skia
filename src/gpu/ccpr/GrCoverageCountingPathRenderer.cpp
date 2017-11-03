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
#include "SkPathOps.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetOpList.h"
#include "GrStyle.h"
#include "ccpr/GrCCPRPathProcessor.h"

using DrawPathsOp = GrCoverageCountingPathRenderer::DrawPathsOp;
using ScissorMode = GrCCPRCoverageOpsBuilder::ScissorMode;

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    return false;
#else
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    return shaderCaps.geometryShaderSupport() &&
           shaderCaps.texelBufferSupport() &&
           shaderCaps.integerSupport() &&
           shaderCaps.flatInterpolationSupport() &&
           shaderCaps.maxVertexSamplers() >= 1 &&
           caps.instanceAttribSupport() &&
           caps.isConfigTexturable(kAlpha_half_GrPixelConfig) &&
           caps.isConfigRenderable(kAlpha_half_GrPixelConfig, /*withMSAA=*/false) &&
           GrCaps::kNone_MapFlags != caps.mapBufferFlags() &&
           !caps.blacklistCoverageCounting();
#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK
}

sk_sp<GrCoverageCountingPathRenderer>
GrCoverageCountingPathRenderer::CreateIfSupported(const GrCaps& caps, bool drawCachablePaths) {
    auto ccpr = IsSupported(caps) ? new GrCoverageCountingPathRenderer(drawCachablePaths) : nullptr;
    return sk_sp<GrCoverageCountingPathRenderer>(ccpr);
}

GrPathRenderer::CanDrawPath
GrCoverageCountingPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (args.fShape->hasUnstyledKey() && !fDrawCachablePaths) {
        return CanDrawPath::kNo;
    }

    if (!args.fShape->style().isSimpleFill() ||
        args.fShape->inverseFilled() ||
        args.fViewMatrix->hasPerspective() ||
        GrAAType::kCoverage != args.fAAType) {
        return CanDrawPath::kNo;
    }

    SkPath path;
    args.fShape->asPath(&path);
    if (SkPathPriv::ConicWeightCnt(path)) {
        return CanDrawPath::kNo;
    }

    SkRect devBounds;
    SkIRect devIBounds;
    args.fViewMatrix->mapRect(&devBounds, path.getBounds());
    devBounds.roundOut(&devIBounds);
    if (!devIBounds.intersect(*args.fClipConservativeBounds)) {
        // Path is completely clipped away. Our code will eventually notice this before doing any
        // real work.
        return CanDrawPath::kYes;
    }

    if (devIBounds.height() * devIBounds.width() > 256 * 256) {
        // Large paths can blow up the atlas fast. And they are not ideal for a two-pass rendering
        // algorithm. Give the simpler direct renderers a chance before we commit to drawing it.
        return CanDrawPath::kAsBackup;
    }

    if (args.fShape->hasUnstyledKey() && path.countVerbs() > 50) {
        // Complex paths do better cached in an SDF, if the renderer will accept them.
        return CanDrawPath::kAsBackup;
    }

    return CanDrawPath::kYes;
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
    SkDEBUGCODE(fDebugSkippedInstances = 0;)
    GrRenderTargetContext* const rtc = args.fRenderTargetContext;

    SkRect devBounds;
    args.fViewMatrix->mapRect(&devBounds, args.fShape->bounds());
    args.fClip->getConservativeBounds(rtc->width(), rtc->height(), &fHeadDraw.fClipBounds, nullptr);
    if (SkTMax(devBounds.height(), devBounds.width()) > (1 << 16)) {
        // The path is too large. We need to crop it or risk running out of fp32 precision for
        // analytic AA.
        SkPath cropPath, path;
        cropPath.addRect(SkRect::Make(fHeadDraw.fClipBounds));
        args.fShape->asPath(&path);
        path.transform(*args.fViewMatrix);
        fHeadDraw.fMatrix.setIdentity();
        if (!Op(cropPath, path, kIntersect_SkPathOp, &fHeadDraw.fPath)) {
            // This can fail if the PathOps encounter NaN or infinities.
            fHeadDraw.fPath.reset();
        }
        devBounds = fHeadDraw.fPath.getBounds();
        fHeadDraw.fScissorMode = ScissorMode::kNonScissored;
    } else {
        fHeadDraw.fMatrix = *args.fViewMatrix;
        args.fShape->asPath(&fHeadDraw.fPath);
        fHeadDraw.fScissorMode = fHeadDraw.fClipBounds.contains(devBounds) ?
                                 ScissorMode::kNonScissored : ScissorMode::kScissored;
    }
    fHeadDraw.fColor = color; // Can't call args.fPaint.getColor() because it has been std::move'd.

    // FIXME: intersect with clip bounds to (hopefully) improve batching.
    // (This is nontrivial due to assumptions in generating the octagon cover geometry.)
    this->setBounds(devBounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);
}

GrDrawOp::RequiresDstTexture DrawPathsOp::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                                   GrPixelConfigIsClamped dstIsClamped) {
    SingleDraw& onlyDraw = this->getOnlyPathDraw();
    GrProcessorSet::Analysis analysis = fProcessors.finalize(
            onlyDraw.fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip, false, caps,
            dstIsClamped, &onlyDraw.fColor);
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
        // The Op is being combined immediately after creation, before a call to wasRecorded. In
        // this case wasRecorded will not be called. So we count its path here instead.
        const SingleDraw& onlyDraw = that->getOnlyPathDraw();
        ++fOwningRTPendingOps->fNumTotalPaths;
        fOwningRTPendingOps->fNumSkPoints += onlyDraw.fPath.countPoints();
        fOwningRTPendingOps->fNumSkVerbs += onlyDraw.fPath.countVerbs();
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
    const SingleDraw& onlyDraw = this->getOnlyPathDraw();
    fOwningRTPendingOps = &fCCPR->fRTPendingOpsMap[opList->uniqueID()];
    ++fOwningRTPendingOps->fNumTotalPaths;
    fOwningRTPendingOps->fNumSkPoints += onlyDraw.fPath.countPoints();
    fOwningRTPendingOps->fNumSkVerbs += onlyDraw.fPath.countVerbs();
    fOwningRTPendingOps->fOpList.addToTail(this);
}

void GrCoverageCountingPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                              const uint32_t* opListIDs, int numOpListIDs,
                                              SkTArray<sk_sp<GrRenderTargetContext>>* results) {
    SkASSERT(!fFlushing);
    SkDEBUGCODE(fFlushing = true;)

    if (fRTPendingOpsMap.empty()) {
        return; // Nothing to draw.
    }

    this->setupPerFlushResources(onFlushRP, opListIDs, numOpListIDs, results);

    // Erase these last, once we are done accessing data from the SingleDraw allocators.
    for (int i = 0; i < numOpListIDs; ++i) {
        fRTPendingOpsMap.erase(opListIDs[i]);
    }
}

void GrCoverageCountingPathRenderer::setupPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                                  const uint32_t* opListIDs,
                                                  int numOpListIDs,
                                                  SkTArray<sk_sp<GrRenderTargetContext>>* results) {
    using PathInstance = GrCCPRPathProcessor::Instance;

    SkASSERT(!fPerFlushIndexBuffer);
    SkASSERT(!fPerFlushVertexBuffer);
    SkASSERT(!fPerFlushInstanceBuffer);
    SkASSERT(fPerFlushAtlases.empty());

    fPerFlushResourcesAreValid = false;

    SkTInternalLList<DrawPathsOp> flushingOps;
    int maxTotalPaths = 0, numSkPoints = 0, numSkVerbs = 0;

    for (int i = 0; i < numOpListIDs; ++i) {
        auto it = fRTPendingOpsMap.find(opListIDs[i]);
        if (fRTPendingOpsMap.end() != it) {
            RTPendingOps& rtPendingOps = it->second;
            SkASSERT(!rtPendingOps.fOpList.isEmpty());
            flushingOps.concat(std::move(rtPendingOps.fOpList));
            maxTotalPaths += rtPendingOps.fNumTotalPaths;
            numSkPoints += rtPendingOps.fNumSkPoints;
            numSkVerbs += rtPendingOps.fNumSkVerbs;
        }
    }

    SkASSERT(flushingOps.isEmpty() == !maxTotalPaths);
    if (flushingOps.isEmpty()) {
        return; // Nothing to draw.
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

    fPerFlushInstanceBuffer = onFlushRP->makeBuffer(kVertex_GrBufferType,
                                                   maxTotalPaths * sizeof(PathInstance));
    if (!fPerFlushInstanceBuffer) {
        SkDebugf("WARNING: failed to allocate path instance buffer. No paths will be drawn.\n");
        return;
    }

    PathInstance* pathInstanceData = static_cast<PathInstance*>(fPerFlushInstanceBuffer->map());
    SkASSERT(pathInstanceData);
    int pathInstanceIdx = 0;

    GrCCPRCoverageOpsBuilder atlasOpsBuilder(maxTotalPaths, numSkPoints, numSkVerbs);
    GrCCPRAtlas* atlas = nullptr;
    SkDEBUGCODE(int skippedTotalPaths = 0;)

    SkTInternalLList<DrawPathsOp>::Iter iter;
    iter.init(flushingOps, SkTInternalLList<DrawPathsOp>::Iter::kHead_IterStart);
    while (DrawPathsOp* drawPathOp = iter.get()) {
        SkASSERT(drawPathOp->fDebugInstanceCount > 0);
        SkASSERT(-1 == drawPathOp->fBaseInstance);
        drawPathOp->fBaseInstance = pathInstanceIdx;

        for (const auto* draw = &drawPathOp->fHeadDraw; draw; draw = draw->fNext) {
            // parsePath gives us two tight bounding boxes: one in device space, as well as a second
            // one rotated an additional 45 degrees. The path vertex shader uses these two bounding
            // boxes to generate an octagon that circumscribes the path.
            SkRect devBounds, devBounds45;
            atlasOpsBuilder.parsePath(draw->fMatrix, draw->fPath, &devBounds, &devBounds45);

            SkRect clippedDevBounds = devBounds;
            if (ScissorMode::kScissored == draw->fScissorMode &&
                !clippedDevBounds.intersect(devBounds, SkRect::Make(draw->fClipBounds))) {
                SkDEBUGCODE(++drawPathOp->fDebugSkippedInstances);
                atlasOpsBuilder.discardParsedPath();
                continue;
            }

            SkIRect clippedDevIBounds;
            clippedDevBounds.roundOut(&clippedDevIBounds);
            const int h = clippedDevIBounds.height(), w = clippedDevIBounds.width();

            SkIPoint16 atlasLocation;
            if (atlas && !atlas->addRect(w, h, &atlasLocation)) {
                // The atlas is out of room and can't grow any bigger.
                atlasOpsBuilder.emitOp(atlas->drawBounds());
                if (pathInstanceIdx > drawPathOp->fBaseInstance) {
                    drawPathOp->addAtlasBatch(atlas, pathInstanceIdx);
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

            atlasOpsBuilder.saveParsedPath(draw->fScissorMode, clippedDevIBounds, offsetX, offsetY);
        }

        SkASSERT(pathInstanceIdx == drawPathOp->fBaseInstance + drawPathOp->fDebugInstanceCount -
                                    drawPathOp->fDebugSkippedInstances);
        if (pathInstanceIdx > drawPathOp->fBaseInstance) {
            drawPathOp->addAtlasBatch(atlas, pathInstanceIdx);
        }

        iter.next();
        SkDEBUGCODE(skippedTotalPaths += drawPathOp->fDebugSkippedInstances;)
    }
    SkASSERT(pathInstanceIdx == maxTotalPaths - skippedTotalPaths);

    if (atlas) {
        atlasOpsBuilder.emitOp(atlas->drawBounds());
    }

    fPerFlushInstanceBuffer->unmap();

    // Draw the coverage ops into their respective atlases.
    SkSTArray<4, std::unique_ptr<GrCCPRCoverageOp>> atlasOps(fPerFlushAtlases.count());
    if (!atlasOpsBuilder.finalize(onFlushRP, &atlasOps)) {
        SkDebugf("WARNING: failed to allocate ccpr atlas buffers. No paths will be drawn.\n");
        return;
    }
    SkASSERT(atlasOps.count() == fPerFlushAtlases.count());

    GrTAllocator<GrCCPRAtlas>::Iter atlasIter(&fPerFlushAtlases);
    for (std::unique_ptr<GrCCPRCoverageOp>& atlasOp : atlasOps) {
        SkAssertResult(atlasIter.next());
        GrCCPRAtlas* atlas = atlasIter.get();
        SkASSERT(atlasOp->bounds() == SkRect::MakeIWH(atlas->drawBounds().width(),
                                                      atlas->drawBounds().height()));
        if (auto rtc = atlas->finalize(onFlushRP, std::move(atlasOp))) {
            results->push_back(std::move(rtc));
        }
    }
    SkASSERT(!atlasIter.next());

    fPerFlushResourcesAreValid = true;
}

void DrawPathsOp::onExecute(GrOpFlushState* flushState) {
    SkASSERT(fCCPR->fFlushing);
    SkASSERT(flushState->rtCommandBuffer());

    if (!fCCPR->fPerFlushResourcesAreValid) {
        return; // Setup failed.
    }

    GrPipeline::InitArgs initArgs;
    initArgs.fFlags = fSRGBFlags;
    initArgs.fProxy = flushState->drawOpArgs().fProxy;
    initArgs.fCaps = &flushState->caps();
    initArgs.fResourceProvider = flushState->resourceProvider();
    initArgs.fDstProxy = flushState->drawOpArgs().fDstProxy;
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

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

        flushState->rtCommandBuffer()->draw(pipeline, coverProc, &mesh, nullptr, 1, this->bounds());
    }

    SkASSERT(baseInstance == fBaseInstance + fDebugInstanceCount - fDebugSkippedInstances);
}

void GrCoverageCountingPathRenderer::postFlush(GrDeferredUploadToken) {
    SkASSERT(fFlushing);
    fPerFlushAtlases.reset();
    fPerFlushInstanceBuffer.reset();
    fPerFlushVertexBuffer.reset();
    fPerFlushIndexBuffer.reset();
    SkDEBUGCODE(fFlushing = false;)
}
