/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCDrawPathsOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrOpFlushState.h"
#include "ccpr/GrCCPerFlushResources.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"

GrCCDrawPathsOp::GrCCDrawPathsOp(GrCoverageCountingPathRenderer* ccpr, GrPaint&& paint,
                                 const SkIRect& clipIBounds, const SkMatrix& viewMatrix,
                                 const SkPath& path, const SkRect& devBounds)
        : INHERITED(ClassID())
        , fCCPR(ccpr)
        , fHeadDraw{clipIBounds, viewMatrix, path, paint.getColor(), nullptr}
        , fSRGBFlags(GrPipeline::SRGBFlagsFromPaint(paint))
        , fProcessors(std::move(paint)) {
    SkDEBUGCODE(fCCPR->incrDrawOpCount_debugOnly());
    SkDEBUGCODE(fBaseInstance = -1);
    SkDEBUGCODE(fInstanceCount = 1);
    SkDEBUGCODE(fNumSkippedInstances = 0);
    // FIXME: intersect with clip bounds to (hopefully) improve batching.
    // (This is nontrivial due to assumptions in generating the octagon cover geometry.)
    this->setBounds(devBounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);
}

GrCCDrawPathsOp::~GrCCDrawPathsOp() {
    if (fOwningRTPendingPaths) {
        // Remove CCPR's dangling pointer to this Op before deleting it.
        fOwningRTPendingPaths->fDrawOps.remove(this);
    }
    SkDEBUGCODE(fCCPR->decrDrawOpCount_debugOnly());
}

GrDrawOp::RequiresDstTexture GrCCDrawPathsOp::finalize(const GrCaps& caps,
                                                       const GrAppliedClip* clip,
                                                       GrPixelConfigIsClamped dstIsClamped) {
    SkASSERT(!fCCPR->isFlushing_debugOnly());
    // There should only be one single path draw in this Op right now.
    SkASSERT(1 == fInstanceCount);
    SkASSERT(&fHeadDraw == fTailDraw);
    GrProcessorSet::Analysis analysis =
            fProcessors.finalize(fHeadDraw.fColor, GrProcessorAnalysisCoverage::kSingleChannel,
                                 clip, false, caps, dstIsClamped, &fHeadDraw.fColor);
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

bool GrCCDrawPathsOp::onCombineIfPossible(GrOp* op, const GrCaps& caps) {
    GrCCDrawPathsOp* that = op->cast<GrCCDrawPathsOp>();
    SkASSERT(fCCPR == that->fCCPR);
    SkASSERT(!fCCPR->isFlushing_debugOnly());
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

void GrCCDrawPathsOp::wasRecorded(GrRenderTargetOpList* opList) {
    SkASSERT(!fOwningRTPendingPaths);
    fOwningRTPendingPaths = fCCPR->lookupRTPendingPaths(opList);
    fOwningRTPendingPaths->fDrawOps.addToTail(this);
}

void GrCCDrawPathsOp::setupResources(GrCCPerFlushResources* resources,
                                     GrOnFlushResourceProvider* onFlushRP) {
    const GrCCAtlas* currentAtlas = nullptr;
    SkASSERT(fInstanceCount > 0);
    SkASSERT(-1 == fBaseInstance);
    fBaseInstance = resources->pathInstanceCount();

    for (const SingleDraw* draw = this->head(); draw; draw = draw->fNext) {
        // addPathToAtlas gives us two tight bounding boxes: one in device space, as well as a
        // second one rotated an additional 45 degrees. The path vertex shader uses these two
        // bounding boxes to generate an octagon that circumscribes the path.
        SkRect devBounds, devBounds45;
        int16_t atlasOffsetX, atlasOffsetY;
        GrCCAtlas* atlas = resources->addPathToAtlas(*onFlushRP->caps(), draw->fClipIBounds,
                                                     draw->fMatrix, draw->fPath, &devBounds,
                                                     &devBounds45, &atlasOffsetX, &atlasOffsetY);
        if (!atlas) {
            SkDEBUGCODE(++fNumSkippedInstances);
            continue;
        }
        if (currentAtlas != atlas) {
            if (currentAtlas) {
                this->addAtlasBatch(currentAtlas, resources->pathInstanceCount());
            }
            currentAtlas = atlas;
        }

        const SkMatrix& m = draw->fMatrix;
        resources->appendDrawPathInstance(
                devBounds,
                devBounds45,
                {{m.getScaleX(), m.getSkewY(), m.getSkewX(), m.getScaleY()}},
                {{m.getTranslateX(), m.getTranslateY()}},
                {{atlasOffsetX, atlasOffsetY}},
                draw->fColor);
    }

    SkASSERT(resources->pathInstanceCount() ==
                     fBaseInstance + fInstanceCount - fNumSkippedInstances);
    if (currentAtlas) {
        this->addAtlasBatch(currentAtlas, resources->pathInstanceCount());
    }
}

void GrCCDrawPathsOp::onExecute(GrOpFlushState* flushState) {
    const GrCCPerFlushResources* resources = fCCPR->getPerFlushResources();
    if (!resources) {
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
        pathProc.drawPaths(flushState, pipeline, resources->indexBuffer(),
                           resources->vertexBuffer(), resources->instanceBuffer(),
                           baseInstance, batch.fEndInstanceIdx, this->bounds());
    }

    SkASSERT(baseInstance == fBaseInstance + fInstanceCount - fNumSkippedInstances);
}
