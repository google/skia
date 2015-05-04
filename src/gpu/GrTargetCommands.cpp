/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTargetCommands.h"

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrInOrderDrawBuffer.h"
#include "GrTemplates.h"
#include "SkPoint.h"

static bool path_fill_type_is_winding(const GrStencilSettings& pathStencilSettings) {
    static const GrStencilSettings::Face pathFace = GrStencilSettings::kFront_Face;
    bool isWinding = kInvert_StencilOp != pathStencilSettings.passOp(pathFace);
    if (isWinding) {
        // Double check that it is in fact winding.
        SkASSERT(kIncClamp_StencilOp == pathStencilSettings.passOp(pathFace));
        SkASSERT(kIncClamp_StencilOp == pathStencilSettings.failOp(pathFace));
        SkASSERT(0x1 != pathStencilSettings.writeMask(pathFace));
        SkASSERT(!pathStencilSettings.isTwoSided());
    }
    return isWinding;
}

GrTargetCommands::Cmd* GrTargetCommands::recordDrawBatch(State* state, GrBatch* batch) {
    // Check if there is a Batch Draw we can batch with
    if (!fCmdBuffer.empty() && Cmd::kDrawBatch_CmdType == fCmdBuffer.back().type()) {
        DrawBatch* previous = static_cast<DrawBatch*>(&fCmdBuffer.back());
        if (previous->fState == state && previous->fBatch->combineIfPossible(batch)) {
            return NULL;
        }
    }

    return GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawBatch, (state, batch, &fBatchTarget));
}

GrTargetCommands::Cmd* GrTargetCommands::recordStencilPath(
                                                        const GrPipelineBuilder& pipelineBuilder,
                                                        const GrPathProcessor* pathProc,
                                                        const GrPath* path,
                                                        const GrScissorState& scissorState,
                                                        const GrStencilSettings& stencilSettings) {
    StencilPath* sp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, StencilPath,
                                               (path, pipelineBuilder.getRenderTarget()));

    sp->fScissor = scissorState;
    sp->fUseHWAA = pipelineBuilder.isHWAntialias();
    sp->fViewMatrix = pathProc->viewMatrix();
    sp->fStencil = stencilSettings;
    return sp;
}

GrTargetCommands::Cmd* GrTargetCommands::recordDrawPath(
                                                  State* state,
                                                  const GrPathProcessor* pathProc,
                                                  const GrPath* path,
                                                  const GrStencilSettings& stencilSettings) {
    DrawPath* dp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawPath, (state, path));
    dp->fStencilSettings = stencilSettings;
    return dp;
}

