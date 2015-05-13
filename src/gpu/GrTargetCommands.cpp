/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTargetCommands.h"

#include "GrInOrderDrawBuffer.h"

void GrTargetCommands::reset() {
    fCmdBuffer.reset();
    fBatchTarget.reset();
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
    gpu->xferBarrier(fRenderTarget.get(), fBarrierType);
}
