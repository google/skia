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
#include "GrOpFlushState.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetOpList.h"
#include "GrStyle.h"
#include "GrTexture.h"
#include "SkMakeUnique.h"
#include "SkMatrix.h"
#include "SkPathOps.h"
#include "ccpr/GrCCClipProcessor.h"

// Shorthand for keeping line lengths under control with nested classes...
using CCPR = GrCoverageCountingPathRenderer;

// If a path spans more pixels than this, we need to crop it or else analytic AA can run out of fp32
// precision.
static constexpr float kPathCropThreshold = 1 << 16;

static void crop_path(const SkPath& path, const SkIRect& cropbox, SkPath* out) {
    SkPath cropPath;
    cropPath.addRect(SkRect::Make(cropbox));
    if (!Op(cropPath, path, kIntersect_SkPathOp, out)) {
        // This can fail if the PathOps encounter NaN or infinities.
        out->reset();
    }
}

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    return shaderCaps.integerSupport() && shaderCaps.flatInterpolationSupport() &&
           caps.instanceAttribSupport() && GrCaps::kNone_MapFlags != caps.mapBufferFlags() &&
           caps.isConfigTexturable(kAlpha_half_GrPixelConfig) &&
           caps.isConfigRenderable(kAlpha_half_GrPixelConfig) &&
           !caps.blacklistCoverageCounting();
}

sk_sp<GrCoverageCountingPathRenderer> GrCoverageCountingPathRenderer::CreateIfSupported(
        const GrCaps& caps, bool drawCachablePaths) {
    auto ccpr = IsSupported(caps) ? new GrCoverageCountingPathRenderer(drawCachablePaths) : nullptr;
    return sk_sp<GrCoverageCountingPathRenderer>(ccpr);
}

GrPathRenderer::CanDrawPath GrCoverageCountingPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    if (args.fShape->hasUnstyledKey() && !fDrawCachablePaths) {
        return CanDrawPath::kNo;
    }

    if (!args.fShape->style().isSimpleFill() || args.fShape->inverseFilled() ||
        args.fViewMatrix->hasPerspective() || GrAAType::kCoverage != args.fAAType) {
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
    auto op = skstd::make_unique<DrawPathsOp>(this, args, args.fPaint.getColor());
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}

CCPR::DrawPathsOp::DrawPathsOp(GrCoverageCountingPathRenderer* ccpr, const DrawPathArgs& args,
                               GrColor color)
        : INHERITED(ClassID())
        , fCCPR(ccpr)
        , fSRGBFlags(GrPipeline::SRGBFlagsFromPaint(args.fPaint))
        , fProcessors(std::move(args.fPaint))
        , fTailDraw(&fHeadDraw)
        , fOwningRTPendingPaths(nullptr) {
    SkDEBUGCODE(++fCCPR->fPendingDrawOpsCount);
    SkDEBUGCODE(fBaseInstance = -1);
    SkDEBUGCODE(fInstanceCount = 1);
    SkDEBUGCODE(fNumSkippedInstances = 0);
    GrRenderTargetContext* const rtc = args.fRenderTargetContext;

    SkRect devBounds;
    args.fViewMatrix->mapRect(&devBounds, args.fShape->bounds());
    args.fClip->getConservativeBounds(rtc->width(), rtc->height(), &fHeadDraw.fClipIBounds,
                                      nullptr);
    if (SkTMax(devBounds.height(), devBounds.width()) > kPathCropThreshold) {
        // The path is too large. We need to crop it or analytic AA can run out of fp32 precision.
        SkPath path;
        args.fShape->asPath(&path);
        path.transform(*args.fViewMatrix);
        fHeadDraw.fMatrix.setIdentity();
        crop_path(path, fHeadDraw.fClipIBounds, &fHeadDraw.fPath);
        devBounds = fHeadDraw.fPath.getBounds();
    } else {
        fHeadDraw.fMatrix = *args.fViewMatrix;
        args.fShape->asPath(&fHeadDraw.fPath);
    }
    fHeadDraw.fColor = color;  // Can't call args.fPaint.getColor() because it has been std::move'd.

    // FIXME: intersect with clip bounds to (hopefully) improve batching.
    // (This is nontrivial due to assumptions in generating the octagon cover geometry.)
    this->setBounds(devBounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);
}