GrTargetCommands::Cmd* GrTargetCommands::recordDrawPaths(
                                                  State* state,
                                                  GrInOrderDrawBuffer* iodb,
                                                  const GrPathProcessor* pathProc,
                                                  const GrPathRange* pathRange,
                                                  const void* indexValues,
                                                  GrDrawTarget::PathIndexType indexType,
                                                  const float transformValues[],
                                                  GrDrawTarget::PathTransformType transformType,
                                                  int count,
                                                  const GrStencilSettings& stencilSettings,
                                                  const GrDrawTarget::PipelineInfo& pipelineInfo) {
    SkASSERT(pathRange);
    SkASSERT(indexValues);
    SkASSERT(transformValues);

    char* savedIndices;
    float* savedTransforms;
    
    iodb->appendIndicesAndTransforms(indexValues, indexType,
                                     transformValues, transformType,
                                     count, &savedIndices, &savedTransforms);

    if (!fCmdBuffer.empty() && Cmd::kDrawPaths_CmdType == fCmdBuffer.back().type()) {
        // The previous command was also DrawPaths. Try to collapse this call into the one
        // before. Note that stenciling all the paths at once, then covering, may not be
        // equivalent to two separate draw calls if there is overlap. Blending won't work,
        // and the combined calls may also cancel each other's winding numbers in some
        // places. For now the winding numbers are only an issue if the fill is even/odd,
        // because DrawPaths is currently only used for glyphs, and glyphs in the same
        // font tend to all wind in the same direction.
        DrawPaths* previous = static_cast<DrawPaths*>(&fCmdBuffer.back());
        if (pathRange == previous->pathRange() &&
            indexType == previous->fIndexType &&
            transformType == previous->fTransformType &&
            stencilSettings == previous->fStencilSettings &&
            path_fill_type_is_winding(stencilSettings) &&
            !pipelineInfo.willBlendWithDst(pathProc) &&
            previous->fState == state) {
                const int indexBytes = GrPathRange::PathIndexSizeInBytes(indexType);
                const int xformSize = GrPathRendering::PathTransformSize(transformType);
                if (&previous->fIndices[previous->fCount*indexBytes] == savedIndices &&
                    (0 == xformSize ||
                     &previous->fTransforms[previous->fCount*xformSize] == savedTransforms)) {
                    // Fold this DrawPaths call into the one previous.
                    previous->fCount += count;
                    return NULL;
                }
        }
    }

    DrawPaths* dp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawPaths, (state, pathRange));
    dp->fIndices = savedIndices;
    dp->fIndexType = indexType;
    dp->fTransforms = savedTransforms;
    dp->fTransformType = transformType;
    dp->fCount = count;
    dp->fStencilSettings = stencilSettings;
    return dp;
}

GrTargetCommands::Cmd* GrTargetCommands::recordClear(const SkIRect* rect,
                                                     GrColor color,
                                                     bool canIgnoreRect,
                                                     GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);

    SkIRect r;
    if (NULL == rect) {
        // We could do something smart and remove previous draws and clears to
        // the current render target. If we get that smart we have to make sure
        // those draws aren't read before this clear (render-to-texture).
        r.setLTRB(0, 0, renderTarget->width(), renderTarget->height());
        rect = &r;
    }
    Clear* clr = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Clear, (renderTarget));
    GrColorIsPMAssert(color);
    clr->fColor = color;
    clr->fRect = *rect;
    clr->fCanIgnoreRect = canIgnoreRect;
    return clr;
}

GrTargetCommands::Cmd* GrTargetCommands::recordClearStencilClip(const SkIRect& rect,
                                                                bool insideClip,
                                                                GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);

    ClearStencilClip* clr = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, ClearStencilClip, (renderTarget));
    clr->fRect = rect;
    clr->fInsideClip = insideClip;
    return clr;
}

GrTargetCommands::Cmd* GrTargetCommands::recordDiscard(GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);

    Clear* clr = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Clear, (renderTarget));
    clr->fColor = GrColor_ILLEGAL;
    return clr;
}

void GrTargetCommands::reset() {
    fCmdBuffer.reset();
}

void GrTargetCommands::flush(GrInOrderDrawBuffer* iodb) {
    if (fCmdBuffer.empty()) {
        return;
    }

    GrGpu* gpu = iodb->getGpu();

    // Loop over all batches and generate geometry
    CmdBuffer::Iter genIter(fCmdBuffer);
    while (genIter.next()) {
        if (Cmd::kDrawBatch_CmdType == genIter->type()) {
            DrawBatch* db = reinterpret_cast<DrawBatch*>(genIter.get());
            fBatchTarget.resetNumberOfDraws();
            db->fBatch->generateGeometry(&fBatchTarget, db->fState->getPipeline());
            db->fBatch->setNumberOfDraws(fBatchTarget.numberOfDraws());
        }
    }

    iodb->getVertexAllocPool()->unmap();
    iodb->getIndexAllocPool()->unmap();
    fBatchTarget.preFlush();

    CmdBuffer::Iter iter(fCmdBuffer);

    while (iter.next()) {
        GrGpuTraceMarker newMarker("", -1);
        SkString traceString;
        if (iter->isTraced()) {
            traceString = iodb->getCmdString(iter->markerID());
            newMarker.fMarker = traceString.c_str();
            gpu->addGpuTraceMarker(&newMarker);
        }

        iter->execute(gpu);
        if (iter->isTraced()) {
            gpu->removeGpuTraceMarker(&newMarker);
        }
    }

    fBatchTarget.postFlush();
}

void GrTargetCommands::StencilPath::execute(GrGpu* gpu) {
    GrGpu::StencilPathState state;
    state.fRenderTarget = fRenderTarget.get();
    state.fScissor = &fScissor;
    state.fStencil = &fStencil;
    state.fUseHWAA = fUseHWAA;
    state.fViewMatrix = &fViewMatrix;

    gpu->stencilPath(this->path(), state);
}

void GrTargetCommands::DrawPath::execute(GrGpu* gpu) {
    if (!fState->fCompiled) {
        gpu->buildProgramDesc(&fState->fDesc, *fState->fPrimitiveProcessor, *fState->getPipeline(),
                              fState->fBatchTracker);
        fState->fCompiled = true;
    }
    DrawArgs args(fState->fPrimitiveProcessor.get(), fState->getPipeline(),
                  &fState->fDesc, &fState->fBatchTracker);
    gpu->drawPath(args, this->path(), fStencilSettings);
}

void GrTargetCommands::DrawPaths::execute(GrGpu* gpu) {
    if (!fState->fCompiled) {
        gpu->buildProgramDesc(&fState->fDesc, *fState->fPrimitiveProcessor, *fState->getPipeline(),
                              fState->fBatchTracker);
        fState->fCompiled = true;
    }
    DrawArgs args(fState->fPrimitiveProcessor.get(), fState->getPipeline(),
                  &fState->fDesc, &fState->fBatchTracker);
    gpu->drawPaths(args, this->pathRange(),
                   fIndices, fIndexType,
                   fTransforms, fTransformType,
                   fCount, fStencilSettings);
}

void GrTargetCommands::DrawBatch::execute(GrGpu*) {
    fBatchTarget->flushNext(fBatch->numberOfDraws());
}

void GrTargetCommands::Clear::execute(GrGpu* gpu) {
    if (GrColor_ILLEGAL == fColor) {
        gpu->discard(this->renderTarget());
    } else {
        gpu->clear(&fRect, fColor, fCanIgnoreRect, this->renderTarget());
    }
}

void GrTargetCommands::ClearStencilClip::execute(GrGpu* gpu) {
    gpu->clearStencilClip(fRect, fInsideClip, this->renderTarget());
}

void GrTargetCommands::CopySurface::execute(GrGpu* gpu) {
    gpu->copySurface(this->dst(), this->src(), fSrcRect, fDstPoint);
}

void GrTargetCommands::XferBarrier::execute(GrGpu* gpu) {
    gpu->xferBarrier(fBarrierType);
}

GrTargetCommands::Cmd* GrTargetCommands::recordCopySurface(GrSurface* dst,
                                                           GrSurface* src,
                                                           const SkIRect& srcRect,
                                                           const SkIPoint& dstPoint) {
    CopySurface* cs = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, CopySurface, (dst, src));
    cs->fSrcRect = srcRect;
    cs->fDstPoint = dstPoint;
    return cs;
}

void GrTargetCommands::recordXferBarrierIfNecessary(const GrPipeline& pipeline,
                                                    GrInOrderDrawBuffer* iodb) {
    const GrXferProcessor& xp = *pipeline.getXferProcessor();
    GrRenderTarget* rt = pipeline.getRenderTarget();

    GrXferBarrierType barrierType;
    if (!xp.willNeedXferBarrier(rt, *iodb->caps(), &barrierType)) {
        return;
    }

    XferBarrier* xb = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, XferBarrier, ());
    xb->fBarrierType = barrierType;

    iodb->recordTraceMarkersIfNecessary(xb);
}