CCPR::DrawPathsOp::~DrawPathsOp() {
    if (fOwningRTPendingPaths) {
        // Remove CCPR's dangling pointer to this Op before deleting it.
        fOwningRTPendingPaths->fDrawOps.remove(this);
    }
    SkDEBUGCODE(--fCCPR->fPendingDrawOpsCount);
}

GrDrawOp::RequiresDstTexture CCPR::DrawPathsOp::finalize(const GrCaps& caps,
                                                         const GrAppliedClip* clip,
                                                         GrPixelConfigIsClamped dstIsClamped) {
    SkASSERT(!fCCPR->fFlushing);
    // There should only be one single path draw in this Op right now.
    SkASSERT(1 == fInstanceCount);
    SkASSERT(&fHeadDraw == fTailDraw);
    GrProcessorSet::Analysis analysis =
            fProcessors.finalize(fHeadDraw.fColor, GrProcessorAnalysisCoverage::kSingleChannel,
                                 clip, false, caps, dstIsClamped, &fHeadDraw.fColor);
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

bool CCPR::DrawPathsOp::onCombineIfPossible(GrOp* op, const GrCaps& caps) {
    DrawPathsOp* that = op->cast<DrawPathsOp>();
    SkASSERT(fCCPR == that->fCCPR);
    SkASSERT(!fCCPR->fFlushing);
    SkASSERT(fOwningRTPendingPaths);
    SkASSERT(fInstanceCount);
    SkASSERT(!that->fOwningRTPendingPaths || that->fOwningRTPendingPaths == fOwningRTPendingPaths);
    SkASSERT(that->fInstanceCount);

    if (this->getFillType() != that->getFillType() || fSRGBFlags != that->fSRGBFlags ||
        fProcessors != that->fProcessors) {
        return false;
    }

    fTailDraw->fNext = &fOwningRTPendingPaths->fDrawsAllocator.push_back(that->fHeadDraw);
    fTailDraw = (that->fTailDraw == &that->fHeadDraw) ? fTailDraw->fNext : that->fTailDraw;

    this->joinBounds(*that);

    SkDEBUGCODE(fInstanceCount += that->fInstanceCount);
    SkDEBUGCODE(that->fInstanceCount = 0);
    return true;
}

void CCPR::DrawPathsOp::wasRecorded(GrRenderTargetOpList* opList) {
    SkASSERT(!fCCPR->fFlushing);
    SkASSERT(!fOwningRTPendingPaths);
    fOwningRTPendingPaths = &fCCPR->fRTPendingPathsMap[opList->uniqueID()];
    fOwningRTPendingPaths->fDrawOps.addToTail(this);
}

bool GrCoverageCountingPathRenderer::canMakeClipProcessor(const SkPath& deviceSpacePath) const {
    if (!fDrawCachablePaths && !deviceSpacePath.isVolatile()) {
        return false;
    }

    if (SkPathPriv::ConicWeightCnt(deviceSpacePath)) {
        return false;
    }

    return true;
}

std::unique_ptr<GrFragmentProcessor> GrCoverageCountingPathRenderer::makeClipProcessor(
        GrProxyProvider* proxyProvider,
        uint32_t opListID, const SkPath& deviceSpacePath, const SkIRect& accessRect,
        int rtWidth, int rtHeight) {
    using MustCheckBounds = GrCCClipProcessor::MustCheckBounds;

    SkASSERT(!fFlushing);
    SkASSERT(this->canMakeClipProcessor(deviceSpacePath));

    ClipPath& clipPath = fRTPendingPathsMap[opListID].fClipPaths[deviceSpacePath.getGenerationID()];
    if (clipPath.isUninitialized()) {
        // This ClipPath was just created during lookup. Initialize it.
        clipPath.init(proxyProvider, deviceSpacePath, accessRect, rtWidth, rtHeight);
    } else {
        clipPath.addAccess(accessRect);
    }

    bool mustCheckBounds = !clipPath.pathDevIBounds().contains(accessRect);
    return skstd::make_unique<GrCCClipProcessor>(&clipPath, MustCheckBounds(mustCheckBounds),
                                                 deviceSpacePath.getFillType());
}

void CCPR::ClipPath::init(GrProxyProvider* proxyProvider,
                          const SkPath& deviceSpacePath, const SkIRect& accessRect,
                          int rtWidth, int rtHeight) {
    SkASSERT(this->isUninitialized());

    fAtlasLazyProxy = proxyProvider->createFullyLazyProxy(
            [this](GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrTexture>();
                }
                SkASSERT(fHasAtlas);
                SkASSERT(!fHasAtlasTransform);

                GrTextureProxy* textureProxy = fAtlas ? fAtlas->textureProxy() : nullptr;
                if (!textureProxy || !textureProxy->instantiate(resourceProvider)) {
                    fAtlasScale = fAtlasTranslate = {0, 0};
                    SkDEBUGCODE(fHasAtlasTransform = true);
                    return sk_sp<GrTexture>();
                }

                SkASSERT(kTopLeft_GrSurfaceOrigin == textureProxy->origin());

                fAtlasScale = {1.f / textureProxy->width(), 1.f / textureProxy->height()};
                fAtlasTranslate = {fAtlasOffsetX * fAtlasScale.x(),
                                   fAtlasOffsetY * fAtlasScale.y()};
                SkDEBUGCODE(fHasAtlasTransform = true);

                return sk_ref_sp(textureProxy->priv().peekTexture());
            },
            GrProxyProvider::Renderable::kYes, kTopLeft_GrSurfaceOrigin, kAlpha_half_GrPixelConfig);

    const SkRect& pathDevBounds = deviceSpacePath.getBounds();
    if (SkTMax(pathDevBounds.height(), pathDevBounds.width()) > kPathCropThreshold) {
        // The path is too large. We need to crop it or analytic AA can run out of fp32 precision.
        crop_path(deviceSpacePath, SkIRect::MakeWH(rtWidth, rtHeight), &fDeviceSpacePath);
    } else {
        fDeviceSpacePath = deviceSpacePath;
    }
    deviceSpacePath.getBounds().roundOut(&fPathDevIBounds);
    fAccessRect = accessRect;
}

void GrCoverageCountingPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                              const uint32_t* opListIDs, int numOpListIDs,
                                              SkTArray<sk_sp<GrRenderTargetContext>>* results) {
    using PathInstance = GrCCPathProcessor::Instance;

    SkASSERT(!fFlushing);
    SkASSERT(!fPerFlushIndexBuffer);
    SkASSERT(!fPerFlushVertexBuffer);
    SkASSERT(!fPerFlushInstanceBuffer);
    SkASSERT(!fPerFlushPathParser);
    SkASSERT(fPerFlushAtlases.empty());
    SkDEBUGCODE(fFlushing = true);

    if (fRTPendingPathsMap.empty()) {
        return;  // Nothing to draw.
    }

    fPerFlushResourcesAreValid = false;

    // Count the paths that are being flushed.
    int maxTotalPaths = 0, maxPathPoints = 0, numSkPoints = 0, numSkVerbs = 0;
    SkDEBUGCODE(int numClipPaths = 0);
    for (int i = 0; i < numOpListIDs; ++i) {
        auto it = fRTPendingPathsMap.find(opListIDs[i]);
        if (fRTPendingPathsMap.end() == it) {
            continue;
        }
        const RTPendingPaths& rtPendingPaths = it->second;

        SkTInternalLList<DrawPathsOp>::Iter drawOpsIter;
        drawOpsIter.init(rtPendingPaths.fDrawOps,
                         SkTInternalLList<DrawPathsOp>::Iter::kHead_IterStart);
        while (DrawPathsOp* op = drawOpsIter.get()) {
            for (const DrawPathsOp::SingleDraw* draw = op->head(); draw; draw = draw->fNext) {
                ++maxTotalPaths;
                maxPathPoints = SkTMax(draw->fPath.countPoints(), maxPathPoints);
                numSkPoints += draw->fPath.countPoints();
                numSkVerbs += draw->fPath.countVerbs();
            }
            drawOpsIter.next();
        }

        maxTotalPaths += rtPendingPaths.fClipPaths.size();
        SkDEBUGCODE(numClipPaths += rtPendingPaths.fClipPaths.size());
        for (const auto& clipsIter : rtPendingPaths.fClipPaths) {
            const SkPath& path = clipsIter.second.deviceSpacePath();
            maxPathPoints = SkTMax(path.countPoints(), maxPathPoints);
            numSkPoints += path.countPoints();
            numSkVerbs += path.countVerbs();
        }
    }

    if (!maxTotalPaths) {
        return;  // Nothing to draw.
    }

    // Allocate GPU buffers.
    fPerFlushIndexBuffer = GrCCPathProcessor::FindIndexBuffer(onFlushRP);
    if (!fPerFlushIndexBuffer) {
        SkDebugf("WARNING: failed to allocate ccpr path index buffer.\n");
        return;
    }

    fPerFlushVertexBuffer = GrCCPathProcessor::FindVertexBuffer(onFlushRP);
    if (!fPerFlushVertexBuffer) {
        SkDebugf("WARNING: failed to allocate ccpr path vertex buffer.\n");
        return;
    }

    fPerFlushInstanceBuffer =
            onFlushRP->makeBuffer(kVertex_GrBufferType, maxTotalPaths * sizeof(PathInstance));
    if (!fPerFlushInstanceBuffer) {
        SkDebugf("WARNING: failed to allocate path instance buffer. No paths will be drawn.\n");
        return;
    }

    PathInstance* pathInstanceData = static_cast<PathInstance*>(fPerFlushInstanceBuffer->map());
    SkASSERT(pathInstanceData);
    int pathInstanceIdx = 0;

    fPerFlushPathParser = sk_make_sp<GrCCPathParser>(maxTotalPaths, maxPathPoints, numSkPoints,
                                                     numSkVerbs);
    SkDEBUGCODE(int skippedTotalPaths = 0);

    // Allocate atlas(es) and fill out GPU instance buffers.
    for (int i = 0; i < numOpListIDs; ++i) {
        auto it = fRTPendingPathsMap.find(opListIDs[i]);
        if (fRTPendingPathsMap.end() == it) {
            continue;
        }
        RTPendingPaths& rtPendingPaths = it->second;

        SkTInternalLList<DrawPathsOp>::Iter drawOpsIter;
        drawOpsIter.init(rtPendingPaths.fDrawOps,
                         SkTInternalLList<DrawPathsOp>::Iter::kHead_IterStart);
        while (DrawPathsOp* op = drawOpsIter.get()) {
            pathInstanceIdx = op->setupResources(onFlushRP, pathInstanceData, pathInstanceIdx);
            drawOpsIter.next();
            SkDEBUGCODE(skippedTotalPaths += op->numSkippedInstances_debugOnly());
        }

        for (auto& clipsIter : rtPendingPaths.fClipPaths) {
            clipsIter.second.placePathInAtlas(this, onFlushRP, fPerFlushPathParser.get());
        }
    }

    fPerFlushInstanceBuffer->unmap();

    SkASSERT(pathInstanceIdx == maxTotalPaths - skippedTotalPaths - numClipPaths);

    if (!fPerFlushAtlases.empty()) {
        auto coverageCountBatchID = fPerFlushPathParser->closeCurrentBatch();
        fPerFlushAtlases.back().setCoverageCountBatchID(coverageCountBatchID);
    }

    if (!fPerFlushPathParser->finalize(onFlushRP)) {
        SkDebugf("WARNING: failed to allocate GPU buffers for CCPR. No paths will be drawn.\n");
        return;
    }

    // Draw the atlas(es).
    GrTAllocator<GrCCAtlas>::Iter atlasIter(&fPerFlushAtlases);
    while (atlasIter.next()) {
        if (auto rtc = atlasIter.get()->finalize(onFlushRP, fPerFlushPathParser)) {
            results->push_back(std::move(rtc));
        }
    }

    fPerFlushResourcesAreValid = true;
}

int CCPR::DrawPathsOp::setupResources(GrOnFlushResourceProvider* onFlushRP,
                                      GrCCPathProcessor::Instance* pathInstanceData,
                                      int pathInstanceIdx) {
    GrCCPathParser* parser = fCCPR->fPerFlushPathParser.get();
    const GrCCAtlas* currentAtlas = nullptr;
    SkASSERT(fInstanceCount > 0);
    SkASSERT(-1 == fBaseInstance);
    fBaseInstance = pathInstanceIdx;

    for (const SingleDraw* draw = this->head(); draw; draw = draw->fNext) {
        // parsePath gives us two tight bounding boxes: one in device space, as well as a second
        // one rotated an additional 45 degrees. The path vertex shader uses these two bounding
        // boxes to generate an octagon that circumscribes the path.
        SkRect devBounds, devBounds45;
        parser->parsePath(draw->fMatrix, draw->fPath, &devBounds, &devBounds45);

        SkIRect devIBounds;
        devBounds.roundOut(&devIBounds);

        int16_t offsetX, offsetY;
        GrCCAtlas* atlas = fCCPR->placeParsedPathInAtlas(onFlushRP, draw->fClipIBounds, devIBounds,
                                                         &offsetX, &offsetY);
        if (!atlas) {
            SkDEBUGCODE(++fNumSkippedInstances);
            continue;
        }
        if (currentAtlas != atlas) {
            if (currentAtlas) {
                this->addAtlasBatch(currentAtlas, pathInstanceIdx);
            }
            currentAtlas = atlas;
        }

        const SkMatrix& m = draw->fMatrix;
        pathInstanceData[pathInstanceIdx++] = {
                devBounds,
                devBounds45,
                {{m.getScaleX(), m.getSkewY(), m.getSkewX(), m.getScaleY()}},
                {{m.getTranslateX(), m.getTranslateY()}},
                {{offsetX, offsetY}},
                draw->fColor};
    }

    SkASSERT(pathInstanceIdx == fBaseInstance + fInstanceCount - fNumSkippedInstances);
    if (currentAtlas) {
        this->addAtlasBatch(currentAtlas, pathInstanceIdx);
    }

    return pathInstanceIdx;
}

void CCPR::ClipPath::placePathInAtlas(GrCoverageCountingPathRenderer* ccpr,
                                      GrOnFlushResourceProvider* onFlushRP,
                                      GrCCPathParser* parser) {
    SkASSERT(!this->isUninitialized());
    SkASSERT(!fHasAtlas);
    parser->parseDeviceSpacePath(fDeviceSpacePath);
    fAtlas = ccpr->placeParsedPathInAtlas(onFlushRP, fAccessRect, fPathDevIBounds, &fAtlasOffsetX,
                                          &fAtlasOffsetY);
    SkDEBUGCODE(fHasAtlas = true);
}

GrCCAtlas* GrCoverageCountingPathRenderer::placeParsedPathInAtlas(
        GrOnFlushResourceProvider* onFlushRP,
        const SkIRect& clipIBounds,
        const SkIRect& pathIBounds,
        int16_t* atlasOffsetX,
        int16_t* atlasOffsetY) {
    using ScissorMode = GrCCPathParser::ScissorMode;

    ScissorMode scissorMode;
    SkIRect clippedPathIBounds;
    if (clipIBounds.contains(pathIBounds)) {
        clippedPathIBounds = pathIBounds;
        scissorMode = ScissorMode::kNonScissored;
    } else if (clippedPathIBounds.intersect(clipIBounds, pathIBounds)) {
        scissorMode = ScissorMode::kScissored;
    } else {
        fPerFlushPathParser->discardParsedPath();
        return nullptr;
    }

    SkIPoint16 atlasLocation;
    int h = clippedPathIBounds.height(), w = clippedPathIBounds.width();
    if (fPerFlushAtlases.empty() || !fPerFlushAtlases.back().addRect(w, h, &atlasLocation)) {
        if (!fPerFlushAtlases.empty()) {
            // The atlas is out of room and can't grow any bigger.
            auto coverageCountBatchID = fPerFlushPathParser->closeCurrentBatch();
            fPerFlushAtlases.back().setCoverageCountBatchID(coverageCountBatchID);
        }
        fPerFlushAtlases.emplace_back(*onFlushRP->caps(), SkTMax(w, h));
        SkAssertResult(fPerFlushAtlases.back().addRect(w, h, &atlasLocation));
    }

    *atlasOffsetX = atlasLocation.x() - static_cast<int16_t>(clippedPathIBounds.left());
    *atlasOffsetY = atlasLocation.y() - static_cast<int16_t>(clippedPathIBounds.top());
    fPerFlushPathParser->saveParsedPath(scissorMode, clippedPathIBounds, *atlasOffsetX,
                                        *atlasOffsetY);

    return &fPerFlushAtlases.back();
}

void CCPR::DrawPathsOp::onExecute(GrOpFlushState* flushState) {
    SkASSERT(fCCPR->fFlushing);
    SkASSERT(flushState->rtCommandBuffer());

    if (!fCCPR->fPerFlushResourcesAreValid) {
        return;  // Setup failed.
    }

    SkASSERT(fBaseInstance >= 0);  // Make sure setupResources has been called.

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
            continue;  // Atlas failed to allocate.
        }

        GrCCPathProcessor pathProc(flushState->resourceProvider(),
                                   sk_ref_sp(batch.fAtlas->textureProxy()), this->getFillType());

        GrMesh mesh(GrCCPathProcessor::MeshPrimitiveType(flushState->caps()));
        mesh.setIndexedInstanced(fCCPR->fPerFlushIndexBuffer.get(),
                                 GrCCPathProcessor::NumIndicesPerInstance(flushState->caps()),
                                 fCCPR->fPerFlushInstanceBuffer.get(),
                                 batch.fEndInstanceIdx - baseInstance, baseInstance);
        mesh.setVertexData(fCCPR->fPerFlushVertexBuffer.get());

        flushState->rtCommandBuffer()->draw(pipeline, pathProc, &mesh, nullptr, 1, this->bounds());
    }

    SkASSERT(baseInstance == fBaseInstance + fInstanceCount - fNumSkippedInstances);
}

void GrCoverageCountingPathRenderer::postFlush(GrDeferredUploadToken, const uint32_t* opListIDs,
                                               int numOpListIDs) {
    SkASSERT(fFlushing);
    fPerFlushAtlases.reset();
    fPerFlushPathParser.reset();
    fPerFlushInstanceBuffer.reset();
    fPerFlushVertexBuffer.reset();
    fPerFlushIndexBuffer.reset();
    // We wait to erase these until after flush, once Ops and FPs are done accessing their data.
    for (int i = 0; i < numOpListIDs; ++i) {
        fRTPendingPathsMap.erase(opListIDs[i]);
    }
    SkDEBUGCODE(fFlushing = false);
}
